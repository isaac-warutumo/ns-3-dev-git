#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink.h"
#include "ns3/simulator.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-module.h"
#include "ns3/mptcp-socket-base.h"

#include "ns3/enum.h"
#include "ns3/config-store-module.h"


using namespace ns3;

int main(int argc, char** argv)
{
NS_LOG_COMPONENT_DEFINE ("WifiAdhoc_Routing");
std::string phyMode ("DsssRate1Mbps");
//uint32_t PacketSize = 256;
std::string DataRate ("500Kbps");
uint16_t num_node = 5;
uint32_t maxBytes = 0;

NodeContainer node;
node.Create (num_node);

MobilityHelper mobility;
mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
"MinX", DoubleValue (0.0),
"MinY", DoubleValue (0.0),
"DeltaX", DoubleValue (1000),
"DeltaY", DoubleValue (1000),
"GridWidth", UintegerValue (5),
"LayoutType", StringValue ("RowFirst"));
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (node);

WifiHelper wifi;
YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
wifiPhy.Set ("RxGain", DoubleValue (-12));
wifiPhy.SetPcapDataLinkType
(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
YansWifiChannelHelper wifiChannel;
wifiChannel.SetPropagationDelay
("ns3::ConstantSpeedPropagationDelayModel");
wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
wifiPhy.SetChannel (wifiChannel.Create());

WifiMacHelper wifiMac;
wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
"DataMode", StringValue(phyMode), "ControlMode", StringValue (phyMode));
wifiMac.SetType ("ns3::AdhocWifiMac");

NetDeviceContainer Devices;
Devices = wifi.Install (wifiPhy, wifiMac, node);

AodvHelper aodv;
Ipv4StaticRoutingHelper staticRouting;
Ipv4ListRoutingHelper list;
list.Add (staticRouting, 0);
list.Add (aodv, 5);

InternetStackHelper internet;
internet.SetRoutingHelper (list);
internet.Install (node);
Ipv4AddressHelper ipv4;
ipv4.SetBase ("192.168.1.0","255.255.255.0");
Ipv4InterfaceContainer j;
j = ipv4.Assign (Devices);

uint16_t UDPport = 9;

 OnOffHelper oo = OnOffHelper ("ns3::TcpSocketFactory",
                         InetSocketAddress (j.GetAddress (0), UDPport)); //link from router1 to host1
  // Set the amount of data to send in bytes.  Zero is unlimited.
  oo.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  oo.SetAttribute ("PacketSize", UintegerValue (1000));
  oo.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.02]"));
  oo.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.01]"));
  oo.SetAttribute ("DataRate", DataRateValue (DataRate));

  ApplicationContainer SourceApp = oo.Install (node.Get (0));
  SourceApp.Start (Seconds (1.0));
  SourceApp.Stop (Seconds (10.0));
  
  // Create a packet sink to receive packets.
  // 
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                        		 InetSocketAddress (Ipv4Address::GetAny (), UDPport));
  ApplicationContainer SinkApp = packetSinkHelper.Install (node.Get(5));
	
  SinkApp.Start (Seconds (1.0));
  SinkApp.Stop (Seconds (10.0));   


AnimationInterface anim ("5animation.xml");
anim.EnablePacketMetadata(true);

AsciiTraceHelper ascii;
//wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("out.tr"));

FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

Simulator::Stop(Seconds(100.0));
Simulator::Run();


monitor->CheckForLostPackets();
Ptr<Ipv4FlowClassifier> classifier =DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
std::map<FlowId,FlowMonitor::FlowStats>stats=monitor->GetFlowStats();
//std::map<FlowId,FlowMonitor::FlowStats>::const_iterator k = stats.begin();
for(std::map<FlowId,FlowMonitor::FlowStats>::const_iterator j =stats.begin(); j!=stats.end(); j++)
{
Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(j->first);
std::cout<<"----Flow"<<j->first<<"("<<t.sourceAddress<<"-"<<t.destinationAddress<<")\n";
std::cout<<"Tx Bytes:\t\t"<<j->second.txBytes<<"\n";
std::cout<<"Rx Bytes:\t\t"<<j->second.rxBytes<<"\n";
std::cout<<"Tx Packets:\t\t"<<j->second.txPackets<<"\n";
std::cout<<"Rx Packets:\t\t"<<j->second.rxPackets<<"\n";
std::cout<<"Packets Delivery Ratio:\t\t"<<(float)j->second.rxPackets/j->second.txPackets*100<<"\n";
std::cout<<"Throughput:\t\t"<<j->second.rxBytes*8.0/25.0<<"bits/sec\n";
std::cout<<"Average Delay:\t\t"<<j->second.delaySum/j->second.rxPackets<<"\n";
std::cout<<"Average Jitter:\t\t"<<j->second.jitterSum/j->second.rxPackets<<"\n";
std::cout<<"Dropped Packets:\t\t"<<(j->second.txPackets-j->second.rxPackets)<<"\n";
std::cout<<"Dropping Ratio:\t\t"<<(float)(j->second.txPackets, j->second.rxPackets)/j->second.txPackets*100<<"\n";
}

Simulator::Destroy();
return 0;
}
