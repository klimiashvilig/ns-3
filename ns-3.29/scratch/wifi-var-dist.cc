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
#include <cstdlib>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhoc");

static const int distanceBetweenNodes = 75;
static int fileSize = 200;
static const double helloInterval = 0.5;
static const double TCInterval = 1;
static const int senderNode = 0;
static int receiverNode; // = numNodes - 1;
std::ofstream myFile;
Ptr < PacketSink > sink1;
DeviceEnergyModelContainer deviceModels;
bool routing = false;
bool writeInFile = false;

std::string fileName = "wifiresults-" + std::to_string(fileSize) + "B" + (routing ? "" : "-nr") + ".txt";

double initialEnergy;
double energyBeforeSending = 0;
double interPacketInterval = 1;
double startTime = 20;

void getInitialEnergy() {
  initialEnergy = 0;
  for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
    initialEnergy += (*iter)->GetTotalEnergyConsumption();
  }
}

void getEnergy() {
  double energyConsumed = 0;
  for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
    energyConsumed += (*iter)->GetTotalEnergyConsumption();
  }
  std::cout << "Energy (@" << Simulator::Now().GetSeconds() <<
    "s) = " << energyConsumed << "J" << std::endl;
  energyBeforeSending = energyConsumed;
  Simulator::Schedule(Seconds(interPacketInterval), & getEnergy);
}

void PacketSinkTraceSink(Ptr<const Packet> packet, const Address & from) {
  if ((int) sink1->GetTotalRx() >= fileSize) {
    double energyConsumed = -energyBeforeSending;
    for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
      energyConsumed += (*iter)->GetTotalEnergyConsumption();
    }
    std::cout << "Packet received at time " << Simulator::Now().GetSeconds() <<
      "s. Total energy consumed by radio = " << energyConsumed << "J" << std::endl;
    if (writeInFile) {
      if (!myFile.is_open()) {
        myFile.open(fileName, std::ofstream::app);
      }
      myFile << Simulator::Now().GetSeconds() - (double)(int)Simulator::Now().GetSeconds() << "," << energyConsumed << ",";
      myFile.close();
    }
    return;
  }
}

