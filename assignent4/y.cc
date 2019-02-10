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
 *
 */

//
// Network topology
//
//  n0
//     \ 5 Mb/s, 2ms
//      \          1.5Mb/s, 10ms
//       n2 -------------------------n3
//      /
//     / 5 Mb/s, 2ms
//   n1
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n3, and from n3 to n1
// - FTP/TCP flow from n0 to n3, starting at time 1.2 to time 1.35 sec.
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues 
// - Tracing of queues and packet receptions to file "simple-global-routing.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
// #include <fstream>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include "ns3/gnuplot.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleGlobalRoutingExample");
Ptr<PacketSink>  cbrsink ,  tcpSink;
AsciiTraceHelper ascii;



using namespace ns3;


int main (int argc, char *argv[])
{
	// Users may find it convenient to turn on explicit debugging
	// for selected modules; the below lines suggest how to do this
#if 0 
	LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

    std::string plotFileName1 = "UDP.plt";
    std::string plotFileName2 = "TcpHighSpeed.plt";
    Gnuplot plot1 ("UDP.png");
    Gnuplot plot2 ("TcpHighSpeed.png");
    plot1.SetTitle ("UDP");
    plot1.SetTerminal ("png");
    plot1.SetLegend ("Packet_size", "Throughput");
    plot1.AppendExtra ("set xrange [0:10500]");
    plot2.SetTitle ("TCP_highspeed");
    plot2.SetTerminal ("png");
    plot2.SetLegend ("Packet_size", "Throughput");
    plot2.AppendExtra ("set xrange [0:10500]");
    Gnuplot2dDataset dataset1,dataset2;
    dataset1.SetTitle ("UDP");
    dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dataset2.SetTitle ("TCP_highspeed");
    dataset2.SetStyle (Gnuplot2dDataset::LINES_POINTS);

	//bool tracing = false;
	// Set up some default values for the simulation.  Use the 
for(uint32_t maxBytes = 500;maxBytes<10500;maxBytes += 100){
	Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
	Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));
	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHighSpeed::GetTypeId ()));

	//DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

	// Allow the user to override any of the defaults and the above
	// DefaultValue::Bind ()s at run-time, via command-line arguments
	CommandLine cmd;
	bool enableFlowMonitor = false;
	cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
	cmd.Parse (argc, argv);
	// Here, we will explicitly create four nodes.  In more sophisticated
	// topologies, we could configure a node factory.
	NS_LOG_INFO ("Create nodes.");
	NodeContainer c;
	c.Create (6);
	NodeContainer n0n2 = NodeContainer (c.Get (0), c.Get (2));
	NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));
	NodeContainer n2n3 = NodeContainer (c.Get (2), c.Get (3));
	// NodeContainer n3n2 = NodeContainer (c.Get (3), c.Get (2));
	NodeContainer n3n4 = NodeContainer (c.Get (3), c.Get (4));
	NodeContainer n3n5 = NodeContainer (c.Get (3), c.Get (5));
	InternetStackHelper internet;
	internet.Install (c);

	// We create the channels first without any IP addressing information
	NS_LOG_INFO ("Create channels.");
	PointToPointHelper p2p;
	// p2p.SetDeviceAttribute ("DataRate", StringValue ("10000000"));
	p2p.SetDeviceAttribute ("DataRate", StringValue ("80Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));

	NetDeviceContainer d0d2 = p2p.Install (n0n2);
	NetDeviceContainer d1d2 = p2p.Install (n1n2);
	NetDeviceContainer d3d4 = p2p.Install (n3n4);
	NetDeviceContainer d3d5 = p2p.Install (n3n5);
	const uint16_t a = 64000;
	(*d1d2.Get(0)).SetMtu(a);
	(*d0d2.Get(0)).SetMtu(a);
	(*d3d4.Get(0)).SetMtu(a);
	(*d3d5.Get(0)).SetMtu(a);

	p2p.SetDeviceAttribute ("DataRate", StringValue ("30Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("100ms"));
	uint32_t qshr = (30*1024*1024)/(10*1024*8);
	p2p.SetQueue ("ns3::DropTailQueue", "MaxPackets" , UintegerValue(qshr));

	// NetDeviceContainer d3d2 = p2p.Install (n3n2);
	NetDeviceContainer d2d3 = p2p.Install (n2n3);
	(*d2d3.Get(0)).SetMtu(a);

	// Later, we add IP addresses.
	NS_LOG_INFO ("Assign IP Addresses.");
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i0i2 = ipv4.Assign (d0d2);
	ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);
	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer i2i3 = ipv4.Assign (d2d3);
	ipv4.SetBase ("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);
	ipv4.SetBase ("10.1.5.0", "255.255.255.0");
	Ipv4InterfaceContainer i3i5 = ipv4.Assign (d3d5);

	// Create router nodes, initialize routing database and set up the routing
	// tables in the nodes.
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	// Create the OnOff application to send UDP datagrams of size
	// 210 bytes at a rate of 448 Kb/s



	//holla niggers chose diffrent data rates here :P :P
	// uint32_t maxBytes = 1024;
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (maxBytes));

	NS_LOG_INFO ("Create Applications.");
	uint16_t port = 9;   // Discard port (RFC 863)
	OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (i3i4.GetAddress (1), port)));
	onoff.SetConstantRate (DataRate ("2Mb/s"));
	onoff.SetAttribute ("PacketSize", UintegerValue (maxBytes));
	ApplicationContainer apps = onoff.Install (c.Get (0));
	apps.Start (Seconds (0.0));
	apps.Stop (Seconds (5.0));

	// Create a packet sink to receive these packets
	PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	apps = sink.Install (c.Get (4));
	apps.Start (Seconds (0.0));
	apps.Stop (Seconds (5.0));
	onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	cbrsink = DynamicCast<PacketSink> (apps.Get (0));

	uint16_t port2 = 12344;  // well-known echo port number

	BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (i3i5.GetAddress (1), port2));
	source.SetAttribute ("MaxBytes", UintegerValue (0));
	ApplicationContainer sourceApps = source.Install (c.Get (1));
	sourceApps.Start (Seconds (5.0));
	sourceApps.Stop (Seconds (10.0));

	PacketSinkHelper sink2 ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port2));
	ApplicationContainer sinkApps = sink2.Install (c.Get (5));
	sinkApps.Start (Seconds (5.0));
	sinkApps.Stop (Seconds (10.0));
	tcpSink = DynamicCast<PacketSink> (sinkApps.Get (0));
