#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/tap-bridge-module.h"

using namespace ns3;

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// --------------- SOURCE
// https://www.nsnam.org/doxygen/main-random-walk_8cc_source.html
// ---------------------------------------------------------------------------------------------------------

int
main (int argc, char *argv[])
{
  int NumNodes = 10;
  double TotalTime = 600.0;

  double distance = 100;  // m
  int gridRowSize = 5;

  int nodeSpeed = 5; //in m/s
  int nodePause = 1; //in s

  double scenarioSizeX = 300.0;
  double scenarioSizeY = 300.0;

  std::string TapBaseName = "emu";

  CommandLine cmd;
  cmd.AddValue ("NumNodes", "Number of nodes/devices", NumNodes);
  cmd.AddValue ("TotalTime", "Total simulation time", TotalTime);
  cmd.AddValue ("TapBaseName", "Base name for tap interfaces", TapBaseName);
  cmd.AddValue ("GridRowSize", "Grid row size", gridRowSize);
  cmd.AddValue ("GridDistance", "Grid distance", distance);
  cmd.AddValue ("SizeX", "Scenario Size in X axis", scenarioSizeX);
  cmd.AddValue ("SizeY", "Scenario Size in Y axis", scenarioSizeY);
  cmd.AddValue ("MobilityPause", "Mobility Pause", nodePause);
  cmd.AddValue ("MobilitySpeed", "Mobility Speed", nodeSpeed);

  cmd.Parse (argc,argv);

  //
  // We are interacting with the outside, real, world.  This means we have to
  // interact in real-time and therefore means we have to use the real-time
  // simulator and take the time to calculate checksums.
  //
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


  //
  // Create NumNodes ghost nodes.
  //
  NodeContainer nodes;
  nodes.Create (NumNodes);

  //
  // We're going to use 802.11 A so set up a wifi helper to reflect that.
  //
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"));

  //
  // No reason for pesky access points, so we'll use an ad-hoc network.
  //
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  //
  // Configure the physcial layer.
  //
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  //
  // Install the wireless devices onto our ghost nodes.
  //
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);


  // ++++++++++++++++++++++++++++++++++++
  // <Mobility>
  // ++++++++++++++++++++++++++++++++++++

  double radio = scenarioSizeX / 2;

  std::stringstream ssSpeed;
  ssSpeed << "ns3::ConstantRandomVariable[Constant=" << nodeSpeed << ".0]";

  std::stringstream ssPause;
  ssPause << "" << nodePause << "s";

  std::stringstream ssBounds;
  ssBounds << "0|" << scenarioSizeX << "|0|" << scenarioSizeY ;

  std::stringstream ssDiscPos;
  ssDiscPos << "" << radio ;

  std::stringstream ssRho;
  ssRho << "ns3::UniformRandomVariable[Min=0|Max=" << radio << "]" ;


    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ( ssPause.str () ));
    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ( ssSpeed.str () ));
    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Bounds", StringValue ( ssBounds.str () ));

    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", StringValue ( ssDiscPos.str () ),
                                 "Y", StringValue ( ssDiscPos.str () ),
                                 "Rho", StringValue ( ssRho.str () ));
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ( ssPause.str () ),
                             "Speed", StringValue ( ssSpeed.str () ),
                             "Bounds", StringValue ( ssBounds.str () ));



  // ++++++++++++++++++++++++++++++++++++
  // </Mobility>
  // ++++++++++++++++++++++++++++++++++++

//  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.Install (nodes);

  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("UseLocal"));

  for (int i = 0; i < NumNodes; i++)
    {
        std::stringstream tapName;
        tapName << "tap-" << TapBaseName << (i+1) ;

        tapBridge.SetAttribute ("DeviceName", StringValue (tapName.str ()));
        tapBridge.Install (nodes.Get (i), devices.Get (i));
    }


  //
  // Run the simulation for TotalTime seconds to give the user time to play around
  //
  Simulator::Stop (Seconds (TotalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

