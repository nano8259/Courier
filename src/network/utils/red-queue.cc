/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright © 2011 Marcos Talau
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
 * Author: Marcos Talau (talau@users.sourceforge.net)
 *
 * Thanks to: Duy Nguyen<duy@soe.ucsc.edu> by RED efforts in NS3
 *
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (c) 1990-1997 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * PORT NOTE: This code was ported from ns-2 (queue/red.cc).  Almost all
 * comments have also been ported from NS-2
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/random-variable-stream.h"
#include "red-queue.h"
#include <cmath>
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("RedQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RedQueue);

void* RedQueue::record = NULL;
std::vector<uint32_t> RedQueue::size;

TypeId RedQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RedQueue")
    .SetParent<Queue> ()
    .AddConstructor<RedQueue> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&RedQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (500),
                   MakeUintegerAccessor (&RedQueue::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("IdlePktSize",
                   "Average packet size used during idle times. Used when m_cautions = 3",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RedQueue::m_idlePktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Wait",
                   "True for waiting between dropped packets",
                   BooleanValue (true),
                   MakeBooleanAccessor (&RedQueue::m_isWait),
                   MakeBooleanChecker ())
    .AddAttribute ("Gentle",
                   "True to increases dropping probability slowly when average queue exceeds maxthresh",
                   BooleanValue (true),
                   MakeBooleanAccessor (&RedQueue::m_isGentle),
                   MakeBooleanChecker ())
    .AddAttribute ("MinTh",
                   "Minimum average length threshold in packets/bytes",
                   DoubleValue (5),
                   MakeDoubleAccessor (&RedQueue::m_minTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxTh",
                   "Maximum average length threshold in packets/bytes",
                   DoubleValue (15),
                   MakeDoubleAccessor (&RedQueue::m_maxTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TargetDelay",
                   "Target Delay upon which we will set MinTh and MaxTh. "
                   "MinTh and MaxTh must be set to 0, so we could automatically"
                   "set them.",
                   TimeValue(Time(0)),
                   MakeTimeAccessor (&RedQueue::m_targetDelay),
                   MakeTimeChecker())
    .AddAttribute ("MinRTT",
                   "Min RTT value to use when calculating m_qW. (100ms is default)",
                   TimeValue(Time("100ms")),
                   MakeTimeAccessor (&RedQueue::m_minRtt),
                   MakeTimeChecker())
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (25),
                   MakeUintegerAccessor (&RedQueue::m_queueLimit),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("QW",
                   "Queue weight related to the exponential weighted moving average (EWMA)",
                   DoubleValue (0.002),
                   MakeDoubleAccessor (&RedQueue::m_qW),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("LInterm",
                   "The maximum probability of dropping a packet",
                   DoubleValue (50),
                   MakeDoubleAccessor (&RedQueue::m_lInterm),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("Ns1Compat",
                   "NS-1 compatibility",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RedQueue::m_isNs1Compat),
                   MakeBooleanChecker ())
    .AddAttribute ("LinkBandwidth",
                   "The RED link bandwidth",
                   DataRateValue (DataRate ("1.5Mbps")),
                   MakeDataRateAccessor (&RedQueue::m_linkBandwidth),
                   MakeDataRateChecker ())
    .AddAttribute ("LinkDelay",
                   "The RED link delay",
                   TimeValue (MilliSeconds (20)),
                   MakeTimeAccessor (&RedQueue::m_linkDelay),
                   MakeTimeChecker ())
    .AddAttribute ("UseCurrent",
                   "Use current queue limit instead of average queue length (needed for DCTCP)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RedQueue::m_useCurrent),
                   MakeBooleanChecker ())
     .AddAttribute ("Adaptive",
                    "Use Adaptive RED mechanism",
                    BooleanValue (false),
                    MakeBooleanAccessor (&RedQueue::m_adaptiveRED),
                    MakeBooleanChecker ())
      .AddAttribute ("AdaptInterval",
                     "Interval to recalculate maxP (0.5s is default)",
                     TimeValue (Time("0.5s")),
                     MakeTimeAccessor (&RedQueue::m_adaptInterval),
                     MakeTimeChecker ())
      .AddAttribute ("AdaptTargetMin",
                     "Lower end of average queue length interval."
                     "If set to 0 it will be calculated from minTh and maxTh.",
                     DoubleValue (0),
                     MakeDoubleAccessor (&RedQueue::m_adaptiveTargetMin),
                     MakeDoubleChecker<double> ())
       .AddAttribute ("AdaptTargetMax",
                      "Higher end of average queue length interval."
                      "If set to 0 it will be calculated from minTh and maxTh.",
                      DoubleValue (0),
                      MakeDoubleAccessor (&RedQueue::m_adaptiveTargetMax),
                      MakeDoubleChecker<double> ())
       .AddAttribute ("AdaptAlpha",
                      "Alpha parameter for Adaptive RED."
                      "If set to 0 it will be calculated on every adaptInterval.",
                      DoubleValue (0),
                      MakeDoubleAccessor (&RedQueue::m_adaptiveAlpha),
                      MakeDoubleChecker<double> ())
       .AddAttribute ("AdaptAlphaMax",
                      "Max value for alpha parameter for Adaptive RED (0.1 is default)",
                      DoubleValue (0.1),
                      MakeDoubleAccessor (&RedQueue::m_adaptiveAlphaMax),
                      MakeDoubleChecker<double> ())
       .AddAttribute ("AdaptBeta",
                      "Beta parameter for Adaptive RED (0.9 is default)",
                      DoubleValue (0.9),
                      MakeDoubleAccessor (&RedQueue::m_adaptiveBeta),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("AdaptMaxMaxP",
                     "Max value for maxP (0.5 is default)",
                     DoubleValue (0.5),
                     MakeDoubleAccessor (&RedQueue::m_maxMaxP),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("AdaptMinMaxP",
                     "Min value for maxP (0.01 is default)",
                     DoubleValue (0.01),
                     MakeDoubleAccessor (&RedQueue::m_minMaxP),
                     MakeDoubleChecker<double> ())
    .AddAttribute ("NoQueues",
                   "Number of queues (deafult is 8). IP packets are queued in (TOS % NoQueues) queue",
                   UintegerValue (8),
                   MakeUintegerAccessor (&RedQueue::SetNoQueues),
                   MakeUintegerChecker<uint64_t> (1, 8))
    .AddAttribute ("DRR",
                   "Deficit Round Robin",
                   BooleanValue (true),
                   MakeBooleanAccessor (&RedQueue::m_DRR),
                   MakeBooleanChecker ())
    .AddAttribute ("WQ1",
                   "Weight of queue 1 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&RedQueue::m_WQ1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WQ2",
                   "Weight of queue 2 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&RedQueue::m_WQ2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WQ3",
                   "Weight of queue 3 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&RedQueue::m_WQ3),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WQ4",
                   "Weight of queue 4 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&RedQueue::m_WQ4),
                   MakeDoubleChecker<double> ())
     .AddAttribute ("WQ5",
                    "Weight of queue 5 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                    DoubleValue (5000),
                    MakeDoubleAccessor (&RedQueue::m_WQ5),
                    MakeDoubleChecker<double> ())
     .AddAttribute ("WQ6",
                    "Weight of queue 6 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                    DoubleValue (5000),
                    MakeDoubleAccessor (&RedQueue::m_WQ6),
                    MakeDoubleChecker<double> ())
      .AddAttribute ("WQ7",
                     "Weight of queue 7 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                     DoubleValue (5000),
                     MakeDoubleAccessor (&RedQueue::m_WQ7),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("WQ8",
                     "Weight of queue 8 for DRR. Weight can be in BYTES or PACKETS depending on Queue mode",
                     DoubleValue (5000),
                     MakeDoubleAccessor (&RedQueue::m_WQ8),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("HeadDrop",
                     "Use Head Drop algorithm",
                     BooleanValue (true),
                     MakeBooleanAccessor (&RedQueue::m_headDrop),
                     MakeBooleanChecker ())
  ;

  return tid;
}

RedQueue::RedQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0),
  m_hasRedStarted (false),
  m_noQueues(0),
  m_calculateAlpha(true)
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

RedQueue::~RedQueue ()
{
  m_adapt.Cancel();
  NS_LOG_FUNCTION (this);
}

void
RedQueue::SetNoQueues (uint8_t noQueues)
{
  NS_ASSERT(!m_hasRedStarted);
  m_noQueues = noQueues;
  m_packetQueues.resize(noQueues);
}

void
RedQueue::SetMode (RedQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

RedQueue::QueueMode
RedQueue::GetMode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
RedQueue::SetQueueLimit (uint64_t lim)
{
  NS_LOG_FUNCTION (this <<lim);
  m_queueLimit = lim;
}

void
RedQueue::SetTh (double minTh, double maxTh)
{
  NS_LOG_FUNCTION (this << minTh << maxTh);
  NS_ASSERT (minTh <= maxTh);
  m_minTh = minTh;
  m_maxTh = maxTh;
}

RedQueue::Stats
RedQueue::GetStats () const
{
  NS_LOG_FUNCTION (this);
  return m_stats;
}

int64_t
RedQueue::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
RedQueue::DropOldest(Ptr<Packet> p, bool ECNmark, uint64_t &dropStats)
{
  int sizeToFree = p->GetSize();
  MasterQueue_t::iterator i = m_packets.begin();
  while (sizeToFree > 0 && i != m_packets.end())
    {
      uint8_t tos = i->second;
      Ptr<Packet> drop = i->first;
      sizeToFree -= (int)drop->GetSize();

      Ptr<Header> header;
      uint32_t headerOffset = drop->GetIpHeader (header);
      if (ECNmark && !!header && header->IsCongestionAware())
        {
          header->SetCongested();
          drop->ReplaceHeader(header, headerOffset);
          m_stats.marked++;
          ++i;
        }
      else
        {
          m_bytesInQueue -= drop->GetSize ();

          Queue_t::iterator iterList = m_packetQueues[tos % m_packetQueues.size()].begin();

          while (iterList->second != i && iterList != m_packetQueues[tos % m_packetQueues.size()].end())
            {
              ++iterList;
            }

          NS_ASSERT(iterList != m_packetQueues[tos % m_noQueues].end() &&
                    iterList->second == i && iterList->first == drop);

          i = m_packets.erase(i);
          m_packetQueues[tos % m_noQueues].erase(iterList);
          Drop(drop, true);
        }
    }
  return (sizeToFree <= 0);
}

bool
RedQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  //std::cout<<"m_queueLimit "<<m_queueLimit<<std::endl;

  if (!m_hasRedStarted )
    {
      NS_LOG_INFO ("Initializing RED params.");
      InitializeParams ();
      m_hasRedStarted = true;
    }

  uint64_t nQueued = 0;

  nQueued = GetQueueSize ();

  Estimator (nQueued);
  m_idle = false;

  NS_LOG_DEBUG ("\t bytesInQueue  " << m_bytesInQueue << "\tQavg " << m_qAvg);
  NS_LOG_DEBUG ("\t packetsInQueue  " << m_packets.size() << "\tQavg " << m_qAvg);

  m_count++;
  m_countBytes += p->GetSize ();

  uint32_t dropType = DTYPE_NONE;
  uint64_t nQueuedNew = nQueued + (GetMode () == QUEUE_MODE_PACKETS ? 1 : p->GetSize());
  if (m_qAvg >= m_minTh && nQueued > 1 && nQueuedNew <= m_queueLimit)
    {
      if (((!m_isGentle && m_qAvg >= m_maxTh) ||
          (m_isGentle && m_qAvg >= 2 * m_maxTh)) &&
          !(m_headDrop && DropOldest(p, true, m_stats.forcedDrop)))
        {
          NS_LOG_DEBUG ("adding DROP UNFORCED HARD MARK");
          dropType = DTYPE_UNFORCED_HARD;
        }
      else if (!m_old)
        {
          /*
           * The average queue size has just crossed the
           * threshold from below to above "minthresh", or
           * from above "minthresh" with an empty queue to
           * above "minthresh" with a nonempty queue.
           */
          m_count = 1;
          m_countBytes = p->GetSize ();
          m_old = true;
        }
      else if (DropEarly (p, nQueued) && !(m_headDrop && DropOldest(p, true, m_stats.unforcedDrop)))
        {
          NS_LOG_LOGIC ("DropEarly returns 1");
          dropType = DTYPE_UNFORCED_SOFT;
        }
    }
  else
    {
      // No packets are being dropped
      m_vProb = 0.0;
      m_old = false;
    }

  if (nQueuedNew > m_queueLimit && !(m_headDrop && DropOldest(p, false, m_stats.qLimDrop)))
    {
      NS_LOG_DEBUG ("\t Dropping due to Queue Full " << nQueued);
      dropType = DTYPE_FORCED;
      m_stats.qLimDrop++;
      //GlobalPropertyNetwork::m_noPacketLoss[2]++;//lkx 20180711

    }

  Ptr<Header> header;
  uint32_t offset = p->GetIpHeader (header);
  uint8_t tos = !!header ? header->GetPrecedence () : 0;

  /* Try to mark ECN bits first */
  if (dropType == DTYPE_UNFORCED_SOFT ||
        dropType == DTYPE_UNFORCED_HARD)
    {
      if (!!header && header->IsCongestionAware())
        {
          header->SetCongested();
          p->ReplaceHeader(header, offset);
          m_stats.marked++;
          /* We marked ECN bits! Packet shouldn't be droped */
          dropType = DTYPE_NONE;
        }
    }

  switch (dropType)
    {
      case DTYPE_UNFORCED_SOFT:
        {
          NS_LOG_DEBUG ("\t Dropping due to Prob Mark " << m_qAvg);
          m_stats.unforcedDrop++;
          GlobalPropertyNetwork::m_noPacketLoss[0]++;//lkx 20180711
          Drop (p);
          return false;
        }
      case DTYPE_UNFORCED_HARD:
        NS_LOG_DEBUG ("\t Dropping due to Hard Mark " << m_qAvg);
        m_stats.forcedDrop++;
        GlobalPropertyNetwork::m_noPacketLoss[1]++;//lkx 20180711
        /* no break */
      case DTYPE_FORCED:
        {
          Drop (p);
          GlobalPropertyNetwork::m_noPacketLoss[2]++;
          if (m_isNs1Compat)
            {
              m_count = 0;
              m_countBytes = 0;
            }
          return false;
        }
      default:
        break;
    };

  m_bytesInQueue += p->GetSize ();

  MasterQueue_t::iterator i = m_packets.end();
  i = m_packets.insert (m_packets.end(), std::make_pair(p, tos));
  m_packetQueues[tos % m_packetQueues.size()].push_back (std::make_pair(p, i));
  //std::cout<<"pkt size "<<p->GetSize()<<" tos "<<int(tos)<<" queue size "<<m_packetQueues.size()<<std::endl;
  // by tbc
  /*if(record==this)
	  size.push_back(m_packets.size());*/

  NS_LOG_LOGIC ("Number packets " << m_packets.size());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}

void
RedQueue::AdaptMaxP()
{
  /* Estimate queue size, before calculating maxP */
  Estimator(GetQueueSize());

  if (m_calculateAlpha)
    {
      m_adaptiveAlpha = std::min(m_adaptiveAlphaMax, m_curMaxP / 4.0);
    }

  if (m_qAvg > m_adaptiveTargetMax && m_curMaxP <= m_maxMaxP)
    {
      m_curMaxP += m_adaptiveAlpha;
    }
  else if (m_qAvg < m_adaptiveTargetMin && m_curMaxP >= m_minMaxP)
    {
      m_curMaxP *= m_adaptiveBeta;
    }
  m_adapt = Simulator::Schedule (m_adaptInterval, &RedQueue::AdaptMaxP, this);
}

/*
 * Note: if the link bandwidth changes in the course of the
 * simulation, the bandwidth-dependent RED parameters do not change.
 * This should be fixed, but it would require some extra parameters,
 * and didn't seem worth the trouble...
 */
void
RedQueue::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_minTh <= m_maxTh);
  m_stats.forcedDrop = 0;
  m_stats.marked = 0;
  m_stats.unforcedDrop = 0;
  m_stats.qLimDrop = 0;

  m_cautious = 0;
  m_ptc = m_linkBandwidth.GetBitRate () / (8.0 * m_meanPktSize);

  m_qAvg = 0.0;
  m_count = 0;
  m_countBytes = 0;
  m_old = false;
  m_idle = true;

  if (!m_minTh)
    {
      double targetQueue = m_targetDelay.GetSeconds() * m_ptc;
      m_minTh = std::max(5.0, targetQueue / 2.0);

      if (GetMode() == QUEUE_MODE_BYTES)
        {
          m_minTh *= m_meanPktSize * 8.0;
        }
    }

  if (!m_maxTh)
    {
      m_maxTh = m_minTh + 3.0;
    }

  double th_diff = (m_maxTh - m_minTh);
  NS_ASSERT(th_diff >= 0);

  if (th_diff == 0)
    {
      th_diff = 1.0;
    }
  m_vA = 1.0 / th_diff;
  m_curMaxP = 1.0 / m_lInterm;
  m_vB = -m_minTh / th_diff;

  if (m_isGentle)
    {
      m_vC = (1.0 - m_curMaxP) / m_maxTh;
      m_vD = 2.0 * m_curMaxP - 1.0;
    }
  m_idleTime = NanoSeconds (0);

/*
 * If m_qW=0, set it to a reasonable value of 1-exp(-1/C)
 * This corresponds to choosing m_qW to be of that value for
 * which the packet time constant -1/ln(1-m_qW) per default RTT
 * of 100ms is an order of magnitude more than the link capacity, C.
 *
 * If m_qW=-1, then the queue weight is set to be a function of
 * the bandwidth and the link propagation delay.  In particular,
 * the default RTT is assumed to be three times the link delay and
 * transmission delay, if this gives a default RTT greater than 100 ms.
 *
 * If m_qW=-2, set it to a reasonable value of 1-exp(-10/C).
 */
  if (m_qW == 0.0)
    {
      m_qW = 1.0 - std::exp (-1.0 / m_ptc);
    }
  else if (m_qW == -1.0)
    {
      double rtt = 3.0 * (m_linkDelay.GetSeconds () + 1.0 / m_ptc);

      if (rtt < m_minRtt.GetSeconds())
        {
          rtt = m_minRtt.GetSeconds();
        }
      m_qW = 1.0 - std::exp (-1.0 / (10 * rtt * m_ptc));
    }
  else if (m_qW == -2.0)
    {
      m_qW = 1.0 - std::exp (-10.0 / m_ptc);
    }

  if (m_adaptiveRED)
    {
      if (m_adaptiveTargetMin == 0)
        {
          NS_ASSERT(m_adaptiveTargetMax == 0);
          m_adaptiveTargetMin = m_minTh + 0.4 * (m_maxTh - m_minTh);
        }
      else
        {
          NS_ASSERT(m_adaptiveTargetMin <= m_adaptiveTargetMax &&
                  m_adaptiveTargetMax < (double)m_queueLimit);
        }

      if (m_adaptiveTargetMax == 0)
        {
          m_adaptiveTargetMax = m_minTh + 0.6 * (m_maxTh - m_minTh);
        }

      if (m_adaptiveAlpha)
        {
          m_calculateAlpha = false;
        }
      m_adapt = Simulator::Schedule (m_adaptInterval, &RedQueue::AdaptMaxP, this);
    }

  // Initialize Queue Weights for DRR
  if (m_noQueues > 0)
    {
	  //std::cout<<"queue number "<<m_noQueues<<std::endl;
      m_packetQueues.resize(m_noQueues);
      m_queueWeights.resize(m_noQueues);
      m_queueCurrentWeights.resize(m_noQueues);

      if (0 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[0] = m_WQ1;
          m_queueCurrentWeights[0] = 0;
      }

      if (1 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[1] = m_WQ2;
          m_queueCurrentWeights[1] = 0;
      }

      if (2 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[2] = m_WQ3;
          m_queueCurrentWeights[2] = 0;
      }

      if (3 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[3] = m_WQ4;
          m_queueCurrentWeights[3] = 0;
      }

      if (4 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[4] = m_WQ5;
          m_queueCurrentWeights[4] = 0;
      }

      if (5 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[5] = m_WQ6;
          m_queueCurrentWeights[5] = 0;
      }

      if (6 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[6] = m_WQ7;
          m_queueCurrentWeights[6] = 0;
      }

      if (7 < std::min(m_noQueues, (int)8))
        {
          m_queueWeights[7] = m_WQ8;
          m_queueCurrentWeights[7] = 0;
      }

      /* Initialize for the last queue! This way we're not loosing any iterations */
      m_queueCurrentWeights[m_noQueues - 1] = m_queueWeights[m_noQueues - 1];

      m_queueIter = m_packetQueues.rbegin();
      m_weightIterCurrent = m_queueCurrentWeights.rbegin();
      m_weightIter = m_queueWeights.rbegin();
    }
  m_noQueues = -1;

  NS_LOG_DEBUG ("\tm_delay " << m_linkDelay.GetSeconds () << "; m_isWait "
                             << m_isWait << "; m_qW " << m_qW << "; m_ptc " << m_ptc
                             << "; m_minTh " << m_minTh << "; m_maxTh " << m_maxTh
                             << "; m_isGentle " << m_isGentle << "; th_diff " << th_diff
                             << "; lInterm " << m_lInterm << "; va " << m_vA <<  "; cur_max_p "
                             << m_curMaxP << "; v_b " << m_vB <<  "; m_vC "
                             << m_vC << "; m_vD " <<  m_vD);
}

// Compute the average queue size
void
RedQueue::Estimator (uint64_t nQueued)
{
  NS_LOG_FUNCTION (this << nQueued);

  // simulate number of packets arrival during idle period
  uint64_t m = 1;

  if (m_idle)
    {
      NS_LOG_DEBUG ("RED Queue is idle.");
      Time now = Simulator::Now ();

      if (m_cautious == 3)
        {
          double ptc = m_ptc * m_meanPktSize / m_idlePktSize;
          m = uint64_t (ptc * (now - m_idleTime).GetSeconds ()) + 1;
        }
      else
        {
          m = uint64_t (m_ptc * (now - m_idleTime).GetSeconds ()) + 1;
        }
      m_idleTime = now;
    }

  if (!m_useCurrent)
    {
      while (--m >= 1)
        {
          m_qAvg *= 1.0 - m_qW;
        }
      m_qAvg *= 1.0 - m_qW;
      m_qAvg += m_qW * nQueued;
    }
  else
    {
      m_qAvg = nQueued;
    }
}

// Check if packet p needs to be dropped due to probability mark
bool
RedQueue::DropEarly (Ptr<Packet> p, uint64_t qSize)
{
  NS_LOG_FUNCTION (this << p << qSize);
  m_vProb1 = CalculatePNew (m_qAvg, m_maxTh, m_isGentle, m_vA, m_vB, m_vC, m_vD, m_curMaxP);
  m_vProb = ModifyP (m_vProb1, m_count, m_countBytes, m_meanPktSize, m_isWait, p->GetSize ());

  // Drop probability is computed, pick random number and act
  if (m_cautious == 1)
    {
      /*
       * Don't drop/mark if the instantaneous queue is much below the average.
       * For experimental purposes only.
       * pkts: the number of packets arriving in 50 ms
       */
      double pkts = m_ptc * 0.05;
      double fraction = std::pow ((1 - m_qW), pkts);

      if ((double) qSize < fraction * m_qAvg)
        {
          // Queue could have been empty for 0.05 seconds
          return false;
        }
    }

  double u = m_uv->GetValue ();

  if (m_cautious == 2)
    {
      /*
       * Decrease the drop probability if the instantaneous
       * queue is much below the average.
       * For experimental purposes only.
       * pkts: the number of packets arriving in 50 ms
       */
      double pkts = m_ptc * 0.05;
      double fraction = std::pow ((1 - m_qW), pkts);
      double ratio = qSize / (fraction * m_qAvg);

      if (ratio < 1.0)
        {
          u *= 1.0 / ratio;
        }
    }

  if (u <= m_vProb)
    {
      NS_LOG_LOGIC ("u <= m_vProb; u " << u << "; m_vProb " << m_vProb);

      // DROP or MARK
      m_count = 0;
      m_countBytes = 0;
      // TODO: Implement set bit to mark

      return true; // drop
    }

  return false; // no drop/mark
}

// Returns a probability using these function parameters for the DropEarly funtion
double
RedQueue::CalculatePNew (double qAvg, double maxTh, bool isGentle, double vA,
                         double vB, double vC, double vD, double maxP)
{
  NS_LOG_FUNCTION (this << qAvg << maxTh << isGentle << vA << vB << vC << vD << maxP);
  double p;

  if (isGentle && qAvg >= maxTh)
    {
      // p ranges from maxP to 1 as the average queue
      // Size ranges from maxTh to twice maxTh
      p = vC * qAvg + vD;
    }
  else if (!isGentle && qAvg >= maxTh)
    {
      /*
       * OLD: p continues to range linearly above max_p as
       * the average queue size ranges above th_max.
       * NEW: p is set to 1.0
       */
      p = 1.0;
    }
  else
    {
      /*
       * p ranges from 0 to max_p as the average queue size ranges from
       * th_min to th_max
       */
      p = vA * qAvg + vB;
      p *= maxP;
    }

  if (p > 1.0)
    {
      p = 1.0;
    }

  return p;
}

// Returns a probability using these function parameters for the DropEarly funtion
double
RedQueue::ModifyP (double p, uint64_t count, uint64_t countBytes,
                   uint32_t meanPktSize, bool isWait, uint32_t size)
{
  NS_LOG_FUNCTION (this << p << count << countBytes << meanPktSize << isWait << size);
  double count1 = (double) count;

  if (GetMode () == QUEUE_MODE_BYTES)
    {
      count1 = (double) (countBytes / meanPktSize);
    }

  if (isWait)
    {
      if (count1 * p < 1.0)
        {
          p = 0.0;
        }
      else if (count1 * p < 2.0)
        {
          p /= (2.0 - count1 * p);
        }
      else
        {
          p = 1.0;
        }
    }
  else
    {
      if (count1 * p < 1.0)
        {
          p /= (1.0 - count1 * p);
        }
      else
        {
          p = 1.0;
        }
    }

  if ((GetMode () == QUEUE_MODE_BYTES) && (p < 1.0))
    {
      p = (p * size) / meanPktSize;
    }

  if (p > 1.0)
    {
      p = 1.0;
    }

  return p;
}

uint64_t
RedQueue::GetQueueSize (void) const
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == QUEUE_MODE_BYTES)
    {
      return m_bytesInQueue;
    }
  else if (GetMode () == QUEUE_MODE_PACKETS)
    {
      return m_packets.size();
    }
  else
    {
      NS_ABORT_MSG ("Unknown RED mode.");
    }
}

Ptr<Packet>
RedQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_packetQueues.size() == m_queueCurrentWeights.size() && m_packetQueues.size() == m_queueWeights.size());
  std::vector<bool> is_empty(m_packetQueues.size(), false);
  std::vector<bool> all_empty(m_packetQueues.size(), true);
  if (!m_DRR)
    {
      m_queueIter = m_packetQueues.rbegin();
      m_weightIterCurrent = m_queueCurrentWeights.rbegin();
      m_weightIter = m_queueWeights.rbegin();
    }

  for (uint32_t i = 0; i < m_packetQueues.size() || m_DRR; m_queueIter = (++m_queueIter == m_packetQueues.rend()) ? m_packetQueues.rbegin() : m_queueIter,
                                          m_weightIterCurrent = (++m_weightIterCurrent == m_queueCurrentWeights.rend()) ? m_queueCurrentWeights.rbegin() : m_weightIterCurrent,
                                          m_weightIter = (++m_weightIter == m_queueWeights.rend()) ? m_queueWeights.rbegin() : m_weightIter,
                                          ++i)
    {
      std::pair<Ptr<Packet>, MasterQueue_t::iterator > pair;
      if (m_DRR && i != 0)
        {
          *m_weightIterCurrent += *m_weightIter;
        }

      if (m_queueIter->empty()) // modified by tbc: size()->empty()
        {
          if (m_DRR)
            {
              is_empty[i % m_packetQueues.size()] = true;
              *m_weightIterCurrent = 0;
              if (is_empty == all_empty)
                break;
            }
          continue;
        }
      else if (m_DRR)
        {
          if (m_mode == QUEUE_MODE_BYTES)
            {
              pair = m_queueIter->front ();
              Ptr<Packet> p = pair.first;
              if (*m_weightIterCurrent > p->GetSize())
                {
                  *m_weightIterCurrent -= p->GetSize();
                }
              else
                {
                  continue;
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
RedQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);
  if (!m_packets.size())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  /* This is only approximation of DRR algorithm! */
  int i = 0;
  QueueContainer::const_reverse_iterator iter = m_DRR ? static_cast<QueueContainer::const_reverse_iterator>(m_queueIter) : m_packetQueues.rbegin();
  for (; i < (int)m_packetQueues.size(); iter = (++iter == m_packetQueues.rend()) ? m_packetQueues.rbegin() : iter, ++i)
    {
      if (!iter->empty()) // modified by tbc: size()->empty()
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
