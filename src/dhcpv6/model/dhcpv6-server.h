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

#ifndef DHCPV6_SERVER_H
#define DHCPV6_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include <ns3/traced-value.h>

#define DHCPV6SOLICIT            0
#define DHCPV6ADVERTISE               1
#define DHCPV6REQ                 2
#define DHCPV6REPLY                 3
#define DHCPV6NREPLY                4

#define WORKING                 0


#define LEASE_TIMEOUT           30
#define GRANTED_LEASE_TIME      3

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup dhcpv6clientserver Dhcpv6ClientServer
 */

/**
 * \ingroup dhcpv6clientserver
 * \class Dhcpv6Server
 * \brief A Dhcpv6 server. 
 */
class Dhcpv6Server : public Application
{
public:
  static TypeId GetTypeId (void);
  Dhcpv6Server();
  virtual ~Dhcpv6Server();
  uint128_t getdiff(Ipv6Address a, Ipv6Address b);
  void dhcpv6add(Ipv6Address a, uint128_t b, uint8_t r[]);
  void RunEfsm(void);
  void NetHandler(Ptr<Socket> socket);
  void TimerHandler(void);


friend inline std::ostream & operator<< (std::ostream& os, Ipv4Address &address)
{
  address.Print (os);
  return os;
}
  
protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  Ptr<Socket> m_socket;
  Address m_local;
  uint16_t m_port;

  uint8_t m_state;
  uint8_t m_share;
  
  Ipv6Address m_poolAddress;
  Ipv6Prefix m_poolMask;
  Ipv6Address m_option1;
  Ipv6Address m_peer;
  std::list<std::pair<uint128_t, uint8_t> > m_leasedAddresses; //and their status (cache memory)
  uint128_t m_IDhost;
  
  EventId m_expiredEvent;
};

} // namespace ns3

#endif /* DHCPV6_SERVER_H */
