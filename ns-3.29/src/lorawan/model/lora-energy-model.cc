/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/energy-source.h"
#include "lora-energy-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (LoraEnergyModel);

TypeId
LoraEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .SetGroupName ("Energy")
    .AddConstructor<LoraEnergyModel> ()
    .AddAttribute ("IdleCurrentA",
                   "The default radio Idle current in Ampere.",
                   DoubleValue (0.0000015),  // idle mode = 1.5µA
                   MakeDoubleAccessor (&LoraEnergyModel::SetIdleCurrentA,
                                       &LoraEnergyModel::GetIdleCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentA",
                   "The radio Tx current in Ampere.",
                   DoubleValue (0.090),    // transmition mode = 90mA
                   MakeDoubleAccessor (&LoraEnergyModel::SetTxCurrentA,
                                       &LoraEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current in Ampere.",
                   DoubleValue (0.011),    // receive mode = 11mA
                   MakeDoubleAccessor (&LoraEnergyModel::SetRxCurrentA,
                                       &LoraEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SleepCurrentA",
                   "The radio Sleep current in Ampere.",
                   DoubleValue (0.0000001),  // sleep mode = 0.1µA
                   MakeDoubleAccessor (&LoraEnergyModel::SetSleepCurrentA,
                                       &LoraEnergyModel::GetSleepCurrentA),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoraEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValueCallback::Double")
  ; 
  return tid;
}

LoraEnergyModel::LoraEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_currentState = LoraPhy::IDLE;  // initially IDLE
  m_lastUpdateTime = Seconds (0.0);
  m_nPendingChangeState = 0;
  m_isSupersededChangeState = false;
  m_energyDepletionCallback.Nullify ();
  m_source = NULL;
  // set callback for LoraPhy listener
  m_listener = new LoraEnergyModelPhyListener;
  m_listener->SetChangeStateCallback (MakeCallback (&DeviceEnergyModel::ChangeState, this));
  // set callback for updating the tx current
  //m_listener->SetUpdateTxCurrentCallback (MakeCallback (&LoraEnergyModel::SetTxCurrentFromModel, this));//Remove
}

LoraEnergyModel::~LoraEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  delete m_listener;
}

void
LoraEnergyModel::SetEnergySource (const Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

double
LoraEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

double
LoraEnergyModel::GetIdleCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_idleCurrentA;
}

void
LoraEnergyModel::SetIdleCurrentA (double idleCurrentA)
{
  NS_LOG_FUNCTION (this << idleCurrentA);
  m_idleCurrentA = idleCurrentA;
}

double
LoraEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txCurrentA;
}

void
LoraEnergyModel::SetTxCurrentA (double txCurrentA)
{
  NS_LOG_FUNCTION (this << txCurrentA);
  m_txCurrentA = txCurrentA;
}

double
LoraEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxCurrentA;
}

void
LoraEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_rxCurrentA = rxCurrentA;
}

double
LoraEnergyModel::GetSleepCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sleepCurrentA;
}

void
LoraEnergyModel::SetSleepCurrentA (double sleepCurrentA)
{
  NS_LOG_FUNCTION (this << sleepCurrentA);
  m_sleepCurrentA = sleepCurrentA;
}

LoraPhy::State
LoraEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
LoraEnergyModel::SetEnergyDepletionCallback (
  LoraEnergyDepletionCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("LoraEnergyModel:Setting NULL energy depletion callback!");
    }
  m_energyDepletionCallback = callback;
}

void
LoraEnergyModel::SetEnergyRechargedCallback (
  LoraEnergyRechargedCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("LoraEnergyModel:Setting NULL energy recharged callback!");
    }
  m_energyRechargedCallback = callback;
}

void
LoraEnergyModel::ChangeState (int newState)
{
  NS_LOG_FUNCTION (this << newState);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  switch (m_currentState)
    {
    case LoraPhy::IDLE:
      energyToDecrease = duration.GetSeconds () * m_idleCurrentA * supplyVoltage;
      break;
    case LoraPhy::TX:
      energyToDecrease = duration.GetSeconds () * m_txCurrentA * supplyVoltage;
      break;
    case LoraPhy::RX:
      energyToDecrease = duration.GetSeconds () * m_rxCurrentA * supplyVoltage;
      break;
    case LoraPhy::SLEEP:
      energyToDecrease = duration.GetSeconds () * m_sleepCurrentA * supplyVoltage;
      break;
    default:
      NS_FATAL_ERROR ("LoraEnergyModel:Undefined radio state: " << m_currentState);
    }

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();

  m_nPendingChangeState++;

  // notify energy source
  m_source->UpdateEnergySource ();

  // in case the energy source is found to be depleted during the last update, a callback might be
  // invoked that might cause a change in the Lora PHY state (e.g., the PHY is put into SLEEP mode).
  // This in turn causes a new call to this member function, with the consequence that the previous
  // instance is resumed after the termination of the new instance. In particular, the state set
  // by the previous instance is erroneously the final state stored in m_currentState. The check below
  // ensures that previous instances do not change m_currentState.

  if (!m_isSupersededChangeState)
    {
      // update current state & last update time stamp
      SetLoraState ((LoraPhy::State) newState);

      // some debug message
      NS_LOG_DEBUG ("LoraEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }

  m_isSupersededChangeState = (m_nPendingChangeState > 1);

  m_nPendingChangeState--;
}

void
LoraEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoraEnergyModel:Energy is depleted!");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      m_energyDepletionCallback ();
    }
}

