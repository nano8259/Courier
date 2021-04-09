/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Andrew McGregor
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
 * Codel, the COntrolled DELay Queueing discipline
 * Based on ns2 simulation code presented by Kathie Nichols
 *
 * This port based on linux kernel code by
 * Authors:	Dave Täht <d@taht.net>
 *		Eric Dumazet <edumazet@google.com>
 *
 * Ported to ns-3 by: Andrew McGregor <andrewmcgr@gmail.com>
 */

#ifndef CODEL_H
#define CODEL_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {

typedef uint16_t rec_inv_sqrt_t;

#define CODEL_SHIFT 10
#define REC_INV_SQRT_BITS (8 * sizeof(rec_inv_sqrt_t))
#define REC_INV_SQRT_SHIFT (32 - REC_INV_SQRT_BITS)

class TraceContainer;

class CoDelTimestampTag : public Tag
{
public:
  CoDelTimestampTag ();
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  Time GetTxTime (void) const;
private:
  Time m_creationTime;
};

/**
 * \ingroup queue
 *
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class CoDelQueue : public Queue {
  class CoDelTime
  {
  public:
    CoDelTime ();
    CoDelTime (const Time &time);
    CoDelTime (int32_t time);

    bool operator >(const CoDelTime& rhs) const;
    bool operator >=(const CoDelTime& rhs) const;
    bool operator <(const CoDelTime& rhs) const;
    bool operator <=(const CoDelTime& rhs) const;
    bool operator ==(const CoDelTime& rhs) const;
    bool operator !=(const CoDelTime& rhs) const;

    CoDelTime operator +(const CoDelTime& rhs) const;
    CoDelTime& operator +=(const CoDelTime& rhs);
    CoDelTime operator *(double rhs) const;
    CoDelTime& operator *=(double rhs);
    CoDelTime operator -(const CoDelTime& rhs) const;
    CoDelTime& operator -=(const CoDelTime& rhs);

    int32_t Get() const { return m_time; }

    static int32_t Time2CodelTime(const Time& time);
  private:
    int32_t m_time;
  };
public:
  friend class Fq_CoDelQueue;

  static TypeId GetTypeId (void);
  /**
   * \brief CoDelQueue Constructor
   *
   * Creates a codel queue with a maximum size of 100 packets by default
   */
  CoDelQueue ();

  virtual ~CoDelQueue();

  /**
   * Enumeration of the modes supported in the class.
   *
   */
  enum Mode {
    ILLEGAL,     /**< Mode not set */
    PACKETS,     /**< Use number of packets for maximum queue size */
    BYTES,       /**< Use number of bytes for maximum queue size */
  };

  enum DropMode {
    NO_ECN,
    ECN_THEN_DROP, /* Mark ECN bits, if it isn't ECN capable DROP */
    DROP_AND_ECN, /* Two targets - lower for ECN and higher for DROPs */
    DROP_THEN_ECN, /* Drop packets and in interval between dropping mark ECN bits */
  };

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (CoDelQueue::Mode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  CoDelQueue::Mode  GetMode (void);

  uint32_t GetQueueSize (void);

  void SetInterval(Time time);
  void SetECNInterval(Time time);

private:
  bool DropOldest (Ptr<Packet> p);
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  bool CoDelDoDequeue (Ptr<Packet>& p, const CoDelTime& now);
  bool CoDelMarkPacket (Ptr<Packet>& p);
  virtual Ptr<const Packet> DoPeek (void) const;
  void NewtonStep(void);
  CoDelTime ControlLaw(CoDelTime t);
  bool ShouldDrop(Ptr<Packet> p, const CoDelTime& now);

  std::queue<Ptr<Packet> > m_packets;
  uint64_t m_maxPackets;
  uint64_t m_maxBytes;
  uint64_t m_bytesInQueue;
  uint64_t *backlog;
  uint32_t m_minbytes;
  CoDelTime m_Interval;
  CoDelTime m_ECNInterval;
  Time m_Target;
  Time m_ECNTarget;
  double m_TargetRatio; /* m_Target / m_ECNTarget = m_TargetRatio */
  bool m_OPD;
  TracedValue<uint32_t> m_count;
  uint32_t m_lastCount;
  TracedValue<uint32_t> m_dropCount;
  TracedValue<uint32_t> m_ECNCount;
  bool m_dropping;
  uint16_t m_recInvSqrt;
  CoDelTime m_firstAboveTime;
  CoDelTime m_firstAboveECNTime;
  CoDelTime m_dropNext;
  uint64_t m_dropOverlimit;
  Mode     m_mode;
  DropMode m_dropMode;
};

} // namespace ns3

#endif /* CODEL_H */
