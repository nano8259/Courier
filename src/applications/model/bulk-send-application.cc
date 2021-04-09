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

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/tcp-socket-base.h"
#include "bulk-send-application.h"

NS_LOG_COMPONENT_DEFINE ("BulkSendApplication");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BulkSendApplication);

TypeId
BulkSendApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BulkSendApplication")
    .SetParent<Application> ()
    .AddConstructor<BulkSendApplication> ()
    .AddAttribute ("SendSize", "The amount of data to send each time.",
                   UintegerValue (512),
                   MakeUintegerAccessor (&BulkSendApplication::m_sendSize),
                   MakeUintegerChecker<uint64_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&BulkSendApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. "
                   "Once these bytes are sent, "
                   "no data  is sent again. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BulkSendApplication::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&BulkSendApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("TOS", "TOS value to use for packets.",
                   UintegerValue(0),
                   MakeUintegerAccessor (&BulkSendApplication::m_tos),
                   MakeUintegerChecker<uint8_t> (0, 255)) // modified by tbc, from 8 to 255
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&BulkSendApplication::m_txTrace))
    .AddTraceSource ("SocketCreateTrace", "Socket was created",
                      MakeTraceSourceAccessor (&BulkSendApplication::m_txTraceSource))
  ;
  return tid;
}


BulkSendApplication::BulkSendApplication ()
  : m_socket (0),
    m_connected (false),
    m_totBytes (0),
	m_isFinished(false), // tbc
	m_noRetries(0), // tbc
	m_lastSentBytes(0), // tbc
	m_speed(0) // tbc
{
  NS_LOG_FUNCTION (this);
  started = false;
  tb = Ptr<TokenBucket>();

  m_lastClock = 0;
}

BulkSendApplication::~BulkSendApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
BulkSendApplication::SetMaxBytes (uint64_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
BulkSendApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
BulkSendApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void BulkSendApplication::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  //std::cout<<Simulator::Now().GetMilliSeconds()<<std::endl;
  if(started==false){
	  started = true;
    // ZTODO why return?
    // for the create time's run
	  return;
  }

  // Create the socket if not already
  if (!m_socket )
    {
       m_socket = Socket::CreateSocket (GetNode (), m_tid);

      // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
      if (m_socket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
          m_socket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
        {
          NS_FATAL_ERROR ("Using BulkSend with an incompatible socket type. "
                          "BulkSend requires SOCK_STREAM or SOCK_SEQPACKET. "
                          "In other words, use TCP instead of UDP.");
        }

      m_txTraceSource (m_socket);
      m_socket->SetIpTos(m_tos);
      m_socket->Bind ();
      m_socket->Connect (m_peer);
      m_socket->ShutdownRecv ();
      m_socket->SetConnectCallback (
        MakeCallback (&BulkSendApplication::ConnectionSucceeded, this),
        MakeCallback (&BulkSendApplication::CheckAndRetry, this)); // by tbc
      m_socket->SetSendCallback (
        MakeCallback (&BulkSendApplication::DataSend, this));
      // by tbc
      m_socket->SetCloseCallbacks(
    		  MakeCallback (&BulkSendApplication::CheckAndRetry, this),
    		  MakeCallback (&BulkSendApplication::CheckAndRetry, this));
    }
  if (m_connected)
    {
      SendData ();
    }
}

void BulkSendApplication::StopWithoutCallback (void){
	Callback<void, Ptr< Socket > > vPS = MakeNullCallback<void, Ptr<Socket> > ();
	if (!!m_socket) {
		m_socket->SetCloseCallbacks(vPS,vPS);
	}
	StopApplication();
	m_connected = false;
	GlobalProperty::m_count_test++;
	//m_socket = Ptr<Socket>(); // XXX enable this line for debugging
	if(m_totBytes < m_maxBytes) m_socket = Ptr<Socket>();
	else m_isFinished = true;
}

void BulkSendApplication::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_connected = false;

      //m_isFinished  = true;//lkx
    }
  else
    {
      NS_LOG_WARN ("BulkSendApplication found null socket to close in StopApplication");
    }
}


// Private helpers

