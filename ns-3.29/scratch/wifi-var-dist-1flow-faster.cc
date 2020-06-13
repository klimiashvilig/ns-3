#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "ns3/packet-sink-helper.h"
#include "ns3/udp-client-server-helper.h"

#include "ns3/command-line.h"

NS_LOG_COMPONENT_DEFINE ("Lab3");
using namespace ns3;

static const double helloInterval = 0.5;
static const double TCInterval = 1;

uint32_t MacTxDropCount, PhyTxDropCount, PhyRxDropCount;
void
MacTxDrop(Ptr<const Packet> p)
{
	NS_LOG_INFO("Packet Drop");
	MacTxDropCount++;
}
void
PrintDrop()
{
	std::cout << Simulator::Now().GetSeconds() << "\t" << MacTxDropCount << "\t"<<
	PhyTxDropCount << "\t" << PhyRxDropCount << "\n";
	Simulator::Schedule(Seconds(5.0), &PrintDrop);
}
void
PhyTxDrop(Ptr<const Packet> p)
{
	NS_LOG_INFO("Packet Drop");
	PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
	NS_LOG_INFO("Packet Drop");
	PhyRxDropCount++;
}

void PacketSinkTraceSink(Ptr<const Packet> packet, const Address & from) {
  // }
    std::cout << "Packet received at time " << Simulator::Now().GetSeconds() <<
      "s." << std::endl;
}

int main (int argc, char *argv[])
{

////////////////////////////////////// Initilize variables /////////////////////////////////////////////

	std::string phyMode ("DsssRate1Mbps");
	double distance = 500; // m
	uint32_t numNodes = 50; // by default, 5x5
	double interval = 1; // seconds
	uint32_t packetSize = 600; // bytes
	uint32_t numPackets = 1000;
	std::string rtslimit = "1000";
	uint32_t numFlows = 1;

	CommandLine cmd;
	cmd.AddValue("numFlows", "Number of flows in the network", numFlows);
	cmd.AddValue("rtslimit", "Threshold for rts", rtslimit);
	cmd.Parse(argc, argv);


	// Convert to time object
	Time interPacketInterval = Seconds (interval);
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

	////////////////////////////////////// Create Nodes /////////////////////////////////////////////

	NodeContainer c;
	c.Create (numNodes);

	////////////////////////////////////// Physical Layer /////////////////////////////////////////////

	// The below set of helpers will help us to put together the wifi NICs we want
	WifiHelper wifi;
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set ("RxGain", DoubleValue (-10) );
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	wifiPhy.SetChannel (wifiChannel.Create ());

	////////////////////////////////////// MAC Layer /////////////////////////////////////////////


	WifiMacHelper wifiMac ;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode",StringValue (phyMode),"ControlMode",StringValue (phyMode));
	// Set it to adhoc mode
	wifiMac.SetType ("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

	////////////////////////////////////// Mobility /////////////////////////////////////////////

	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
	"MinX", DoubleValue (0.0),
	"MinY", DoubleValue (0.0),
	"DeltaX", DoubleValue (distance),
	"DeltaY", DoubleValue (distance),
	"GridWidth", UintegerValue (5),
	"LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (c);

	////////////////////////////////////// Enable OLSR (Routing protocol) /////////////////////////////////////////////

	OlsrHelper olsr;
	olsr.Set("HelloInterval", TimeValue(Seconds(helloInterval)));
	olsr.Set("TcInterval", TimeValue(Seconds(TCInterval)));
	Ipv4ListRoutingHelper list;
	list.Add (olsr, 10);
	InternetStackHelper internet;
	internet.SetRoutingHelper (list); // has effect on the next Install ()
	internet.Install (c);
	Ipv4AddressHelper ipv4;
	NS_LOG_INFO ("Assign IP Addresses.");
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);

	/////////////////////////////////////////////////////////////////////////////////////////////

	uint16_t sinkPort = 6; // use the same for all apps

	// UDP connection from N0 to N24

	Address sinkAddress1 (InetSocketAddress (ifcont.GetAddress (24), sinkPort)); // interface of n24
	PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
	ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (24)); //n24 as sink
	sinkApps1.Start (Seconds (0.));
	sinkApps1.Stop (Seconds (100.));
	Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c.Get (0),
	UdpSocketFactory::GetTypeId ()); //source at n0


	// Create UDP application at N0


	UdpClientHelper client (sinkAddress1, sinkPort);
	client.SetAttribute ("MaxPackets", UintegerValue (numPackets));
	client.SetAttribute ("Interval", TimeValue (Seconds (interval)));
	client.SetAttribute ("PacketSize", UintegerValue (packetSize));
	ApplicationContainer apps ;
	apps = client.Install (c.Get (0));
	apps.Start (Seconds (10.0));
	apps.Stop (Seconds (100.0));

	if (numFlows >= 2) {
		NS_LOG_UNCOND("Creating 2nd flow...");
		// UDP connection from N0 to N24

		Address sinkAddress2 (InetSocketAddress (ifcont.GetAddress (4), sinkPort)); // interface of n24
		PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
		ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (4)); //n24 as sink
		sinkApps2.Start (Seconds (0.));
		sinkApps2.Stop (Seconds (100.));
		Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c.Get (20), UdpSocketFactory::GetTypeId ()); //source at n0


		// Create UDP application at N0


		UdpClientHelper client2 (sinkAddress2, sinkPort);
		client2.SetAttribute ("MaxPackets", UintegerValue (numPackets));
		client2.SetAttribute ("Interval", TimeValue (Seconds (interval)));
		client2.SetAttribute ("PacketSize", UintegerValue (packetSize));
		ApplicationContainer apps2;
		apps2 = client.Install (c.Get (20));
		apps2.Start (Seconds (10.0));
		apps2.Stop (Seconds (100.0));
	}
	if (numFlows >= 3) {
		// UDP connection from N0 to N24
		NS_LOG_UNCOND("Creating 3rd flow...");

		Address sinkAddress3 (InetSocketAddress (ifcont.GetAddress (14), sinkPort)); // interface of n24
		PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
		ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (14)); //n24 as sink
		sinkApps3.Start (Seconds (0.));
		sinkApps3.Stop (Seconds (100.));
		Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c.Get (10),
		UdpSocketFactory::GetTypeId ()); //source at n0


		// Create UDP application at N0


		UdpClientHelper client3 (sinkAddress3, sinkPort);
		client3.SetAttribute ("MaxPackets", UintegerValue (numPackets));
		client3.SetAttribute ("Interval", TimeValue (Seconds (interval)));
		client3.SetAttribute ("PacketSize", UintegerValue (packetSize));
		ApplicationContainer apps3;
		apps3 = client.Install (c.Get (10));
		apps3.Start (Seconds (10.0));
		apps3.Stop (Seconds (100.0));
	}

	////////////////////////////////////// Install FlowMonitor on all nodes /////////////////////////////////////////////

	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();
	// Trace Collisions

	Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));

	Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));

	Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

	std::string str = "/NodeList/" + std::to_string(24) + "/ApplicationList/0/$ns3::PacketSink/Rx";
  	Config::ConnectWithoutContext(str, MakeCallback( & PacketSinkTraceSink));

	//Simulator::Schedule(Seconds(5.0), &PrintDrop);
	Simulator::Stop (Seconds (100.0));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
 }
