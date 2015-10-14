/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB
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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ns3/ipv6.h>
#include "ns3/log.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <ns3/ipv6-static-routing-helper.h>
#include "dhcpv6-client.h"
#include "dhcpv6-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Dhcpv6Client");
NS_OBJECT_ENSURE_REGISTERED (Dhcpv6Client);

TypeId
Dhcpv6Client::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Dhcpv6Client")
    .SetParent<Application> ()
    .AddConstructor<Dhcpv6Client> ()
//    .AddAttribute ("Interval",
//                   "The time to wait between packets", TimeValue (Seconds (1.0)),
//                   MakeTimeAccessor (&Dhcpv6Client::m_interval),
//                   MakeTimeChecker ())
    .AddAttribute (
      "RemoteAddress",
      "The destination Ipv4Address of the outbound packets",
      Ipv6AddressValue (),
      MakeIpv6AddressAccessor (&Dhcpv6Client::m_peerAddress),
      MakeIpv6AddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (546),
                   MakeUintegerAccessor (&Dhcpv6Client::m_peerPort), 
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

Dhcpv6Client::Dhcpv6Client() : m_option1(Ipv6Address::GetAny())
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_ipc_socket = 0;
  m_refreshEvent = EventId ();
  m_rtrsEvent = EventId();
  m_state = IDLEE;
}

Dhcpv6Client::~Dhcpv6Client()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
Dhcpv6Client::SetRemote (Ipv6Address ip, uint16_t port)
{
  m_peerAddress = ip;
  m_peerPort = port;
}

Ipv6Address Dhcpv6Client::GetOption_1(void){
  return m_option1;
}


void
Dhcpv6Client::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
Dhcpv6Client::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
 
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (), 546);
//      Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> (); 
//      m_socket->BindToNetDevice(ipv4->GetNetDevice(0));
      m_socket->SetAllowBroadcast (true);
      m_socket->Bind (local);
      //Additional line: binding socket to one of the interfaces
      m_socket->BindToNetDevice(GetNode()->GetDevice(0));
    }
    m_socket->SetRecvCallback(MakeCallback(&Dhcpv6Client::NetHandler, this)); 

  if (m_ipc_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_ipc_socket = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetLoopback(), DHCPV6_IPC_PORT);
      m_ipc_socket->Bind (local);
    }
    m_ipc_socket->SetRecvCallback(MakeCallback(&Dhcpv6Client::IPCHandler, this)); 
    GetNode()->GetDevice(0)->AddLinkChangeCallback(MakeCallback(&Dhcpv6Client::LinkStateHandler, this));

    RunEfsm(); //assumes the link is up at this moment!
}

void
Dhcpv6Client::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_refreshEvent);
}

void Dhcpv6Client::IPCHandler (Ptr<Socket> socket){
  NS_LOG_FUNCTION (this << socket);
  Address from;
  uint8_t data[8]; //at length of the option name i.e. "Option_1" !!!
  
  socket->RecvFrom(data, 8, 0, from);
//    NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  Hiiiiiighhhhh!4");
  if(m_state == IDLEE) m_option1 = Ipv6Address("::");
  socket->SendTo((uint8_t*) &m_option1, 4, 0, from); //send "Option_1" value
  //TODO: to add here code to deal with new defined options _2,3,...
}

void Dhcpv6Client::NetHandler (Ptr<Socket> socket){
    NS_LOG_FUNCTION (this << socket);
    Dhcpv6Client::RunEfsm();
}

void Dhcpv6Client::LinkStateHandler(void){
   
   if(GetNode()->GetDevice(0)->IsLinkUp()){  //assumes a single net device on node !!!
      NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  " << "LINK UP!!!! at " << Simulator::Now().GetSeconds());
    m_socket->SetRecvCallback(MakeCallback(&Dhcpv6Client::NetHandler, this)); 
//      m_refreshEvent = Simulator::Schedule (Seconds (1.0), &Dhcpv6Client::RunEfsm, this);
    RunEfsm();
   }
   else{
      NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "LINK DOWN!!!! at " << Simulator::Now().GetSeconds()); //reinitialization
      Simulator::Remove(m_refreshEvent); //stop refresh timer!!!!
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());  //stop receiving DHCPV6 messages !!!
      m_state = IDLEE;
      
      Ptr<Ipv6> ipv6MN = GetNode()->GetObject<Ipv6> ();
      uint32_t ifIndex = ipv6MN->GetInterfaceForDevice(GetNode()->GetDevice(0)); //it is assumed this node has a single net device installed !
      ipv6MN->RemoveAddress(ifIndex, 0); //it is assumed this node has a single IP address settled ! 
       Ipv6StaticRoutingHelper ipv6RoutingHelper;
      Ptr<Ipv6StaticRouting> staticRouting = ipv6RoutingHelper.GetStaticRouting (ipv6MN);
      staticRouting->RemoveRoute(0); //it is assumed this node has a single route settled at this moment! 
      
      ipv6MN->AddAddress (ifIndex, Ipv6InterfaceAddress (Ipv6Address ("::"), Ipv6Prefix ("/0")));
      //and flush ARP cache !!!
   }

}