//   // Trace changes to the congestion window
//   // ...and schedule the sending "Application"; This is similar to what an 
	// ns3::Application subclass would do internally.

	AsciiTraceHelper ascii;
	p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-global-routing.tr"));
	p2p.EnablePcapAll ("simple-global-routing");

	// Flow Monitor
	FlowMonitorHelper flowmonHelper;
	if (enableFlowMonitor){flowmonHelper.InstallAll ();}

	NS_LOG_INFO ("Run Simulation.");
	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();
	Simulator::Stop (Seconds (11));
	Simulator::Run ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();
	// std::fstream fs;
	// fs.open("/home/sour/Documents/ns-allinone-3.28/ns-3.28/z0.txt", std::fstream::in | std::fstream::out);
	// myfile.open ("example.txt");
	// ofstream myfile;
	// myfile.open ("example.txt", ios_base::openmode mode = ios_base::in | ios_base::out);
// myfile.open ("example.bin", ios::out );
	for(std::map<FlowId, FlowMonitor::FlowStats>::iterator i = stats.begin();i != stats.end();i++)
	{
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
		if(t.sourceAddress == "10.1.1.1"){
			double x = maxBytes;
			double y = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000000;
	  		dataset1.Add (x, y);
			
		}		
		if(t.sourceAddress == "10.1.2.1"){
			double x = maxBytes;
			double y = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000000;
	  		dataset2.Add (x, y);
		}
	}
	flowMonitor->SerializeToXmlFile("data.flowmon", true, true);
	Simulator::Destroy ();
}
	plot1.AddDataset (dataset1);
	plot2.AddDataset (dataset2);

	std::ofstream plotFile1 (plotFileName1.c_str());
	plot1.GenerateOutput (plotFile1);
	plotFile1.close ();

	std::ofstream plotFile2 (plotFileName2.c_str());
	plot2.GenerateOutput (plotFile2);
	plotFile2.close ();
	// plot2.AddDataset (dataset2);
	// std::ofstream plotFile (plotFileName2.c_str());
	// plot2.GenerateOutput (plotFile2);
	// plotFile2.close ();
	NS_LOG_INFO ("Done.");
	return 0;
}
