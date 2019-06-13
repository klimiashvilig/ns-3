/*
 * This script simulates a complex scenario with multiple gateways and end
 * devices. The metric of interest for this script is the throughput of the
 * network.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/node-container.h"
#include "ns3/mobility-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/command-line.h"
#include <algorithm>
#include <ctime>

#include "ns3/random-variable-stream.h"
#include "ns3/string.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/large-app-sender-helper.h"

#include "ns3/basic-energy-source-helper.h"
#include "ns3/energy-module.h"
#include "ns3/lora-energy-model-helper.h"

using namespace ns3;

int nEndDevices = 5;
int nGatways = 5;
static const int defaultDistance = 200;
static int fileSize = 2000;

std::ofstream myFile;
std::string fileName = "LoRaresults-" + std::to_string(fileSize) + "B.txt";
bool writeInFile = true;
bool variableDistance = false;

DeviceEnergyModelContainer endDeviceModels;
DeviceEnergyModelContainer gatewayModels;

void 
PacketReceptionCallback(Ptr<Packet const> packet, uint32_t systemId) {
  std::cout << "Packet received" << std::endl;
  std::cout << "Simulation time - " << Simulator::Now ().GetSeconds () << "s" << std::endl;
}

void 
PacketReceptionCallback1(Ptr<Packet const> packet, uint32_t systemId) {
  std::cout << "1:Packet received" << std::endl;
  std::cout << "1:Simulation time - " << Simulator::Now ().GetSeconds () << "s" << std::endl;
}

int main (int argc, char *argv[])
{
  if (writeInFile)
    myFile.open(fileName, std::ofstream::app);
  for (int distance = (variableDistance ? 75:defaultDistance); distance <= (variableDistance ? 600:defaultDistance); distance += 75) {
    CommandLine cmd;
    cmd.Parse (argc, argv);

    // Mobility
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0, 100, 0.0));
    positionAlloc->Add (Vector (20, 150, 0.0));
    positionAlloc->Add (Vector (40, 150, 0.0));
    positionAlloc->Add (Vector (60, 150, 0.0));
    positionAlloc->Add (Vector (80, 150, 0.0));
    positionAlloc->Add (Vector (100, 100, 0.0));
    positionAlloc->Add (Vector (20, 50, 0.0));
    positionAlloc->Add (Vector (40, 50, 0.0));
    positionAlloc->Add (Vector (60, 50, 0.0));
    positionAlloc->Add (Vector (80, 50, 0.0));
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    /************************
    *  Create the channel  *
    ************************/

    // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
    loss->SetPathLossExponent (3);
    loss->SetReference (1, 31.23);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);
    Ptr<LoraChannel> channel1 = CreateObject<LoraChannel> (loss, delay);
    Ptr<LoraChannel> channel2 = CreateObject<LoraChannel> (loss, delay);
    Ptr<LoraChannel> channel3 = CreateObject<LoraChannel> (loss, delay);
    Ptr<LoraChannel> channel4 = CreateObject<LoraChannel> (loss, delay);

    /************************
    *  Create the helpers  *
    ************************/

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper ();
    phyHelper.SetChannel (channel);
    LoraPhyHelper phyHelper1 = LoraPhyHelper ();
    phyHelper1.SetChannel (channel1);
    LoraPhyHelper phyHelper2 = LoraPhyHelper ();
    phyHelper2.SetChannel (channel2);
    LoraPhyHelper phyHelper3 = LoraPhyHelper ();
    phyHelper3.SetChannel (channel3);
    LoraPhyHelper phyHelper4 = LoraPhyHelper ();
    phyHelper4.SetChannel (channel4);

    // Create the LoraMacHelper
    LoraMacHelper macHelper = LoraMacHelper ();
    LoraMacHelper macHelper1 = LoraMacHelper ();
    LoraMacHelper macHelper2 = LoraMacHelper ();
    LoraMacHelper macHelper3 = LoraMacHelper ();
    LoraMacHelper macHelper4 = LoraMacHelper ();

    // Create the LoraHelper
    LoraHelper helper = LoraHelper ();
    LoraHelper helper1 = LoraHelper ();
    LoraHelper helper2 = LoraHelper ();
    LoraHelper helper3 = LoraHelper ();
    LoraHelper helper4 = LoraHelper ();

    /************************
    *  Create End Devices  *
    ************************/

    // Create a set of nodes
    NodeContainer endDevices;
    endDevices.Create (nEndDevices);

    // Assign a mobility model to each node
    mobility.Install (endDevices);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType (LoraPhyHelper::ED);
    macHelper.SetDeviceType (LoraMacHelper::ED);
    NetDeviceContainer endDevicesContainer = helper.Install (phyHelper, macHelper, endDevices.Get(0));
    phyHelper1.SetDeviceType (LoraPhyHelper::ED);
    macHelper1.SetDeviceType (LoraMacHelper::ED);
    NetDeviceContainer endDevicesContainer1 = helper1.Install (phyHelper1, macHelper1, endDevices.Get(1));
    phyHelper2.SetDeviceType (LoraPhyHelper::ED);
    macHelper2.SetDeviceType (LoraMacHelper::ED);
    NetDeviceContainer endDevicesContainer2 = helper2.Install (phyHelper2, macHelper2, endDevices.Get(2));
    phyHelper3.SetDeviceType (LoraPhyHelper::ED);
    macHelper3.SetDeviceType (LoraMacHelper::ED);
    NetDeviceContainer endDevicesContainer3 = helper3.Install (phyHelper3, macHelper3, endDevices.Get(3));
    phyHelper4.SetDeviceType (LoraPhyHelper::ED);
    macHelper4.SetDeviceType (LoraMacHelper::ED);
    NetDeviceContainer endDevicesContainer4 = helper4.Install (phyHelper4, macHelper4, endDevices.Get(4));

    /*********************
    *  Create Gateways  *
    *********************/

    // Create the gateway nodes
    NodeContainer gateways;
    gateways.Create (nGatways);
    mobility.Install (gateways);

    // Create a netdevice for each gateway
    phyHelper.SetDeviceType (LoraPhyHelper::GW);
    macHelper.SetDeviceType (LoraMacHelper::GW);
    NetDeviceContainer gatewayContainer = helper.Install (phyHelper, macHelper, gateways.Get(0));
    phyHelper1.SetDeviceType (LoraPhyHelper::GW);
    macHelper1.SetDeviceType (LoraMacHelper::GW);
    NetDeviceContainer gatewayContainer1 = helper1.Install (phyHelper1, macHelper1, gateways.Get(1));
    phyHelper2.SetDeviceType (LoraPhyHelper::GW);
    macHelper2.SetDeviceType (LoraMacHelper::GW);
    NetDeviceContainer gatewayContainer2 = helper2.Install (phyHelper2, macHelper2, gateways.Get(2));
    phyHelper3.SetDeviceType (LoraPhyHelper::GW);
    macHelper3.SetDeviceType (LoraMacHelper::GW);
    NetDeviceContainer gatewayContainer3 = helper3.Install (phyHelper3, macHelper3, gateways.Get(3));
    phyHelper4.SetDeviceType (LoraPhyHelper::GW);
    macHelper4.SetDeviceType (LoraMacHelper::GW);
    NetDeviceContainer gatewayContainer4 = helper4.Install (phyHelper4, macHelper4, gateways.Get(4));

    /************************
    *  Configure Gateways  *
    ************************/

    /**********************************************
    *  Set up the end device's spreading factor  *
    **********************************************/

    macHelper.SetSpreadingFactorsUp (NodeContainer(endDevices.Get(0)), NodeContainer(gateways.Get(0)), channel);
    macHelper1.SetSpreadingFactorsUp (NodeContainer(endDevices.Get(1)), NodeContainer(gateways.Get(1)), channel1);
    macHelper2.SetSpreadingFactorsUp (NodeContainer(endDevices.Get(2)), NodeContainer(gateways.Get(2)), channel2);
    macHelper3.SetSpreadingFactorsUp (NodeContainer(endDevices.Get(3)), NodeContainer(gateways.Get(3)), channel3);
    macHelper4.SetSpreadingFactorsUp (NodeContainer(endDevices.Get(4)), NodeContainer(gateways.Get(4)), channel4);

    /*********************************************
    *  Install applications on the end devices  *
    *********************************************/

    Time appStopTime = Seconds (5000);
    LargeAppSenderHelper appHelper = LargeAppSenderHelper ();
    appHelper.SetFileSize(fileSize);
    ApplicationContainer appContainer = appHelper.Install (NodeContainer(endDevices.Get(0)));

    appContainer.Start (Seconds (0));
    appContainer.Stop (appStopTime);

    LargeAppSenderHelper appHelper1 = LargeAppSenderHelper ();
    appHelper1.SetFileSize(fileSize);
    ApplicationContainer appContainer1 = appHelper1.Install (NodeContainer(endDevices.Get(1)));

    appContainer1.Start (Seconds (0));
    appContainer1.Stop (appStopTime);

    LargeAppSenderHelper appHelper2 = LargeAppSenderHelper ();
    appHelper2.SetFileSize(fileSize);
    ApplicationContainer appContainer2 = appHelper2.Install (NodeContainer(endDevices.Get(2)));

    appContainer2.Start (Seconds (0));
    appContainer2.Stop (appStopTime);

    LargeAppSenderHelper appHelper3 = LargeAppSenderHelper ();
    appHelper3.SetFileSize(fileSize);
    ApplicationContainer appContainer3 = appHelper3.Install (NodeContainer(endDevices.Get(3)));

    appContainer3.Start (Seconds (0));
    appContainer3.Stop (appStopTime);

    LargeAppSenderHelper appHelper4 = LargeAppSenderHelper ();
    appHelper4.SetFileSize(fileSize);
    ApplicationContainer appContainer4 = appHelper4.Install (NodeContainer(endDevices.Get(4)));

    appContainer4.Start (Seconds (0));
    appContainer4.Stop (appStopTime);

    Ptr<Node> object = gateways.Get(0);
    // Get the device
    Ptr<NetDevice> netDevice = object->GetDevice (0);
    Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
    NS_ASSERT (loraNetDevice != 0);
    Ptr<GatewayLoraPhy> gwPhy = loraNetDevice->GetPhy ()->GetObject<GatewayLoraPhy> ();

    // Global callbacks (every gateway)
    gwPhy->TraceConnectWithoutContext ("ReceivedPacket",
                                             MakeCallback (&PacketReceptionCallback));


    Ptr<Node> object1 = gateways.Get(1);
    // Get the device
    Ptr<NetDevice> netDevice1 = object1->GetDevice (0);
    Ptr<LoraNetDevice> loraNetDevice1 = netDevice1->GetObject<LoraNetDevice> ();
    NS_ASSERT (loraNetDevice1 != 0);
    Ptr<GatewayLoraPhy> gwPhy1 = loraNetDevice1->GetPhy ()->GetObject<GatewayLoraPhy> ();

    // Global callbacks (every gateway)
    gwPhy1->TraceConnectWithoutContext ("ReceivedPacket",
                                             MakeCallback (&PacketReceptionCallback1));

    /** Energy Model **/
    /* energy source */
    BasicEnergySourceHelper basicSourceHelper;
    // configure energy source
    basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (10000));
    // install source
    basicSourceHelper.Set ("BasicEnergySupplyVoltageV",DoubleValue (3.6)); // in Volts

    EnergySourceContainer endDeviceSources = basicSourceHelper.Install (endDevices);
    EnergySourceContainer gatewaySources = basicSourceHelper.Install (gateways);
    /* device energy model */
    LoraEnergyModelHelper loraEnergyHelper;
    // configure radio energy model
    // install device model
    endDeviceModels = loraEnergyHelper.Install (endDevicesContainer, endDeviceSources);
    gatewayModels = loraEnergyHelper.Install (gatewayContainer, gatewaySources);

    Simulator::Stop (appStopTime + Hours (2));

    // PrintSimulationTime ();

    Simulator::Run ();

    Simulator::Destroy ();
  }
  return 0;
}
