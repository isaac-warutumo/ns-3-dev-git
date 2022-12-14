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
#include "ns3/aodv-helper.h"
#include "ns3/enum.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MpTcpTestingExample");


int 
main (int argc, char *argv[])
{
  
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1400));
 
  //Enable MPTCP 
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagerMode", EnumValue (MpTcpSocketBase::FullMesh));
  Config::SetDefault ("ns3::MpTcpNdiffPorts::MaxSubflows", UintegerValue (1));
 
  //Enable LTE
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
  
 Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
 
//Variables Declaration
   uint16_t port = 999;
   //uint32_t maxBytes = 0;
   //uint32_t sentPackets = 0;
   //uint32_t receivedPackets = 0;
   //uint32_t lostPackets = 0;
  std::string phyMode ("DsssRate1Mbps");
//uint32_t PacketSize = 256;
//std::string DataRate ("500Kbps");
//uint16_t num_node = 5;
//uint32_t maxBytes = 0;
 

 
//Initialize Internet Stack and Routing Protocols
		
	InternetStackHelper internet;
	Ipv4AddressHelper ipv4;

//creating routers, source and destination. Installing internet stack
  NodeContainer relay;				// NodeContainer for relay	
		relay.Create (3);
		internet.Install (relay);	
  NodeContainer host;				// NodeContainer for source and destination	
		host.Create (2);
		internet.Install (host);		
  NodeContainer enb;
                enb.Create (1);
                internet.Install (enb);
                
MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(host);
  mobility.Install(relay);
  mobility.Install(enb);
/////////////////////creating toplogy////////////////

//connecting routers and hosts and assign ip addresses

  NodeContainer e0r0 = NodeContainer (enb.Get (0), relay.Get (0));
  NodeContainer e0r1 = NodeContainer (enb.Get (0), relay.Get (1));
  NodeContainer e0r2 = NodeContainer (enb.Get (0), relay.Get (2));
  NodeContainer e0h1 = NodeContainer (enb.Get (0), host.Get (1));
  NodeContainer e0h0 = NodeContainer (enb.Get (0), host.Get (0));                 
  
  NodeContainer r0h1 = NodeContainer (relay.Get (0), host.Get (1));
  NodeContainer r1h1 = NodeContainer (relay.Get (1), host.Get (1));
  NodeContainer r2h1 = NodeContainer (host.Get (1), relay.Get (2));
  
  //Install an LTE protocol stack on the eNB(s) and ue(s)

  NetDeviceContainer enbDevs;
  enbDevs = lteHelper->InstallEnbDevice (enb);
  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice (relay);
  ueDevs = lteHelper->InstallUeDevice (host);
  
   //Attach the UEs to an eNB. This will configure each UE according to the eNB configuration, and create an RRC connection between them:

  lteHelper->Attach (ueDevs, enbDevs.Get (0));
  
  //NodeContainer h1r3 = NodeContainer (host.Get (1), relay.Get (3));
  //NodeContainer r0r1 = NodeContainer (router.Get (0), router.Get (1));
  //We create the channels first without any IP addressing information
  
  NS_LOG_INFO ("Create channels.");
  
  PointToPointHelper p2p;
 
  p2p.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer l0e0r0 = p2p.Install (e0r0);
  NetDeviceContainer l1e0r1 = p2p.Install (e0r1);
  NetDeviceContainer l2e0r2 = p2p.Install (e0r2);
  //NetDeviceContainer l3e0h1 = p2p.Install (e0h1);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("2Gbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer l4e0h0 = p2p.Install (e0h0);
 
 YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
channel.AddPropagationLoss("ns3::FriisPropagationLossModel","MinLoss",DoubleValue(250));


YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
phy.SetChannel(channel.Create());


  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");

  WifiMacHelper mac;

NetDeviceContainer l5r0h1 = wifi.Install (phy, mac, r0h1);
NetDeviceContainer l6r1h1 = wifi.Install (phy, mac, r1h1);
NetDeviceContainer l7r2h1 = wifi.Install (phy, mac, r2h1);

  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (true));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, host);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "BeaconGeneration", BooleanValue (true));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, relay);

  ssid = Ssid ("network-UE1");
  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  l5r0h1 = wifi.Install (phy, mac, host.Get (1));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  l5r0h1 = wifi.Install (phy, mac, relay.Get (0));

  ssid = Ssid ("network-UE2");
  phy.Set ("ChannelNumber", UintegerValue (40));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  l6r1h1 = wifi.Install (phy, mac, host.Get (1));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  l6r1h1 = wifi.Install (phy, mac, relay.Get (1));

  ssid = Ssid ("network-UE3");
  phy.Set ("ChannelNumber", UintegerValue (44));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  l7r2h1 = wifi.Install (phy, mac, host.Get (1));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  l7r2h1 = wifi.Install (phy, mac, relay.Get (2));

  /*ssid = Ssid ("network-UE4");
  phy.Set ("ChannelNumber", UintegerValue (48));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  dev4 = wifi.Install (phy, mac, ueNodes.Get (3));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  dev4 = wifi.Install (phy, mac, enbNodes.Get (0));

  ssid = Ssid ("network-d2d1");
  phy.Set ("ChannelNumber", UintegerValue (52));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

  dev5 = wifi.Install (phy, mac, ueNodes.Get (0));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  dev5 = wifi.Install (phy, mac, ueNodes.Get (1));

  ssid = Ssid ("network-d2d2");
  phy.Set ("ChannelNumber", UintegerValue (56));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  dev6 = wifi.Install (phy, mac, ueNodes.Get (0));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  dev6 = wifi.Install (phy, mac, ueNodes.Get (2));
*/
  

  
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
      source.SetAttribute("MaxBytes", UintegerValue(5000000000000));
      
      ApplicationContainer sourceApps;
      for (uint32_t i = 0; i < 5; i++)
        {
          sourceApps.Add(source.Install(host.Get(0)));
        }

      sourceApps.Start(Seconds(0.0));
      sourceApps.Stop(Seconds(5.0));

      PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
      ApplicationContainer sinkApps = sink.Install(host.Get(1));
      //sinkApps.Start(Seconds(0.0));
      //sinkApps.Stop(Seconds(1.0));
      
  
  
                                        
  //=========== Start the simulation ===========//

	std::cout << "Start Simulation.. "<<"\n";

  ////////////////////////// Use of NetAnim model/////////////////////////////////////////

 AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("mptcp-example2+lte-wifi1.tr"));
 
 
   AnimationInterface anim ("mptcp-example-default2+lte-wifi1.xml");
 
  //NodeContainer for spine switches
	  	
		anim.SetConstantPosition(relay.Get(0), 0.0, 20.0);
		anim.SetConstantPosition(relay.Get(1), 25.0, 20.0);
		anim.SetConstantPosition(relay.Get(2), 75.0, 20.0);
		anim.SetConstantPosition(host.Get(0), 75.0, 0.0);
		anim.SetConstantPosition(enb.Get(0), 50.0, 0.0);
		anim.SetConstantPosition(host.Get(1), 50.0 , 100.0);  
	       
  /////////////////////////////////////////////////////////////////////////////////////////////
  //Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll (); 
  
p2p.EnablePcapAll ("p2pCapwifi");



        NS_LOG_INFO ("Run Simulation.");
  	Simulator::Stop (Seconds(5.0));
  	Simulator::Run ();

    

        NS_LOG_INFO ("Done.");
      
	std::cout << "Simulation finished "<<"\n";
       
  	Simulator::Destroy ();
 monitor->SerializeToXmlFile ("example-lte-wifi1.xml", true, true); 	

  
}




