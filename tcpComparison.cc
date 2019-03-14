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
#include "ns3/packet-sink.h"
#include "ns3/tcp-header.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-layout-module.h"
#include <string>


using namespace ns3;

int
main (int argc, char *argv[])
{
    double simTime=50.0;


    Time::SetResolution (Time::NS);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));

    PointToPointHelper bottleNeckLink;
    bottleNeckLink.SetDeviceAttribute ("DataRate", StringValue ("0.5Mbps"));
    bottleNeckLink.SetChannelAttribute ("Delay", StringValue ("50ms"));

    PointToPointDumbbellHelper devices (4, pointToPoint, 4, pointToPoint, bottleNeckLink);

    InternetStackHelper stack;
    devices.InstallStack (stack);

    devices.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                          		   Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                           		   Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t port = 5000;
    ApplicationContainer sourceApps, sinkApps;

    for (uint8_t n = 0;n < 3; n++)
       {
    BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (devices.GetRightIpv4Address (n), port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (1e8));
    sourceApps.Add (source.Install (devices.GetLeft (n)));
       }

    sourceApps.Start (MilliSeconds (0.0));
    sourceApps.Stop (Seconds (simTime));

    for (uint8_t n = 0;n < 3;n++)
       {
    PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
    sinkApps.Add(sink.Install (devices.GetRight (n)));
       }

    PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
    sinkApps.Add(sink.Install (devices.GetRight (3)));
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (simTime));

	   //creating pcap files
    /*AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("pointToPoint.tr"));
    bottleNeckLink.EnableAsciiAll(ascii.CreateFileStream("bottleNeckLink.tr"));
    pointToPoint.EnablePcapAll("pointToPoint");
    bottleNeckLink.EnablePcapAll("bottleNeckLink");*/

    Simulator::Stop (Seconds (simTime + 1.0));
    Simulator::Run ();
    uint64_t totalRxBytesReceived=0;

    for (uint8_t n = 0;n < 3;n++)
       {
    Ptr <PacketSink> sink1 = DynamicCast <PacketSink> (sinkApps.Get (n));
    std::cout << "Bytes received by " << std::to_string(n) <<"th packetsink application: " << sink1->GetTotalRx() << std::endl;
    totalRxBytesReceived += sink1->GetTotalRx();
       }

  // Creating UDP trafic
  OnOffHelper onoff ("ns3::UdpSocketFactory", Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (200));
  onoff.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s
  ApplicationContainer onOffApps;

  AddressValue remoteAddress (InetSocketAddress (devices.GetRightIpv4Address(3), port));
  onoff.SetAttribute ("Remote", remoteAddress);
  onOffApps.Add (onoff.Install (devices.GetLeft(3)));
  onOffApps.Start (Seconds (0.0));
  onOffApps.Stop (Seconds (simTime));

    double averageThroughput = ((totalRxBytesReceived * 8) / (1e6*Simulator::Now ().GetSeconds ()));
    std::cout << "Average Throughput: " << averageThroughput << " Mbits/sec\n";

    Simulator::Destroy ();
    NS_LOG_UNCOND("Done");

    return 0;
}