void
LoraEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoraEnergyModel:Energy is recharged!");
  // invoke energy recharged callback, if set.
  if (!m_energyRechargedCallback.IsNull ())
    {
      m_energyRechargedCallback ();
    }
}

void
LoraEnergyModel::HandleEnergyChanged (void)
{
  NS_LOG_FUNCTION (this);
}

LoraEnergyModelPhyListener *
LoraEnergyModel::GetPhyListener (void)
{
  NS_LOG_FUNCTION (this);
  return m_listener;
}

/*
 * Private functions start here.
 */

void
LoraEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
  m_energyDepletionCallback.Nullify ();
}

double
LoraEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  switch (m_currentState)
    {
    case LoraPhy::IDLE:
      return m_idleCurrentA;
    case LoraPhy::TX:
      return m_txCurrentA;
    case LoraPhy::RX:
      return m_rxCurrentA;
    case LoraPhy::SLEEP:
      return m_sleepCurrentA;
    default:
      NS_FATAL_ERROR ("LoraEnergyModel:Undefined radio state:" << m_currentState);
    }
}

void
LoraEnergyModel::SetLoraState (const LoraPhy::State state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;
  switch (state)
    {
    case LoraPhy::IDLE:
      stateName = "IDLE";
      break;
    case LoraPhy::TX:
      stateName = "TX";
      break;
    case LoraPhy::RX:
      stateName = "RX";
      break;
    case LoraPhy::SLEEP:
      stateName = "SLEEP";
      break;
    }
  NS_LOG_DEBUG ("LoraEnergyModel:Switching to state: " << stateName <<
                " at time = " << Simulator::Now ());
}

// -------------------------------------------------------------------------- //

LoraEnergyModelPhyListener::LoraEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
  m_changeStateCallback.Nullify ();
  m_updateTxCurrentCallback.Nullify ();
}

LoraEnergyModelPhyListener::~LoraEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
}

void
LoraEnergyModelPhyListener::SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
  NS_ASSERT (!callback.IsNull ());
  m_changeStateCallback = callback;
}

void
LoraEnergyModelPhyListener::SetUpdateTxCurrentCallback (UpdateTxCurrentCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
  NS_ASSERT (!callback.IsNull ());
  m_updateTxCurrentCallback = callback;
}

void
LoraEnergyModelPhyListener::NotifyRxStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::RX);
  m_switchToIdleEvent.Cancel ();
}

void
LoraEnergyModelPhyListener::NotifyRxEndOk (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::IDLE);
}

void
LoraEnergyModelPhyListener::NotifyRxEndError (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::IDLE);
}

void
LoraEnergyModelPhyListener::NotifyTxStart (Time duration, double txPowerDbm)
{
  NS_LOG_FUNCTION (this << duration << txPowerDbm);
  //m_updateTxCurrentCallback (txPowerDbm);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::TX);
  // schedule changing state back to IDLE after TX duration
  m_switchToIdleEvent.Cancel ();
  m_switchToIdleEvent = Simulator::Schedule (duration, &LoraEnergyModelPhyListener::SwitchToIdle, this);
}

void
LoraEnergyModelPhyListener::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::SLEEP);
  m_switchToIdleEvent.Cancel ();
}

void
LoraEnergyModelPhyListener::NotifyIdle (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::IDLE);
  //m_switchToIdleEvent = Simulator::Schedule (duration, &LoraEnergyModelPhyListener::SwitchToIdle, this);
}

void
LoraEnergyModelPhyListener::NotifyWakeup(void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::IDLE);
}

/*
 * Private function state here.
 */

void
LoraEnergyModelPhyListener::SwitchToIdle (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (LoraPhy::IDLE);
}

} // namespace ns3