int main(int argc, char * argv[]) {

  ////////////////////////////////////// Initilize variables /////////////////////////////////////////////

  int distance = 300;
  int runNum = 1;
  bool endLine = false;
  int numPackets = 100;

  CommandLine cmd;
  cmd.AddValue("distance", "Distance between source and sink", distance);
  cmd.AddValue("runNum", "Run number", runNum);
  cmd.AddValue("writeInFile", "Whether we want to save output in a file", writeInFile);
  cmd.AddValue("routing", "Whether routing has converged", routing);
  cmd.AddValue("fileSize", "File size", fileSize);
  cmd.AddValue("numPackets", "Number of packets to be sent", numPackets);
  cmd.AddValue("endLine", "Whether we want to end the line in the file", endLine);
  cmd.Parse(argc, argv);
  
  std::cout << "Distance = " << distance << std::endl;
  std::cout << "runNum = " << runNum << std::endl;
  std::cout << "writeInFile = " << writeInFile << std::endl;
  std::cout << "routing = " << routing << std::endl;
  std::cout << "fileSize = " << fileSize << std::endl;
  std::cout << "numPackets = " << numPackets << std::endl;
  std::cout << "endLine = " << endLine << std::endl;

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(runNum);

  if (routing) {
    numPackets = 1;
  }

  int numNodes = distance / distanceBetweenNodes + 1;
  receiverNode = numNodes - 1;

  std::string phyMode("ErpOfdmRate54Mbps");

  fileName = "wifiresults-" + std::to_string(fileSize) + "B" + (routing ? "" : "-nr") + ".txt";
  std::cout << fileName << std::endl;
  if (writeInFile)
    myFile.open(fileName, std::ofstream::app);

  //LogComponentEnable ("RoutingProtocol", LOG_LEVEL_ALL);

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
    StringValue(phyMode));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
    UintegerValue(200));

 
  ////////////////////////////////////// Create Nodes /////////////////////////////////////////////

  NodeContainer c;
  c.Create(numNodes);

  
  ////////////////////////////////////// Physical Layer /////////////////////////////////////////////

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211g);

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set("RxGain", DoubleValue(10));
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
    "Exponent", DoubleValue(3.0),
    "ReferenceDistance", DoubleValue(1.0),
    "ReferenceLoss", DoubleValue(40.04)); //ADD
  wifiPhy.SetChannel(wifiChannel.Create());

  
  ////////////////////////////////////// MAC Layer /////////////////////////////////////////////

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
    "DataMode", StringValue(phyMode),
    "ControlMode", StringValue(phyMode));
  // Set it to adhoc mode
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

  
  ////////////////////////////////////// Mobility /////////////////////////////////////////////

  MobilityHelper mobility;
  Ptr < ListPositionAllocator > positionAlloc = CreateObject < ListPositionAllocator > ();
  double x, y;
  for (int i = 0; i < numNodes; i++) {
    x = i * distanceBetweenNodes + 70;
    y = 70;
    positionAlloc->Add (Vector (x, y, 0.0));
  }
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  ////////////////////////////////////// Enable OLSR /////////////////////////////////////////////
  OlsrHelper olsr;
  olsr.Set("HelloInterval", TimeValue(Seconds(helloInterval)));
  olsr.Set("TcInterval", TimeValue(Seconds(TCInterval)));
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add(staticRouting, 0);
  list.Add(olsr, 10);
  //END

  InternetStackHelper internet;
  internet.SetRoutingHelper(list); /*ADD*/ // has effect on the next Install ()
  internet.Install(c);

  //ADD

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO("Assign IP Addresses.");
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devices);

  
  ////////////////////////////////////// Create Application Layer /////////////////////////////////////////////

  uint16_t sinkPort = 6; // use the same for all apps

  // UDP connection from N0 to "receiverNode"

  Address sinkAddress (InetSocketAddress (i.GetAddress(receiverNode), sinkPort)); // interface of n24
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = sink.Install (c.Get(receiverNode)); //n24 as sink
  sinkApps.Start (Seconds ((routing ? 0 : startTime)));
  sinkApps.Stop (Seconds (numPackets * interPacketInterval + (routing ? 0 : startTime)));
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (c.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0


  // Create UDP application at N0


  UdpClientHelper client (sinkAddress, sinkPort);
  client.SetAttribute ("MaxPackets", UintegerValue (100));
  client.SetAttribute ("Interval", TimeValue (Seconds (interPacketInterval)));
  client.SetAttribute ("PacketSize", UintegerValue (512));
  ApplicationContainer apps ;
  apps = client.Install (c.Get (0));
  apps.Start (Seconds (routing ? 0 : startTime));
  apps.Stop (Seconds (numPackets * interPacketInterval + (routing ? 0 : startTime)));


  ////////////////////////////////////// Attach Energy Model /////////////////////////////////////////////

  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000));
  // install source
  basicSourceHelper.Set("BasicEnergySupplyVoltageV", DoubleValue(3.6)); // in Volts

  EnergySourceContainer sources = basicSourceHelper.Install(c);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // install device model
  deviceModels = radioEnergyHelper.Install(devices, sources);


  ////////////////////////////////////// Tracing /////////////////////////////////////////////

  sink1 = DynamicCast < PacketSink > (sinkApps.Get(0)); // get sink

  std::string str = "/NodeList/" + std::to_string(receiverNode) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(str, MakeCallback( & PacketSinkTraceSink));

  if (!routing) {
    Simulator::Schedule(Seconds(startTime), & getInitialEnergy);
    Simulator::Schedule(Seconds(20), & getEnergy);
  } else
    Simulator::Schedule(Seconds(0), & getEnergy);

  Simulator::Stop (Seconds (numPackets * interPacketInterval + (routing ? 0 : startTime) + 5));
  Simulator::Run();
  Simulator::Destroy();
  if (writeInFile && endLine) {
    if (!myFile.is_open()) {
      myFile.open(fileName, std::ofstream::app);
    }
    myFile << "\n";
    myFile.close();
  }
return 0;
}