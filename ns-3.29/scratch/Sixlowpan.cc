//Author: George Klimiashvili
// Network topology
//
//       n0   n1   n2      nk  
//       |    |    |  ...  |
//       =============   ====
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

int distance = 75;
int fileSize = 200;
bool writeInFile = false;
const int distanceBetweenNodes = 75;
int packetSize = 75;
double numHops;
int numNodes;

bool varDistance = true;

bool firstPacket = true;

Ptr<PacketSink> sink1;
double energyBeforeSending = 0;
Ptr<LrWpanRadioEnergyModel> *emp;

std::ofstream myFile;
std::string fileName = "802-15-4-results-" + (varDistance? (std::to_string(fileSize) + "B") : (std::to_string(distance) + "m")) + "-" + std::to_string(packetSize) + "B" + ".txt";
std::string fileName_time = "802-15-4-time-" + (varDistance? (std::to_string(fileSize) + "B") : (std::to_string(distance) + "m")) + "-" + std::to_string(packetSize) + "B" + ".txt";

double startTime = 10;
double interPacketInterval = 1;
int packet_id = 1;

int maxPacketSize[8] = {84, 80, 78, 76, 74, 72, 70, 68};
int routing_overhead[8] = {0, 1, 1, 8, 10, 12, 14, 16};
double delay_between_packets[8] = {0, 0.023, 0.04, 0.028, 0.035, 0.043, 0.05, 0.058};

void getEnergy() {
  double energyConsumed = 0;
  for (int i = 0; i < numNodes; i++)
    energyConsumed += emp[i]->GetTotalEnergyConsumption();
  energyBeforeSending = energyConsumed;
  if (firstPacket) {
    Simulator::Schedule(Seconds(0.995), & getEnergy);
  } else {
    Simulator::Schedule(Seconds(interPacketInterval), & getEnergy);
  }
}

void PacketSinkTraceSink(Ptr<const Packet> packet, const Address & from) {
  double energyConsumed = -energyBeforeSending;
  for (int i = 0; i < numNodes; i++)
    energyConsumed += emp[i]->GetTotalEnergyConsumption();
  std::cout << "Packet of size " << packet->GetSize() << "B received at " 
      << Simulator::Now().GetSeconds() - startTime << "s" << std::endl;
  if (packet_id == 1) {
    if (packet->GetSize() == (uint32_t)(packetSize - routing_overhead[numNodes - 2])) {
      packet_id = 2;
    } else {
      // stay in the same state
      std::cout << "Failed to deliver the file\n";
    }
  } else if (packet_id == 2) {
    if (packet->GetSize() == (uint32_t)(packetSize - routing_overhead[numNodes - 2])) {
      packet_id = 3;
    } else {
      packet_id = 1;
      std::cout << "Failed to deliver the file\n";
    }
  } else {
    if (packet->GetSize() == (uint32_t)(packetSize - routing_overhead[numNodes - 2])) {
      packet_id = 2;
    } else {
      packet_id = 1;
      std::cout << "File of size " << fileSize << "B received at " 
          << Simulator::Now().GetSeconds() - startTime << "s. Energy consumed = " << energyConsumed << std::endl;
      if (writeInFile) {
        if (!myFile.is_open()) {
          myFile.open(fileName, std::ofstream::app);
        }
        myFile << Simulator::Now().GetSeconds() - startTime << "," << Simulator::Now().GetSeconds() << "," << energyConsumed << ",";
        myFile.close();
      }
    }
  }
}

void PrintRoutingTable (Ptr<Node> n)
{
  Ptr<Ipv6StaticRouting> routing = 0;
  Ipv6StaticRoutingHelper routingHelper;
  Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();
  uint32_t nbRoutes = 0;
  Ipv6RoutingTableEntry route;

  routing = routingHelper.GetStaticRouting (ipv6);
  nbRoutes = routing->GetNRoutes ();

  std::cout << "Routing table of " << n << " that has " << nbRoutes << " : " << std::endl;
  std::cout << "Destination\t" << "Gateway\t\t" << "Interface\t" <<  "Prefix to use" << std::endl;

  for (uint32_t i = 0; i < nbRoutes; i++)
   {
     route = routing->GetRoute (i);
     std::cout << route.GetDest () << "\t\t"
               << route.GetGateway () << "\t\t"
               << route.GetInterface () << "\t\t"
               << route.GetPrefixToUse () << "\t\t"
               << std::endl;
   }
}

