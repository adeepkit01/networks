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
#include "dhcp-helper.h"
#include "ns3/dhcp-server.h"
#include "ns3/dhcp-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {
  
  DhcpClientHelper::DhcpClientHelper (uint16_t port)
  {
    m_factory.SetTypeId (DhcpClient::GetTypeId ());
    SetAttribute ("RemoteAddress", Ipv4AddressValue ("255.255.255.255"));
    SetAttribute ("RemotePort", UintegerValue (port));
  }
  
  DhcpServerHelper::DhcpServerHelper (Ipv4Address pool_addr, Ipv4Mask pool_mask, Ipv4Address option_1, uint16_t port)
  {
    m_factory.SetTypeId (DhcpServer::GetTypeId ());
    SetAttribute ("Port", UintegerValue (port));
    SetAttribute ("PoolAddresses", Ipv4AddressValue(pool_addr));
    SetAttribute ("PoolMask", Ipv4MaskValue(pool_mask));
    SetAttribute ("Option_1", Ipv4AddressValue(option_1));
    
  }
  
  ////////////////
  
  void DhcpClientHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }
  
  void DhcpServerHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }
  
  ApplicationContainer DhcpClientHelper::Install (Ptr<Node> node) const
  {
    return ApplicationContainer (InstallPriv (node));
  }
  
  ApplicationContainer DhcpServerHelper::Install (Ptr<Node> node) const
  {
    return ApplicationContainer (InstallPriv (node));
  }
  
  Ptr<Application> DhcpClientHelper::InstallPriv (Ptr<Node> node) const
  {
    Ptr<Application> app = m_factory.Create<DhcpClient>();
    node->AddApplication (app);
    
    return app;
  }
  
  Ptr<Application> DhcpServerHelper::InstallPriv (Ptr<Node> node) const
  {
    Ptr<Application> app = m_factory.Create<DhcpServer>();
    node->AddApplication (app);
    
    return app;
  }
  
} // namespace ns3
