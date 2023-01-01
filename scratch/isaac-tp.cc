#include <ns3/core-module.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

//iomanip required for setprecision(3)
#include <iomanip>

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
#include "ns3/isaac-module.h"
#include <vector>
#include <algorithm>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MpTcpTestingExample");

//input parameters
size_t cluster_nodes = 2; //number of relays (2,10)
int relay_capacity = 5; //relay capacity range(2,50) Mbps
int node_mobility = 5; //relay speeds range(0,5) m/s
int relay_distance = 5; //range (5,50)m
int tbr = 15; //range (5,15)%
int rx_buffer_size_pct = 25; //range (25,75)%
int mptcp = 0; // udp=0, tcp=1;
int required_capacity = 50; //10Mbps for latency sensitive and 50Mbps for capacity sensitive expts

//Variables Declaration
uint16_t port = 999;
uint32_t maxBytes = 10000; //1MBs=1048576 //0=unlimited
int *array_relay_capacities;
size_t subset_sum_relays = 0;

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("cluster_nodes", "cluster_nodes", cluster_nodes);
  cmd.AddValue ("relay_capacity", "relay_capacity", relay_capacity);
  cmd.AddValue ("node_mobility", "node_mobility", node_mobility);
  cmd.AddValue ("relay_distance", "relay_distance", relay_distance);
  cmd.AddValue ("tbr", "tbr", tbr);
  cmd.AddValue ("rx_buffer_size", "rx_buffer_size", rx_buffer_size_pct);
  cmd.AddValue ("mptcp", "mptcp", mptcp); // udp=0, tcp=1;
  cmd.AddValue ("required_capacity", "required_capacity", required_capacity);

  cmd.Parse (argc, argv);
  cout << "You added " << argc << " arguments" << endl;
  for (int i = 0; i < argc; i++)
    {
      cout << "Argument " << i << ": " << argv[i] << endl;
    }

  cout << endl;

  //a pointer to array to hold cluster node capacities
  array_relay_capacities = new int[cluster_nodes];
  //create an array with capacity
  for (size_t i = 0; i < cluster_nodes; i++)
    {
      array_relay_capacities[i] = relay_capacity; //remove i
    }

  // The below value configures the default behavior of global routing.
  // By default, it is disabled.  To respond to interface events, set to true

  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  Config::SetDefault ("ns3::BulkSendApplication::SendSize", UintegerValue (1024));

  uint32_t rcv_buffer_size =
      131072 * rx_buffer_size_pct / 100; //change the buffer size to the percentage given

  Config::SetDefault ("ns3::UdpSocket::RcvBufSize", UintegerValue (rcv_buffer_size));

  //Enable MPTCP
  Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue (true));
  Config::SetDefault ("ns3::MpTcpSocketBase::PathManagerMode",
                      EnumValue (MpTcpSocketBase::FullMesh));

  Cluster myCluster (cluster_nodes, array_relay_capacities, relay_capacity);

  int min_required_capacity, max_required_capacity;

  min_required_capacity = (int) floor (required_capacity * (1 - ((double) tbr / 100)));
  max_required_capacity = (int) ceil (required_capacity * (1 + ((double) tbr / 100)));

  // Find the number of subsets with desired Sum
  if (myCluster.findAndPrintSubsets (array_relay_capacities, cluster_nodes, min_required_capacity,
                                     max_required_capacity) > 0)
    cout << "Yes Subsets found!!!" << endl;

  else
    cout << "No Subsets found!!!" << endl;

  //Initialize Internet Stack and Routing Protocols
  InternetStackHelper internet;
  Ipv4AddressHelper ipv4;

  //creating source and destination. Installing internet stack
  NodeContainer nc_host; // NodeContainer for source and destination
  nc_host.Create (2);
  internet.Install (nc_host);

  //enb
  NodeContainer nc_enb;
  nc_enb.Create (1);
  internet.Install (nc_enb);

  // relays
  subset_sum_relays = myCluster.getMaxSubSumRelays ();

  NodeContainer nc_relay; // NodeContainer for relay
  nc_relay.Create (subset_sum_relays); //cluster_nodes
  internet.Install (nc_relay);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nc_enb);
  mobility.Install (nc_host.Get (0));
  mobility.Install (nc_host.Get (1));

  //define stringValue for the node_mobility
  stringstream node_mobility_stream;
  node_mobility_stream.str (string ()); //reset stream to empty
  node_mobility_stream << "ns3::ConstantRandomVariable[Constant=" << node_mobility << "]";
  string node_speed = node_mobility_stream.str (); //"ns3::ConstantRandomVariable[Constant=5.0]"

  //define stringValue for the relay_host_distance
  stringstream relay_host_distance_stream;
  relay_host_distance_stream.str (string ()); //reset stream to empty
  relay_host_distance_stream << "ns3::UniformRandomVariable[Min=0|Max=" << relay_distance << "]";
  string relay_host_distance =
      relay_host_distance_stream.str (); //ns3::UniformRandomVariable[Min=0|Max=5]

  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator", "X", StringValue ("50"), "Y",
                                 StringValue ("50"), "Rho", StringValue (relay_host_distance));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Mode", StringValue ("Time"), "Time",
                             StringValue ("2s"), "Speed", StringValue (node_speed), "Bounds",
                             StringValue ("0|100|0|100"));
  mobility.Install (nc_relay);

  /////////////////////creating topology////////////////
  //remember to deallocate these dynamic arrays
  NodeContainer *e0Ri = new NodeContainer[subset_sum_relays];
  NodeContainer *riH1 = new NodeContainer[subset_sum_relays];

  NodeContainer e0h0 = NodeContainer (nc_enb, nc_host.Get (0)); //nc_enb.Get (0)

  //another dynamic array to be deallocated
  NetDeviceContainer *chiE0Ri = new NetDeviceContainer[subset_sum_relays];

  //loop through the relays
  for (size_t i = 0; i < subset_sum_relays; i++)
    {
      e0Ri[i] = NodeContainer (nc_enb, nc_relay.Get (i)); //nc_enb.Get (0)
      riH1[i] = NodeContainer (nc_host.Get (1), nc_relay.Get (i));
    }
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2pRelay;

  //define stringValue depending on the node capacity
  stringstream nodeCapacityStream;
  string nodeCapacity = "";
  //p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4InterfaceContainer *intfiE0Ri = new Ipv4InterfaceContainer[subset_sum_relays];
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  for (size_t i = 0; i < subset_sum_relays; i++)
    {
      //reset stream to empty
      nodeCapacityStream.str (string ());
      nodeCapacityStream << array_relay_capacities[i] << "Mbps";
      nodeCapacity = nodeCapacityStream.str ();

      cout << "node: " << i << " Capacity: " << nodeCapacity << endl;
      p2pRelay.SetDeviceAttribute ("DataRate", StringValue (nodeCapacity));
      p2pRelay.SetChannelAttribute ("Delay", StringValue ("1ns")); //ms
      chiE0Ri[i] = p2pRelay.Install (e0Ri[i]);
      intfiE0Ri[i] = ipv4.Assign (chiE0Ri[i]);
    }
  PointToPointHelper p2pEnbHost;
  p2pEnbHost.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2pEnbHost.SetChannelAttribute ("Delay", StringValue ("1ns"));
  NetDeviceContainer ch4e0h0 = p2pEnbHost.Install (e0h0);

  PointToPointHelper p2pD2d;
  p2pD2d.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2pD2d.SetChannelAttribute ("Delay", StringValue ("1ns"));

  //some additional dynamic arrays to be deallocated
  NetDeviceContainer *chiRiH1 = new NetDeviceContainer[subset_sum_relays];
  Ipv4InterfaceContainer *intfiRiH1 = new Ipv4InterfaceContainer[subset_sum_relays];

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  for (size_t i = 0; i < subset_sum_relays; i++)
    {
      chiRiH1[i] = p2pD2d.Install (riH1[i]);
      intfiRiH1[i] = ipv4.Assign (chiRiH1[i]);
    }

  //Later, we add IP addresses.

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i4e0h0 = ipv4.Assign (ch4e0h0);

  //initialize routing database and set up the routing tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Create applications.");

  if (mptcp == 1)
    {
     
      //tcp source
      cout << "TCP: OnOffHelper source - 1 app" << endl;
      OnOffHelper source =
          OnOffHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address ("10.1.1.1"), port));

      //Set the amount of data to send in bytes.  Zero is unlimited.
      source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
      source.SetAttribute ("PacketSize", UintegerValue (500));
      // source.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      // source.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
      // source.SetAttribute ("DataRate", DataRateValue (DataRate ("10000kb/s")));
      source.SetConstantRate (DataRate ("14.22Mbps"));

      ApplicationContainer sourceApps;

      sourceApps.Add (source.Install (nc_host.Get (0)));
      sourceApps.Start (Seconds (0.001));
      sourceApps.Stop (Seconds (100.0));

      //TCP sink
      PacketSinkHelper sink ("ns3::TcpSocketFactory",
                             InetSocketAddress (Ipv4Address::GetAny (), port));

      ApplicationContainer sinkApps = sink.Install (nc_host.Get (1));

      sinkApps.Start (Seconds (0.0));
      sinkApps.Stop (Seconds (100.0));
    }

  else
    {
      cout << "UDP: OnOffHelper source - 1 app" << endl;
      //udp source
      OnOffHelper source ("ns3::UdpSocketFactory",
                          InetSocketAddress (Ipv4Address ("10.1.1.1"), port));

      source.SetConstantRate (DataRate ("100Mbps"));
      source.SetAttribute ("PacketSize", UintegerValue (500));

      source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

      ApplicationContainer sourceApps;
      sourceApps.Add (source.Install (nc_host.Get (0)));
      sourceApps.Start (Seconds (0.001));
      sourceApps.Stop (Seconds (100.0));
      //UDP sink
      PacketSinkHelper sink ("ns3::UdpSocketFactory",
                             Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
      ApplicationContainer sinkApps = sink.Install (nc_host.Get (1));

      sinkApps.Start (Seconds (0.0));
      sinkApps.Stop (Seconds (100.0));
    }
  //=========================extra nodes that dont cooperate======================//
  // create extra relays that cant cooperate
  if (cluster_nodes > subset_sum_relays)
    {
      NodeContainer nc_relay_extras;
      nc_relay_extras.Create (cluster_nodes - subset_sum_relays);

      MobilityHelper mobility;

      //define stringValue for the node_mobility
      stringstream node_mobility_stream;
      node_mobility_stream.str (string ()); //reset stream to empty
      node_mobility_stream << "ns3::ConstantRandomVariable[Constant=" << node_mobility << "]";
      string node_speed = node_mobility_stream.str (); //"ns3::ConstantRandomVariable[Constant=5.0]"

      //define stringValue for the relay_host_distance
      stringstream relay_host_distance_stream;
      relay_host_distance_stream.str (string ()); //reset stream to empty
      relay_host_distance_stream << "ns3::UniformRandomVariable[Min=0|Max=" << relay_distance + 30
                                 << "]"; //farther 30m away
      string relay_host_distance =
          relay_host_distance_stream.str (); //ns3::UniformRandomVariable[Min=0|Max=5]

      mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator", "X", StringValue ("25"),
                                     "Y", StringValue ("50"), "Rho",
                                     StringValue (relay_host_distance));

      mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Mode", StringValue ("Time"),
                                 "Time", StringValue ("2s"), "Speed", StringValue (node_speed),
                                 "Bounds", StringValue ("0|100|0|100"));
      mobility.Install (nc_relay_extras);
    }

  //=========== Start the simulation ===========//

  std::cout << "Start Simulation.\n ";

  //tracemetrics trace file
  AsciiTraceHelper ascii;

  p2pRelay.EnableAsciiAll (ascii.CreateFileStream ("tracemetrics/isaac-mptcp.tr")); //, nc_relay);
  ////////////////////////// Use of NetAnim model/////////////////////////////////////////
  AnimationInterface anim ("netanim/isaac-tp.xml");

  //NodeContainer server
  anim.SetConstantPosition (nc_host.Get (0), 35.0, 5.0);
  uint32_t resourceIdIconServer =
      anim.AddResource ("/home/ns3/ns-3-dev-git/netanim/icons/server.png");
  anim.UpdateNodeImage (0, resourceIdIconServer);
  anim.UpdateNodeSize (0, 5, 5);

  //enb
  anim.SetConstantPosition (nc_enb.Get (0), 25.0, 10.0); //nc_enb.Get (0)
  uint32_t resourceIdIconEnb = anim.AddResource ("/home/ns3/ns-3-dev-git/netanim/icons/enb.png");
  anim.UpdateNodeImage (2, resourceIdIconEnb);
  anim.UpdateNodeSize (2, 7, 7);
  //relays
  for (size_t i = 0; i < subset_sum_relays; i++)
    {
      int x = i * 10;
      anim.SetConstantPosition (nc_relay.Get (i), x, 20.0);
    }

  uint32_t resourceIdIconPhone =
      anim.AddResource ("/home/ns3/ns-3-dev-git/netanim/icons/phone.png");
  for (size_t i = 0; i < subset_sum_relays; i++)
    {
      anim.UpdateNodeImage (i + 3, resourceIdIconPhone);
      anim.UpdateNodeSize (i + 3, 4, 4);
    }

  //requester
  anim.SetConstantPosition (nc_host.Get (1), 25.0, 50.0);
  anim.UpdateNodeImage (1, resourceIdIconPhone);
  anim.UpdateNodeSize (1, 4, 4);

  /////////////////////////////////////////////////////////////////////////////////////////////
  //Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("isaac-tp.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();

  p2pRelay.EnablePcapAll ("pcap/isaac-p2pCapmptcp");

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (15.0));

  //schedule events
  Ptr<Node> n1 = nc_relay.Get (1);
  Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  uint32_t ipv4ifIndex1 = 2;
  Simulator::Schedule (MilliSeconds (2), &Ipv4::SetDown, ipv41, ipv4ifIndex1);
  Simulator::Schedule (MilliSeconds (40), &Ipv4::SetUp, ipv41, ipv4ifIndex1);

  Simulator::Schedule (Seconds (1), &Ipv4GlobalRoutingHelper::RecomputeRoutingTables);

  Simulator::Run ();

  //   flowmon tracefiles
  //   Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  int64x64_t totalThroughput = 0;

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      //do not print ack flows

      std::cout << "Flow " << i->first - 1 << " (" << t.sourceAddress << " -> "
                << t.destinationAddress << ")\n";

      //rxbytes*8=bits; nanoseconds=pow(10,9); Mbps=pow(10,6);
      //Mbps/nanoseconds=pow(10,3)
      std::cout << "  Throughput: " << setiosflags (ios::fixed) << setprecision (3)
                << i->second.rxBytes * 8.0 * pow (10, 3) /
                       (i->second.timeLastRxPacket - i->second.timeFirstRxPacket)
                << " Mbps\n";
      totalThroughput += i->second.rxBytes * 8.0 * pow (10, 3) /
                         (i->second.timeLastRxPacket - i->second.timeFirstRxPacket);

      std::cout << "  Tx Data:   " << i->second.txBytes / pow (2, 20) << "MBs \n";
      std::cout << "  Rx Data:   " << i->second.rxBytes / pow (2, 20) << "MBs \n";
    }

  monitor->SerializeToXmlFile ("flowmon/isaac-tp.xml", true, true);

  NS_LOG_INFO ("Done.");

  std::cout << "Simulation finished "
            << "\n";

  Simulator::Destroy ();

  //remember to destroy dynamic arrays
  delete[] array_relay_capacities;
  delete[] e0Ri;
  delete[] riH1;
  delete[] chiE0Ri;
  delete[] intfiE0Ri;
  delete[] chiRiH1;
  delete[] intfiRiH1;
}
