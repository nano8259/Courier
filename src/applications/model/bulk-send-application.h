/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Georgia Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: George F. Riley <riley@ece.gatech.edu>
 */

#ifndef BULK_SEND_APPLICATION_H
#define BULK_SEND_APPLICATION_H
#include <iostream>
#include <map>
#include <vector>
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include "ns3/global-property.h"
#include "ns3/token-bucket.h"

namespace ns3 {

class Address;
class Socket;

/**
 * \ingroup applications
 * \defgroup bulksend BulkSendApplication
 *
 * This traffic generator simply sends data
 * as fast as possible up to MaxBytes or until
 * the appplication is stopped if MaxBytes is
 * zero. Once the lower layer send buffer is
 * filled, it waits until space is free to
 * send more data, essentially keeping a
 * constant flow of data. Only SOCK_STREAM
 * and SOCK_SEQPACKET sockets are supported.
 * For example, TCP sockets can be used, but
 * UDP sockets can not be used.
 */
class BulkSendApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  BulkSendApplication ();

  virtual ~BulkSendApplication ();

  /**
   * \param maxBytes the upper bound of bytes to send
   *
   * Set the upper bound for the total number of bytes to send. Once
   * this bound is reached, no more application bytes are sent. If the
   * application is stopped during the simulation and restarted, the
   * total number of bytes sent is not reset; however, the maxBytes
   * bound is still effective and the application will continue sending
   * up to maxBytes. The value zero for maxBytes means that
   * there is no upper bound; i.e. data is sent until the application
   * or simulation is stopped.
   */
  void SetMaxBytes (uint64_t maxBytes);

