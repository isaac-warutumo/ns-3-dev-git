#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/flow-monitor-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink.h"
#include "ns3/simulator.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-module.h"
#include "ns3/mptcp-socket-base.h"
#include <ns3/mobility-module.h>
#include "ns3/enum.h"
#include "ns3/config-store-module.h"















using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("MpTcpTestingExample2");

int
main(int argc, char *argv[])
{


Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
//Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
//Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagerMode", EnumValue (MpTcpSocketBase::FullMesh));
  Config::SetDefault ("ns::MpTcpNdiffPorts::MaxSubflows", UintegerValue (2));
//Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(4)); // Sink
//Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("RTT_Compensator"));
//Config::SetDefault("ns3::MpTcpSocketBase::PathManagement", StringValue("NdiffPorts"));

// NodeContainer nodes;
// nodes.Create(2);
/* Build nodes. */
NodeContainer term_0;
term_0.Create(1);
NodeContainer term_1;
term_1.Create(1);

 /*MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(term_0);
  mobility.Install(term_1);
*/
// PointToPointHelper pointToPoint;
// pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
// pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));
/* Build link. */
PointToPointHelper p2p_p2p_0;
p2p_p2p_0.SetDeviceAttribute("DataRate", DataRateValue(100000000));
p2p_p2p_0.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
PointToPointHelper p2p_p2p_1;
p2p_p2p_1.SetDeviceAttribute("DataRate", DataRateValue(100000000));
p2p_p2p_1.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

//NetDeviceContainer devices;
//devices = pointToPoint.Install(nodes);

/* Build link net device container. */
NodeContainer all_p2p_0;
all_p2p_0.Add(term_0);
all_p2p_0.Add(term_1);
NetDeviceContainer ndc_p2p_0 = p2p_p2p_0.Install(all_p2p_0);
NodeContainer all_p2p_1;
all_p2p_1.Add(term_0);
all_p2p_1.Add(term_1);
NetDeviceContainer ndc_p2p_1 = p2p_p2p_1.Install(all_p2p_1);
// InternetStackHelper internet;
// internet.Install(nodes);
/* Install the IP stack. */
InternetStackHelper internetStackH;
internetStackH.Install(term_0);
internetStackH.Install(term_1);

//Ipv4AddressHelper ipv4;
//ipv4.SetBase("10.1.1.0", "255.255.255.0");
//Ipv4InterfaceContainer i = ipv4.Assign(devices);
/* IP assign. */
Ipv4AddressHelper ipv4;
ipv4.SetBase("10.0.0.0", "255.255.255.0");
Ipv4InterfaceContainer iface_ndc_p2p_0 = ipv4.Assign(ndc_p2p_0);
ipv4.SetBase("10.0.1.0", "255.255.255.0");
Ipv4InterfaceContainer iface_ndc_p2p_1 = ipv4.Assign(ndc_p2p_1);\

 Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

uint16_t port = 9;
PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
ApplicationContainer sinkApps = sink.Install(term_1.Get(0));
sinkApps.Start(Seconds(0.0));
sinkApps.Stop(Seconds(100.0));

BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(iface_ndc_p2p_0.GetAddress(0)), port));
source.SetAttribute("MaxBytes", UintegerValue(512));
ApplicationContainer sourceApps = source.Install(term_0.Get(0));
sourceApps.Start(Seconds(0.0));
sourceApps.Stop(Seconds(100.0));



NS_LOG_INFO ("Run Simulation.");
Simulator::Stop(Seconds(200.0));

AnimationInterface anim ("mptcp.xml");
 
anim.SetMobilityPollInterval(Seconds(1.00));
anim.SetMaxPktsPerTraceFile (100000000000);
Simulator::Run();
Simulator::Destroy();
NS_LOG_INFO ("Done.");

}
