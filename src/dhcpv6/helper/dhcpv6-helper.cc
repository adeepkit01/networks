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
#include "dhcpv6-helper.h"
#include "ns3/dhcpv6-server.h"
#include "ns3/dhcpv6-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

Dhcpv6ClientHelper::Dhcpv6ClientHelper (uint16_t port)
{
  m_factory.SetTypeId (Dhcpv6Client::GetTypeId ());
  SetAttribute ("RemoteAddress", Ipv6AddressValue ("ff02::1:2"));
  SetAttribute ("RemotePort", UintegerValue (port));
}

Dhcpv6ServerHelper::Dhcpv6ServerHelper (Ipv6Address pool_addr, Ipv6Prefix pool_mask, Ipv6Address option_1, uint16_t port)
{
  m_factory.SetTypeId (Dhcpv6Server::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("PoolAddresses", Ipv6AddressValue(pool_addr));
  SetAttribute ("PoolPrefix", Ipv6PrefixValue(pool_mask));
  SetAttribute ("Option_1", Ipv6AddressValue(option_1));
  
}

////////////////

void Dhcpv6ClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void Dhcpv6ServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer Dhcpv6ClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer Dhcpv6ServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application> Dhcpv6ClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Dhcpv6Client>();
  node->AddApplication (app);

  return app;
}

Ptr<Application> Dhcpv6ServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Dhcpv6Server>();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
