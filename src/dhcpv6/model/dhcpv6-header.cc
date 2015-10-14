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
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "dhcpv6-header.h"
#include "ns3/address-utils.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Dhcpv6Header");
NS_OBJECT_ENSURE_REGISTERED (Dhcpv6Header);


Dhcpv6Header::Dhcpv6Header ()
//  : m_type (0)
//    m_ts (Simulator::Now ().GetTimeStep ())
{
}

void Dhcpv6Header::SetType (uint8_t type)
{
  m_op = type;
}
uint8_t Dhcpv6Header::GetType (void) const
{
  return m_op;
}

void Dhcpv6Header::SetChaddr (Mac48Address addr)
{
  m_chaddr = addr;
}
Mac48Address Dhcpv6Header::GetChaddr (void) 
{
  return m_chaddr;
}

void Dhcpv6Header::SetYiaddr (Ipv6Address addr)
{
  m_yiaddr = addr;
}
Ipv6Address Dhcpv6Header::GetYiaddr (void) const
{
  return m_yiaddr;
}

void Dhcpv6Header::SetOption_1 (Ipv6Address addr)
{
  m_option_1 = addr;
}
Ipv6Address Dhcpv6Header::GetOption_1 (void) const
{
  return m_option_1;
}

TypeId Dhcpv6Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Dhcpv6Header")
    .SetParent<Header> ()
    .AddConstructor<Dhcpv6Header> ()
  ;
  return tid;
}
TypeId Dhcpv6Header::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void Dhcpv6Header::Print (std::ostream &os) const
{
  os << "(type=" << m_op << ")";
}
uint32_t Dhcpv6Header::GetSerializedSize (void) const
{
  return DHCPV6_HEADER_LENGTH; 
}

void
Dhcpv6Header::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_op);
  WriteTo (i, m_chaddr);
  WriteTo (i, m_yiaddr);
  WriteTo (i, m_option_1);
}
uint32_t Dhcpv6Header::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_op = i.ReadU8 ();
  ReadFrom (i, m_chaddr);
  ReadFrom(i,m_yiaddr);
  ReadFrom(i,m_option_1);
  return GetSerializedSize ();
}

} // namespace ns3
