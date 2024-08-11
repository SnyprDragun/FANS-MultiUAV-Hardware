 /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
3  * Copyright (c) 2012 University of Washington, 2012 INRIA
4  *
5  * This program is free software; you can redistribute it and/or modify
6  * it under the terms of the GNU General Public License version 2 as
7  * published by the Free Software Foundation;
8  *
9  * This program is distributed in the hope that it will be useful,
10  * but WITHOUT ANY WARRANTY; without even the implied warranty of
11  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
12  * GNU General Public License for more details.
13  *
14  * You should have received a copy of the GNU General Public License
15  * along with this program; if not, write to the Free Software
16  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
17  *
18  * Author: Alina Quereilhac <alina.quereilhac@inria.fr>
19  *
20  */
 
// +----------------------+     +-----------------------+
// |      client host     |     |      server host      |
// +----------------------+     +-----------------------+
// |     ns-3 Node 0      |     |      ns-3 Node 1      |
// |  +----------------+  |     |   +----------------+  |
// |  |    ns-3 TCP    |  |     |   |    ns-3 TCP    |  |
// |  +----------------+  |     |   +----------------+  |
// |  |    ns-3 IPv4   |  |     |   |    ns-3 IPv4   |  |
// |  +----------------+  |     |   +----------------+  |
// |  |   FdNetDevice  |  |     |   |   FdNetDevice  |  |
// |  |    10.1.1.1    |  |     |   |    10.1.1.2    |  |
// |  +----------------+  |     |   +----------------+  |
// |  |   raw socket   |  |     |   |   raw socket   |  |
// |  +----------------+  |     |   +----------------+  |
// |       | eth0 |       |     |        | eth0 |       |
// +-------+------+-------+     +--------+------+-------+
//
//         10.1.1.11                     10.1.1.12
//
//             |                            |
//             +----------------------------+
//
// This example is aimed at measuring the throughput of the FdNetDevice
// when using the EmuFdNetDeviceHelper. This is achieved by saturating
// the channel with TCP traffic. Then the throughput can be obtained from 
// the generated .pcap files.
//
// To run this example you will need two hosts (client & server).
// Steps to run the experiment:
//
// 1 - Connect the 2 computers with an Ethernet cable.
// 2 - Set the IP addresses on both Ethernet devices.
//
// client machine: $ sudo ip addr add dev eth0 10.1.1.11/24
// server machine: $ sudo ip addr add dev eth0 10.1.1.12/24
//
// 3 - Set both Ethernet devices to promiscuous mode.
//
// both machines: $ sudo ip link set wlan0 promisc on
//
// 4 - Give root suid to the raw socket creator binary.
//     If the --enable-sudo option was used to configure ns-3 with waf, then the following
//     step will not be necessary.
//
// both hosts: $ sudo chown root.root build/src/fd-net-device/ns3-dev-raw-sock-creator
// both hosts: $ sudo chmod 4755 build/src/fd-net-device/ns3-dev-raw-sock-creator
//
// 5 - Run the server side:
//
// server host: $ ./waf --run="fd-emu-onoff --serverMode=1"
//
// 6 - Run the client side:
//       
// client host: $ ./waf --run="fd-emu-onoff"
//
 
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
 
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/fd-net-device-module.h"

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "turtlesim/Pose.h"
#include "geometry_msgs/PoseStamped.h"
#include "mavros_msgs/CommandBool.h"
#include "mavros_msgs/CommandTOL.h"
#include "mavros_msgs/State.h"
#include "mavros_msgs/SetMode.h"

#include <sstream>

using namespace ns3;
mavros_msgs::State current_state_takeoff;
ros::Publisher pub;
std::string msg = "NOT CONNECTED";
bool flag = true;
bool connected;
uint32_t pktcount = 30;
Ptr<Socket> source;

NS_LOG_COMPONENT_DEFINE ("EmuFdNetDeviceSaturationExample");

static void GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize, uint32_t pktcount, Time pktInterval)
{
        if (pktcount > 0 && pktcount <31)
        {
                ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>((uint8_t*)::msg.c_str(),::msg.length());
                socket->Send(packet);

                //new code with ros
                //std::cout<<"mode"<<current_state_takeoff.mode<<std::endl;
                ros::spinOnce();
                if(current_state_takeoff.connected){
                        msg = current_state_takeoff.mode;
                }
                else{
                        msg = "Not connected";
                }
                
                std::cout<<"Sending data to uav4: "<<msg<<std::endl;
                std::cout<<"pktCount: "<<pktcount<<std::endl;
                Simulator::Schedule(pktInterval, &GenerateTraffic, socket, pktSize, pktcount - 1, pktInterval);
        }
        else
        {       
                std::cout<<"Closing Connection"<<std::endl;
                ros::spin();
                //socket->Close();
        }
}

