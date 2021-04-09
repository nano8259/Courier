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

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/abort.h"
#include "codel-queue.h"

NS_LOG_COMPONENT_DEFINE ("CoDelQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CoDelQueue);
NS_OBJECT_ENSURE_REGISTERED (CoDelTimestampTag);

static inline uint32_t reciprocalDivide(uint32_t A, uint32_t R)
{
	return (uint32_t)(((uint64_t)A * R) >> 32);
}


int32_t
CoDelQueue::CoDelTime::Time2CodelTime(const Time& time)
{
  return time.GetNanoSeconds() >> CODEL_SHIFT;
}

CoDelQueue::CoDelTime::CoDelTime()
  :m_time(Time2CodelTime(Simulator::Now()))
{
}

CoDelQueue::CoDelTime::CoDelTime(const Time &time)
  :m_time(Time2CodelTime(time))
{
}

CoDelQueue::CoDelTime::CoDelTime(int32_t time)
  :m_time(time)
{
}

bool CoDelQueue::CoDelTime::operator >(const CoDelQueue::CoDelTime& rhs) const
{
  return m_time > rhs.m_time;
}

bool CoDelQueue::CoDelTime::operator >=(const CoDelQueue::CoDelTime& rhs) const
{
  return m_time >= rhs.m_time;
}

bool CoDelQueue::CoDelTime::operator <(const CoDelQueue::CoDelTime& rhs) const
{
  return m_time < rhs.m_time;
}

bool CoDelQueue::CoDelTime::operator <=(const CoDelQueue::CoDelTime& rhs) const
{
  return m_time <= rhs.m_time;
}

bool CoDelQueue::CoDelTime::operator ==(const CoDelQueue::CoDelTime& rhs) const
{
  return m_time == rhs.m_time;
}

bool CoDelQueue::CoDelTime::operator !=(const CoDelQueue::CoDelTime& rhs) const
{
  return m_time != rhs.m_time;
}

CoDelQueue::CoDelTime CoDelQueue::CoDelTime::operator +(const CoDelQueue::CoDelTime& rhs) const
{
  return CoDelTime(m_time + rhs.m_time);
}

CoDelQueue::CoDelTime& CoDelQueue::CoDelTime::operator +=(const CoDelQueue::CoDelTime& rhs)
{
  m_time += rhs.m_time;
  return *this;
}

CoDelQueue::CoDelTime CoDelQueue::CoDelTime::operator *(double rhs) const
{
  return CoDelTime(m_time * rhs);
}

CoDelQueue::CoDelTime& CoDelQueue::CoDelTime::operator *=(double rhs)
{
  m_time *= rhs;
  return *this;
}

CoDelQueue::CoDelTime CoDelQueue::CoDelTime::operator -(const CoDelQueue::CoDelTime& rhs) const
{
  return CoDelTime(m_time - rhs.m_time);
}

CoDelQueue::CoDelTime& CoDelQueue::CoDelTime::operator -=(const CoDelQueue::CoDelTime& rhs)
{
  m_time -= rhs.m_time;
  return *this;
}

CoDelTimestampTag::CoDelTimestampTag ()
  : m_creationTime (Simulator::Now ())
{
}

TypeId
CoDelTimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CoDelTimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<CoDelTimestampTag> ()
  ;
  return tid;
}

TypeId
CoDelTimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
CoDelTimestampTag::GetSerializedSize (void) const
{
  return 8;
}

void
CoDelTimestampTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_creationTime.ToDouble(Time::NS));
}

void
CoDelTimestampTag::Deserialize (TagBuffer i)
{
  m_creationTime = Time::FromDouble(i.ReadDouble (), Time::NS);
}

void
CoDelTimestampTag::Print (std::ostream &os) const
{
  os << "CreationTime = " << m_creationTime;
}

Time
CoDelTimestampTag::GetTxTime (void) const
{
  return m_creationTime;
}

