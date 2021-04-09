/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "queue.h"

NS_LOG_COMPONENT_DEFINE ("Queue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Queue);
NS_OBJECT_ENSURE_REGISTERED (QueueTimestampTag);

uint64_t Queue::max_real_length = 0;
uint64_t Queue::dropped_packets = 0;


TypeId
Queue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Queue")
    .SetParent<Object> ()
    .AddTraceSource ("Enqueue", "Enqueue a packet in the queue.",
                     MakeTraceSourceAccessor (&Queue::m_traceEnqueue))
    .AddTraceSource ("Dequeue", "Dequeue a packet from the queue.",
                     MakeTraceSourceAccessor (&Queue::m_traceDequeue))
    .AddTraceSource ("Drop", "Drop a packet stored in the queue.",
                     MakeTraceSourceAccessor (&Queue::m_traceDrop))
  ;
  return tid;
}

Queue::Queue() :
  m_nBytes (0),
  m_nTotalReceivedBytes (0),
  m_nPackets (0),
  m_nTotalReceivedPackets (0),
  m_nTotalSentPackets (0),
  m_nTotalDroppedBytes (0),
  m_nTotalDroppedPackets (0),
  m_packetJurneySum (0)
{
  NS_LOG_FUNCTION (this);
}

Queue::~Queue()
{
  NS_LOG_FUNCTION (this);
}


bool
Queue::Enqueue (Ptr<Packet> p, bool tagit)
{
  NS_LOG_FUNCTION (this << p);

  //
  // If DoEnqueue fails, Queue::Drop is called by the subclass
  //
  if (tagit)
    {
      QueueTimestampTag tag;
      p->AddPacketTag (tag);
    }
  bool retval = DoEnqueue (p);
  if (retval)
    {
      NS_LOG_LOGIC ("m_traceEnqueue (p)");
      m_traceEnqueue (p);

      uint32_t size = p->GetSize ();
      m_nBytes += size;
      m_nTotalReceivedBytes += size;

      m_nPackets++;
      m_nTotalReceivedPackets++;

      //added by lkx
      max_real_length = std::max(max_real_length,m_nBytes);
    }
  return retval;
}

Ptr<Packet>
Queue::Dequeue (bool untagit)
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet = DoDequeue ();

  if (packet != 0)
    {
      NS_ASSERT (m_nBytes >= packet->GetSize ());
      NS_ASSERT (m_nPackets > 0);

      if (untagit)
        {
          QueueTimestampTag tag;
          packet->PeekPacketTag (tag);
          m_packetJurneySum += Simulator::Now () - tag.GetTxTime ();
          packet->RemovePacketTag(tag);
        }
      m_nTotalSentPackets++;


      m_nBytes -= packet->GetSize ();
      m_nPackets--;

      NS_LOG_LOGIC ("m_traceDequeue (packet)");
      m_traceDequeue (packet);
    }
  return packet;
}

void
Queue::DequeueAll (void)
{
  NS_LOG_FUNCTION (this);
  while (!IsEmpty ())
    {
      Dequeue ();
    }
}

Ptr<const Packet>
Queue::Peek (void) const
{
  NS_LOG_FUNCTION (this);
  return DoPeek ();
}


uint64_t
Queue::GetNPackets (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nPackets);
  return m_nPackets;
}

uint64_t
Queue::GetNBytes (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (" returns " << m_nBytes);
  return m_nBytes;
}

Time
Queue::GetPacketJurneyAvg (void) const
{
  Time jurneyAvg = m_nTotalSentPackets ? Time::FromDouble(m_packetJurneySum.GetSeconds() / m_nTotalSentPackets, Time::S) : Time(0);
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC (" returns " << jurneyAvg);
  return jurneyAvg;
}

bool
Queue::IsEmpty (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << (m_nPackets == 0));
  return m_nPackets == 0;
}

uint64_t
Queue::GetTotalReceivedBytes (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalReceivedBytes);
  return m_nTotalReceivedBytes;
}

uint64_t
Queue::GetTotalReceivedPackets (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalReceivedPackets);
  return m_nTotalReceivedPackets;
}

uint64_t
Queue:: GetTotalDroppedBytes (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalDroppedBytes);
  return m_nTotalDroppedBytes;
}

uint64_t
Queue::GetTotalDroppedPackets (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalDroppedPackets);
  return m_nTotalDroppedPackets;
}

void
Queue::ResetStatistics (void)
{
  NS_LOG_FUNCTION (this);
  m_nTotalReceivedBytes = 0;
  m_nTotalReceivedPackets = 0;
  m_nTotalSentPackets = 0;
  m_nTotalDroppedBytes = 0;
  m_nTotalDroppedPackets = 0;
}

void
Queue::Drop (Ptr<Packet> p, bool wasEnqueued)
{
  NS_LOG_FUNCTION (this << p);

  //added by lkx
  dropped_packets++;

  m_nTotalDroppedPackets++;
  m_nTotalDroppedBytes += p->GetSize ();

  if (wasEnqueued)
    {
      m_nBytes -= p->GetSize ();
      m_nPackets--;
    }

  NS_LOG_LOGIC ("m_traceDrop (p)");
  m_traceDrop (p);
}

QueueTimestampTag::QueueTimestampTag ()
  : m_creationTime (Simulator::Now ())
{
}

TypeId
QueueTimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QueueTimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<QueueTimestampTag> ()
  ;
  return tid;
}

TypeId
QueueTimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
QueueTimestampTag::GetSerializedSize (void) const
{
  return sizeof(double);
}
void
QueueTimestampTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_creationTime.ToDouble(Time::NS));
}
void
QueueTimestampTag::Deserialize (TagBuffer i)
{
  m_creationTime = Time::FromDouble(i.ReadDouble (), Time::NS);
}
void
QueueTimestampTag::Print (std::ostream &os) const
{
  os << "CreationTime = " << m_creationTime << " ";
}
Time
QueueTimestampTag::GetTxTime (void) const
{
  return m_creationTime;
}

} // namespace ns3