void state_cb(const mavros_msgs::State& msg){
        current_state_takeoff = msg;
        // Simulator::Schedule(Seconds(1.0), &GenerateTraffic, source, 1000, 10, Seconds(1.0));
        std::cout<<"callback"<<std::endl;
        // Simulator::Run ();
        // std::cout<<"run"<<std::endl;
}

static void CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
        NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
        *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

void sendCall(Ptr<Socket>){

}

void success(Ptr<Socket>){
        std::cout<<"Lessgoo!"<<std::endl;
        connected = true;
}
 
void failure(Ptr<Socket>){
        std::cout<<"Oh no!"<<std::endl;
}

void handleRead(Ptr<Socket>){
        std::cout<<"Handled data read"<<std::endl;
}

int main (int argc, char *argv[])
{
        uint16_t sinkPort = 80;
        uint32_t packetSize = 1000; // bytes
        uint32_t pktcount = 30;
        double interval = 1.0;
        Time interPacketInterval = Seconds(interval);
        std::string dataRate("1000Mb/s");
        std::string deviceName ("wlan0");
        std::string client ("10.1.1.4");
        std::string server ("10.1.1.1");
        std::string netmask ("255.255.255.0");
        std::string macClient ("e8:4e:06:32:17:a6");
        std::string macServer("e8:4e:06:32:28:dd");
        
        ros::init(argc,argv,"NS3_pub");
        ros::NodeHandle nh;
        ros::Subscriber sub = nh.subscribe("/mavros/state",10,state_cb);
        pub = nh.advertise<std_msgs::String>("ns3_to_ros", 5);
        ros::Rate loop_rate(1);

 
        Ipv4Address remoteIp;
        Ipv4Address localIp;
        Mac48AddressValue localMac;
   
    
        remoteIp = Ipv4Address (client.c_str ());
        localIp = Ipv4Address (server.c_str ());
        localMac =  Mac48AddressValue (macServer.c_str ());
 
        Ipv4Mask localMask (netmask.c_str ());
        GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
        GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
        
        NS_LOG_INFO ("Create Node");
        Ptr<Node> node = CreateObject<Node> ();
        
        NS_LOG_INFO ("Create Device");
        EmuFdNetDeviceHelper emu;
        emu.SetDeviceName (deviceName);
        NetDeviceContainer devices = emu.Install(node);
        Ptr<NetDevice> device = devices.Get (0);
        device->SetAttribute ("Address", localMac);
 
        NS_LOG_INFO ("Add Internet Stack");
        InternetStackHelper internetStackHelper;
        internetStackHelper.SetIpv4StackInstall(true);
        internetStackHelper.Install (node);
 
        NS_LOG_INFO ("Create IPv4 Interface");
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
        uint32_t interface = ipv4->AddInterface (device);
        Ipv4InterfaceAddress address = Ipv4InterfaceAddress (localIp, localMask);
        ipv4->AddAddress (interface, address);
        ipv4->SetMetric (interface, 1);
        ipv4->SetUp (interface);

    
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        //Ptr<Socket> 
        source = Socket::CreateSocket(node, tid);
        InetSocketAddress remote = InetSocketAddress(remoteIp, sinkPort);

        source->Connect(remote);
        source->SetConnectCallback(MakeCallback(&success),MakeCallback(&failure));
        source->SetRecvCallback(MakeCallback(&handleRead));

        AsciiTraceHelper asciiTraceHelper;
        Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("sender_udp.cwnd");
        source->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream));

        std::cout<<"before"<<std::endl;
        
        Simulator::Schedule(Seconds(1.0), &GenerateTraffic, source, packetSize, pktcount, interPacketInterval);
        std::cout<<"after"<<std::endl;
        emu.EnablePcap ("3nt-source", device); //name of pcap file that you should look for
        //ros::spin();
        Simulator::Run ();
        std::cout<<"run"<<std::endl;
        
        Simulator::Stop (Seconds (1.0));
        std::cout<<"stop"<<std::endl;
        Simulator::Destroy ();
        std::cout<<"destroy"<<std::endl;
        

        return 0;
}
