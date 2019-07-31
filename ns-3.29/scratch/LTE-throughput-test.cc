/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"

using namespace ns3;
const int distance = 100;
const int fileSize = 1000000;
int totalRx = 0;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
Ptr<PacketSink> sink1;

void
PacketSinkTraceSink (Ptr<const Packet> packet, const Address &from)
{
  std::cout << "Packet of size " << packet->GetSize() << " received at the sink at " << Simulator::Now ().GetSeconds() << ". The size of total data received: " << (int)sink1->GetTotalRx() << std::endl;
  if ((int)sink1->GetTotalRx() >= fileSize) {
    NS_LOG_UNCOND ("File Received in " << Simulator::Now ().GetSeconds() - 5);
    Simulator::Stop();
    return;
  }
}

void
Sent (Ptr<const Packet> packet)
{
  NS_LOG_UNCOND("Hege");
}

void ReceivePacketUl(std::string context, Ptr<const Packet> packet) {
  /*if (packet->GetSize() > 85)
    totalRx += packet->GetSize();
  //NS_LOG_UNCOND(totalRx);
  if (totalRx >= fileSize) {
    NS_LOG_UNCOND ("File Received at eNB in " << Simulator::Now ().GetSeconds());
    return;
  }*/
  totalRx += packet->GetSize();
  std::cout << "Received a packet of size " << packet->GetSize() << " at eNB at " << Simulator::Now ().GetSeconds() << ". The size of total data received: " << totalRx << std::endl;
}

NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");

int
main (int argc, char *argv[])
{

  // Command line arguments
  CommandLine cmd;
  cmd.Parse(argc, argv);

  //LogComponentEnable ("UdpClient", LOG_LEVEL_ALL); // udpclient

  NodeContainer ueNodes; ueNodes.Create(1); 
  NodeContainer enbNodes; enbNodes.Create(1); 

  Ptr<LteHelper> lte = CreateObject<LteHelper> (); 
  Ptr<PointToPointEpcHelper>  epc = CreateObject<PointToPointEpcHelper> (); 
  lte->SetEpcHelper (epc); 

  Ptr<Node> pgw = epc->GetPgwNode (); 

  NodeContainer servers;
  servers.Create (1); 
  Ptr<Node> server = servers.Get (0); 
  InternetStackHelper internet; 
  internet.Install (servers); 

  PointToPointHelper p2p; 
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  NetDeviceContainer iDevs = p2p.Install (pgw, server); 
  Ipv4AddressHelper ipv4; 
  ipv4.SetBase ("1.0.0.0", "255.0.0.0"); 
  Ipv4InterfaceContainer ifaces = ipv4.Assign (iDevs); 
  Ipv4Address serverAddr = ifaces.GetAddress (1); 

  Ipv4StaticRoutingHelper routing; 
  Ptr<Ipv4StaticRouting> pdnStaticRouting = routing.GetStaticRouting (server->GetObject<Ipv4> ()); 
  pdnStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1); 

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < 4; i++)
    {
      if (i == 0)
        positionAlloc->Add(Vector(0, 0, 0));
      else
        positionAlloc->Add(Vector(distance + (i - 1) * 50, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(ueNodes);
  mobility.Install(enbNodes);
  mobility.Install(pgw);
  mobility.Install(server);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lte->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lte->InstallUeDevice (ueNodes);


  internet.Install (ueNodes); 
  Ipv4InterfaceContainer ueIface; 
  ueIface = epc->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs)); 
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u) 
  { 
      Ptr<Node> ueNode = ueNodes.Get (u); 
      Ptr<Ipv4StaticRouting> ueStaticRouting = routing.GetStaticRouting (ueNode->GetObject<Ipv4> ()); 
      ueStaticRouting->SetDefaultRoute (epc->GetUeDefaultGatewayAddress (), 1); 
  } 

  lte->Attach (ueLteDevs); 


  UdpClientHelper ulClientHelper(serverAddr, 20000);
  ulClientHelper.SetAttribute("Interval", TimeValue(Seconds(1472.0*8.0/54000000.0)));
  ulClientHelper.SetAttribute("PacketSize", UintegerValue(1472));
  ulClientHelper.SetAttribute("MaxPackets", UintegerValue(10000));

  //OnOffHelper source ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address ("7.0.0.2"), 9)); 
  //source.SetConstantRate(DataRate("54Mbps"));
  ApplicationContainer sourceApps; //= source.Install (ueNodes.Get(0)); 
  sourceApps.Add(ulClientHelper.Install(ueNodes.Get(0)));

  
  sourceApps.Start (Seconds (5.0)); 
  sourceApps.Stop (Seconds (60.0)); 

  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 20000)); 
  ApplicationContainer sinkApps = sink.Install (server); 
  sinkApps.Start (Seconds (5.0)); 
  sinkApps.Stop (Seconds (60.0)); 


  sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
  std::string str = "/NodeList/" + std::to_string(3) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext (str, MakeCallback (&PacketSinkTraceSink));
  //Config::Connect ("/NodeList/1/DeviceList/0/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/UlSpectrumPhy/RxEndOk", MakeCallback (&ReceivePacketUl));

  Simulator::Run (); 
  Simulator::Destroy (); 

  return 0;

}

