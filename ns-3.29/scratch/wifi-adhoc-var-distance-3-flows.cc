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

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");

static const int defaultDistance = 450;
static const int defaultRunNum = 1;
static int fileSize = 200;
static const double helloInterval = 0.5;
static const double TCInterval = 1;
static const int senderNode = 0;
static int receiverNode;// = numNodes - 1;
std::ofstream myFile;
Ptr<PacketSink> sink1;
DeviceEnergyModelContainer deviceModels;

std::string fileName = "wifiresults-" + std::to_string(fileSize) + "B-random-3flows-3" + ".txt";
bool writeInFile = false;
bool variableDistance = true;
bool variableRunNum = true;

void stop() {
  double energyConsumed = 0;
  for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin (); 
                      iter != deviceModels.End (); iter ++)
  {
    energyConsumed += (*iter)->GetTotalEnergyConsumption ();
  }
  std::cout << "End of simulation (" << Simulator::Now ().GetSeconds()
    << "s) Total energy consumed by radio = " << energyConsumed << "J" << std::endl;
  if (writeInFile) {
    if (!myFile.is_open()) {
      myFile.open(fileName, std::ofstream::app);
    }
    myFile << Simulator::Now ().GetSeconds() << "," << energyConsumed << ",";
    myFile.close();
  }
  Simulator::Stop();
}

void
PacketSinkTraceSink (Ptr<const Packet> packet, const Address &from)
{
  if ((int)sink1->GetTotalRx() >= fileSize) {
    double energyConsumed = 0;
    for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin (); 
                        iter != deviceModels.End (); iter ++)
    {
      energyConsumed += (*iter)->GetTotalEnergyConsumption ();
    }
    std::cout << "End of simulation (" << Simulator::Now ().GetSeconds()
      << "s) Total energy consumed by radio = " << energyConsumed << "J" << std::endl;
    if (writeInFile) {
      if (!myFile.is_open()) {
        myFile.open(fileName, std::ofstream::app);
      }
      myFile << Simulator::Now ().GetSeconds() << "," << energyConsumed << ",";
      myFile.close();
    }
    Simulator::Stop ();
    return;
  }
}