void Dhcpv6Client::RtrsHandler(void){
   NS_LOG_INFO("DHCPV6_SOLICIT retransmission event at " << Simulator::Now().GetSeconds());

   m_state = IDLEE;
   RunEfsm();
}




void Dhcpv6Client::RunEfsm(void){
    NS_LOG_FUNCTION_NOARGS ();
    Dhcpv6Header header;  
    Ptr<Packet> packet;
    Address from; 
   
    switch(m_state){
    case IDLEE:
        packet = Create<Packet> (DHCPV6_HEADER_LENGTH); 
        header.SetType (DHCPV6SOLICIT);
        header.SetChaddr(Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
        packet->AddHeader (header);
        
//        NS_LOG_INFO("AICICIIII!=" <<  Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
        if ((m_socket->SendTo(packet, 0, Inet6SocketAddress(Ipv6Address("ff02::1:2"), m_peerPort))) >= 0){
            NS_LOG_INFO ("[node " << GetNode()->GetId() << "] Trace TX: DHCPV6 SOLICIT" );
        }else{
            NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Error while sending DHCPV6 SOLICIT to " << m_peerAddress);
        }
        m_state = WAIT_ADVERTISE;
        m_socket->SetRecvCallback(MakeCallback(&Dhcpv6Client::NetHandler, this)); 
        m_rtrsEvent = Simulator::Schedule (Seconds (RTRS_TIMEOUT), &Dhcpv6Client::RtrsHandler, this);
        break;

    case WAIT_ADVERTISE:
        packet = Create<Packet> (DHCPV6_HEADER_LENGTH); 
        packet = m_socket->RecvFrom(from);

        if (packet->GetSize () > 0){
            packet->RemoveHeader (header);
            if (header.GetType() == DHCPV6ADVERTISE){

                if(header.GetChaddr() != Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress())){
//                NS_LOG_INFO ("Baaaaaaaaaaiiii!!!!!!!!! " << header.GetChaddr() << "SIIIII ");
//                NS_LOG_INFO (Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
                 break; //this ADVERTISE is not for me 
                }
                Simulator::Remove (m_rtrsEvent);
                m_myAddress = header.GetYiaddr();
                m_option1 = header.GetOption_1();
                
                NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace RX: DHCPV6 ADVERTISE leased addr = " << m_myAddress << " Option_1 value= " << m_option1); 
                //set up the IP address for this node 
packet = Create<Packet> (DHCPV6_HEADER_LENGTH);
                 header.SetType (DHCPV6REQ);
        header.SetOption_1(m_myAddress);
        header.SetChaddr(Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
        packet->AddHeader (header);
                  NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 ADDRESS"<<Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
//        NS_LOG_INFO("AICICIIII!=" <<  Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
      m_socket->SendTo(packet, 0, Inet6SocketAddress(Ipv6Address("ff02::1:2"), m_peerPort));
}}
 m_state = WAIT_REPLY;
        m_rtrsEvent = Simulator::Schedule (Seconds (100), &Dhcpv6Client::RtrsHandler, this);
        break;
case WAIT_REPLY:
case WAIT_RREPLY:

 
packet = Create<Packet> (DHCPV6_HEADER_LENGTH); 
        packet = m_socket->RecvFrom(from);
         
        if (packet->GetSize () > 0){
            packet->RemoveHeader (header);
            while(header.GetChaddr() != Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()))
            { packet = m_socket->RecvFrom(from);packet->RemoveHeader (header);}
            if (header.GetType() == DHCPV6REPLY){

                if(header.GetChaddr() != Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress())){
//                NS_LOG_INFO ("Baaaaaaaaaaiiii!!!!!!!!! " << header.GetChaddr() << "SIIIII ");
//                NS_LOG_INFO (Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
                            NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 ADDRESS"<<Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 ADDRESS RECV"<<header.GetChaddr());
                  NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 REPLY NOT MINE");
                 break; //this ADVERTISE is not for me 
                }
                 NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 REPLY RECEIVED from "<<m_option1);
                Simulator::Remove (m_rtrsEvent);
                Ptr<Ipv6> ipv6 = GetNode()->GetObject<Ipv6> ();
                if(m_state==WAIT_REPLY)
                {
                int32_t ifIndex = ipv6->AddInterface(GetNode()->GetDevice(0));
             
                //ipv6->RemoveAddress(ifIndex, 0); //it is assumed this node has a single IP address settled ! 
                
                ipv6->AddAddress (ifIndex, Ipv6InterfaceAddress ((Ipv6Address)m_myAddress, Ipv6Prefix (64))); //BEAWARE: the node IP Prefix is hardcoded here !!!!!!!!!!!!!
                ipv6->SetUp (ifIndex); 
                
                Inet6SocketAddress remote = Inet6SocketAddress (Inet6SocketAddress::ConvertFrom (from).GetIpv6 (), m_peerPort);
                m_socket->Connect(remote);//!!!!!!!!!!!!!de sters

                Ipv6StaticRoutingHelper ipv6RoutingHelper; //setup the gw IP addr
                Ptr<Ipv6StaticRouting> staticRouting = ipv6RoutingHelper.GetStaticRouting (ipv6);
                staticRouting->SetDefaultRoute(m_option1, ifIndex,Ipv6Address("::") , 0);
                }
                m_peerAddress= Inet6SocketAddress::ConvertFrom (from).GetIpv6 ();
                NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Current DHCPV6 Server is =" << m_peerAddress <<" with global address "<<m_option1);

                ipv6->SetUp (0); //the loopback dev i/f status is changing whenever link is changing !!!!
                
                m_state = REFRESH_LEASE;
                   //m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
                m_refreshEvent = Simulator::Schedule (Seconds (REFRESH_TIMEOUT), &Dhcpv6Client::RunEfsm, this); 
            }
          else if(header.GetType() == DHCPV6NREPLY)
          {
             if(header.GetChaddr() != Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress())){
//                NS_LOG_INFO ("Baaaaaaaaaaiiii!!!!!!!!! " << header.GetChaddr() << "SIIIII ");
//                NS_LOG_INFO (Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
                 break; //this ADVERTISE is not for me 
                }
            RtrsHandler();
         }

        }
        break;
    case REFRESH_LEASE:
        if(Simulator::IsExpired(m_refreshEvent)){
            uint8_t addr[16]; 
            m_myAddress.GetBytes(addr);
            packet = Create<Packet>(addr, sizeof(header));
            header.SetType (DHCPV6REQ);
            header.SetOption_1(m_myAddress);
            header.SetChaddr(Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
           NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 ADDRESS"<<Mac48Address::ConvertFrom(GetNode()->GetDevice(0)->GetAddress()));
            packet->AddHeader (header);    
            
            if ((m_socket->SendTo(packet, 0, Inet6SocketAddress(m_peerAddress, m_peerPort))) >= 0){
                NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Trace TX: DHCPV6 REQUEST");
            }else{
                NS_LOG_INFO ("[node " << GetNode()->GetId() << "]  "<< "Error while sending DHCPV6 REQ to " << m_peerAddress);
            }
            m_state= WAIT_RREPLY;
                 m_rtrsEvent = Simulator::Schedule (Seconds (REFRESH_TIMEOUT), &Dhcpv6Client::RtrsHandler, this);
            //m_socket->SetRecvCallback(MakeCallback(&Dhcpv6Client::NetHandler, this)); 

            //m_refreshEvent = Simulator::Schedule (Seconds (REFRESH_TIMEOUT), &Dhcpv6Client::RunEfsm, this); 
        }
        //here to add code to deal with DHCPV6 REPLY!!!
        //it will stay in REFRESH state until link-down, thereafter will switch into IDLEE state;
        break;
    }
}


} // Namespace ns3
