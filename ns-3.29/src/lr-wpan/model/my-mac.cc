/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: George Klimiashvili <gklimias@u.rochester.edu>
 */
#include "my-mac.h"
#include "lr-wpan-csmaca.h"
#include "lr-wpan-mac-header.h"
#include "lr-wpan-mac-trailer.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/random-variable-stream.h>
#include <ns3/double.h>

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                   \
  std::clog << "[address " << GetShortAddress () << "] ";

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyMac");

NS_OBJECT_ENSURE_REGISTERED (MyMac);

TypeId
MyMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyMac")
    .SetParent<LrWpanMac> ()
    .SetGroupName ("LrWpan")
    .AddConstructor<MyMac> ()
  ;
  return tid;
}

MyMac::MyMac ()
{
  m_rdcRetries = 0;
}

MyMac::~MyMac ()
{
}

void
MyMac::DoInitialize ()
{
  //Sleep
  m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);

  LrWpanMac::DoInitialize ();
}

void
MyMac::DoDispose ()
{
  if (m_wakeUp.IsRunning ())
    {
      m_wakeUp.Cancel ();
    }

  LrWpanMac::DoDispose ();
}

void
MyMac::Sleep ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sleep.IsExpired ());
  if (m_lrWpanMacState == MAC_IDLE
      && (m_phy->GetTRXState () == IEEE_802_15_4_PHY_RX_ON || m_phy->GetTRXState () == IEEE_802_15_4_PHY_TX_ON))   //MAC_IDLE and TRX_ON
  {
    NS_LOG_UNCOND("Sleeping...");
    m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);
  }
  else {
    NS_LOG_UNCOND("Could not switch to sleep");
  }
}

void
MyMac::WakeUp ()
{
  NS_LOG_FUNCTION (this);

  if (m_phy->GetTRXState () == IEEE_802_15_4_PHY_TRX_OFF)
  {
    NS_LOG_INFO("Waking up!");
    m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TX_ON);
  }
  else
    NS_LOG_INFO("Radio is already awake!");
}

void
MyMac::McpsDataRequest (McpsDataRequestParams params, Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p << p->GetSize() << Simulator::Now());

  WakeUp();

  LrWpanMac::McpsDataRequest (params, p);
}

