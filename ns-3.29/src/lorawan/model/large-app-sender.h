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

#ifndef LARGE_APP_SENDER_H
#define LARGE_APP_SENDER_H

#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/lora-mac.h"
#include "ns3/attribute.h"

#include "ns3/callback.h"

namespace ns3 {

class LargeAppSender : public Application {
public:

  LargeAppSender (int fileSize, int dataRate);
  ~LargeAppSender ();

  static TypeId GetTypeId (void);

  /**
   * Send a packet using the LoraNetDevice's Send method.
   */
  void SendPacket (void);

  /**
   * Set the file size.
   */
  void SetFileSize (int fileSize);

  /**
   * Start the application by scheduling the first SendPacket event.
   */
  void StartApplication (void);

  /**
   * Stop the application.
   */
  void StopApplication (void);

  void SendNextPacket(Ptr<const Packet> packet);

private:

  /**
   * The time at which to send the packet.
   */
  int file_size;

  /**
   * The sending event.
   */
  EventId m_sendEvent;

  /**
   * The MAC layer of this node.
   */
  Ptr<LoraMac> m_mac;
  /**
   * The data rate
   */
  int data_rate;

  /**
   * The maximum payload size
   */
  int maximum_payload;
};

} //namespace ns3

#endif /* ONE_SHOT_APPLICATION */
