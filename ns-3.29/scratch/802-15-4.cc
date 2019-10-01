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
#include "ns3/lr-wpan-radio-energy-model.h"
#include "ns3/basic-energy-source.h"


using namespace ns3;

int packetsReceived = 0;
Ptr<PacketSink> onOffSink1;
int distance = 300;
int fileSize = 10;
bool writeInFile = false;
const int distanceBetweenNodes = 75;
std::ofstream myFile;
double numHops;
int numNodes;
Ptr<LrWpanRadioEnergyModel> *emp;
bool varDistance = true;

std::string fileName = "802-15-4-results-" + (varDistance? (std::to_string(fileSize) + "B") : (std::to_string(distance) + "m")) + ".txt";

void PacketSinkTraceSink(Ptr<const Packet> packet, const Address & from) {
  double energyConsumed = 0;
  for (int i = 0; i < numNodes; i++)
    energyConsumed += emp[i]->GetTotalEnergyConsumption();
  std::cout << "Packet of size " << packet->GetSize() << "B received at " 
  << Simulator::Now().GetSeconds() << "s. Total size received "
  << (int)onOffSink1->GetTotalRx() << std::endl;  
  std::cout << "Total energy consumed = " << energyConsumed << std::endl;
  std::cout << "Throughput = " << (int)onOffSink1->GetTotalRx() * 8.0 / 1000.0 / Simulator::Now().GetSeconds() << "kbps" << std::endl;
  std::cout << "Throughput per hop = " << (int)onOffSink1->GetTotalRx() * 8.0 * numHops / 1000.0 / Simulator::Now().GetSeconds() << "kbps" << std::endl;
  if ((int) onOffSink1->GetTotalRx() >= fileSize) {
    if (writeInFile) {
      if (!myFile.is_open()) {
        myFile.open(fileName, std::ofstream::app);
      }
      myFile << Simulator::Now().GetSeconds() << "," << energyConsumed << ",";
      myFile.close();
    }
    Simulator::Stop();
  }
}

