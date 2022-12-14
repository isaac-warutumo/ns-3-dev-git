#include <ns3/core-module.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
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
#include <ns3/lte-module.h>
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

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
  //Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (536));
 
  //Enable MPTCP 
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagerMode", EnumValue (MpTcpSocketBase::FullMesh));
  Config::SetDefault ("ns3::MpTcpNdiffPorts::MaxSubflows", UintegerValue (2));
 
  //Enable LTE
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
  
 Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
 
//Variables Declaration
   uint16_t port = 999;
   uint32_t maxBytes = 1048576;
   //uint32_t sentPackets = 0;
   //uint32_t receivedPackets = 0;
   //uint32_t lostPackets = 0;
    

 
//Initialize Internet Stack and Routing Protocols
		
	InternetStackHelper internet;
	Ipv4AddressHelper ipv4;

//creating routers, source and destination. Installing internet stack
 	
  NodeContainer host;				// NodeContainer for source and destination	
		host.Create (2);
		internet.Install (host);		
  NodeContainer enb;
                enb.Create (1);
                internet.Install (enb);
  NodeContainer relay;				// NodeContainer for relay	
		relay.Create (3);
		internet.Install (relay);  
				            
                
  MobilityHelper mobility;
 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
 
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
  mobility.Install (relay);
 
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enb);
  mobility.Install (host);
/////////////////////creating toplogy////////////////

//connecting routers and hosts and assign ip addresses

  NodeContainer e0r0 = NodeContainer (enb.Get (0), relay.Get (0));
  NodeContainer e0r1 = NodeContainer (enb.Get (0), relay.Get (1));
  NodeContainer e0r2 = NodeContainer (enb.Get (0), relay.Get (2));
  //NodeContainer e0h1 = NodeContainer (enb.Get (0), host.Get (1));
  NodeContainer e0h0 = NodeContainer (enb.Get (0), host.Get (0));                 
  
  NodeContainer r0h1 = NodeContainer (relay.Get (0), host.Get (1));
  NodeContainer r1h1 = NodeContainer (relay.Get (1), host.Get (1));
  NodeContainer r2h1 = NodeContainer (host.Get (1), relay.Get (2));
 
  NS_LOG_INFO ("Create channels.");
  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
 
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
 
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
               
  NetDeviceContainer l0e0r0 = wifi.Install (phy, mac, e0r0);
  NetDeviceContainer l1e0r1 = wifi.Install (phy, mac, e0r1); 
  NetDeviceContainer l2e0r2 = wifi.Install (phy, mac, e0r2);           
  
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconGeneration", BooleanValue (true));             
  
  NetDeviceContainer l4e0h0 = wifi.Install (phy, mac, e0h0);
  NetDeviceContainer l5r0h1 = wifi.Install (phy, mac, r0h1);
  NetDeviceContainer l6r1h1 = wifi.Install (phy, mac, r1h1);
  NetDeviceContainer l7r2h1 = wifi.Install (phy, mac, r2h1);
  
  //Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0e0r0 = ipv4.Assign (l0e0r0);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1e0r1 = ipv4.Assign (l1e0r1);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2e0r2 = ipv4.Assign (l2e0r2);

  //ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  //Ipv4InterfaceContainer i3e0h1 = ipv4.Assign (l3e0h1);
  
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  
  
  Ipv4InterfaceContainer i4e0h0 = ipv4.Assign (l4e0h0);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i5r0h1 = ipv4.Assign (l5r0h1);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer i6r1h1 = ipv4.Assign (l6r1h1);
  
  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer i7r2h1 = ipv4.Assign (l7r2h1);

//Attach the UEs to an eNB. This will configure each UE according to the eNB configuration, and create an RRC connection between them:

  /*lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  */

  //Create router nodes, initialize routing database and set up the routing
  //tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Create applications.");
   

  //Create OnOff Application

  //uint16_t port = 1500;
      BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address("10.1.8.1"), port));
      source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
      
      ApplicationContainer sourceApps;
      for (uint32_t i = 0; i < 1; i++)
        {
          sourceApps.Add(source.Install(host.Get(0)));
        }

      sourceApps.Start(NanoSeconds(2.0));
      sourceApps.Stop(Seconds(100.0));

      PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
      ApplicationContainer sinkApps = sink.Install(host.Get(1));
      sinkApps.Start(NanoSeconds(1.0));
      sinkApps.Stop(Seconds(100.0));  
  
                                      
  //=========== Start the simulation ===========//

	std::cout << "Start Simulation.. "<<"\n";

  ////////////////////////// Use of NetAnim model/////////////////////////////////////////

 AsciiTraceHelper ascii;
  phy.EnableAsciiAll (ascii.CreateFileStream ("mptcpphy-1048576.tr"));
 
 
   AnimationInterface anim ("mptcpphy-1048576.xml");
 
  //NodeContainer for spine switches
	  	
		anim.SetConstantPosition(relay.Get(0), 0.0, 20.0);
		anim.SetConstantPosition(relay.Get(1), 25.0, 20.0);
		anim.SetConstantPosition(relay.Get(2), 75.0, 20.0);
		anim.SetConstantPosition(host.Get(0), 75.0, 0.0);
		anim.SetConstantPosition(enb.Get(0), 50.0, 0.0);
		anim.SetConstantPosition(host.Get(1), 50.0 , 100.0);  
	       
  /////////////////////////////////////////////////////////////////////////////////////////////
  //Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributesmptcpphy-1048576.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll (); 
  
phy.EnablePcapAll ("phyCapmptcp-1048576");



        NS_LOG_INFO ("Run Simulation.");
  	Simulator::Stop (Seconds(100.0));
  	Simulator::Run ();

    

        NS_LOG_INFO ("Done.");
      
	std::cout << "Simulation finished "<<"\n";
       
  	Simulator::Destroy ();
 monitor->SerializeToXmlFile ("mptcpphy-1048576flow.xml", true, true); 	
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
  
}

