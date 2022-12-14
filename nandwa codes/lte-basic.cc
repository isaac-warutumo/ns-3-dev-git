#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
using namespace ns3;

int main (int argc, char *argv[])
{
//Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
lteHelper->SetEpcHelper (epcHelper);

NodeContainer enbNodes;
enbNodes.Create (1);
NodeContainer ueNodes;
ueNodes.Create (2);

MobilityHelper mobility;
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (enbNodes);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (ueNodes);

NetDeviceContainer enbDevs;
enbDevs = lteHelper->InstallEnbDevice (enbNodes);

NetDeviceContainer ueDevs;
ueDevs = lteHelper->InstallUeDevice (ueNodes);

lteHelper->Attach (ueDevs, enbDevs.Get (0));

enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
EpsBearer bearer (q);
lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

AnimationInterface anim ("lte-basic.xml");
 
  //NodeContainer for spine switches
	  	
		anim.SetConstantPosition(ueNodes.Get(0), 0.0, 20.0);
		//anim.SetConstantPosition(relay.Get(1), 25.0, 20.0);
		anim.SetConstantPosition(ueNodes.Get(1), 75.0, 20.0);
		//anim.SetConstantPosition(host.Get(0), 75.0, 0.0);
		anim.SetConstantPosition(enbNodes.Get(0), 50.0, 0.0);
		//anim.SetConstantPosition(host.Get(1), 50.0 , 100.0);  

Simulator::Stop (Seconds (10));

Simulator::Run ();

Simulator::Destroy ();
return 0;
}