int main(int argc, char * argv[])
{
  int runNum = 1;
  bool endLine = false;
  bool logging = false;
  int packetSize = 512;

  CommandLine cmd;
  cmd.AddValue("distance", "Distance between source and sink", distance);
  cmd.AddValue("runNum", "Run number", runNum);
  cmd.AddValue("writeInFile", "Whether we want to save output in a file", writeInFile);
  cmd.AddValue("fileSize", "File size", fileSize);
  cmd.AddValue("endLine", "Whether we want to end the line in the file", endLine);
  cmd.AddValue("logging", "Enable logging", logging);
  cmd.AddValue("packetSize", "Size of the UDP packet", packetSize);
  cmd.AddValue("varDistance", "Variable distance for true / variable file size for false", varDistance);
  cmd.Parse(argc, argv);

  std::cout << "Distance = " << distance << std::endl;
  std::cout << "runNum = " << runNum << std::endl;
  std::cout << "writeInFile = " << writeInFile << std::endl;
  std::cout << "fileSize = " << fileSize << std::endl;
  std::cout << "endLine = " << endLine << std::endl;
  std::cout << "logging = " << logging << std::endl;
  std::cout << "packetSize = " << packetSize << std::endl;
  std::cout << "varDistance = " << varDistance << std::endl;

  if (logging) {
    LogComponentEnable ("LrWpanMac", LOG_LEVEL_ALL);
    LogComponentEnable ("LrWpanPhy", LOG_LEVEL_ALL);
  }

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(runNum);

  numNodes = distance / distanceBetweenNodes + 1;
  numHops = distance / distanceBetweenNodes;

  fileName = "802-15-4-results-" + (varDistance? (std::to_string(fileSize) + "B") : (std::to_string(distance) + "m")) + ".txt";
  std::cout << fileName << std::endl;
  if (writeInFile)
    myFile.open(fileName, std::ofstream::app);

  
  NodeContainer c;
  c.Create(numNodes);
  double txPower = -5;
  uint32_t channelNumber = 11;

  Ptr<LrWpanNetDevice> dev[numNodes];
  NetDeviceContainer lrwpanDevices;
  char ma[6] = "00:00";
  for (int i = 0; i < numNodes; i++) {
    dev[i] = CreateObject<LrWpanNetDevice>();
    lrwpanDevices.Add(dev[i]);
    ma[3] = '0' + (i + 1) / 10;
    ma[4] = '0' + (i + 1) % 10;
    dev[i]->SetAddress(Mac16Address(ma));
  }

  //Each device must be attached to same channel
  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel>();
  propModel->SetPathLossExponent(3);
  propModel->SetReference(1, 40.04);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);


  Ptr<LrWpanRadioEnergyModel> em[numNodes];
  Ptr<BasicEnergySource> es[numNodes];

  for (int i = 0; i < numNodes; i++) {
    dev[i]->SetChannel(channel);
    c.Get(i)->AddDevice(dev[i]);
    em[i] = CreateObject<LrWpanRadioEnergyModel> ();
    es[i] = CreateObject<BasicEnergySource> ();
    es[i]->SetSupplyVoltage(3.6);
    es[i]->SetInitialEnergy(10000.0);
    es[i]->SetNode (c.Get(i));
    es[i]->AppendDeviceEnergyModel (em[i]);
    em[i]->SetEnergySource (es[i]);
    em[i]->AttachPhy (dev[i]->GetPhy());
  }
  emp = em;

  //Set distance between nodes
  Ptr<ConstantPositionMobilityModel> mobility[numNodes];
  for (int i = 0; i < numNodes; i++) {
    mobility[i] = CreateObject<ConstantPositionMobilityModel>();
    mobility[i]->SetPosition(Vector(i * distanceBetweenNodes, 0, 0));
    dev[i]->GetPhy()->SetMobility(mobility[i]);
    std::cout << "Node #" << i << " - " << dev[i]->GetPhy() << std::endl;
  }

  //std::cout << "Data rate = " << dev[0]->GetPhy()->GetDataOrSymbolRate(true) << std::endl;

  //configure power of trasmission
  LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
  for (int i = 0; i < numNodes; i++)
    dev[i]->GetPhy()->SetTxPowerSpectralDensity(psd);
  
  //Install protocol stack at the nodes
  InternetStackHelper internetv6;
  internetv6.Install (c);

  SixLowPanHelper sixlowpan;
  NetDeviceContainer devices = sixlowpan.Install (lrwpanDevices); 
 
  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer deviceInterfaces;
  deviceInterfaces = ipv6.Assign (devices);
  for (int i = 1; i < numNodes - 1; i++)
    deviceInterfaces.SetForwarding(i, true);
  //deviceInterfaces.SetDefaultRouteInAllNodes (0);


  //Add static routing
  Ipv6StaticRoutingHelper ipv6RoutingHelper;

  Ptr<Ipv6StaticRouting> staticRouting[numNodes - 1];
  for (int i = 0; i < numNodes - 1; i++) {
    staticRouting[i] = ipv6RoutingHelper.GetStaticRouting (c.Get(i)->GetObject<Ipv6>());
    staticRouting[i]->AddHostRouteTo (deviceInterfaces.GetAddress (numNodes - 1,1), deviceInterfaces.GetAddress (i + 1,1), 1);
  }

  OnOffHelper onOff("ns3::UdpSocketFactory", Inet6SocketAddress(deviceInterfaces.GetAddress(numNodes - 1,1), 4000));
  onOff.SetConstantRate(DataRate("250kbps"));
  onOff.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer sourceApps = onOff.Install(c.Get(0));
  sourceApps.Start(Seconds(0));
  sourceApps.Stop(Seconds(1000.0));

  PacketSinkHelper onOffSink("ns3::UdpSocketFactory", Inet6SocketAddress(Ipv6Address::GetAny(), 4000));

  ApplicationContainer sinkApps = onOffSink.Install(c.Get(numNodes - 1));

  sinkApps.Start(Seconds(0));
  sinkApps.Stop(Seconds(1000.0));

  // Tracing
  onOffSink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); // get sink

  std::string str = "/NodeList/" + std::to_string(numNodes - 1) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(str, MakeCallback(&PacketSinkTraceSink));

  Simulator::Stop (Seconds (100.0));
  Simulator::Run ();
  Simulator::Destroy ();

  if (writeInFile && endLine) {
    if (!myFile.is_open()) {
      myFile.open(fileName, std::ofstream::app);
    }
    myFile << "\n";
    myFile.close();
  }

  return 0;
}