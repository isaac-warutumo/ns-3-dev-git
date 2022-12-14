#include <fstream>
#include <ns3/core-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-helper.h>
#include <ns3/epc-helper.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/flow-classifier.h>
#include <ns3/netanim-module.h>
#include <ns3/buildings-helper.h>
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include <iostream>
#include <string>
#include <cassert>
#include "ns3/flow-monitor-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/simulator.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-module.h"
#include "ns3/mptcp-socket-base.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("usercooperationproject5");

Ptr<PacketSink> sink;
int
main (int argc, char *argv[])
{

  double lat = 2.0;
  uint64_t rate = 5000000;
  double interval = 0.05;
  double simulation_time = 10.0;
  bool verbose = true;
  uint32_t nWifi = 4;

  CommandLine cmd;
  cmd.AddValue ("latency", "P2P link latency in milliseconds", lat);
  cmd.AddValue ("rate", "P2P data rate in bps", rate);
  cmd.AddValue ("interval", "UDP client packet interval", interval);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  NS_LOG_INFO ("Create nodes.");
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (4);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY",
                                 DoubleValue (0.0), "DeltaX", DoubleValue (5.0), "DeltaY",
                                 DoubleValue (10.0), "GridWidth", UintegerValue (3), "LayoutType",
                                 StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds",
                             RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (ueNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> position_enb = CreateObject<ListPositionAllocator> ();
  position_enb->Add (Vector (5, -20, 0));
  mobility.SetPositionAllocator (position_enb);
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevOne;
  NetDeviceContainer ueDevTwo;
  NetDeviceContainer ueDevThree;
  NetDeviceContainer ueDevFour;

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevOne = lteHelper->InstallUeDevice (ueNodes.Get (0));
  ueDevTwo = lteHelper->InstallUeDevice (ueNodes.Get (1));
  ueDevThree = lteHelper->InstallUeDevice (ueNodes.Get (2));
  ueDevFour = lteHelper->InstallUeDevice (ueNodes.Get (3));

  NS_LOG_INFO ("Create Channels");

YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
channel.AddPropagationLoss("ns3::FriisPropagationLossModel","MinLoss",DoubleValue(250));


YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();;
phy.SetChannel(channel.Create());


  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");

  WifiMacHelper mac;

  NetDeviceContainer dev, dev1, dev2, dev3, dev4, dev5, dev6;

  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, ueNodes);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "BeaconGeneration", BooleanValue (false));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, enbNodes);

  ssid = Ssid ("network-UE1");
  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  dev = wifi.Install (phy, mac, ueNodes.Get (0));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  dev = wifi.Install (phy, mac, enbNodes.Get (0));

  ssid = Ssid ("network-UE2");
  phy.Set ("ChannelNumber", UintegerValue (40));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  dev2 = wifi.Install (phy, mac, ueNodes.Get (1));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  dev2 = wifi.Install (phy, mac, enbNodes.Get (0));

  ssid = Ssid ("network-UE3");
  phy.Set ("ChannelNumber", UintegerValue (44));
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  dev3 = wifi.Install (phy, mac, ueNodes.Get (2));

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (false));
  dev3 = wifi.Install (phy, mac, enbNodes.Get (0));

  ssid = Ssid ("network-UE4");
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

  InternetStackHelper internet;
  internet.Install (ueNodes);
  internet.Install (enbNodes);
  //internet.Install (wifiApNode);

  Ipv4AddressHelper ipv4;

  NS_LOG_INFO ("Assign IP Addresses.");

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (dev);
  Ipv4InterfaceContainer i2;
  i2 = ipv4.Assign (dev2);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
 

 ipv4.SetBase ("10.1.3.0", "255.255.255.0");
 Ipv4InterfaceContainer i3 = ipv4.Assign (dev3);

 ipv4.SetBase ("10.1.4.0", "255.255.255.0");
 Ipv4InterfaceContainer i4 = ipv4.Assign (dev4);

 ipv4.SetBase ("10.1.5.0", "255.255.255.0");
 Ipv4InterfaceContainer i5= ipv4.Assign (dev5);
 
 ipv4.SetBase ("10.1.6.0", "255.255.255.0");
 Ipv4InterfaceContainer i6= ipv4.Assign (dev6);


  lteHelper->Attach (ueDevOne, enbDevs.Get (0));
  lteHelper->Attach (ueDevTwo, enbDevs.Get (0));
  lteHelper->Attach (ueDevThree, enbDevs.Get (0));
  lteHelper->Attach (ueDevFour, enbDevs.Get (0));

  NS_LOG_INFO ("Create applications.");

  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApp = sinkHelper.Install (ueNodes);
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (i.GetAddress (0), 9)));
  server.SetAttribute ("PacketSize", UintegerValue (1000));
  server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute ("DataRate", DataRateValue (DataRate ("500kb/s")));
  ApplicationContainer serverApp = server.Install (enbNodes);

  
  serverApp.Stop (Seconds (10.0));
  sinkApp.Stop (Seconds (10.0));

  
  AnimationInterface anim ("ucoop5.xml");
  anim.SetStartTime (Seconds (1.0));
  anim.SetStopTime (Seconds (simulation_time));
  anim.UpdateNodeColor (enbNodes.Get (0), 0, 0, 255);
  //anim.UpdateNodeColor(enbNodes.Get (0), 0, 0, 255);
  anim.UpdateNodeDescription (enbNodes.Get (0), "eNodeB");
  //anim.UpdateNodeDescription(enbNodes.Get (0), "RemoteHost");
  anim.UpdateNodeColor (ueNodes.Get (0), 0, 255, 0);
  anim.UpdateNodeColor (ueNodes.Get (1), 0, 255, 0);
  anim.UpdateNodeColor (ueNodes.Get (2), 0, 255, 0);
  anim.UpdateNodeColor (ueNodes.Get (3), 0, 255, 0);
  anim.UpdateNodeDescription (ueNodes.Get (0), "UE1");
  anim.UpdateNodeDescription (ueNodes.Get (1), "UE2");
  anim.UpdateNodeDescription (ueNodes.Get (2), "UE3");
  anim.UpdateNodeDescription (ueNodes.Get (3), "UE4");
  anim.EnablePacketMetadata (true);
  anim.EnableIpv4RouteTracking ("ucoop5_1.xml", Seconds (0), Seconds (10.0), Seconds (0.5));

  AsciiTraceHelper ascii;
  phy.EnableAscii (ascii.CreateFileStream ("ucoop5.tr"), enbDevs);
  phy.EnablePcap ("ucoop5", enbDevs, false);

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (simulation_time));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> "
                << t.destinationAddress << ")\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 8.0 / 1024 / 1024 << " Mbps\n";
      std::cout << "  Tx Packets:   " << i->second.txPackets << "\n";
      std::cout << "  Rx Packets:   " << i->second.rxPackets << "\n";
      std::cout << "  Delay Sum:   " << i->second.delaySum << "\n";
      std::cout << "  Average Delay:   " << i->second.delaySum / i->second.rxPackets << "\n";
    }

  monitor->SerializeToXmlFile ("ucoop5.flowmon", true, true);

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
