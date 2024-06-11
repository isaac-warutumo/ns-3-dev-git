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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor.h"

//iomanip required for setprecision(3)
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  int mptcp = 1;
  uint16_t port = 9;
  uint32_t maxBytes = 10;

  CommandLine cmd;
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  // LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  // LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nc_host;
  nc_host.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ns"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nc_host);

  InternetStackHelper stack;
  stack.Install (nc_host);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  if (mptcp == 1)
    {
      //tcp source
      cout << "TCP: OnOffHelper source - 1 app" << endl;
      OnOffHelper source =
          OnOffHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address ("10.1.1.2"), port));

      //Set the amount of data to send in bytes.  Zero is unlimited.
      source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
      source.SetAttribute ("PacketSize", UintegerValue (500));
      
      source.SetConstantRate (DataRate ("10.22Mbps"));

      ApplicationContainer sourceApps;

      sourceApps.Add (source.Install (nc_host.Get (0)));
      sourceApps.Start (NanoSeconds (0.1));
      sourceApps.Stop (Seconds (100.0));

      //TCP sink
      PacketSinkHelper sink ("ns3::TcpSocketFactory",
                             InetSocketAddress (Ipv4Address::GetAny (), port));

      ApplicationContainer sinkApps = sink.Install (nc_host.Get (1));

      sinkApps.Start (NanoSeconds (0.0));
      sinkApps.Stop (Seconds (100.0));
    }
  else
    {

      UdpEchoServerHelper echoServer (9);

      ApplicationContainer serverApps = echoServer.Install (nc_host.Get (1));
      serverApps.Start (NanoSeconds (1.0));
      serverApps.Stop (Seconds (10.0));

      UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
      echoClient.SetAttribute ("Interval", TimeValue (NanoSeconds (.01)));
      echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

      ApplicationContainer clientApps = echoClient.Install (nc_host.Get (0));
      clientApps.Start (NanoSeconds (2.0));
      clientApps.Stop (Seconds (10.0));
    }

  AnimationInterface anim ("netanim/first.xml");
  anim.SetConstantPosition (nc_host.Get (0), 10.0, 10.0);
  anim.SetConstantPosition (nc_host.Get (1), 30.0, 10.0);

  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tracemetrics/first.tr"));
  pointToPoint.EnablePcapAll ("pcap/first");

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