void BulkSendApplication::SendData (void)
{
  NS_LOG_FUNCTION (this);
  //Log0("SendData");

  bool hasSent =false;

  if(!m_connected) return; // 20171013 segmentation fault fix: a paused socket may not cancel all of its events thus this function can be called

  while ((m_totBytes < m_maxBytes) ){
      // Z 令牌桶，没有的话5ms后再发
	  if(!!tb && !tb->Get()){
      // Z 这里的这个令牌桶是没有用的令牌桶，因为get前面加了！
      // std::cout << m_tid << "no token to use at " << Simulator::Now() << std::endl;
		  if(!hasSent) Simulator::Schedule(MilliSeconds(5),&BulkSendApplication::SendData,this);
		  break;
	  }
	  hasSent = true;

	  // Time to send more
	  //std::cout<<"send"<<std::endl;
	 /* uint64_t now = Simulator::Now().GetMilliSeconds();

		if(now - m_lastClock >= 1000){
			Log1("[Clock]  Time: %u send application", static_cast<uint32_t>(now));
			m_lastClock = now;
		}*/
    
    uint32_t chosenMf;
    bool isPrioritized = false;
    if (m_prioritizedMf.empty()){
      // if the prioritized vector is empty, chosen the frist Mf in the active vector
      chosenMf = m_activeMf.front();
    }else{
      chosenMf = m_prioritizedMf.front();
      isPrioritized = true;
      // std::cout << "the priorirized mf is chosen" << std::endl;
    }

    uint64_t toSend = m_sendSize;
    // Make sure we don't send too many
    if (m_maxBytes > 0){
        toSend = std::min (m_sendSize, m_maxMfBytes[chosenMf] - m_totMfBytes[chosenMf]);
    }
    NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
    Ptr<Packet> packet = Create<Packet> ((uint32_t)toSend);
    m_txTrace (packet);
    int actual = m_socket->Send (packet);

    GlobalProperty::m_totalSendBytes_all += toSend;
    if (actual > 0){
      m_totBytes += actual;
      m_totMfBytes[chosenMf] += actual;
      GlobalProperty::m_totalSendBytes += actual;
      
      // check if this mf is finished
      if(m_totMfBytes[chosenMf] >= m_maxMfBytes[chosenMf]){
        m_isMfFinished[chosenMf] = 1;
        // delete the chosen mf
        std::vector<uint32_t>::iterator it;
        for(it = m_activeMf.begin(); it != m_activeMf.end(); it++){
          if(*it == chosenMf){
            m_activeMf.erase(it);
            break;
          }
        }
        if(isPrioritized){
          std::vector<uint32_t>::iterator it;
          for(it = m_prioritizedMf.begin(); it != m_prioritizedMf.end(); it++){
            if(*it == chosenMf){
              m_prioritizedMf.erase(it);
              break;
            }
          }
        }
      }
    }
    // if (toSend != 0){
    //   std::cout << "tosend = " << toSend << std::endl;
    // }else{
    //   std::cout << "tosend = " << toSend << std::endl;
    // }
    // std::cout << "tosend = " << toSend << std::endl;
    // std::cout << "actual = " << actual << std::endl;

    // We exit this loop when actual < toSend as the send side
    // buffer is full. The "DataSent" callback will pop when
    // some buffer space has freed ip.
    if ((unsigned)actual != toSend){
      //if(!!tb)tb->UnGet();
      //Log2("[Clock]  Time: actual %d toSend %d", actual, int(toSend));
      break;
    }

  }

  // Check if time to close (all sent)
  if (m_totBytes == m_maxBytes && m_connected)
    {
      //static int t=0;
      if(m_maxBytes == 0){
        std::cout << "here is 0" <<std::endl;
      }
      m_isFinished = true;
      
	    m_socket->Close ();
      
      //std::cout<<++t<<std::endl;
      // m_connected = false;
    }
}

void BulkSendApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("BulkSendApplication Connection succeeded");
  m_connected = true;
  SendData ();
}

void BulkSendApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("BulkSendApplication, Connection Failed");
}

void BulkSendApplication::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);

  if (m_connected)
    { // Only send new data if the connection has completed
      Simulator::ScheduleNow (&BulkSendApplication::SendData, this);
    }
}

// by tbc
bool BulkSendApplication::IsFinished() const
{
	NS_LOG_FUNCTION (this);
	return m_isFinished;
}

//by tbc
void BulkSendApplication::CheckAndRetry (Ptr<Socket> socket)
{
	NS_LOG_FUNCTION (this);

	StopApplication();
	m_connected = false;

	if(m_totBytes < m_maxBytes){
		std::cout<<"[BulkSendApplication::CheckAndRetry] "<<this<<" send failed, retry #"<< ++m_noRetries;
		std::cout<<"  -  FlowBytes: "<<m_maxBytes<<"  SentBytes: "<<m_totBytes<<std::endl;
		NS_LOG_WARN ("BulkSendApplication, Send Failed, Retry");
		m_socket = Ptr<Socket>(); // set to 0
		//if(m_noRetries<=100){ // retry at most 100 times
			StartApplication();
			return;
		//}
	}
  
	m_isFinished = true;
}

void BulkSendApplication::OutputStates(uint32_t job, uint32_t mapper, uint32_t reducer) const
{
	if(!m_isFinished){
		std::cout<<job<<"-"<<mapper<<"-"<<reducer<<"  FlowBytes: "<<m_maxBytes<<"  SentBytes: "<<m_totBytes<<std::endl;
		if(m_socket){
			TcpSocketBase* socket = static_cast<TcpSocketBase* >(m_socket.operator ->());
			socket->Output();
			std::cout<<std::endl;
		}
	}
}

uint64_t BulkSendApplication::GetSentBytes() const
{
	//const uint64_t txBufferSize = 128*1024;
	int64_t x = m_totBytes - (m_isFinished ? 0 : 0);
	return x>0 ? static_cast<uint64_t>(x) : 0;
}


} // Namespace ns3
