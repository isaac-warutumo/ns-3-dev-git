#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/epc-helper.h"

using namespace ns3;

int main (int argc, char *argv[])
{
//Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
lteHelper->SetEpcHelper (epcHelper);

Ptr<Node> pgw = epcHelper->GetPgwNode ();

// Create a single RemoteHost
NodeContainer remoteHostContainer;
remoteHostContainer.Create (1);
Ptr<Node> remoteHost = remoteHostContainer.Get (0);
InternetStackHelper internet;
internet.Install (remoteHostContainer);

// Create the internet
PointToPointHelper p2p;
p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
p2p.SetDeviceAttribute ("Mtu", UintegerValue (1500));
p2p.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
NetDeviceContainer internetDevices = p2p.Install (pgw, remoteHost);

Ipv4AddressHelper ipv4h;
ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
// interface 0 is localhost, 1 is the p2p device
Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);


Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

NodeContainer enbNodes;
enbNodes.Create (1);
NodeContainer ueNodes;
ueNodes.Create (2);

MobilityHelper mobility;
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (enbNodes);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (ueNodes);

NetDeviceContainer enbLteDevs;
enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);

NetDeviceContainer ueLteDevs;
ueLteDevs = lteHelper->InstallUeDevice (ueNodes);



//lteHelper->Attach (ueDevs, enbDevs.Get (0));

// we install the IP stack on the UEs

internet.Install (ueNodes);
Ipv4InterfaceContainer ueIpIface;
// assign IP address to UEs
for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ue = ueNodes.Get (u);
    Ptr<NetDevice> ueLteDevice = ueLteDevs.Get (u);
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice));
    // set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting;
    ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

lteHelper->Attach (ueLteDevs, enbLteDevs.Get (0));
//enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
//EpsBearer bearer (q);
//lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

/*Ptr<EpcTft> tft = Create<EpcTft> ();
EpcTft::PacketFilter pf;
pf.localPortStart = 1234;
pf.localPortEnd = 1234;
tft->Add (pf);
lteHelper->ActivateDedicatedEpsBearer (ueLteDevs,
                                       EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT),
                                       tft);
 */
                                       
uint16_t dlPort = 1234;
PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), dlPort));
ApplicationContainer serverApps = packetSinkHelper.Install (ueNodes);
serverApps.Start (Seconds (0.01));

UdpClientHelper client (ueIpIface.GetAddress (0), dlPort);
ApplicationContainer clientApps = client.Install (remoteHost);
clientApps.Start (Seconds (0.01));

AsciiTraceHelper ascii;
p2p.EnableAsciiAll (ascii.CreateFileStream ("lte-epc.tr"));
                                                                               

AnimationInterface anim ("lte-basic.xml");
 
  //NodeContainer for spine switches
	  	
		anim.SetConstantPosition(ueNodes.Get(0), 0.0, 37.5);
		anim.SetConstantPosition((pgw), 75.0, 0.0);
		anim.SetConstantPosition(ueNodes.Get(1), 100.0, 37.5);
		anim.SetConstantPosition((remoteHost), 100.0, 0.0);
		anim.SetConstantPosition(enbNodes.Get(0), 50.0, 0.0);
		//anim.SetConstantPosition(host.Get(1), 50.0 , 100.0);  

Simulator::Stop (Seconds (10));

Simulator::Run ();

Simulator::Destroy ();
return 0;
}