void
MyMac::PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t lqi)
{
  NS_ASSERT (m_lrWpanMacState == MAC_IDLE || m_lrWpanMacState == MAC_ACK_PENDING || m_lrWpanMacState == MAC_CSMA);

  NS_LOG_FUNCTION (this << psduLength << p << (uint16_t)lqi);

  bool acceptFrame;

  // from sec 7.5.6.2 Reception and rejection, Std802.15.4-2006
  // level 1 filtering, test FCS field and reject if frame fails
  // level 2 filtering if promiscuous mode pass frame to higher layer otherwise perform level 3 filtering
  // level 3 filtering accept frame
  // if Frame type and version is not reserved, and
  // if there is a dstPanId then dstPanId=m_macPanId or broadcastPanI, and
  // if there is a shortDstAddr then shortDstAddr =shortMacAddr or broadcastAddr, and
  // if beacon frame then srcPanId = m_macPanId
  // if only srcAddr field in Data or Command frame,accept frame if srcPanId=m_macPanId

  Ptr<Packet> originalPkt = p->Copy (); // because we will strip headers

  m_promiscSnifferTrace (originalPkt);

  m_macPromiscRxTrace (originalPkt);
  // XXX no rejection tracing (to macRxDropTrace) being performed below

  LrWpanMacTrailer receivedMacTrailer;
  p->RemoveTrailer (receivedMacTrailer);
  if (Node::ChecksumEnabled ())
  {
    receivedMacTrailer.EnableFcs (true);
  }

  // level 1 filtering
  if (!receivedMacTrailer.CheckFcs (p))
  {
    m_macRxDropTrace (originalPkt);
  }
  else
  {
    LrWpanMacHeader receivedMacHdr;
    p->RemoveHeader (receivedMacHdr);

    McpsDataIndicationParams params;
    params.m_dsn = receivedMacHdr.GetSeqNum ();
    params.m_mpduLinkQuality = lqi;
    params.m_srcPanId = receivedMacHdr.GetSrcPanId ();
    params.m_srcAddrMode = receivedMacHdr.GetSrcAddrMode ();
    switch (params.m_srcAddrMode)
    {
      case SHORT_ADDR:
        params.m_srcAddr = receivedMacHdr.GetShortSrcAddr ();
        NS_LOG_DEBUG ("Packet from " << params.m_srcAddr);
        break;
      case EXT_ADDR:
        params.m_srcExtAddr = receivedMacHdr.GetExtSrcAddr ();
        NS_LOG_DEBUG ("Packet from " << params.m_srcExtAddr);
        break;
      default:
        break;
    }
    params.m_dstPanId = receivedMacHdr.GetDstPanId ();
    params.m_dstAddrMode = receivedMacHdr.GetDstAddrMode ();
    switch (params.m_dstAddrMode)
    {
      case SHORT_ADDR:
        params.m_dstAddr = receivedMacHdr.GetShortDstAddr ();
        NS_LOG_DEBUG ("Packet to " << params.m_dstAddr);
        break;
      case EXT_ADDR:
        params.m_dstExtAddr = receivedMacHdr.GetExtDstAddr ();
        NS_LOG_DEBUG ("Packet to " << params.m_dstExtAddr);
        break;
      default:
        break;
    }

    if (m_macPromiscuousMode)
    {
      //level 2 filtering
      if (!m_mcpsDataIndicationCallback.IsNull ())
      {
        NS_LOG_DEBUG ("promiscuous mode, forwarding up");
        m_mcpsDataIndicationCallback (params, p);
      }
      else
      {
        NS_LOG_ERROR (this << " Data Indication Callback not initialised");
      }
    }
    else
    {
      //level 3 frame filtering
      acceptFrame = (receivedMacHdr.GetType () != LrWpanMacHeader::LRWPAN_MAC_RESERVED);

      if (acceptFrame)
      {
        acceptFrame = (receivedMacHdr.GetFrameVer () <= 1);
      }

      if (acceptFrame && (receivedMacHdr.GetDstAddrMode () > 1))
      {
        acceptFrame = receivedMacHdr.GetDstPanId () == m_macPanId
          || receivedMacHdr.GetDstPanId () == 0xffff;
      }

      if (acceptFrame && (receivedMacHdr.GetDstAddrMode () == 2))
      {
        acceptFrame = receivedMacHdr.GetShortDstAddr () == m_shortAddress
          || receivedMacHdr.GetShortDstAddr () == Mac16Address ("ff:ff");        // check for broadcast addrs
      }

      if (acceptFrame && (receivedMacHdr.GetDstAddrMode () == 3))
      {
        acceptFrame = (receivedMacHdr.GetExtDstAddr () == m_selfExt);
      }

      if (acceptFrame && (receivedMacHdr.GetType () == LrWpanMacHeader::LRWPAN_MAC_BEACON))
      {
        if (m_macPanId == 0xffff)
        {
          // TODO: Accept only if the frame version field is valid
          acceptFrame = true;
        }
        else
        {
          acceptFrame = receivedMacHdr.GetSrcPanId () == m_macPanId;
        }
      }

      if (acceptFrame && ((receivedMacHdr.GetType () == LrWpanMacHeader::LRWPAN_MAC_DATA)
          || (receivedMacHdr.GetType () == LrWpanMacHeader::LRWPAN_MAC_COMMAND)) && (receivedMacHdr.GetSrcAddrMode () > 1))
      {
        acceptFrame = receivedMacHdr.GetSrcPanId () == m_macPanId; // \todo need to check if PAN coord
      }

      if (acceptFrame)
      {
        m_macRxTrace (originalPkt);
        // \todo: What should we do if we receive a frame while waiting for an ACK?
        //        Especially if this frame has the ACK request bit set, should we reply with an ACK, possibly missing the pending ACK?

        // If the received frame is a frame with the ACK request bit set, we immediately send back an ACK.
        // If we are currently waiting for a pending ACK, we assume the ACK was lost and trigger a retransmission after sending the ACK.
        if ((receivedMacHdr.IsData () || receivedMacHdr.IsCommand ()) && receivedMacHdr.IsAckReq ()
          && !(receivedMacHdr.GetDstAddrMode () == SHORT_ADDR && receivedMacHdr.GetShortDstAddr () == "ff:ff"))
        {
          // If this is a data or mac command frame, which is not a broadcast,
          // with ack req set, generate and send an ack frame.
          // If there is a CSMA medium access in progress we cancel the medium access
          // for sending the ACK frame. A new transmission attempt will be started
          // after the ACK was send.
          if (m_lrWpanMacState == MAC_ACK_PENDING)
          {
            m_ackWaitTimeout.Cancel ();
            PrepareRetransmission ();
          }
          else if (m_lrWpanMacState == MAC_CSMA)
          {
            // \todo: If we receive a packet while doing CSMA/CA, should  we drop the packet because of channel busy,
            //        or should we restart CSMA/CA for the packet after sending the ACK?
            // Currently we simply restart CSMA/CA after sending the ACK.
            m_csmaCa->Cancel ();
          }
          // Cancel any pending MAC state change, ACKs have higher priority.
          m_setMacState.Cancel ();
          ChangeMacState (MAC_IDLE);
          m_setMacState = Simulator::ScheduleNow (&MyMac::SendAck, this, receivedMacHdr.GetSeqNum ());
        }
        if (receivedMacHdr.IsData () && !m_mcpsDataIndicationCallback.IsNull ())
        {
          // If it is a data frame, push it up the stack.
          NS_LOG_DEBUG ("PdDataIndication():  Packet is for me; forwarding up");
          m_mcpsDataIndicationCallback (params, p);
        }
        else if (receivedMacHdr.IsAcknowledgment () && m_txPkt && m_lrWpanMacState == MAC_ACK_PENDING)
        {
          std::cout << m_phy << " Acked" << std::endl;
          NS_LOG_DEBUG("Acked");
          LrWpanMacHeader macHdr;
          m_txPkt->PeekHeader (macHdr);
          if (receivedMacHdr.GetSeqNum () == macHdr.GetSeqNum ())
          {
            m_macTxOkTrace (m_txPkt);
            // If it is an ACK with the expected sequence number, finish the transmission
            // and notify the upper layer.
            m_ackWaitTimeout.Cancel ();
            if (!m_mcpsDataConfirmCallback.IsNull ())
            {
              TxQueueElement *txQElement = m_txQueue.front ();
              McpsDataConfirmParams confirmParams;
              confirmParams.m_msduHandle = txQElement->txQMsduHandle;
              confirmParams.m_status = IEEE_802_15_4_SUCCESS;
              m_mcpsDataConfirmCallback (confirmParams);
            }
            RemoveFirstTxQElement ();
            m_setMacState.Cancel ();
            m_setMacState = Simulator::ScheduleNow (&MyMac::SetLrWpanMacState, this, MAC_IDLE);
          }
          else
          {
            // If it is an ACK with an unexpected sequence number, mark the current transmission as failed and start a retransmit. (cf 7.5.6.4.3)
            m_ackWaitTimeout.Cancel ();
            if (!PrepareRetransmission ())
            {
              m_setMacState.Cancel ();
              m_setMacState = Simulator::ScheduleNow (&MyMac::SetLrWpanMacState, this, MAC_IDLE);
            }
            else
            {
              m_setMacState.Cancel ();
              m_setMacState = Simulator::ScheduleNow (&MyMac::SetLrWpanMacState, this, MAC_CSMA);
            }
          }
        }   //IsAcknowledgment
        //Simulator::ScheduleNow (&MyMac::Sleep, this);
      }       //accepetFrame
      else
      {
        m_macRxDropTrace (originalPkt);
      }
    }           //!m_macPromiscuousMode
  }
}

