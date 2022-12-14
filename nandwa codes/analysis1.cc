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
   uint32_t maxBytes = 0;
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
                
MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(host);
 
/////////////////////creating toplogy////////////////

//connecting routers and hosts and assign ip addresses

  NodeContainer h0h1 = NodeContainer (host.Get (0), host.Get (1));
  //NodeContainer e0r1 = NodeContainer (enb.Get (0), relay.Get (1));
  
  NS_LOG_INFO ("Create channels.");
  
  PointToPointHelper p2p;
 
  p2p.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer l0h0h1 = p2p.Install (host);
  
  
  //Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0e0r0 = ipv4.Assign (l0h0h1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Create applications.");
   

  //Create OnOff Application

  //uint16_t port = 1500;
      BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address("10.1.1.1"), port));
      source.SetAttribute("MaxBytes", UintegerValue(maxBytes)); //50B
      
      ApplicationContainer sourceApps;
      for (uint32_t i = 0; i < 1; i++)
        {
          sourceApps.Add(source.Install(host.Get(0)));
        }

      sourceApps.Start(Seconds(0.0));
      sourceApps.Stop(Seconds(10.0));

      PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
      ApplicationContainer sinkApps = sink.Install(host.Get(1));
      sinkApps.Start(Seconds(0.0));
      sinkApps.Stop(Seconds(10.0));
      
  
  
                                        
  //=========== Start the simulation ===========//

	std::cout << "Start Simulation.. "<<"\n";

  ////////////////////////// Use of NetAnim model/////////////////////////////////////////

 AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("analysis.tr"));
 
 
   AnimationInterface anim ("analysis1.xml");
 
  //NodeContainer for spine switches
	  	
		anim.SetConstantPosition(host.Get(0), 0.0, 20.0);
		//anim.SetConstantPosition(relay.Get(1), 25.0, 20.0);
		anim.SetConstantPosition(host.Get(1), 75.0, 20.0);
		//anim.SetConstantPosition(host.Get(0), 75.0, 0.0);
		//anim.SetConstantPosition(enb.Get(0), 50.0, 0.0);
		//anim.SetConstantPosition(host.Get(1), 50.0 , 100.0);  
	       
  /////////////////////////////////////////////////////////////////////////////////////////////
  //Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes-analysis1.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll (); 
  
p2p.EnablePcapAll ("p2pCap-analysis1");



        NS_LOG_INFO ("Run Simulation.");
  	Simulator::Stop (Seconds(10.0));
  	Simulator::Run ();
  
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";           std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 8.0 / 1024 / 1024  << " Mbps\n";
      std::cout << "  Tx Packets:   " << i->second.txPackets << "\n";
      std::cout << "  Rx Packets:   " << i->second.rxPackets << "\n";
      std::cout << "  Delay Sum:   " << i->second.delaySum << "\n";
      std::cout << "  Average Delay:   " << i->second.delaySum / i->second.rxPackets<< "\n";

    }

    

        NS_LOG_INFO ("Done.");
      
	std::cout << "Simulation finished "<<"\n";
       
  	Simulator::Destroy ();
 monitor->SerializeToXmlFile ("analysis-mon.xml", true, true); 	

Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
	return 0;
  
}

