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
//               |  CSMA  |    |  CSMA  |
//               +--------+    +--------+
//                   |             |
//                   |             |
//                   |             |
//                   ===============
//                      CSMA LAN
//
#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/tap-bridge-module.h"

 #include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TapCsmaVirtualMachineExample");

int 
main (int argc, char *argv[])
{
  bool AnimationOn = false;
  int NumNodes = 10;
  double TotalTime = 600.0;

  std::string TapBaseName = "emu";

  CommandLine cmd;
  cmd.AddValue ("NumNodes", "Number of nodes/devices", NumNodes);
  cmd.AddValue ("TotalTime", "Total simulation time", TotalTime);
  cmd.AddValue ("TapBaseName", "Base name for tap interfaces", TapBaseName);
  cmd.AddValue ("AnimationOn", "Enable animation", AnimationOn);

  cmd.Parse (argc,argv);

  //
  // We are interacting with the outside, real, world.  This means we have to 
  // interact in real-time and therefore means we have to use the real-time
  // simulator and take the time to calculate checksums.
  //
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


  //
  // Create two ghost nodes.  The first will represent the virtual machine host
  // on the left side of the network; and the second will represent the VM on 
  // the right side.
  //
  NS_LOG_UNCOND ("Creating nodes");
  NodeContainer nodes;
  nodes.Create (NumNodes);

  //
  // Use a CsmaHelper to get a CSMA channel created, and the needed net 
  // devices installed on both of the nodes.  The data rate and delay for the
  // channel can be set through the command-line parser.  For example,
  //
  // ./waf --run "tap=csma-virtual-machine --ns3::CsmaChannel::DataRate=10000000"
  //
  CsmaHelper csma;
  NetDeviceContainer devices = csma.Install (nodes);

  //
  // Use the TapBridgeHelper to connect to the pre-configured tap devices for 
  // the left side.  We go with "UseBridge" mode since the CSMA devices support
  // promiscuous mode and can therefore make it appear that the bridge is 
  // extended into ns-3.  The install method essentially bridges the specified
  // tap to the specified CSMA device.
  //
  NS_LOG_UNCOND ("Creating tap bridges");
  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("UseBridge"));

  for (int i = 0; i < NumNodes; i++)
    {
        std::stringstream tapName;
        tapName << "tap-" << TapBaseName << (i+1) ;
        NS_LOG_UNCOND ("Tap bridge = " + tapName.str ());

        tapBridge.SetAttribute ("DeviceName", StringValue (tapName.str ()));
        tapBridge.Install (nodes.Get (i), devices.Get (i));
    }

  if( AnimationOn )
  {
    NS_LOG_UNCOND ("Activating Animation");
    AnimationInterface anim ("animation.xml"); // Mandatory 
    for (uint32_t i = 0; i < nodes.GetN (); ++i)
      {
        std::stringstream ssi;
        ssi << i;
        anim.UpdateNodeDescription (nodes.Get (i), "Node" + ssi.str()); // Optional
        anim.UpdateNodeColor (nodes.Get (i), 255, 0, 0); // Optional
      }

    anim.EnablePacketMetadata (); // Optional
    // anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
    anim.EnableWifiMacCounters (Seconds (0), Seconds (TotalTime)); //Optional
    anim.EnableWifiPhyCounters (Seconds (0), Seconds (TotalTime)); //Optional
  }

  //
  // Run the simulation for TotalTime seconds to give the user time to play around
  //
  NS_LOG_UNCOND ("Running simulation in csma mode");
  Simulator::Stop (Seconds (TotalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}
