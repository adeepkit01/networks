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
#ifndef DHCPV6_HELPER_H
#define DHCPV6_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv6-address.h"

namespace ns3 {
/**
 * \brief create an dhcpv6 service 
 */


class Dhcpv6ClientHelper{
public:
  /**
   * Create Dhcpv6ClientHelper
   *
   * \param port The port number of the dhcpv6 server
   */
  Dhcpv6ClientHelper (uint16_t port);
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;
//  ApplicationContainer Install (NodeContainer c);
private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

class Dhcpv6ServerHelper{
public:
  /**
   * Create Dhcpv6ServerHelper
   *
   * \param pool_addr The domain of the IP addr that will be assigned dynamically
   * \param pool_mask The mask of the domain 
   * \param option_1 Here of type IPv4 address (e.g. IP addr of some network server)
   * \param port The port number of the dhcpv6 server
   *
   */
  Dhcpv6ServerHelper (Ipv6Address pool_addr, Ipv6Prefix pool_mask, Ipv6Address option_1, uint16_t port);
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;
private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};


} // namespace ns3

#endif /* DHCPV6_HELPER_H */
