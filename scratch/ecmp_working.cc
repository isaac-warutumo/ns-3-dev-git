#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("EcmpExample");

int
main (int argc, char *argv[])
{

  uint32_t socket = 0;

  //input parameters
  int no_relays = 2; //number of relays (2,10)
  int relay_capacity = 10; //relay capacity range(2,50) Mbps
  int node_mobility = 5; //relay speeds range(0,5) m/s
  int relay_distance = 5; //range (5,50)m
  int tbr = 5; //range (5,15)%
  int rx_buffer_size = 25; //range (25,75)%

  // Allow the user to override any of the defaults and the above
  // Bind ()s at run-time, via command-line arguments
  CommandLine cmd;

  cmd.AddValue ("Socket", "Socket: (0 UDP, 1 TCP)", socket);
  cmd.Parse (argc, argv);

  //configure defaults
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  Config::SetDefault ("ns3::BulkSendApplication::SendSize", UintegerValue (512));
  
  //Enable MPTCP
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault ("ns3::MpTcpSocketBase::PathManagerMode",
                      EnumValue (MpTcpSocketBase::FullMesh));

  //display the input parameters
  cout << "No of Relays: " << no_relays << endl;
  cout << "Capacity of each Relay: " << relay_capacity << endl;
  cout << "Relay's Mobility: " << node_mobility << endl;
  cout << "Relay-Host Distance: " << relay_distance << endl;
  cout << "Target Bit Rate: " << tbr << endl;
  cout << "Relay Rx buffer size: " << rx_buffer_size << endl;

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (8);

  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n0n2 = NodeContainer (c.Get (0), c.Get (2));
  NodeContainer n0n3 = NodeContainer (c.Get (0), c.Get (3));
  NodeContainer n0n4 = NodeContainer (c.Get (0), c.Get (4));

  NodeContainer n1n5 = NodeContainer (c.Get (1), c.Get (5));
  NodeContainer n1n6 = NodeContainer (c.Get (1), c.Get (6));

  NodeContainer n2n5 = NodeContainer (c.Get (2), c.Get (5));
  NodeContainer n2n6 = NodeContainer (c.Get (2), c.Get (6));

  NodeContainer n3n5 = NodeContainer (c.Get (3), c.Get (5));
  NodeContainer n3n6 = NodeContainer (c.Get (3), c.Get (6));

  NodeContainer n4n5 = NodeContainer (c.Get (4), c.Get (5));
  NodeContainer n4n6 = NodeContainer (c.Get (4), c.Get (6));

  NodeContainer n5n7 = NodeContainer (c.Get (5), c.Get (7));
  NodeContainer n6n7 = NodeContainer (c.Get (6), c.Get (7));

  //int x = 0;
  int y = 0;
  for (uint32_t i = 0; i < c.GetN (); i++)
    {
      Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
      c.Get (i)->AggregateObject (loc);

      if (i == 0)
        loc->SetPosition (Vector (1, 5, 0));
      else if (i == 1 || i == 2 || i == 3 || i == 4)
        {
          y += 2;
          loc->SetPosition (Vector (10, y, 0));
        }
      else if (i == 5 || i == 6)
        {
          y -= 2;
          loc->SetPosition (Vector (20, y, 0));
        }
      else if (i == 7)
        loc->SetPosition (Vector (23, 5, 0));
    }

  InternetStackHelper internet;
  internet.Install (c);

  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer d0d1 = p2p.Install (n0n1);
  NetDeviceContainer d0d2 = p2p.Install (n0n2);
  NetDeviceContainer d0d3 = p2p.Install (n0n3);
  NetDeviceContainer d0d4 = p2p.Install (n0n4);

  NetDeviceContainer d1d5 = p2p.Install (n1n5);
  NetDeviceContainer d1d6 = p2p.Install (n1n6);

  NetDeviceContainer d2d5 = p2p.Install (n2n5);
  NetDeviceContainer d2d6 = p2p.Install (n2n6);

  NetDeviceContainer d3d5 = p2p.Install (n3n5);
  NetDeviceContainer d3d6 = p2p.Install (n3n6);

  NetDeviceContainer d4d5 = p2p.Install (n4n5);
  NetDeviceContainer d4d6 = p2p.Install (n4n6);

  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));

  NetDeviceContainer d5d7 = p2p.Install (n5n7);
  NetDeviceContainer d6d7 = p2p.Install (n6n7);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  ipv4.Assign (d0d1);
  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  ipv4.Assign (d0d2);
  ipv4.SetBase ("10.0.3.0", "255.255.255.0");
  ipv4.Assign (d0d3);
  ipv4.SetBase ("10.0.4.0", "255.255.255.0");
  ipv4.Assign (d0d4);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  ipv4.Assign (d1d5);
  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  ipv4.Assign (d1d6);

  ipv4.SetBase ("10.2.5.0", "255.255.255.0");
  ipv4.Assign (d2d5);
  ipv4.SetBase ("10.2.6.0", "255.255.255.0");
  ipv4.Assign (d2d6);

  ipv4.SetBase ("10.3.5.0", "255.255.255.0");
  ipv4.Assign (d3d5);
  ipv4.SetBase ("10.3.6.0", "255.255.255.0");
  ipv4.Assign (d3d6);

  ipv4.SetBase ("10.4.5.0", "255.255.255.0");
  ipv4.Assign (d4d5);
  ipv4.SetBase ("10.4.6.0", "255.255.255.0");
  ipv4.Assign (d4d6);

  ipv4.SetBase ("10.5.7.0", "255.255.255.0");
  ipv4.Assign (d5d7);
  ipv4.SetBase ("10.6.7.0", "255.255.255.0");
  ipv4.Assign (d6d7);

  NS_LOG_INFO ("Populate routing tables.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create Applications.");
  if (socket == 0)
    {
      uint16_t port = 9; // Discard port (RFC 863)
      OnOffHelper onoff ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address ("10.5.7.2"), port));
      onoff.SetConstantRate (DataRate ("10Mbps"));
      onoff.SetAttribute ("PacketSize", UintegerValue (500));

      ApplicationContainer apps;
      for (uint32_t i = 0; i < 10; i++)
        {
          apps.Add (onoff.Install (c.Get (0)));
        }

      apps.Start (Seconds (0.0));
      apps.Stop (Seconds (1.0));

      PacketSinkHelper sink ("ns3::UdpSocketFactory",
                             Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
      sink.Install (c.Get (7));
    }
  else if (socket == 1)
    {
      uint16_t port = 1500;
      BulkSendHelper source ("ns3::TcpSocketFactory",
                             InetSocketAddress (Ipv4Address ("10.6.7.2"), port));

      ApplicationContainer sourceApps;
      for (uint32_t i = 0; i < 5; i++)
        {
          sourceApps.Add (source.Install (c.Get (0)));
        }

      sourceApps.Start (Seconds (0.0));
      sourceApps.Stop (Seconds (1.0));

      PacketSinkHelper sink ("ns3::TcpSocketFactory",
                             InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApps = sink.Install (c.Get (7));
    }
    else{
      uint16_t port = 1500;
      uint16_t maxBytes =0;//unlimited

      //tcp source
      cout << "TCP: OnOffHelper source - 1 app" << endl;
      OnOffHelper source =
          OnOffHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address ("10.6.7.2"), port));

      //Set the amount of data to send in bytes.  Zero is unlimited.
      source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
      source.SetAttribute ("PacketSize", UintegerValue (500));
      source.SetConstantRate (DataRate ("30.22Mbps"));
      source.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      source.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.01]"));
      //source.SetAttribute ("DataRate", DataRateValue (DataRate ("10000kb/s")));

      ApplicationContainer sourceApps;

     for (uint32_t i = 0; i < 5; i++)
        {
          sourceApps.Add (source.Install (c.Get (0)));
        }

      sourceApps.Start (Seconds (0.0));
      sourceApps.Stop (Seconds (1.0));

      PacketSinkHelper sink ("ns3::TcpSocketFactory",
                             InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApps = sink.Install (c.Get (7));
    }

  // Trace the right-most (second) interface on nodes 2, 3, and 4
  //  p2p.EnablePcap ("ecmp-global-routing", 2, 2);
  //  p2p.EnablePcap ("ecmp-global-routing", 3, 2);
  //  p2p.EnablePcap ("ecmp-global-routing", 4, 2);

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("tracemetrics/ecmp.tr"));

  AnimationInterface anim ("netanim/ecmp.xml");
  anim.SetMaxPktsPerTraceFile (100000000);
  for (uint32_t i = 1; i < 7; i++)
    anim.UpdateNodeColor (c.Get (i), 0, 128, 0);
  // anim.EnablePacketMetadata(true);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Simulation is ended!");
}