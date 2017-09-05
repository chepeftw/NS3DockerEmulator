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


// Large Scale Simulations with ns3 using linux namespace
// PROBLEMS :(
// https://groups.google.com/forum/#!topic/ns-3-users/zy2VIrgh-Qo


// Running NS3 optimized
// ./waf distclean
// ./waf -d optimized configure --disable-examples --disable-tests --no-task-lines
// ./waf

// Running NS3 with MPI
// https://www.nsnam.org/docs/models/html/distributed.html#current-implementation-details
// it seems not feasible due to AWS restrictions, MAYBE it works in a dedicated server


//
// This is an illustration of how one could use virtualization techniques to
// allow running applications on virtual machines talking over simulated
// networks.
//
// The actual steps required to configure the virtual machines can be rather
// involved, so we don't go into that here.  Please have a look at one of
// our HOWTOs on the nsnam wiki for more details about how to get the
// system confgured.  For an example, have a look at "HOWTO Use Linux
// Containers to set up virtual networks" which uses this code as an
// example.
//
// The configuration you are after is explained in great detail in the
// HOWTO, but looks like the following:
//
//  +----------+                           +----------+
//  | virtual  |                           | virtual  |
//  |  Linux   |                           |  Linux   |
//  |   Host   |                           |   Host   |
//  |          |                           |          |
//  |   eth0   |                           |   eth0   |
//  +----------+                           +----------+
//       |                                      |
//  +----------+                           +----------+
//  |  Linux   |                           |  Linux   |
//  |  Bridge  |                           |  Bridge  |
//  +----------+                           +----------+
//       |                                      |
//  +------------+                       +-------------+
//  | "tap-left" |                       | "tap-right" |
//  +------------+                       +-------------+
//       |           n0            n1           |
//       |       +--------+    +--------+       |
//       +-------|  tap   |    |  tap   |-------+
//               | bridge |    | bridge |
//               +--------+    +--------+
//               |  wifi  |    |  wifi  |
//               +--------+    +--------+
//                   |             |
//                 ((*))         ((*))
//
//                       Wifi LAN
//
//                        ((*))
//                          |
//                     +--------+
//                     |  wifi  |
//                     +--------+
//                     | access |
//                     |  point |
//                     +--------+
//



#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
//#include "ns3/mpi-interface.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/tap-bridge-module.h"




using namespace ns3;

//NS_LOG_COMPONENT_DEFINE ("TapWifiVirtualMachineExample");

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
//  cmd.AddValue ("AnimationOn", "Enable animation", AnimationOn);

  cmd.Parse (argc,argv);

//  printf("NumNodes = %d\n", NumNodes);
//  printf("TotalTime = %f\n", TotalTime);
//  printf("gridRowSize = %d\n", gridRowSize);
//  printf("distance = %f\n", distance);
//  printf("scenarioSizeX = %f\n", scenarioSizeX);
//  printf("scenarioSizeY = %f\n", scenarioSizeY);
//  printf("nodeSpeed = %d\n", nodeSpeed);
//  printf("nodePause = %d\n", nodePause);

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
//  NS_LOG_UNCOND ("Creating nodes");
  NodeContainer nodes;
  nodes.Create (NumNodes);

  //
  // We're going to use 802.11 A so set up a wifi helper to reflect that.
  //
//  NS_LOG_UNCOND ("Creating wifi");
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"));

  //
  // No reason for pesky access points, so we'll use an ad-hoc network.
  //
//  NS_LOG_UNCOND ("Creating ad hoc wifi mac");
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  //
  // Configure the physcial layer.
  //
//  NS_LOG_UNCOND ("Configuring physical layer");
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  //
  // Install the wireless devices onto our ghost nodes.
  //
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  //
  // We need location information since we are talking about wifi, so add a
  // constant position to the ghost nodes.
  //
//  NS_LOG_UNCOND ("Configuring mobility");
  MobilityHelper mobility;

  // ++++++++++++++++++++++++++++++++++++
  // <Positioning>
  // ++++++++++++++++++++++++++++++++++++

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  std::stringstream xAxisMax;
  xAxisMax << "ns3::UniformRandomVariable[Min=0.0|Max=" << scenarioSizeX << "]";
  std::stringstream yAxisMax;
  yAxisMax << "ns3::UniformRandomVariable[Min=0.0|Max=" << scenarioSizeY << "]";
  pos.Set ("X", StringValue ( xAxisMax.str () ));
  pos.Set ("Y", StringValue ( yAxisMax.str () ));
//  NS_LOG_UNCOND ("Allocation => ns3::RandomRectanglePositionAllocator");
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();

  // ObjectFactory pos;
  // pos.SetTypeId ("ns3::GridPositionAllocator");
  // pos.Set ("MinX", DoubleValue ( 0.0 ));
  // pos.Set ("MinY", DoubleValue ( 0.0 ));
  // pos.Set ("DeltaX", DoubleValue ( distance ));
  // pos.Set ("DeltaY", DoubleValue ( distance ));
  // pos.Set ("GridWidth", UintegerValue ( gridRowSize ));
  // pos.Set ("LayoutType", StringValue ( "RowFirst" ));
  // NS_LOG_UNCOND ("Allocation => ns3::GridPositionAllocator");
  // Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();

  // ++++++++++++++++++++++++++++++++++++
  // </Positioning>
  // ++++++++++++++++++++++++++++++++++++


  // ++++++++++++++++++++++++++++++++++++
  // <Mobility>
  // ++++++++++++++++++++++++++++++++++++

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
//  NS_LOG_UNCOND ("Mobility => ns3::RandomWaypointMobilityModel");
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));



  // NS_LOG_UNCOND ("Mobility => ns3::ConstantPositionMobilityModel");
  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");



  // Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
  // Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
  // Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  // Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Bounds", StringValue ("0|300|0|300"));

  // NS_LOG_UNCOND ("Mobility => ns3::RandomWalk2dMobilityModel");
  // mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  //                            "Mode", StringValue ("Time"),
  //                            "Time", StringValue ("2s"),
  //                            "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=5.0]"),
  //                            "Bounds", StringValue ("0|300|0|300"));

  // ++++++++++++++++++++++++++++++++++++
  // </Mobility>
  // ++++++++++++++++++++++++++++++++++++

  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.Install (nodes);

  //
  // Use the TapBridgeHelper to connect to the pre-configured tap devices for
  // the left side.  We go with "UseLocal" mode since the wifi devices do not
  // support promiscuous mode (because of their natures0.  This is a special
  // case mode that allows us to extend a linux bridge into ns-3 IFF we will
  // only see traffic from one other device on that bridge.  That is the case
  // for this configuration.
  //
//  NS_LOG_UNCOND ("Creating tap bridges");
  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("UseLocal"));

  for (int i = 0; i < NumNodes; i++)
    {
        std::stringstream tapName;
        tapName << "tap-" << TapBaseName << (i+1) ;
        // NS_LOG_UNCOND ("Tap bridge = " + tapName.str ());

        tapBridge.SetAttribute ("DeviceName", StringValue (tapName.str ()));
        tapBridge.Install (nodes.Get (i), devices.Get (i));
    }


  //
  // Run the simulation for TotalTime seconds to give the user time to play around
  //
//  NS_LOG_UNCOND ("Running simulation in wifi mode");
  Simulator::Stop (Seconds (TotalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  // Exit the MPI execution environment
//  MpiInterface::Disable ();
}

