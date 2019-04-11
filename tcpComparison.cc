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
#include "ns3/traffic-control-module.h"
#include <string>


using namespace ns3;

Ptr<OutputStreamWrapper> streamWrapper;
std::string dir = "results/";

void
CalculateThroughputA (Ptr<PacketSink> sink)
{
  static uint64_t lastBytesReceivedYet = 0;
  static double lastTPrecord = 0.0;
  uint64_t bytesReceivedYet = sink->GetTotalRx();
  std::cout << bytesReceivedYet << "\t" << lastBytesReceivedYet << "\t" << lastTPrecord << "\n";

  Simulator::Schedule (Seconds (1.0), &CalculateThroughputA, sink);

  std::ofstream fPlotQueue (std::stringstream (dir + "throughput-tcpVegas.dat").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << ((bytesReceivedYet - lastBytesReceivedYet)*8.0)/1e3*(Simulator::Now ().GetSeconds() - lastTPrecord) << std::endl;
  fPlotQueue.close ();
  lastBytesReceivedYet=bytesReceivedYet;
  lastTPrecord=Simulator::Now ().GetSeconds ();
}

void
CalculateThroughputB (Ptr<PacketSink> sink)
{
  static uint64_t lastBytesReceivedYet = 0;
  static Time lastTPrecord = Time::Min ();
  uint64_t bytesReceivedYet = sink->GetTotalRx();
  //std::cout << bytesReceivedYet << "\t" << lastBytesReceivedYet << "\t" << lastTPrecord << "\n";

  Simulator::Schedule (Seconds (1.0), &CalculateThroughputB, sink);

  std::ofstream fPlotQueue (std::stringstream (dir + "throughput-newReno.dat").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << ((bytesReceivedYet - lastBytesReceivedYet)*8.0)/1e3*(Simulator::Now () - lastTPrecord).GetSeconds () << std::endl;
  fPlotQueue.close ();
  lastBytesReceivedYet=bytesReceivedYet;
  lastTPrecord=Simulator::Now();
}

void
CalculateThroughputC (Ptr<PacketSink> sink)
{
  static uint64_t lastBytesReceivedYet = 0;
  static Time lastTPrecord = Time::Min ();
  uint64_t bytesReceivedYet = sink->GetTotalRx();
  //std::cout << bytesReceivedYet << "\t" << lastBytesReceivedYet << "\t" << lastTPrecord << "\n";

  Simulator::Schedule (Seconds (1.0), &CalculateThroughputC, sink);

  std::ofstream fPlotQueue (std::stringstream (dir + "throughput-tcpHybla.dat").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << ((bytesReceivedYet - lastBytesReceivedYet)*8.0)/1e3*(Simulator::Now () - lastTPrecord).GetSeconds () << std::endl;
  fPlotQueue.close ();
  lastBytesReceivedYet=bytesReceivedYet;
  lastTPrecord=Simulator::Now();
}

void
CalculateThroughputD (Ptr<PacketSink> sink)
{
  static uint64_t lastBytesReceivedYet = 0;
  static Time lastTPrecord = Time::Min ();
  uint64_t bytesReceivedYet = sink->GetTotalRx();
  //std::cout << bytesReceivedYet << "\t" << lastBytesReceivedYet << "\t" << lastTPrecord << "\n";

  Simulator::Schedule (Seconds (0.1), &CalculateThroughputD, sink);

  std::ofstream fPlotQueue (std::stringstream (dir + "throughput-udp.dat").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << ((bytesReceivedYet - lastBytesReceivedYet)*8.0)/1e3*(Simulator::Now () - lastTPrecord).GetSeconds () << std::endl;
  fPlotQueue.close ();
  lastBytesReceivedYet=bytesReceivedYet;
  lastTPrecord=Simulator::Now();
}

void
ChangeQueueSize(uint16_t packets)
{
  Simulator::Schedule (Seconds (10.0), &ChangeQueueSize,packets + 10);

  Config::SetDefault ("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize (std::to_string(packets) + "p")));
}
int
main (int argc, char *argv[])
{
    double simTime=100.0;
    std::string queueDiscType= "PfifoFast";

    Time::SetResolution (Time::NS);

    CommandLine cmd;
    cmd.AddValue ("queueDiscType", "Set a QueueDisc", queueDiscType);
    cmd.Parse (argc,argv);

    Config::SetDefault ("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize ("25p")));
    Config::SetDefault ("ns3::PfifoFastQueueDisc::MaxSize", QueueSizeValue (QueueSize ("25p")));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));


    PointToPointHelper bottleNeckLink;
    bottleNeckLink.SetDeviceAttribute ("DataRate", StringValue ("0.5Mbps"));
    bottleNeckLink.SetQueue ("ns3::DropTailQueue","MaxSize", QueueSizeValue(QueueSize ("25p")));
    bottleNeckLink.SetChannelAttribute ("Delay", StringValue ("50ms"));

    PointToPointDumbbellHelper devices (4, pointToPoint, 4, pointToPoint, bottleNeckLink);

    InternetStackHelper stack;
    devices.InstallStack (stack);


    if(queueDiscType == "PfifoFast")
  {
    TrafficControlHelper tch;
    //default queuedisc is pfifo
    tch.Install(devices.GetLeft()->GetDevice(0));
    tch.Install(devices.GetRight()->GetDevice(0));
    NS_LOG_UNCOND("PfifoFast is active");
  }
  //Activating RED
  if(queueDiscType == "RED")
  {
  TrafficControlHelper tch;
  tch.SetRootQueueDisc("ns3::RedQueueDisc");
  tch.Install(devices.GetLeft()->GetDevice(0)); 
  tch.Install(devices.GetRight()->GetDevice(0));
  NS_LOG_UNCOND("RED is active");
  }


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
    source.SetAttribute ("MaxBytes", UintegerValue (1e6));
    source.SetAttribute ("SendSize", UintegerValue (1024));
    sourceApps.Add (source.Install (devices.GetLeft (n)));
       }

    sourceApps.Start (MilliSeconds (0.0));
    sourceApps.Stop (Seconds (simTime));

  // Creating UDP trafic
  OnOffHelper onoff ("ns3::UdpSocketFactory", Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (200));
  onoff.SetAttribute ("DataRate", StringValue ("0.5Mbps")); //bit/s
  ApplicationContainer onOffApps;

  AddressValue remoteAddress (InetSocketAddress (devices.GetRightIpv4Address(3), port));
  onoff.SetAttribute ("Remote", remoteAddress);
  onOffApps.Add (onoff.Install (devices.GetLeft(3)));
  onOffApps.Start (Seconds (0.0));
  onOffApps.Stop (Seconds (simTime));



    for (uint8_t n = 0;n < 3;n++)
       {
    PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
    sinkApps.Add(sink.Install (devices.GetRight (n)));
       }

    PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
    sinkApps.Add(sink.Install (devices.GetRight (3)));
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (simTime));

    TypeId tid1 = TypeId::LookupByName ("ns3::TcpVegas");
    TypeId tid3 = TypeId::LookupByName ("ns3::TcpHybla");
    
    //Setting bulksend nodes
    Config::Set ("/NodeList/0/$ns3::TcpL4Protocol/SocketType", TypeIdValue (tid1));
    Config::Set ("/NodeList/2/$ns3::TcpL4Protocol/SocketType", TypeIdValue (tid3));
    //setting packetsink nodes
    Config::Set ("/NodeList/4/$ns3::TcpL4Protocol/SocketType", TypeIdValue (tid1));
    Config::Set ("/NodeList/6/$ns3::TcpL4Protocol/SocketType", TypeIdValue (tid3));

    std::string dirToSave = "mkdir -p " + dir;
    system (dirToSave.c_str ());

	   
    Ptr <PacketSink> sink0 = DynamicCast <PacketSink> (sinkApps.Get (0));
    Simulator::ScheduleNow (&CalculateThroughputA, sink0);
    Ptr <PacketSink> sink1 = DynamicCast <PacketSink> (sinkApps.Get (1));
    Simulator::ScheduleNow (&CalculateThroughputB, sink1);
    Ptr <PacketSink> sink2 = DynamicCast <PacketSink> (sinkApps.Get (2));
    Simulator::ScheduleNow (&CalculateThroughputC, sink2);
    Ptr <PacketSink> sink3 = DynamicCast <PacketSink> (sinkApps.Get (3));
    Simulator::ScheduleNow (&CalculateThroughputD, sink3);

    //uint16_t packets = 10;
    //Simulator::ScheduleNow (&ChangeQueueSize, packets);
   
    Simulator::Stop (Seconds (simTime + 1.0));
    Simulator::Run ();

    uint64_t totalRxBytesReceived=0;

    for (uint8_t n = 0;n <= 3;n++)
       {
    Ptr <PacketSink> sink1 = DynamicCast <PacketSink> (sinkApps.Get (n));
    std::cout << "Bytes received by " << std::to_string(n) <<"th packetsink application: " << sink1->GetTotalRx() << std::endl;
    totalRxBytesReceived += sink1->GetTotalRx();
       }

    double averageThroughput = ((totalRxBytesReceived * 8) / (1e6*Simulator::Now ().GetSeconds ()));
    std::cout << "Average Throughput: " << averageThroughput << " Mbits/sec\n";
    Simulator::Destroy ();
    NS_LOG_UNCOND("Done");

    return 0;
}