int main (int argc, char *argv[])
{
  if (writeInFile)
    myFile.open(fileName, std::ofstream::app);
  for (int distance = (variableDistance ? 75:defaultDistance); distance <= (variableDistance ? 450:defaultDistance); distance += 75) {
    int numNodes = (distance * distance / 10) / 500 + 3; // area = distance * distance / 10
    double alternativeApplicationStartTime = 2;
    double alternativeApplicationEndTime = 4;
    if (distance == 75) {
      numNodes = 6;
      alternativeApplicationStartTime = 0;
      alternativeApplicationEndTime = 0.1;
    }
	else if (distance == 150) {
	  numNodes = 9;
	  alternativeApplicationStartTime = 1;
      alternativeApplicationEndTime = 1.1;
	}
    receiverNode = numNodes - 1;
    std::cout << "Distance = " << distance << std::endl;
    for (int runNum = (variableRunNum ? 21:defaultRunNum); runNum <= (variableRunNum ? 30:defaultRunNum); runNum++) {
      RngSeedManager::SetSeed (1);  // Changes seed from default of 1 to 3 (1,1 3,7 4,5 2,3 6,5 7,2 2,6 5,3 4,7 5,5)
      RngSeedManager::SetRun (runNum);   // Changes run number from default of 1 to 7 (3,3 3,2 3,5 3,4 3,1)

      std::string phyMode ("ErpOfdmRate54Mbps");

      CommandLine cmd;
      cmd.Parse (argc, argv);

      //LogComponentEnable ("RoutingProtocol", LOG_LEVEL_ALL);

      // Fix non-unicast data rate to be the same as that of unicast
      Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                          StringValue (phyMode));

      NodeContainer c;
      c.Create (numNodes);

      // The below set of helpers will help us to put together the wifi NICs we want
      WifiHelper wifi;
      wifi.SetStandard (WIFI_PHY_STANDARD_80211g);

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
            "ReferenceLoss", DoubleValue(40.04)); //ADD
      wifiPhy.SetChannel (wifiChannel.Create ());

      // Add a mac and disable rate control
      WifiMacHelper wifiMac;
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode));
      // Set it to adhoc mode
      wifiMac.SetType ("ns3::AdhocWifiMac");
      NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

      // Note that with FixedRssLossModel, the positions below are not
      // used for received signal strength.
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      srand (time(NULL));
      double x, y;
      positionAlloc->Add(Vector(0, 70, 0.0));
      for (int i = 0; i < numNodes - 2; i++) {
            x = rand() % distance;
            y = rand() % (distance / 10) + 70 - distance / 20;
            positionAlloc->Add (Vector (x, y, 0.0));
      }
      positionAlloc->Add(Vector(distance, 70, 0.0));
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install(c);

      //ADD
      // Enable OLSR
      OlsrHelper olsr;
      olsr.Set("HelloInterval", TimeValue(Seconds(helloInterval)));
      olsr.Set("TcInterval", TimeValue(Seconds(TCInterval)));
      Ipv4StaticRoutingHelper staticRouting;

      Ipv4ListRoutingHelper list;
      list.Add (staticRouting, 0);
      list.Add (olsr, 10);
      //END

      InternetStackHelper internet;
      internet.SetRoutingHelper (list); /*ADD*/ // has effect on the next Install ()
      internet.Install (c);

      //ADD

      Ipv4AddressHelper ipv4;
      NS_LOG_INFO ("Assign IP Addresses.");
      ipv4.SetBase ("10.1.1.0", "255.255.255.0");
      Ipv4InterfaceContainer i = ipv4.Assign (devices);

      //---------------------------------------------------
      OnOffHelper onOff ("ns3::UdpSocketFactory",
                         InetSocketAddress (i.GetAddress (receiverNode), 9));
      onOff.SetConstantRate(DataRate("54Mbps"));
      ApplicationContainer sourceApps = onOff.Install (c.Get (senderNode));
      sourceApps.Start (Seconds (0.0));
      sourceApps.Stop (Seconds (10000.0));

      PacketSinkHelper sink ("ns3::UdpSocketFactory",
                             InetSocketAddress (Ipv4Address::GetAny (), 9));

      ApplicationContainer sinkApps = sink.Install (c.Get (receiverNode));

      sinkApps.Start (Seconds (0.0));
      sinkApps.Stop (Seconds (10000.0));

      //---------------------------------------------------

      /** Energy Model **/
      /***************************************************************************/
      /* energy source */
      BasicEnergySourceHelper basicSourceHelper;
      // configure energy source
      basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (10000));
      // install source
      basicSourceHelper.Set ("BasicEnergySupplyVoltageV",DoubleValue (3.6)); // in Volts

      EnergySourceContainer sources = basicSourceHelper.Install (c);
      /* device energy model */
      WifiRadioEnergyModelHelper radioEnergyHelper;
      // install device model
      deviceModels = radioEnergyHelper.Install (devices, sources);

      /***************************************************************************/
      if (numNodes >= 6) {
        OnOffHelper secondFlowOnOff("ns3::UdpSocketFactory",
                         InetSocketAddress (i.GetAddress (receiverNode - 1), 9));
        secondFlowOnOff.SetConstantRate(DataRate("54Mbps"));
        ApplicationContainer secondFlowSourceApps = secondFlowOnOff.Install (c.Get (senderNode + 1));
        double secondStartTime = alternativeApplicationStartTime + (float)rand()/(float)(RAND_MAX/(alternativeApplicationEndTime - alternativeApplicationStartTime));
        std::cout << "Second Start Time = " << secondStartTime << std::endl;
        secondFlowSourceApps.Start (Seconds (secondStartTime));
        secondFlowSourceApps.Stop (Seconds (10000.0));

        PacketSinkHelper secondFlowSink ("ns3::UdpSocketFactory",
                               InetSocketAddress (Ipv4Address::GetAny (), 9));

        ApplicationContainer secondFlowSinkApps = secondFlowSink.Install (c.Get (receiverNode - 1));

        secondFlowSinkApps.Start (Seconds (secondStartTime));
        secondFlowSinkApps.Stop (Seconds (10000.0));

        OnOffHelper thirdFlowOnOff("ns3::UdpSocketFactory",
                         InetSocketAddress (i.GetAddress (receiverNode - 2), 9));
        thirdFlowOnOff.SetConstantRate(DataRate("54Mbps"));
        ApplicationContainer thirdFlowSourceApps = thirdFlowOnOff.Install (c.Get (senderNode + 2));
        double thirdStartTime = alternativeApplicationStartTime + (float)rand()/(float)(RAND_MAX/(alternativeApplicationEndTime - alternativeApplicationStartTime));
        std::cout << "Third Start Time = " << thirdStartTime << std::endl;
        thirdFlowSourceApps.Start (Seconds (thirdStartTime));
        thirdFlowSourceApps.Stop (Seconds (10000.0));

        PacketSinkHelper thirdFlowSink ("ns3::UdpSocketFactory",
                               InetSocketAddress (Ipv4Address::GetAny (), 9));

        ApplicationContainer thirdFlowSinkApps = thirdFlowSink.Install (c.Get (receiverNode - 2));

        thirdFlowSinkApps.Start (Seconds (thirdStartTime));
        thirdFlowSinkApps.Stop (Seconds (10000.0));
      }

      // Tracing
      wifiPhy.EnablePcap ("wifi-adhoc", devices);

      sink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); // get sink

      std::string str = "/NodeList/" + std::to_string(receiverNode) + "/ApplicationList/0/$ns3::PacketSink/Rx";
      Config::ConnectWithoutContext (str, MakeCallback (&PacketSinkTraceSink));

      Simulator::Schedule(Seconds(20), &stop);

      Simulator::Run ();
      Simulator::Destroy ();
    }
    if (writeInFile) {
      if (!myFile.is_open()) {
        myFile.open(fileName, std::ofstream::app);
      }
      myFile << "\n";
      myFile.close();
    }
  }
  return 0;
}

