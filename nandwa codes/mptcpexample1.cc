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

NS_LOG_COMPONENT_DEFINE ("MpTcpTestingExample");


int 
main (int argc, char *argv[])
{
  
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000));
 
  //Enable MPTCP 
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagerMode", EnumValue (MpTcpSocketBase::FullMesh));
  Config::SetDefault ("ns::MpTcpNdiffPorts::MaxSubflows", UintegerValue (2));

 
// Variables Declaration
   uint16_t port = 999;
   uint32_t maxBytes = 512;
 
// Initialize Internet Stack and Routing Protocols
		
	InternetStackHelper internet;
	Ipv4AddressHelper ipv4;

// creating routers, source and destination. Installing internet stack
  NodeContainer router;				// NodeContainer for router	
		router.Create (2);
		internet.Install (router);	
  NodeContainer host;				// NodeContainer for source and destination	
		host.Create (2);
		internet.Install (host);
		
  NodeContainer h0r0 = NodeContainer (host.Get (0), router.Get (0));
  NodeContainer h0r1 = NodeContainer (host.Get (0), router.Get (1));
  
  NodeContainer h1r0 = NodeContainer (host.Get (1), router.Get (0));
  NodeContainer h1r1 = NodeContainer (host.Get (1), router.Get (1));
  
  
  PointToPointHelper p2p;
 
  p2p.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer l0h0r0 = p2p.Install (h0r0);
  NetDeviceContainer l1h0r1 = p2p.Install (h0r1);
  NetDeviceContainer l2h1r0 = p2p.Install (h1r0);
  NetDeviceContainer l3h1r1 = p2p.Install (h1r1);
  
  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0h0r0 = ipv4.Assign (l0h0r0);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1h0r1 = ipv4.Assign (l1h0r1);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2h1r0 = ipv4.Assign (l2h1r0);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3h1r1 = ipv4.Assign (l3h1r1);
  
  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  // Create OnOff Application

 // Create BuldSend Application to send Tcp packets  //

  OnOffHelper oo = OnOffHelper ("ns3::TcpSocketFactory",
                         InetSocketAddress (i0h0r0.GetAddress (0), port)); //link from router1 to host1
  // Set the amount of data to send in bytes.  Zero is unlimited.
  oo.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  oo.SetAttribute ("PacketSize", UintegerValue (1000));
  oo.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.02]"));
  oo.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.01]"));
  oo.SetAttribute ("DataRate", DataRateValue (DataRate ("500kb/s")));

  ApplicationContainer SourceApp = oo.Install (host.Get (0));
  SourceApp.Start (Seconds (1.0));
  SourceApp.Stop (Seconds (20.0));
  
  // Create a packet sink to receive packets.
  // 
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                        		 InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer SinkApp = packetSinkHelper.Install (host.Get(1));
	
  SinkApp.Start (Seconds (2.0));
  SinkApp.Stop (Seconds (20.0));   
  
  //=========== Start the simulation ===========//

	std::cout << "Start Simulation.. "<<"\n";

  ////////////////////////// Use of NetAnim model/////////////////////////////////////////

 
   AnimationInterface anim ("mptcpexample1.xml");
 
// NodeContainer for spine switches
	  	
		anim.SetConstantPosition(router.Get(0), 2.0, 0.0);
		anim.SetConstantPosition(router.Get(1), 20.0, 0.0);
		anim.SetConstantPosition(host.Get(0), 2.0, 20.0);
		anim.SetConstantPosition(host.Get(1), 20.0 , 20.0);  
	       
/////////////////////////////////////////////////////////////////////////////////////////////
       // Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes1.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();


        NS_LOG_INFO ("Run Simulation.");
  	Simulator::Stop (Seconds(20.0));
  	Simulator::Run ();
        NS_LOG_INFO ("Done.");
      
	std::cout << "Simulation finished "<<"\n";
       
  	Simulator::Destroy ();
  	
/*
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (SinkApp.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
	return 0;
	*/
}
		