int main(int argc, char * argv[])
{

  ////////////////////////////////////// Initilize variables /////////////////////////////////////////////

  int runNum = 1;
  bool endLine = false;
  bool logging = false;
  double numPackets = 20;

  CommandLine cmd;
  cmd.AddValue("distance", "Distance between source and sink", distance);
  cmd.AddValue("runNum", "Run number", runNum);
  cmd.AddValue("writeInFile", "Whether we want to save output in a file", writeInFile);
  cmd.AddValue("fileSize", "File size", fileSize);
  cmd.AddValue("numPackets", "Number of packets to be sent", numPackets);
  cmd.AddValue("endLine", "Whether we want to end the line in the file", endLine);
  cmd.AddValue("logging", "Enable logging", logging);
  cmd.AddValue("packetSize", "Size of the UDP packet", packetSize);
  cmd.AddValue("varDistance", "Variable distance for true / variable file size for false", varDistance);
  cmd.Parse(argc, argv);

  std::cout << "Distance = " << distance << std::endl;
  std::cout << "runNum = " << runNum << std::endl;
  std::cout << "writeInFile = " << writeInFile << std::endl;
  std::cout << "fileSize = " << fileSize << std::endl;
  std::cout << "numPackets = " << numPackets << std::endl;
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

  int receiverNode = numNodes - 1;

  fileName = "802-15-4-results-" + (varDistance? (std::to_string(fileSize) + "B") : (std::to_string(distance) + "m")) + "-" + std::to_string(packetSize) + "B" + ".txt";
  fileName_time = "802-15-4-time-" + (varDistance? (std::to_string(fileSize) + "B") : (std::to_string(distance) + "m")) + "-" + std::to_string(packetSize) + "B" + ".txt";
  std::cout << fileName << std::endl;
  if (writeInFile)
    myFile.open(fileName, std::ofstream::app);


  ////////////////////////////////////// Create Nodes /////////////////////////////////////////////
  
  NodeContainer c;
  c.Create(numNodes);

  double txPower = 12;
  uint32_t channelNumber = 11;


  ////////////////////////////////////// Create Net Devices /////////////////////////////////////////////

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


  ////////////////////////////////////// Mobility /////////////////////////////////////////////

  //Set distance between nodes
  Ptr<ConstantPositionMobilityModel> mobility[numNodes];
  for (int i = 0; i < numNodes; i++) {
    mobility[i] = CreateObject<ConstantPositionMobilityModel>();
    mobility[i]->SetPosition(Vector(i * distanceBetweenNodes, 0, 0));
    dev[i]->GetPhy()->SetMobility(mobility[i]);
    std::cout << "Node #" << i << " - " << dev[i]->GetPhy() << std::endl;
  }

  //std::cout << "Data rate = " << dev[0]->GetPhy()->GetDataOrSymbolRate(true) << std::endl;

  
  ////////////////////////////////////// Set Power /////////////////////////////////////////////

  //configure power of trasmission
  LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
  for (int i = 0; i < numNodes; i++)
    dev[i]->GetPhy()->SetTxPowerSpectralDensity(psd);
  
  
  ////////////////////////////////////// Create Sixlowpan Net Devices /////////////////////////////////////////////

  //Install protocol stack at the nodes
  InternetStackHelper internetv6;
  internetv6.Install (c);

  SixLowPanHelper sixlowpan;
  NetDeviceContainer devices = sixlowpan.Install (lrwpanDevices); 
 
  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2002::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer deviceInterfaces = ipv6.Assign (devices);
  for (int i = 0; i < numNodes; i++) {
    deviceInterfaces.SetForwarding(i, true);
  }
  // deviceInterfaces.SetDefaultRouteInAllNodes (1);

  // PrintRoutingTable(c.Get(0));

  //Add static routing
  Ipv6StaticRoutingHelper ipv6RoutingHelper;

  Ptr<Ipv6StaticRouting> staticRouting[numNodes - 1];
  for (int i = 0; i < numNodes - 1; i++) {
    staticRouting[i] = ipv6RoutingHelper.GetStaticRouting (c.Get(i)->GetObject<Ipv6>());
    staticRouting[i]->AddHostRouteTo (deviceInterfaces.GetAddress (numNodes - 1,1), deviceInterfaces.GetAddress (i + 1,1), 1);
  }
  // PrintRoutingTable(c.Get(0));

  
  ////////////////////////////////////// Create Application Layer /////////////////////////////////////////////


  uint16_t sinkPort = 4000; // use the same for all apps

  // UDP connection from N0 to "receiverNode"

  Address sinkAddress (Inet6SocketAddress (deviceInterfaces.GetAddress(receiverNode, 1), sinkPort)); // interface of n24
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Inet6SocketAddress(Ipv6Address::GetAny(), sinkPort));
  ApplicationContainer sinkApps = sink.Install (c.Get(receiverNode));
  sinkApps.Start (Seconds (startTime));
  sinkApps.Stop (Seconds(startTime + numPackets * interPacketInterval));
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (c.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0


  // Create UDP application at N0


  UdpClientHelper client (sinkAddress, sinkPort);
  client.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  client.SetAttribute ("Interval", TimeValue (Seconds (interPacketInterval)));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize - routing_overhead[numNodes - 2]));
  ApplicationContainer apps ;
  apps = client.Install (c.Get (0));
  apps.Start (Seconds (startTime));
  apps.Stop (Seconds(startTime + numPackets * interPacketInterval));

  UdpClientHelper client1 (sinkAddress, sinkPort);
  client1.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  client1.SetAttribute ("Interval", TimeValue (Seconds (interPacketInterval)));
  client1.SetAttribute ("PacketSize", UintegerValue (packetSize - routing_overhead[numNodes - 2]));
  ApplicationContainer apps1;
  apps1 = client1.Install (c.Get (0));
  apps1.Start (Seconds (startTime + delay_between_packets[numNodes - 2]));
  apps1.Stop (Seconds(startTime + numPackets * interPacketInterval));

  UdpClientHelper client2 (sinkAddress, sinkPort);
  client2.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  client2.SetAttribute ("Interval", TimeValue (Seconds (interPacketInterval)));
  client2.SetAttribute ("PacketSize", UintegerValue (fileSize - 2*maxPacketSize[numNodes - 2] - 9));
  ApplicationContainer apps2;
  apps2 = client2.Install (c.Get (0));
  apps2.Start (Seconds (startTime + 2.0*delay_between_packets[numNodes - 2]));
  apps2.Stop (Seconds(startTime + numPackets * interPacketInterval));


  ////////////////////////////////////// Tracing /////////////////////////////////////////////


  sink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); // get sink

  std::string str = "/NodeList/" + std::to_string(receiverNode) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(str, MakeCallback(&PacketSinkTraceSink));

  Simulator::Schedule(Seconds(startTime + 0.005), & getEnergy);

  Simulator::Stop(Seconds(startTime + numPackets * interPacketInterval + 5.0));
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