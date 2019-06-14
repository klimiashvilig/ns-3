/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
 *
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/large-app-sender.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/lora-net-device.h"
#include "ns3/simulator.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LargeAppSender");

NS_OBJECT_ENSURE_REGISTERED (LargeAppSender);

TypeId
LargeAppSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LargeAppSender")
    .SetParent<Application> ()
    .AddConstructor<LargeAppSender> ()
    .SetGroupName ("lorawan");
  return tid;
}

LargeAppSender::LargeAppSender (int fileSize = 100, int dataRate = 6) :
  file_size (fileSize)
{
  NS_LOG_FUNCTION_NOARGS ();
  data_rate = dataRate;
  if (data_rate <= 2) maximum_payload = 59;
  else if (data_rate == 3) maximum_payload = 123;
  else maximum_payload = 230;
}

LargeAppSender::~LargeAppSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LargeAppSender::SetFileSize (int fileSize)
{
  NS_LOG_FUNCTION (this << fileSize);

  file_size = fileSize;
}

void
LargeAppSender::SendPacket (void)
{
  NS_LOG_FUNCTION (this);

  // Create and send a new packet
  Ptr<Packet> packet;
  if (file_size > maximum_payload) {
        packet = Create<Packet>(maximum_payload);
        file_size -= maximum_payload;
  }
  else if (file_size != 0) {
        packet = Create<Packet>(file_size);
        file_size = 0;
  }
  else return;
  m_mac->GetObject<EndDeviceLoraMac> ()->SetMType(LoraMacHeader::CONFIRMED_DATA_UP);
  m_mac->Send (packet);
  m_mac->GetPhy()->SetTxFinishedCallback(MakeCallback(&LargeAppSender::SendNextPacket, this));
}

void
LargeAppSender::SendNextPacket(Ptr<const Packet> packet) {
  Ptr<LoraNetDevice> loraNetDevice = m_node->GetDevice (0)->GetObject<LoraNetDevice> ();
  m_mac = loraNetDevice->GetMac ();
  NS_ASSERT (m_mac != 0);
  Ptr<EndDeviceLoraMac> endDeviceLoraMac = m_mac->GetObject<EndDeviceLoraMac> ();
  Time waitingTime = endDeviceLoraMac->GetWaitingTimeForTx();
  Simulator::Schedule (waitingTime, &LargeAppSender::SendPacket, this);
}

void
LargeAppSender::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // Make sure we have a MAC layer
  if (m_mac == 0)
    {
      // Assumes there's only one device
      Ptr<LoraNetDevice> loraNetDevice = m_node->GetDevice (0)->GetObject<LoraNetDevice> ();

      m_mac = loraNetDevice->GetMac ();
      NS_ASSERT (m_mac != 0);
      Ptr<EndDeviceLoraMac> endDeviceLoraMac = m_mac->GetObject<EndDeviceLoraMac> ();
      endDeviceLoraMac->SetDataRate(data_rate);
      //NS_LOG_UNCOND(data_rate);
    }

  // Schedule the next SendPacket event
  Simulator::Cancel (m_sendEvent);
  m_sendEvent = Simulator::Schedule (Seconds(0), &LargeAppSender::SendPacket,
                                     this);
}

void
LargeAppSender::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}
}
