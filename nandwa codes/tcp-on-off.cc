/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//       n0 ----------- n1
//            500 Kbps
//             5 ms
//
// - Flow from n0 to n1 using BulkSendApplication.
// - Tracing of queues and packet receptions to file "tcp-bulk-send.tr"
//   and pcap tracing available when tracing is turned on.

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h" 
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpOnOffExample");

int
main (int argc, char *argv[])
{

    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (4288));
  uint32_t maxBytes = 10485760;

//
// Allow the user to override any of the defaults at
// run-time, via command-line arguments
//


//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (2);

MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);



  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ns"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (nodes);

//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  NS_LOG_INFO ("Create Applications.");

//
// Create a BulkSendApplication and install it on node 0
//
  uint16_t port = 9;  // well-known echo port number


  OnOffHelper oo = OnOffHelper ("ns3::TcpSocketFactory",
                         InetSocketAddress (i.GetAddress (1), port)); 
  
  //Set the amount of data to send in bytes.  Zero is unlimited.
  oo.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  oo.SetAttribute ("PacketSize", UintegerValue (1024));
  oo.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  oo.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
  oo.SetAttribute ("DataRate", DataRateValue (DataRate ("4000kb/s")));

  ApplicationContainer SourceApp = oo.Install (nodes.Get (0));
  SourceApp.Start (NanoSeconds (1.0));
  SourceApp.Stop (Seconds (100.0));
  
  //Create a packet sink to receive packets.
  
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                        		 InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get(1));
	
  sinkApp.Start (NanoSeconds (2.0));
  sinkApp.Stop (Seconds (100.0));   
//
// Set up tracing if enabled
//
 
      AsciiTraceHelper ascii;
      pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-on-off.tr"));
      pointToPoint.EnablePcapAll ("tcp-on-off", true);
  
AnimationInterface anim("tcp-on-off.xml");
  anim.SetConstantPosition (nodes.Get(0), 10.0, 10.0);
  anim.SetConstantPosition (nodes.Get(1), 30.0, 10.0);
  
Ptr <FlowMonitor>  flowMonitor;
 FlowMonitorHelper flowHelper;
 flowMonitor = flowHelper.InstallAll();  
 
Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributestcponoff.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes (); 
  
//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (100.0));
  Simulator::Run ();
  
 flowMonitor->SerializeToXmlFile("tcp-on-offflow.xml", true, true); 
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApp.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
}