  /**
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;

  bool IsFinished() const; // by tbc
  void OutputStates(uint32_t job, uint32_t mapper, uint32_t reducer) const;
  uint64_t GetSentBytes() const;
  inline uint64_t GetFlowSize() const { return m_maxBytes; }

  inline void InitiallizeSpeedEstimator() {
	  m_estimateInterval = GlobalProperty::m_simulationStep.GetSeconds()*GlobalProperty::m_speedEstimatorPeriod;
	  m_alpha = GlobalProperty::m_speedEstimatorEma;
  }
  inline void EstimateCurrentSpeed() {
	  uint32_t current = GetSentBytes();
	  uint32_t delta = current - m_lastSentBytes + 1;
	  m_lastSentBytes = current;
	  double speed = delta / m_estimateInterval; // in Bps
    // Z modify current speed
	  m_speed = (1-m_alpha)*m_speed + m_alpha*speed;

    // and then, estimate all mf's speed
    std::vector<uint32_t>::iterator it;
    for(it=m_activeMf.begin(); it!=m_activeMf.end(); it++){
      EstimateMfCurrentSpeed(*it);
    }
  }
  inline double GetCurrentSpeed() { return m_speed; }

  inline void SetMapperHostNumber(uint32_t mapperHostNumber){ m_mapperHostNumber = mapperHostNumber; }
  inline uint32_t GetMapperHostNumber() const { return m_mapperHostNumber; }
  void StopWithoutCallback (void);

  inline void SetParent(void* mf){ m_parent = mf; }
  inline void* GetParent() const{return m_parent;}
  inline void Continue(){ 
    // if(!m_isFinished)
    StartApplication(); }
  inline bool HasStarted() const {return started;}
  inline void SetTokenBucket(Ptr<TokenBucket> tb) {this->tb = tb; }
  inline void AssumeFinished(uint64_t bytes){ // create a fake application that never runs
	  this->InitiallizeSpeedEstimator();
	  m_totBytes = bytes; m_maxBytes = bytes; m_isFinished = true;started = true;
  }

  // added by ZZC
  inline void AddMf(uint32_t mfid, uint64_t mfsize){
    // for debug
    m_maxMfBytes.insert(std::make_pair(mfid, mfsize));
    m_totMfBytes.insert(std::make_pair(mfid, 0));
    m_isMfFinished.insert(std::make_pair(mfid, 0));
    m_activeMf.push_back(mfid);
    m_maxBytes += mfsize;
  }
  inline uint64_t GetMfFlowSize(uint32_t mfid) { return m_maxMfBytes[mfid]; }
  // inline uint64_t GetMfSentBytes(uint32_t mfid) { return m_totMfBytes[mfid]; }
  inline void EstimateMfCurrentSpeed(uint32_t mfid) {
	  uint32_t current = m_totMfBytes[mfid];
	  uint32_t delta = current - m_lastMfSentBytes[mfid] + 1;
	  m_lastMfSentBytes[mfid] = current;
	  double speed = delta / m_estimateInterval; // in Bps
    // Z modify current speed
	  m_mfSpeed[mfid] = (1-m_alpha)*m_mfSpeed[mfid] + m_alpha*speed;
  }
  inline double GetMfCurrentSpeed(uint32_t mfid) { return m_mfSpeed[mfid]; }
  inline double GetMfSentBytes(uint32_t mfid) { return m_totMfBytes[mfid]; }
  // BUG!! 为啥会重复添加prioritized？
  inline void AddPrioritizedMf(uint32_t mfid) { if(!IsMfPrioritized(mfid) && !IsMfFinished(mfid)) m_prioritizedMf.push_back(mfid); }
  inline void RemovePrioritizedMf(uint32_t mfid) {
    std::vector<uint32_t>::iterator it;
    for(it=m_prioritizedMf.begin(); it!=m_prioritizedMf.end(); it++){
      if(*it == mfid){
        m_prioritizedMf.erase(it);
        break;
      }
    }
  }
  inline bool IsMfFinished(uint32_t mfid) {
    // std::vector<uint32_t>::iterator it;
    // // bool isFinished = true;
    // for(it=m_activeMf.begin(); it!=m_activeMf.end(); it++){
    //   if(*it == mfid){
    //     return false;
    //   }
    // }
    // return true;
    return m_isMfFinished[mfid] == 1 ? true : false;
  }
  inline bool IsMfPrioritized(uint32_t mfid) {
    std::vector<uint32_t>::iterator it;
    // bool isFinished = true;
    for(it=m_prioritizedMf.begin(); it!=m_prioritizedMf.end(); it++){
      if(*it == mfid){
        return true;
      }
    }
    return false;
  }
  inline bool IsConnected(){
    return m_connected;
  }

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  void SendData ();

  Ptr<Socket>     m_socket;       // Associated socket
  Address         m_peer;         // Peer address
  bool            m_connected;    // True if connected
  uint64_t        m_sendSize;     // Size of data to send each time
  uint64_t        m_maxBytes;     // Limit total number of bytes sent
  uint64_t        m_totBytes;     // Total bytes sent so far
  uint8_t         m_tos;          // TOS value for socket to use
  TypeId          m_tid;

  uint32_t m_isFinished; // by tbc
  uint32_t m_noRetries;
  uint32_t m_lastSentBytes; // for speed estimation
  double m_estimateInterval; // in seconds
  double m_alpha; // EMA
  double m_speed; // estimated flow speed

  Ptr<TokenBucket> tb;

  uint32_t m_mapperHostNumber;
  void* m_parent; // macroflow pointer
  bool started;

  TracedCallback<Ptr<const Packet> > m_txTrace;
  TracedCallback<Ptr<Socket> > m_txTraceSource;

  uint64_t m_lastClock;//added by lkx

  // added by zzc <MacroflowId, Bytes>
  std::map<uint32_t, uint64_t> m_maxMfBytes;
  std::map<uint32_t, uint64_t> m_totMfBytes;
  std::map<uint32_t, uint32_t> m_isMfFinished;
  std::map<uint32_t, uint32_t> m_lastMfSentBytes;
  std::map<uint32_t, uint32_t> m_mfSpeed;
  std::vector<uint32_t> m_prioritizedMf;
  std::vector<uint32_t> m_activeMf;


private:
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback
  void Ignore (Ptr<Socket> socket);

  void CheckAndRetry (Ptr<Socket> socket); // by tbc


};

} // namespace ns3

#endif /* BULK_SEND_APPLICATION_H */
