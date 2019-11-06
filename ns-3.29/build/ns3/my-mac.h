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
 * Author: Vishwesh Rege <vrege2012@gmail.com>
 */
#ifndef MY_MAC
#define MY_MAC

#include "lr-wpan-mac.h"


namespace ns3 {

/**
 * \ingroup lr-wpan
 *
 * Class that implements the LR-WPAN Mac state machine
 */
class MyMac : public LrWpanMac
{
public:

  /**
   * Get the type ID.
   *
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  MyMac (void);
  virtual ~MyMac (void);

  // Override some base MAC functions.
  virtual void McpsDataRequest (McpsDataRequestParams params, Ptr<Packet> p);
  virtual void PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t lqi);
  void SetLrWpanMacState (LrWpanMacState macState);
  // virtual void SendAck (uint8_t seqno);

protected:
  // Inherited from Object.
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

private:

  LrWpanPhyEnumeration m_currentState;  // current state the radio is in
  /**
   * \ingroup lr-wpan
   * \brief Neighbor description.
   *
   * This structure takes into account the known neighbors and their wakeup time (if known).
   */

  /**
   * Implements ContikiMAC sleep mechanism
   *
   */
  void Sleep (void);

  /**
   * Implements ContikiMAC wakeup mechanism
   *
   */
  void WakeUp (void);

  /**
   * Sets current state.
   */
  void SetLrWpanRadioState (const LrWpanPhyEnumeration state);

  // void ChangeAckSent();
  /**
   * The number of already used retransmission for the currently transmitted
   * packet.
   */
  uint8_t m_rdcRetries;

  /**
   * Scheduler event of a scheduled WakeUp.
   */
  EventId m_wakeUp;

  /**
   * Scheduler event of a scheduled Sleep.
   */
  EventId m_sleep;

  /**
   * Scheduler event of a repeated data packet.
   */
  EventId m_repeatPkt;

  // bool ackSent;

};


} // namespace ns3

#endif /* MY_MAC */
