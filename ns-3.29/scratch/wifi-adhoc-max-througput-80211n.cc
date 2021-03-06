/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
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
 */

// This script configures two nodes on an 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to the other node.  The physical layer is configured
// to receive at a fixed RSS (regardless of the distance and transmit
// power); therefore, changing position of the nodes has no effect.
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc --help"
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when rss drops below -97 dBm.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --rss=-97 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-98 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-99 --numPackets=20"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the documentation.
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
///
// ./waf --run "wifi-simple-adhoc --verbose=1"
//
// When you are done, you will notice two pcap trace files in your directory.
// If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-0-0.pcap -nn -tt
//

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/nstime.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/energy-module.h"

#include "ns3/ssid.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");

static const int distance = 40;
static const int runNum = 1;
static const int fileSize = 1000000;
static const double helloInterval = 20;
static const double TCInterval = 40;
static const int senderNode = 0;
static int receiverNode = 1;
Ptr<PacketSink> sink1;

void
PacketSinkTraceSink (Ptr<const Packet> packet, const Address &from)
{
  if ((int)sink1->GetTotalRx() >= fileSize) {
    std::cout << "End of simulation (" << Simulator::Now ().GetSeconds()
      << "s). Total size received = " << (int)sink1->GetTotalRx() << std::endl;
    std::cout << "Throughput = " << (int)sink1->GetTotalRx() * 8.0 / Simulator::Now().GetSeconds() / 1000000 << "Mbps" << std::endl;
    Simulator::Stop ();
    return;
  }
}

int main (int argc, char *argv[])
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (runNum);

  CommandLine cmd;
  cmd.Parse (argc, argv);  

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue ("HtMcs7"));

  NodeContainer c;
  c.Create (2);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue(10));
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel", 
        "Exponent", DoubleValue(3.0),
        "ReferenceDistance", DoubleValue(1.0),
        "ReferenceLoss", DoubleValue(40.046)); //ADD
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue ("HtMcs7"),
                                "ControlMode",StringValue ("HtMcs7"));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  // Note that with FixedRssLossModel, the positions below are not
  // used for received signal strength.
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add(Vector(0, 70, 0.0));
  positionAlloc->Add(Vector(distance, 70, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  // Enable OLSR
  OlsrHelper olsr;
  olsr.Set("HelloInterval", TimeValue(Seconds(helloInterval)));
  olsr.Set("TcInterval", TimeValue(Seconds(TCInterval)));
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); /*ADD*/ // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  //---------------------------------------------------
  OnOffHelper onOff ("ns3::UdpSocketFactory",
                     InetSocketAddress (i.GetAddress (receiverNode), 9));
  onOff.SetConstantRate(DataRate("54Mbps"));
  onOff.SetAttribute ("PacketSize", UintegerValue (1472));
  ApplicationContainer sourceApps = onOff.Install (c.Get (senderNode));
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10000.0));

  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApps = sink.Install (c.Get (receiverNode));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10000.0));

  sink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); // get sink

  std::string str = "/NodeList/" + std::to_string(receiverNode) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext (str, MakeCallback (&PacketSinkTraceSink));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

