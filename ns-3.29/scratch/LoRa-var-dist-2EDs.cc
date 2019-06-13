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

int nEndDevices = 2;
int nGatways = 1;
static const int defaultDistance = 200;
static int fileSize = 232;

std::ofstream myFile;
std::string fileName = "LoRaresults-" + std::to_string(fileSize) + "B.txt";
bool writeInFile = true;
bool variableDistance = false;

DeviceEnergyModelContainer endDeviceModels;
DeviceEnergyModelContainer gatewayModels;

void 
PacketReceptionCallback(Ptr<Packet const> packet, uint32_t systemId) {
  std::cout << "Packet received" << std::endl;
  std::cout << "End of simulation! Simulation time - " << Simulator::Now ().GetSeconds () << "s" << std::endl;
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
    positionAlloc->Add (Vector (100, 100, 0.0));
    positionAlloc->Add (Vector (55, 150, 0.0));
    positionAlloc->Add (Vector (10, 100, 0.0));
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

    /************************
    *  Create the helpers  *
    ************************/

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper ();
    phyHelper.SetChannel (channel);

    // Create the LoraMacHelper
    LoraMacHelper macHelper = LoraMacHelper ();

    // Create the LoraHelper
    LoraHelper helper = LoraHelper ();

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
    NetDeviceContainer endDevicesContainer = helper.Install (phyHelper, macHelper, endDevices);

    /*********************
    *  Create Gateways  *
    *********************/

    // Create the gateway nodes (allocate them uniformely on the disc)
    NodeContainer gateways;
    gateways.Create (nGatways);
    mobility.Install (gateways);

    // Create a netdevice for each gateway
    phyHelper.SetDeviceType (LoraPhyHelper::GW);
    macHelper.SetDeviceType (LoraMacHelper::GW);
    NetDeviceContainer gatewayContainer = helper.Install (phyHelper, macHelper, gateways);

    /************************
    *  Configure Gateways  *
    ************************/

    /**********************************************
    *  Set up the end device's spreading factor  *
    **********************************************/

    macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

    /*********************************************
    *  Install applications on the end devices  *
    *********************************************/

    Time appStopTime = Seconds (5000);
    LargeAppSenderHelper appHelper1 = LargeAppSenderHelper ();
    appHelper1.SetFileSize(fileSize);
    ApplicationContainer appContainer1 = appHelper1.Install (endDevices.Get(0));
    LargeAppSenderHelper appHelper2 = LargeAppSenderHelper ();
    appHelper2.SetFileSize(fileSize);
    ApplicationContainer appContainer2 = appHelper2.Install (endDevices.Get(1));

    appContainer1.Start (Seconds (0));
    appContainer1.Stop (appStopTime);
    appContainer2.Start (Seconds (0));
    appContainer2.Stop (appStopTime);

    Ptr<Node> object = gateways.Get(0);
    // Get the device
    Ptr<NetDevice> netDevice = object->GetDevice (0);
    Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
    NS_ASSERT (loraNetDevice != 0);
    Ptr<GatewayLoraPhy> gwPhy = loraNetDevice->GetPhy ()->GetObject<GatewayLoraPhy> ();

    // Global callbacks (every gateway)
    gwPhy->TraceConnectWithoutContext ("ReceivedPacket",
                                             MakeCallback (&PacketReceptionCallback));

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