TypeId CoDelQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CoDelQueue")
    .SetParent<Queue> ()
    .AddConstructor<CoDelQueue> ()
    .AddAttribute ("Mode",
                   "Whether to use Bytes (see MaxBytes) or Packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (BYTES),
                   MakeEnumAccessor (&CoDelQueue::SetMode),
                   MakeEnumChecker (BYTES, "Bytes",
                                    PACKETS, "Packets"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this CoDelQueue.",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&CoDelQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this CoDelQueue.",
                   UintegerValue (1500 * 1000),
                   MakeUintegerAccessor (&CoDelQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinBytes",
                   "The CoDel algorithm minbytes parameter.",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&CoDelQueue::m_minbytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The CoDel algorithm interval",
                   TimeValue (Time("100ms")),
                   MakeTimeAccessor (&CoDelQueue::SetInterval),
                   MakeTimeChecker ())
    .AddAttribute ("ECNInterval",
                   "The CoDel algorithm interval",
                   TimeValue (Time(0)),
                   MakeTimeAccessor (&CoDelQueue::SetECNInterval),
                   MakeTimeChecker ())
    .AddAttribute ("Target",
                   "The CoDel algorithm target queue delay",
                   TimeValue (Time("5ms")),
                   MakeTimeAccessor (&CoDelQueue::m_Target),
                   MakeTimeChecker ())
    .AddAttribute ("ECNTarget",
                   "The ECN target queue delay",
                   TimeValue (Time(0)),
                   MakeTimeAccessor (&CoDelQueue::m_ECNTarget),
                   MakeTimeChecker ())
    .AddAttribute ("TargetRatio",
                   "The ECN target queue delay",
                   DoubleValue (0.7),
                   MakeDoubleAccessor (&CoDelQueue::m_TargetRatio),
                   MakeDoubleChecker<double> (0, 1))
    .AddAttribute ("OPD",
                   "DropOldest",
                   BooleanValue (false),
                   MakeBooleanAccessor (&CoDelQueue::m_OPD),
                   MakeBooleanChecker ())
    .AddAttribute ("EcnMode",
                   "Which ECN marking mode to use",
                   EnumValue (NO_ECN),
                   MakeEnumAccessor (&CoDelQueue::m_dropMode),
                   MakeEnumChecker (NO_ECN, "NoEcn",
                                    ECN_THEN_DROP, "EcnThenDrop",
                                    DROP_AND_ECN, "DropAndEcn",
                                    DROP_THEN_ECN, "DropThenEcn"))
    .AddTraceSource("count",
                    "CoDel count",
                    MakeTraceSourceAccessor(&CoDelQueue::m_count))
    .AddTraceSource("dropCount",
                    "CoDel drop count",
                    MakeTraceSourceAccessor(&CoDelQueue::m_dropCount))
    .AddTraceSource("ECNCount",
                    "CoDel ECN mark count",
                    MakeTraceSourceAccessor(&CoDelQueue::m_ECNCount))
  ;

  return tid;
}

CoDelQueue::CoDelQueue () :
  Queue (),
  m_packets (),
  m_maxBytes(),
  m_bytesInQueue(0),
  backlog(&m_bytesInQueue),
  m_count(0),
  m_lastCount(0),
  m_dropCount(0),
  m_dropping(false),
  m_recInvSqrt(~0U >> REC_INV_SQRT_SHIFT),
  m_firstAboveTime(Time(0)),
  m_firstAboveECNTime(Time(0)),
  m_dropNext(Time(0)),
  m_dropOverlimit(0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

CoDelQueue::~CoDelQueue ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
CoDelQueue::NewtonStep(void)
{
  uint32_t invsqrt = ((uint32_t) m_recInvSqrt) << REC_INV_SQRT_SHIFT;
  uint32_t invsqrt2 = ((uint64_t) invsqrt * invsqrt) >> 32;
  uint64_t val = ((uint64_t)3 << 32) - ((uint64_t) m_count * invsqrt2);

  val >>= 2; /* avoid overflow */
  val = (val * invsqrt) >> (32 - 2 + 1);
  m_recInvSqrt = val >> REC_INV_SQRT_SHIFT;
}

CoDelQueue::CoDelTime
CoDelQueue::ControlLaw(CoDelQueue::CoDelTime t)
{
  return t + reciprocalDivide(m_Interval.Get(), m_recInvSqrt << REC_INV_SQRT_SHIFT);
}

void
CoDelQueue::SetMode (enum Mode mode)
{
  NS_LOG_FUNCTION (mode);
  m_mode = mode;
}

CoDelQueue::Mode
CoDelQueue::GetMode (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mode;
}

bool
CoDelQueue::DropOldest (Ptr<Packet> p)
{
  int sizeToFree = (m_mode == BYTES) ? p->GetSize() : 1;
  Ptr<Packet> drop = m_packets.front();
  while (sizeToFree > 0 && !m_packets.empty())
    {
      sizeToFree -= (m_mode == BYTES) ? (int)drop->GetSize() : 1;

      Drop(drop, true);
      ++m_dropOverlimit;

      m_bytesInQueue -= drop->GetSize ();
      m_packets.pop();
      drop = m_packets.front();
    }
  return (sizeToFree <= 0);
}

bool
CoDelQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  //Initialize ECN Target
  if (m_TargetRatio > 0 && m_ECNTarget == Time(0))
    {
      m_ECNTarget = Time::FromDouble(m_Target.ToDouble(Time::NS) * m_TargetRatio, Time::NS);
    }
  if (m_TargetRatio > 0 && m_ECNInterval == Time(0))
    {
      m_ECNInterval = m_Interval * m_TargetRatio;
    }
  m_TargetRatio = -1;
  NS_ASSERT(m_ECNTarget <= m_Target);
  NS_ASSERT(m_ECNInterval <= m_Interval);

  if ((m_mode == PACKETS && (m_packets.size () + 1 >= m_maxPackets)) ||
        (m_mode == BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes)))
    {
      NS_LOG_LOGIC ("Queue full -- dropping pkt");
      if (!m_OPD)
        {
          Drop (p);
          ++m_dropOverlimit;
          return false;
        }
      else
        {
          if (!DropOldest(p))
            return false;
        }
    }

  CoDelTimestampTag tag;
  p->AddPacketTag (tag);
  m_bytesInQueue += p->GetSize ();
  m_packets.push (p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}

void
CoDelQueue::SetInterval(Time time)
{
  m_Interval = time;
}

void
CoDelQueue::SetECNInterval(Time time)
{
  m_ECNInterval = time;
}

bool
CoDelQueue::ShouldDrop(Ptr<Packet> p, const CoDelQueue::CoDelTime& now)
{
  CoDelTimestampTag tag;
  bool drop = false;

  p->PeekPacketTag (tag);
  Time delta = Simulator::Now () - tag.GetTxTime ();
  NS_LOG_INFO ("Sojourn time " << delta.GetSeconds ());
  CoDelQueue::CoDelTime sojournTime(delta);

  if (m_dropMode == DROP_AND_ECN)
    {
      if (sojournTime < CoDelQueue::CoDelTime(m_ECNTarget) || m_bytesInQueue < m_minbytes)
        {
          /* went below so we'll stay below for at least q->interval */
          m_firstAboveECNTime = Time(0);
        }
      else if (m_firstAboveECNTime == Time(0))
        {
          /* just went above from below. If we stay above
           * for at least q->interval we'll say it's ok to drop
           */
          m_firstAboveECNTime = now + m_ECNInterval;
        }
      else if (now > m_firstAboveECNTime)
        {
          bool marked = CoDelMarkPacket(p);
          if (!marked)
            {
              drop = true;
            }
        }
    }

  if (sojournTime < CoDelQueue::CoDelTime(m_Target) || m_bytesInQueue < m_minbytes)
    {
      /* went below so we'll stay below for at least q->interval */
      m_firstAboveTime = Time(0);
      return drop;
    }

  if (m_dropMode == DROP_THEN_ECN)
    {
      CoDelMarkPacket(p);
    }

  if (m_firstAboveTime == Time(0))
    {
      /* just went above from below. If we stay above
       * for at least q->interval we'll say it's ok to drop
       */
      m_firstAboveTime = now + m_Interval;
    }
  else if (now > m_firstAboveTime)
    {
      drop = true;
    }

  return drop;
}

bool
CoDelQueue::CoDelDoDequeue (Ptr<Packet>& p, const CoDelQueue::CoDelTime& now)
{
  if (m_packets.empty ())
    {
      m_dropping = false;
      m_firstAboveTime = Time(0);
      m_firstAboveECNTime = Time(0);
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
  p = m_packets.front ();
  m_packets.pop ();
  m_bytesInQueue -= p->GetSize ();

  NS_LOG_LOGIC ("Popped " << p);
  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return ShouldDrop(p, now);
}

bool
CoDelQueue::CoDelMarkPacket (Ptr<Packet> &p)
{
  Ptr<Header> ipHdr;
  bool marked = false;
  uint32_t offset = p->GetIpHeader(ipHdr);
  if (!!ipHdr && ipHdr->IsCongestionAware())
    {
      ipHdr->SetCongested();
      p->ReplaceHeader(ipHdr, offset);
      m_ECNCount++;
      marked = true;
    }
  return marked;
}

Ptr<Packet>
CoDelQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p;
  CoDelTime now; /* default constructor creates CoDelTime for Simulator::Now() */

  bool drop = CoDelDoDequeue(p, now);

  if (m_dropping)
    {
      if (!drop)
        {
          /* sojourn time below target - leave dropping state */
          m_dropping = false;
        }
      else if (now >= m_dropNext)
        {
          /* It's time for the next drop. Drop the current
           * packet and dequeue the next. The dequeue might
           * take us out of dropping state.
           * If not, schedule the next drop.
           * A large backlog might result in drop rates so high
           * that the next drop should happen now,
           * hence the while loop.
           */
          bool marked = false;
          while (m_dropping && now >= m_dropNext && !marked)
            {
              if (m_dropMode == ECN_THEN_DROP)
                {
                  marked = CoDelMarkPacket(p);
                }

              if (!marked)
                {
                  Drop(p, true);
                  ++m_dropCount;
                  ++m_count;
                  NewtonStep();

                  drop = CoDelDoDequeue(p, now);
                }

              if (!drop)
                {
                  /* leave dropping state */
                  m_dropping = false;
                }
              else
                {
                  /* and schedule the next drop */
                  m_dropNext = ControlLaw(m_dropNext);
                }
            }
        }
    }
  else if (drop &&
              ((now - m_dropNext) < m_Interval || (now - m_firstAboveTime) >= m_Interval))
    {
      bool marked = false;
      if (m_dropMode == ECN_THEN_DROP)
        {
          marked = CoDelMarkPacket(p);
        }

      if (!marked)
        {
          Drop(p, true);
          ++m_dropCount;
          drop = CoDelDoDequeue(p, now);
        }
      m_dropping = true;

      /*
       * if min went above target close to when we last went below it
       * assume that the drop rate that controlled the queue on the
       * last cycle is a good starting point to control it now.
       */
      uint32_t delta = m_count - m_lastCount;
      if (delta > 1 && (now - m_dropNext) > m_Interval)
        {
          m_count = delta;
          NewtonStep();
        }
      else
        {
          m_count = 1;
          m_recInvSqrt = ~0U >> REC_INV_SQRT_SHIFT;
        }
      m_lastCount = m_count;
      m_dropNext = ControlLaw(now);
    }

  if (!!p)
    {
      CoDelTimestampTag tag;
      p->RemovePacketTag (tag);
    }
  return p;
}

uint32_t
CoDelQueue::GetQueueSize (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (GetMode () == BYTES)
    {
      return m_bytesInQueue;
    }
  else if (GetMode () == PACKETS)
    {
      return m_packets.size ();
    }
  else
    {
      NS_ABORT_MSG ("Unknown mode.");
    }
}

Ptr<const Packet>
CoDelQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

} // namespace ns3

