/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Igor Maravic <igorm@etf.rs>
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
 */

#ifndef DROP_OLDEST_QUEUE_H_
#define DROP_OLDEST_QUEUE_H_

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue.h"

namespace ns3 {

class TraceContainer;

/**
 * \ingroup queue
 *
 * \brief A packet queue that drops oldest packets on overflow
 */
class DropOldestQueue : public Queue {
public:
  static TypeId GetTypeId (void);
  /**
   * \brief DropOldestQueue Constructor
   *
   * Creates a OPD queue with a maximum size of 100 packets by default
   */
  DropOldestQueue ();

  virtual ~DropOldestQueue();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (DropOldestQueue::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  DropOldestQueue::QueueMode GetMode (void);

private:
  bool DropOldest (Ptr<Packet> p);
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  typedef std::pair<Ptr<Packet>, uint8_t> PacketTosPair;
  typedef std::list<PacketTosPair> MasterQueue_t;
  MasterQueue_t m_packets;

  typedef std::pair<Ptr<Packet>, MasterQueue_t::iterator> PacketMasterIterPair; /* Packet and it's iterator in oldest packet list */
  typedef std::vector<std::list<PacketMasterIterPair> > QueuesContainer;

  QueuesContainer m_packetQueues;
  QueuesContainer::reverse_iterator m_queueIter;

  std::vector<double> m_queueWeights;
  std::vector<double>::reverse_iterator m_weightIter;

  std::vector<double> m_queueCurrentWeights;
  std::vector<double>::reverse_iterator m_weightIterCurrent;
  double m_WQ1;
  double m_WQ2;
  double m_WQ3;
  double m_WQ4;
  double m_WQ5;
  double m_WQ6;
  double m_WQ7;
  double m_WQ8;

  uint32_t m_noQueues;
  uint32_t m_maxPackets;
  uint32_t m_maxBytes;
  uint32_t m_bytesInQueue;
  bool m_DRR;
  QueueMode m_mode;
};

} // namespace ns3

#endif /* DROP_OLDEST_QUEUE_H_ */
