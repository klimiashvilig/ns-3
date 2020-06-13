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
//
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
#include "ns3/constant-position-mobility-model.h"
#include "ns3/propagation-loss-model.h"

#include <iostream>
#include <fstream>
#include "ns3/command-line.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");


int main (int argc, char *argv[])
{
  double distance = 100;
  bool useFriiModel = true;
  bool writeToFile = false;
  double passLossExponent = 3;

  CommandLine cmd;
  cmd.AddValue("distance", "Distance between transmitter and receiver", distance);
  cmd.AddValue("useFrii", "Whether we want to use Frii's or Log Distance propagation model", useFriiModel);
  cmd.AddValue("writeToFile", "Whether we want to write results to a file", writeToFile);
  cmd.AddValue("passLossExponent", "pass loss exponent for LogDistancePropagationLossModel", passLossExponent);
  cmd.Parse(argc, argv);

  std::string phyMode ("DsssRate1Mbps");
  double txPowerW = 0.001;
  double txPowerdBm = 10 * std::log10 (txPowerW) + 30;
  bool verbose = false;

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (2);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (0) );

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b

  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");


  if (useFriiModel) {  
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    Config::SetDefault("ns3::FriisPropagationLossModel::Frequency",
    DoubleValue (2.4e9)); 
  } else {
    wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
    Config::SetDefault("ns3::LogDistancePropagationLossModel::Exponent",
    DoubleValue (passLossExponent));
    Config::SetDefault("ns3::LogDistancePropagationLossModel::ReferenceLoss",
    DoubleValue (40.0459970202808)); 
  }

  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

   // The following lines set the postion of the two nodes 
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<MobilityModel> a = CreateObject<ns3::ConstantPositionMobilityModel> (); 
  a->SetPosition (Vector (0,0,0)); 

  Ptr<MobilityModel> b = CreateObject<ns3::ConstantPositionMobilityModel> (); 
  b->SetPosition (Vector (distance,0,0)); 

  double resultdBm;
  if (useFriiModel) {
    Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> (); 
    // Calculate the recevied power 
    resultdBm = lossModel->CalcRxPower(txPowerdBm, a, b);
  }
  else {
    Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> (); 
    // Calculate the recevied power 
    resultdBm = lossModel->CalcRxPower(txPowerdBm, a, b);
  }

// print the result to the terminal 
  NS_LOG_UNCOND ("Received Power (dBm) for " << distance <<"m =  " << resultdBm);

  std::ofstream myFile;
  if (writeToFile) {
    myFile.open("ECE445_Lab2_Results.txt", std::ofstream::app);
    myFile << distance << "," << resultdBm << "\n";
    myFile.close();
  }


  
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}