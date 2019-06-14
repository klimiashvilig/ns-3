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

#include "ns3/large-app-sender-helper.h"
#include "ns3/large-app-sender.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LargeAppSenderHelper");

LargeAppSenderHelper::LargeAppSenderHelper ()
{
  m_factory.SetTypeId ("ns3::LargeAppSender");
  file_size = 100;
}

LargeAppSenderHelper::~LargeAppSenderHelper ()
{
}

void
LargeAppSenderHelper::SetFileSize (int fileSize)// change to setfilesize
{
  file_size = fileSize;
}

int
LargeAppSenderHelper::GetFileSize ()// added
{
  return file_size;
}

void
LargeAppSenderHelper::SetAttribute (std::string name,
                                   const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
LargeAppSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LargeAppSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LargeAppSenderHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<LargeAppSender> app = m_factory.Create<LargeAppSender> ();

  app->SetFileSize (file_size); // change

  app->SetNode (node);
  node->AddApplication (app);

  return app;
}
} // namespace ns3
