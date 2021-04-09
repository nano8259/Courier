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

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "drop-oldest-queue.h"

NS_LOG_COMPONENT_DEFINE ("DropOldestQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DropOldestQueue);

TypeId DropOldestQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DropOldestQueue")
    .SetParent<Queue> ()
    .AddConstructor<DropOldestQueue> ()
    .AddAttribute ("Mode",
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_BYTES),
                   MakeEnumAccessor (&DropOldestQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this DropOldestQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&DropOldestQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this DropOldestQueue.",
                   UintegerValue (100 * 1000),
                   MakeUintegerAccessor (&DropOldestQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NoQueues",
                   "Number of priority queues.",
                   UintegerValue (8),
                   MakeUintegerAccessor (&DropOldestQueue::m_noQueues),
                   MakeUintegerChecker<uint32_t> (1, 8))
    .AddAttribute ("DRR",
                   "Deficit Round Robin",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DropOldestQueue::m_DRR),
                   MakeBooleanChecker ())
    .AddAttribute ("WQ1",
                   "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&DropOldestQueue::m_WQ1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WQ2",
                   "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&DropOldestQueue::m_WQ2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WQ3",
                   "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&DropOldestQueue::m_WQ3),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WQ4",
                   "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&DropOldestQueue::m_WQ4),
                   MakeDoubleChecker<double> ())
     .AddAttribute ("WQ5",
                    "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                    DoubleValue (5000),
                    MakeDoubleAccessor (&DropOldestQueue::m_WQ5),
                    MakeDoubleChecker<double> ())
     .AddAttribute ("WQ6",
                    "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                    DoubleValue (5000),
                    MakeDoubleAccessor (&DropOldestQueue::m_WQ6),
                    MakeDoubleChecker<double> ())
      .AddAttribute ("WQ7",
                     "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                     DoubleValue (5000),
                     MakeDoubleAccessor (&DropOldestQueue::m_WQ7),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("WQ8",
                     "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on DropOldestQueue mode",
                     DoubleValue (5000),
                     MakeDoubleAccessor (&DropOldestQueue::m_WQ8),
                     MakeDoubleChecker<double> ())
  ;

  return tid;
}

DropOldestQueue::DropOldestQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

DropOldestQueue::~DropOldestQueue ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
DropOldestQueue::SetMode (DropOldestQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (mode);
  m_mode = mode;
}

DropOldestQueue::QueueMode
DropOldestQueue::GetMode (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mode;
}

bool
DropOldestQueue::DropOldest(Ptr<Packet> p)
{
  int sizeToFree = p->GetSize();

  while (sizeToFree > 0 && !m_packets.empty())
    {
      MasterQueue_t::iterator i = m_packets.begin();
      uint8_t tos = i->second;
      Ptr<Packet> drop = i->first;

      sizeToFree -= (int)drop->GetSize();
      m_bytesInQueue -= drop->GetSize ();

      NS_ASSERT(m_packetQueues[tos % m_packetQueues.size()].begin()->second == m_packets.begin());

      m_packets.pop_front();
      m_packetQueues[tos % m_packetQueues.size()].pop_front();
      Drop(drop, true);
    }
  return (sizeToFree <= 0);
}

bool
DropOldestQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  // Initialize
  if (m_noQueues > 0)
    {
      m_queueWeights.resize(m_noQueues);
      m_queueCurrentWeights.resize(m_noQueues);
      m_packetQueues.resize(m_noQueues);

      if (0 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[0] = m_WQ1;
            m_queueCurrentWeights[0] = 0;
        }

        if (1 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[1] = m_WQ2;
            m_queueCurrentWeights[1] = 0;
        }

        if (2 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[2] = m_WQ3;
            m_queueCurrentWeights[2] = 0;
        }

        if (3 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[3] = m_WQ4;
            m_queueCurrentWeights[3] = 0;
        }

        if (4 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[4] = m_WQ5;
            m_queueCurrentWeights[4] = 0;
        }

        if (5 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[5] = m_WQ6;
            m_queueCurrentWeights[5] = 0;
        }

        if (6 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[6] = m_WQ7;
            m_queueCurrentWeights[6] = 0;
        }

        if (7 < std::min(m_noQueues, (uint32_t)8)) {
            m_queueWeights[7] = m_WQ8;
            m_queueCurrentWeights[7] = 0;
        }

        /* Initialize for the last queue! This way we're not loosing any iteartions */
        m_queueCurrentWeights[m_noQueues - 1] = m_queueWeights[m_noQueues - 1];

        m_queueIter = m_packetQueues.rbegin();
        m_weightIterCurrent = m_queueCurrentWeights.rbegin();
        m_weightIter = m_queueWeights.rbegin();
    }
  m_noQueues = 0;

  if ((m_mode == QUEUE_MODE_PACKETS && (m_packets.size () >= m_maxPackets)) ||
        (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes)))
    {
      if (!DropOldest(p))
        return false;
    }

  Ptr<Header> header;
  uint8_t tos = 0;
  p->GetIpHeader (header);

  if (!!header)
    {
      tos = header->GetPrecedence();
    }

  m_bytesInQueue += p->GetSize ();
  MasterQueue_t::iterator i = m_packets.end();
  i = m_packets.insert(m_packets.end(), std::make_pair(p, tos));
  m_packetQueues[tos % m_packetQueues.size()].push_back (std::make_pair(p, i));

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}

