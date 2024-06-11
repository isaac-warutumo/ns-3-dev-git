#include <ns3/core-module.h>
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
#include <ns3/lte-module.h>

#include "ns3/enum.h"
#include "ns3/config-store-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MpTcpExample");

int
main (int argc, char *argv[])
{

  LogComponentEnable ("MpTcpExample", LOG_LEVEL_INFO);

  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1400));

  //Enable MPTCP
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault ("ns3::MpTcpSocketBase::PathManagerMode",
                      EnumValue (MpTcpSocketBase::FullMesh));
  // Config::SetDefault ("ns3::MpTcpNdiffPorts::MaxSubflows", UintegerValue (1));

  //Enable LTE
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  //Variables Declaration
  uint16_t port = 999;
  uint32_t maxBytes = 10000;
  //uint32_t sentPackets = 0;
  //uint32_t receivedPackets = 0;
  //uint32_t lostPackets = 0;

  //Initialize Internet Stack and Routing Protocols

  InternetStackHelper internet;
  Ipv4AddressHelper ipv4;

  //creating routers, source and destination. Installing internet stack
  NodeContainer relay; // NodeContainer for relay
  relay.Create (3);
  internet.Install (relay);
  NodeContainer nc_host; // NodeContainer for source and destination
  nc_host.Create (2);
  internet.Install (nc_host);
  NodeContainer enb;
  enb.Create (1);
  internet.Install (enb);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nc_host);
  mobility.Install (relay);
  mobility.Install (enb);
  /////////////////////creating toplogy////////////////

  //connecting routers and hosts and assign ip addresses

  NodeContainer e0r0 = NodeContainer (enb.Get (0), relay.Get (0));
  NodeContainer e0r1 = NodeContainer (enb.Get (0), relay.Get (1));
  NodeContainer e0r2 = NodeContainer (enb.Get (0), relay.Get (2));
  NodeContainer e0h1 = NodeContainer (enb.Get (0), nc_host.Get (1));
  NodeContainer e0h0 = NodeContainer (enb.Get (0), nc_host.Get (0));

  NodeContainer r0h1 = NodeContainer (relay.Get (0), nc_host.Get (1));
  NodeContainer r1h1 = NodeContainer (relay.Get (1), nc_host.Get (1));
  NodeContainer r2h1 = NodeContainer (nc_host.Get (1), relay.Get (2));

  //Install an LTE protocol stack on the eNB(s) and ue(s)

  NetDeviceContainer enbDevs;
  enbDevs = lteHelper->InstallEnbDevice (enb);
  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice (relay);
  ueDevs = lteHelper->InstallUeDevice (nc_host);

  //Attach the UEs to an eNB. This will configure each UE according to the eNB configuration, and create an RRC connection between them:

  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  //NodeContainer h1r3 = NodeContainer (host.Get (1), relay.Get (3));
  //NodeContainer r0r1 = NodeContainer (router.Get (0), router.Get (1));
  //We create the channels first without any IP addressing information

  NS_LOG_INFO ("Create channels.");

  PointToPointHelper p2p;

  p2p.SetDeviceAttribute ("DataRate", StringValue ("Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ns"));
  NetDeviceContainer l0e0r0 = p2p.Install (e0r0);
  NetDeviceContainer l1e0r1 = p2p.Install (e0r1);
  NetDeviceContainer l2e0r2 = p2p.Install (e0r2);
  //NetDeviceContainer l3e0h1 = p2p.Install (e0h1);
 
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  NetDeviceContainer l4e0h0 = p2p.Install (e0h0);

  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ns"));

  NetDeviceContainer l5r0h1 = p2p.Install (r0h1);
  NetDeviceContainer l6r1h1 = p2p.Install (r1h1);
  NetDeviceContainer l7r2h1 = p2p.Install (r2h1);
  //NetDeviceContainer l7h1r3 = p2p.Install (h1r3);

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

  int source = 0;
  //Create OnOff Application
  if (source == 0)
    {

      OnOffHelper onOffSource = OnOffHelper (
          "ns3::TcpSocketFactory",
          InetSocketAddress (i7r2h1.GetAddress (0), port)); //link from router1 to host1

      //Set the amount of data to send in bytes.  Zero is unlimited.
      onOffSource.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
      onOffSource.SetAttribute ("PacketSize", UintegerValue (1000));

      onOffSource.SetConstantRate (DataRate ("10Mbps"));

      ApplicationContainer SourceApp = onOffSource.Install (nc_host.Get (0));
      SourceApp.Start (NanoSeconds (0.0));
      SourceApp.Stop (Seconds (10.0));

      //Create a packet sink to receive packets.

      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                                         InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer SinkApp = packetSinkHelper.Install (nc_host.Get (1));

      SinkApp.Start (NanoSeconds (0.0));
      SinkApp.Stop (Seconds (10.0));

      // Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (SinkApp.Get (0));
      // std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
    }
  else //echoserver
    {

      UdpEchoServerHelper echoServer (9);

      ApplicationContainer serverApps = echoServer.Install (nc_host.Get (1));
      serverApps.Start (NanoSeconds (1.0));
      serverApps.Stop (Seconds (10.0));

      UdpEchoClientHelper echoClient (i7r2h1.GetAddress (0), 9);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
      echoClient.SetAttribute ("Interval", TimeValue (NanoSeconds (.01)));
      echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

      ApplicationContainer clientApps = echoClient.Install (nc_host.Get (0));
      clientApps.Start (NanoSeconds (2.0));
      clientApps.Stop (Seconds (10.0));
      //       Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (serverApps.Get (0));
      // std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
    }

  //=========== Start the simulation ===========//

  std::cout << "Start Simulation.. "
            << "\n";

  ////////////////////////// Use of NetAnim model/////////////////////////////////////////
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("tracemetrics/mptcp-example.tr"));

  AnimationInterface anim ("netanim/mptcp-example.xml");

  //NodeContainer for spine switches

  anim.SetConstantPosition (relay.Get (0), 0.0, 20.0);
  anim.SetConstantPosition (relay.Get (1), 25.0, 20.0);
  anim.SetConstantPosition (relay.Get (2), 75.0, 20.0);
  anim.SetConstantPosition (nc_host.Get (0), 75.0, 0.0);
  anim.SetConstantPosition (enb.Get (0), 50.0, 0.0);
  anim.SetConstantPosition (nc_host.Get (1), 50.0, 100.0);

  p2p.EnablePcapAll ("pcap/mptcp-example-p2p");

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();

  NS_LOG_INFO ("Done.");

  std::cout << "Simulation finished "
            << "\n";

  Simulator::Destroy ();

  return 0;
}
