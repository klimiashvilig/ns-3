#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "ns3/file-helper.h"
#include "ns3/names.h"
#include <algorithm>
#include <ctime>


using namespace ns3;
using namespace lorawan;

int nEndDevices = 1;
int nGatways = 1;
static const int defaultDistance = 300;
static int fileSize = 200;

std::ofstream myFile;
std::string fileName = "LoRaresults-" + std::to_string(fileSize) + "B.txt";
bool writeInFile = false;
bool variableDistance = true;

DeviceEnergyModelContainer endDeviceModels;
DeviceEnergyModelContainer gatewayModels;

void 
PacketReceptionCallback(Ptr<Packet const> packet, uint32_t systemId) {
    double energyConsumed = 0;
    for (DeviceEnergyModelContainer::Iterator iter = endDeviceModels.Begin(); iter != endDeviceModels.End(); iter++)
        energyConsumed += (*iter)->GetTotalEnergyConsumption();
    std::cout << "EndDevice energy consumed - " << energyConsumed << std::endl;
    for (DeviceEnergyModelContainer::Iterator iter = gatewayModels.Begin(); iter != gatewayModels.End(); iter++)
        energyConsumed += (*iter)->GetTotalEnergyConsumption();
    std::cout << "Simulation time - " << Simulator::Now ().GetSeconds () << "s  |  Total energy consumed - " << energyConsumed << "J" << std::endl;
}

int main (int argc, char *argv[])
{
  if (writeInFile)
    myFile.open(fileName, std::ofstream::app);
  for (int distance = (variableDistance ? 75:defaultDistance); distance <= (variableDistance ? 600:defaultDistance); distance += 75) {
    CommandLine cmd;
    cmd.Parse (argc, argv);
    // RngSeedManager::SetSeed (1);
    // RngSeedManager::SetRun (1);

    std::cout << "distance = " << distance << "m" << std::endl;

    // Mobility
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (distance, 100, 0.0));
    positionAlloc->Add (Vector (0, 100, 0.0));
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

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper ();

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
    macHelper.SetDeviceType (LorawanMacHelper::ED);
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
    macHelper.SetDeviceType (LorawanMacHelper::GW);
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
    LargeAppSenderHelper appHelper = LargeAppSenderHelper ();
    appHelper.SetFileSize(fileSize);
    ApplicationContainer appContainer = appHelper.Install (endDevices);

    appContainer.Start (Seconds (0));
    appContainer.Stop (Hours(2));

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
    // configure energy model
    // install device model
    endDeviceModels = loraEnergyHelper.Install (endDevicesContainer, endDeviceSources);
    gatewayModels = loraEnergyHelper.Install (gatewayContainer, gatewaySources);

    Simulator::Stop (Hours (2));

    // PrintSimulationTime ();

    Simulator::Run ();

    Simulator::Destroy ();
  }
  return 0;
}