Ptr<Packet>
DropOldestQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT(m_packetQueues.size() == m_queueCurrentWeights.size() && m_packetQueues.size() == m_queueWeights.size());
  std::vector<bool> is_empty(m_packetQueues.size(), false);
  std::vector<bool> all_empty(m_packetQueues.size(), true);
  if (!m_DRR)
    {
      m_queueIter = m_packetQueues.rbegin();
      m_weightIterCurrent = m_queueCurrentWeights.rbegin();
      m_weightIter = m_queueWeights.rbegin();
    }
  /* If we use DRR spin until we dequeue packet or if all queues are empty! */
  for (uint32_t i = 0; i < m_packetQueues.size() || m_DRR; m_queueIter = (++m_queueIter == m_packetQueues.rend()) ? m_packetQueues.rbegin() : m_queueIter,
                                          m_weightIterCurrent = (++m_weightIterCurrent == m_queueCurrentWeights.rend()) ? m_queueCurrentWeights.rbegin() : m_weightIterCurrent,
                                          m_weightIter = (++m_weightIter == m_queueWeights.rend()) ? m_queueWeights.rbegin() : m_weightIter,
                                          ++i)
    {
      PacketMasterIterPair pair;
      if (m_DRR && i != 0) {
          *m_weightIterCurrent += *m_weightIter;
      }

      if (m_queueIter->size() == 0) {
          if (m_DRR)
            {
              is_empty[i % m_packetQueues.size()] = true;
              *m_weightIterCurrent = 0;
              if (is_empty == all_empty)
                break;
            }
          continue;
      } else if (m_DRR) {
          if (m_mode == QUEUE_MODE_BYTES)
            {
              pair = m_queueIter->front ();
              Ptr<Packet> p = pair.first;
              if (p->GetSize() > *m_weightIterCurrent)
                {
                  continue;
                }
              else
                {
                  *m_weightIterCurrent -= p->GetSize();
                }
            }
          else
            {
              if (*m_weightIterCurrent > 1)
                {
                  *m_weightIterCurrent -= 1;
                }
              else
                {
                  continue;
                }
            }
      }

      pair = m_queueIter->front ();
      Ptr<Packet> p = pair.first;

      m_packets.erase(pair.second);
      m_queueIter->pop_front ();
      m_bytesInQueue -= p->GetSize ();

      NS_LOG_LOGIC ("Popped " << p);

      NS_LOG_LOGIC ("Number packets " << m_packets.size());
      NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

      return p;
    }
  if (m_DRR)
    {
      *m_weightIterCurrent += *m_weightIter;
    }

  return 0;
}

Ptr<const Packet>
DropOldestQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);
  if (!m_packets.size())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  /* This is only approximation of DRR algorithm! */
  int i = 0;
  QueuesContainer::const_reverse_iterator iter = m_DRR ? static_cast<QueuesContainer::const_reverse_iterator>(m_queueIter) : m_packetQueues.rbegin();
  for (; i < (int)m_packetQueues.size(); iter = (++iter == m_packetQueues.rend()) ? m_packetQueues.rbegin() : iter, ++i)
    {
      if (iter->size() != 0)
        {
          Ptr<Packet> p = iter->front ().first;
          NS_LOG_LOGIC ("Number packets " << m_packets.size());
          NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

          return p;
        }
    }

  /* UNREACHABLE */
  return 0;
}

} // namespace ns3