void
MyMac::SetLrWpanMacState (LrWpanMacState macState)
{
  NS_LOG_FUNCTION (this << "mac state = " << macState);

  McpsDataConfirmParams confirmParams;

  if (macState == MAC_IDLE)
    {
      ChangeMacState (MAC_IDLE);
      if (m_macRxOnWhenIdle)
        {
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_RX_ON);
        }
      else
        {
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);
        }

      CheckQueue ();
    }
  else if (macState == MAC_ACK_PENDING)
    {
      ChangeMacState (MAC_ACK_PENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_RX_ON);
    }
  else if (macState == MAC_CSMA)
    {
      NS_ASSERT (m_lrWpanMacState == MAC_IDLE || m_lrWpanMacState == MAC_ACK_PENDING);

      ChangeMacState (MAC_CSMA);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_RX_ON);
    }
  else if (m_lrWpanMacState == MAC_CSMA && macState == CHANNEL_IDLE)
    {
      // Channel is idle, set transmitter to TX_ON
      ChangeMacState (MAC_SENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TX_ON);
    }
  else if (m_lrWpanMacState == MAC_CSMA && macState == CHANNEL_ACCESS_FAILURE)
    {
      NS_ASSERT (m_txPkt);

      // cannot find a clear channel, drop the current packet.
      NS_LOG_DEBUG ( this << " cannot find clear channel");
      confirmParams.m_msduHandle = m_txQueue.front ()->txQMsduHandle;
      confirmParams.m_status = IEEE_802_15_4_CHANNEL_ACCESS_FAILURE;
      m_macTxDropTrace (m_txPkt);
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      // remove the copy of the packet that was just sent
      RemoveFirstTxQElement ();

      ChangeMacState (MAC_IDLE);
    }
}

void
MyMac::SetLrWpanRadioState (const LrWpanPhyEnumeration state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;
  switch (state)
    {
    case IEEE_802_15_4_PHY_TX_ON:
      stateName = "TX_ON";
      break;
    case IEEE_802_15_4_PHY_RX_ON:
      stateName = "RX_ON";
      break;
    case IEEE_802_15_4_PHY_TRX_OFF:
      stateName = "TRX_OFF";
      break;
    case IEEE_802_15_4_PHY_UNSPECIFIED:
      stateName = "TRANSITION";
      break;
    default:
      NS_FATAL_ERROR ("LrWpanRadioEnergyModel:Undefined radio state:" << m_currentState);
    }
  NS_LOG_DEBUG ("MyMac:Switching to state: " << stateName << " at time = " << Simulator::Now ());
}

} // namespace ns3
