#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"


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

#include "ns3/enum.h"
#include "ns3/config-store-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MpTcpBulkSendExample");

/*void PrintSocket(Ptr <MpTcpBulkSendApplication> app)
{
        std::cout << "-------------------------------------------------------------------" << std::endl << std::flush;
        Ptr<MpTcpSocketBase> sock = DynamicCast<MpTcpSocketBase>(app->GetSocket());
        std::cout << sock->subflows.size() << std::endl;
}
*/
int
main(int argc, char *argv[])
{
  LogComponentEnable("MpTcpSocketBase", LOG_INFO);

  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000));
 
  //Enable MPTCP 
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagerMode", EnumValue (MpTcpSocketBase::FullMesh));
  Config::SetDefault ("ns::MpTcpNdiffPorts::MaxSubflows", UintegerValue (2));
  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));

  PointToPointHelper subflow;
  subflow.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  subflow.SetChannelAttribute("Delay", StringValue("1ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);

  NetDeviceContainer subDevices;
  subDevices = subflow.Install(nodes);

  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devices);  
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = ipv4.Assign(subDevices);
  
  uint16_t port = 9;
  uint32_t maxBytes = 0;
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  OnOffHelper oo = OnOffHelper ("ns3::TcpSocketFactory",
                         InetSocketAddress (i.GetAddress (0), port)); //link from router1 to host1
  // Set the amount of data to send in bytes.  Zero is unlimited.
  oo.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  oo.SetAttribute ("PacketSize", UintegerValue (1000));
  oo.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.02]"));
  oo.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.01]"));
  oo.SetAttribute ("DataRate", DataRateValue (DataRate ("500kb/s")));

  ApplicationContainer SourceApp = oo.Install (nodes.Get (0));
  SourceApp.Start (Seconds (0.1));
  SourceApp.Stop (Seconds (5.0));
  
  // Create a packet sink to receive packets.
  // 
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                        		 InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer SinkApp = packetSinkHelper.Install (nodes.Get(1));
	
  SinkApp.Start (Seconds (0.4));
  SinkApp.Stop (Seconds (6.0));  
  
  AnimationInterface anim ("mptcpsubflow-example.xml");
 
// NodeContainer for spine switches
	  	
		anim.SetConstantPosition(nodes.Get(0), 2.0, 0.0);
		anim.SetConstantPosition(nodes.Get(1), 20.0, 0.0);
		//anim.SetConstantPosition(host.Get(0), 2.0, 20.0);
		//anim.SetConstantPosition(host.Get(1), 20.0 , 20.0); 
  
  

  /*uint16_t port = 9;
  MpTcpPacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApps = sink.Install(nodes.Get(1));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));

  MpTcpBulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(i.GetAddress(1)), port));
  source.SetAttribute("MaxBytes", UintegerValue(0));
  ApplicationContainer sourceApps = source.Install(nodes.Get(0));
//  Ptr<MpTcpBulkSendApplication> app = DynamicCast<MpTcpBulkSendApplication>(sourceApps.Get(0));
//  Ptr<MpTcpSocketBase> socket = DynamicCast<MpTcpSocketBase>(app->GetSocket());
//  std::cout << socket->subflows.len
  sourceApps.Start(Seconds(0.0));
  sourceApps.Stop(Seconds(10.0));
  */

  NS_LOG_INFO ("Run Simulation.");
  //Simulator::Schedule(Seconds(9), &PrintSocket, DynamicCast<MpTcpBulkSendApplication>(sourceApps.Get(0)));
  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO ("Done.");

}
