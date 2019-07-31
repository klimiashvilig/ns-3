//Author: Moab Rodrigues de Jesus
// Network topology
//
//       n0   n1   n2   n3  
//       |    |    |    |
//       =================
//        WSN (802.15.4)
//
//Based on the examples wsn-ping6.cc, ping6.cc, example-ping-lr-wpan.cc and udp-echo.cc

#include "ns3/core-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/single-model-spectrum-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/applications-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/internet-stack-helper.h"
#include <ns3/ipv6-address-helper.h>
#include <ns3/sixlowpan-helper.h>
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include <ns3/spectrum-value.h>


using namespace ns3;

int packetsReceived = 0;
Ptr<PacketSink> onOffSink1;
int fileSize = 100000;

void PacketSinkTraceSink(Ptr<const Packet> packet, const Address & from) {
  std::cout << "Packet of size " << packet->GetSize() << "B received at " 
  << Simulator::Now().GetSeconds() << "s. Total size received "
  << (int)onOffSink1->GetTotalRx() << std::endl;  
  std::cout << "Throughput = " << (int)onOffSink1->GetTotalRx() * 8.0 / 1000.0 / Simulator::Now().GetSeconds() << "kbps" << std::endl;
  std::cout << "Throughput per hop = " << (int)onOffSink1->GetTotalRx() * 8.0 * 3.0 / 1000.0 / Simulator::Now().GetSeconds() << "kbps" << std::endl;
  if ((int) onOffSink1->GetTotalRx() >= fileSize)
    Simulator::Stop();
}

int main()
{

  //LogComponentEnable ("WSN", LOG_LEVEL_INFO);
  //LogComponentEnable ("Socket", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

  //Create 4 nodes, and a device for each one
  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();
  Ptr<Node> n2 = CreateObject<Node>();
  Ptr<Node> n3 = CreateObject<Node>();
  double txPower = 0;
  uint32_t channelNumber = 11;
  
  NodeContainer nodes (n0, n1, n2, n3);

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev2 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev3 = CreateObject<LrWpanNetDevice>();

  NetDeviceContainer lrwpanDevices;
  lrwpanDevices.Add(dev0);
  lrwpanDevices.Add(dev1);
  lrwpanDevices.Add(dev2);
  lrwpanDevices.Add(dev3);
  std::cout << lrwpanDevices.Get(0) << std::endl;

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));
  dev2->SetAddress(Mac16Address("00:03"));
  dev3->SetAddress(Mac16Address("00:04"));

  //Each device must be attached to same channel
  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel>();
  propModel->SetPathLossExponent(3);
  propModel->SetReference(1, 40.04);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  dev0->SetChannel(channel);
  dev1->SetChannel(channel);
  dev2->SetChannel(channel);
  dev3->SetChannel(channel);
  
  //To complete configuration, a LrWpanNetDevice must be added to a node
  n0->AddDevice(dev0);
  n1->AddDevice(dev1);
  n2->AddDevice(dev2);
  n3->AddDevice(dev3);

  //Set distance between nodes
  Ptr<ConstantPositionMobilityModel> mobility0 = CreateObject<ConstantPositionMobilityModel>();
  mobility0->SetPosition(Vector(0,0,0));
  dev0->GetPhy()->SetMobility(mobility0);
  Ptr<ConstantPositionMobilityModel> mobility1 = CreateObject<ConstantPositionMobilityModel>();
  mobility1->SetPosition(Vector(75,0,0));
  dev1->GetPhy()->SetMobility(mobility1);
  Ptr<ConstantPositionMobilityModel> mobility2 = CreateObject<ConstantPositionMobilityModel>();
  mobility2->SetPosition(Vector(150,0,0));
  dev2->GetPhy()->SetMobility(mobility2);
  Ptr<ConstantPositionMobilityModel> mobility3 = CreateObject<ConstantPositionMobilityModel>();
  mobility3->SetPosition(Vector(225,0,0));
  dev3->GetPhy()->SetMobility(mobility3);

  //std::cout << "Data rate = " << dev1->GetPhy()->GetDataOrSymbolRate(true) << std::endl;

  //configure power of trasmission
  LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
  dev0->GetPhy ()->SetTxPowerSpectralDensity (psd);
  dev1->GetPhy ()->SetTxPowerSpectralDensity (psd);
  dev2->GetPhy ()->SetTxPowerSpectralDensity (psd);
  dev3->GetPhy ()->SetTxPowerSpectralDensity (psd);
  
  //Install protocol stack at the nodes
  InternetStackHelper internetv6;
  internetv6.Install (nodes);

  SixLowPanHelper sixlowpan;
  NetDeviceContainer devices = sixlowpan.Install (lrwpanDevices); 
 
  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer deviceInterfaces;
  deviceInterfaces = ipv6.Assign (devices);
  deviceInterfaces.SetForwarding (1, true);
  deviceInterfaces.SetForwarding (2, true);
  //deviceInterfaces.SetDefaultRouteInAllNodes (0);


  //Add static routing
  Ipv6StaticRoutingHelper ipv6RoutingHelper;

  Ptr<Ipv6StaticRouting> staticRouting0 = ipv6RoutingHelper.GetStaticRouting (n0->GetObject<Ipv6>());
  staticRouting0->AddHostRouteTo (deviceInterfaces.GetAddress (3,1), deviceInterfaces.GetAddress (1,1), 1);

  Ptr<Ipv6StaticRouting> staticRouting1 = ipv6RoutingHelper.GetStaticRouting (n1->GetObject<Ipv6>());
  staticRouting1->AddHostRouteTo (deviceInterfaces.GetAddress (3,1), deviceInterfaces.GetAddress (2,1), 1);

  Ptr<Ipv6StaticRouting> staticRouting2 = ipv6RoutingHelper.GetStaticRouting (n2->GetObject<Ipv6>());
  staticRouting2->AddHostRouteTo (deviceInterfaces.GetAddress (3,1), deviceInterfaces.GetAddress (3,1), 1);

  OnOffHelper onOff("ns3::UdpSocketFactory", Inet6SocketAddress(deviceInterfaces.GetAddress(3,1), 4000));
  onOff.SetConstantRate(DataRate("1Mbps"));
  ApplicationContainer sourceApps = onOff.Install(n0);
  sourceApps.Start(Seconds(0));
  sourceApps.Stop(Seconds(10000.0));

  PacketSinkHelper onOffSink("ns3::UdpSocketFactory", Inet6SocketAddress(Ipv6Address::GetAny(), 4000));

  ApplicationContainer sinkApps = onOffSink.Install(n3);

  sinkApps.Start(Seconds(0));
  sinkApps.Stop(Seconds(10000.0));

  // Tracing
  onOffSink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); // get sink

  std::string str = "/NodeList/" + std::to_string(3) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(str, MakeCallback(&PacketSinkTraceSink));

  Simulator::Stop (Seconds (1000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}