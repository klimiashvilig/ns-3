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
 * Authors: Sidharth Nabar <snabar@uw.edu>
 *          He Wu <mdzz@u.washington.edu>
 */

#ifndef LORA_ENERGY_MODEL_H
#define LORA_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"
#include "lora-phy.h"

namespace ns3 {

//class LoraTxCurrentModel; //Remove

/**
 * \ingroup energy
 * A LoraPhy listener class for notifying the LoraEnergyModel of Lora
 * state change.
 *
 */
class LoraEnergyModelPhyListener : public LoraPhyListener
{
public:
  /**
   * Callback type for updating the transmit current based on the nominal tx power.
   */
  typedef Callback<void, double> UpdateTxCurrentCallback;

  LoraEnergyModelPhyListener ();
  virtual ~LoraEnergyModelPhyListener ();

  /**
   * \brief Sets the change state callback. Used by helper class.
   *
   * \param callback Change state callback.
   */
  void SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback);

  /**
   * \brief Sets the update tx current callback.
   *
   * \param callback Update tx current callback.
   */
  void SetUpdateTxCurrentCallback (UpdateTxCurrentCallback callback);

  /**
   * \brief Switches the LoraEnergyModel to RX state.
   *
   * \param duration the expected duration of the packet reception.
   *
   * Defined in ns3::LoraPhyListener
   */
  void NotifyRxStart (Time duration);

  /**
   * \brief Switches the LoraEnergyModel back to IDLE state.
   *
   * Defined in ns3::LoraPhyListener
   *
   * Note that for the LoraEnergyModel, the behavior of the function is the
   * same as NotifyRxEndError.
   */
  void NotifyRxEndOk (void);

  /**
   * \brief Switches the LoraEnergyModel back to IDLE state.
   *
   * Defined in ns3::LoraPhyListener
   *
   * Note that for the LoraEnergyModel, the behavior of the function is the
   * same as NotifyRxEndOk.
   */
  void NotifyRxEndError (void);

  /**
   * \brief Switches the LoraEnergyModel to TX state and switches back to
   * IDLE after TX duration.
   *
   * \param duration the expected transmission duration.
   * \param txPowerDbm the nominal tx power in dBm
   *
   * Defined in ns3::LoraPhyListener
   */
  void NotifyTxStart (Time duration, double txPowerDbm);

  /**
   * Defined in ns3::LoraPhyListener
   */
  void NotifySleep (void);

  /**
   * Defined in ns3::LoraPhyListener
   */
  void NotifyIdle (void);

  /**
   * Defined in ns3::LoraPhyListener
   */
  void NotifyWakeup (void);


private:
  /**
   * A helper function that makes scheduling m_changeStateCallback possible.
   */
  void SwitchToIdle (void);

  /**
   * Change state callback used to notify the LoraEnergyModel of a state
   * change.
   */
  DeviceEnergyModel::ChangeStateCallback m_changeStateCallback;

  /**
   * Callback used to update the tx current stored in LoraEnergyModel based on
   * the nominal tx power used to transmit the current frame.
   */
  UpdateTxCurrentCallback m_updateTxCurrentCallback;

  EventId m_switchToIdleEvent; ///< switch to idle event
};


/**
 * \ingroup energy
 * \brief A Lora radio energy model.
 *
 * 4 states are defined for the radio: TX, RX, IDLE, SLEEP. Default state is
 * IDLE.
 * The different types of transactions that are defined are:
 *  1. Tx: State goes from IDLE to TX, radio is in TX state for TX_duration,
 *     then state goes from TX to IDLE.
 *  2. Rx: State goes from IDLE to RX, radio is in RX state for RX_duration,
 *     then state goes from RX to IDLE.
 *  3. Go_to_Sleep: State goes from IDLE to SLEEP.
 *  4. End_of_Sleep: State goes from SLEEP to IDLE.
 * The class keeps track of what state the radio is currently in.
 *
 * Energy calculation: For each transaction, this model notifies EnergySource
 * object. The EnergySource object will query this model for the total current.
 * Then the EnergySource object uses the total current to calculate energy.
 *
 * Default values for power consumption are based on measurements reported in:
 *
 * Daniel Halperin, Ben Greenstein, Anmol Sheth, David Wetherall,
 * "Demystifying 802.11n power consumption", Proceedings of HotPower'10
 *
 * Power consumption in Watts (single antenna):
 *
 * \f$ P_{tx} = 1.14 \f$ (transmit at 0dBm)
 *
 * \f$ P_{rx} = 0.94 \f$
 *
 * \f$ P_{idle} = 0.82 \f$
 *
 * \f$ P_{sleep} = 0.10 \f$
 *
 * Hence, considering the default supply voltage of 3.0 V for the basic energy
 * source, the default current values in Ampere are:
 *
 * \f$ I_{tx} = 0.380 \f$
 *
 * \f$ I_{rx} = 0.313 \f$
 *
 * \f$ I_{idle} = 0.273 \f$
 *
 * \f$ I_{sleep} = 0.033 \f$
 *
 * The dependence of the power consumption in transmission mode on the nominal
 * transmit power can also be achieved through a Lora tx current model.
 *
 */
class LoraEnergyModel : public DeviceEnergyModel
{
public:
  /**
   * Callback type for energy depletion handling.
   */
  typedef Callback<void> LoraEnergyDepletionCallback;

  /**
   * Callback type for energy recharged handling.
   */
  typedef Callback<void> LoraEnergyRechargedCallback;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoraEnergyModel ();
  virtual ~LoraEnergyModel ();

  /**
   * \brief Sets pointer to EnergySouce installed on node.
   *
   * \param source Pointer to EnergySource installed on node.
   *
   * Implements DeviceEnergyModel::SetEnergySource.
   */
  void SetEnergySource (const Ptr<EnergySource> source);

  /**
   * \returns Total energy consumption of the Lora device.
   *
   * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
   */
  double GetTotalEnergyConsumption (void) const;

  // Setter & getters for state power consumption.
  /**
   * \brief Gets idle current.
   *
   * \returns idle current of the Lora device.
   */
  double GetIdleCurrentA (void) const;
  /**
   * \brief Sets idle current.
   *
   * \param idleCurrentA the idle current
   */
  void SetIdleCurrentA (double idleCurrentA);
  /**
   * \brief Gets transmit current.
   *
   * \returns transmit current of the Lora device.
   */
  double GetTxCurrentA (void) const;
  /**
   * \brief Sets transmit current.
   *
   * \param txCurrentA the transmit current
   */
  void SetTxCurrentA (double txCurrentA);
  /**
   * \brief Gets receive current.
   *
   * \returns receive current of the Lora device.
   */
  double GetRxCurrentA (void) const;
  /**
   * \brief Sets receive current.
   *
   * \param rxCurrentA the receive current
   */
  void SetRxCurrentA (double rxCurrentA);
  /**
   * \brief Gets sleep current.
   *
   * \returns sleep current of the Lora device.
   */
  double GetSleepCurrentA (void) const;
  /**
   * \brief Sets sleep current.
   *
   * \param sleepCurrentA the sleep current
   */
  void SetSleepCurrentA (double sleepCurrentA);

  /**
   * \returns Current state.
   */
  LoraPhy::State GetCurrentState (void) const;

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy depletion handling.
   */
  void SetEnergyDepletionCallback (LoraEnergyDepletionCallback callback);

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy recharged handling.
   */
  void SetEnergyRechargedCallback (LoraEnergyRechargedCallback callback);

  /**
   * \param model the model used to compute the Lora tx current.
   */
  //void SetTxCurrentModel (const Ptr<LoraTxCurrentModel> model); //Remove

  /**
   * \brief Calls the CalcTxCurrent method of the tx current model to
   *        compute the tx current based on such model
   *
   * \param txPowerDbm the nominal tx power in dBm
   */
  //void SetTxCurrentFromModel (double txPowerDbm); //Remove

  /**
   * \brief Changes state of the LoraEnergyMode.
   *
   * \param newState New state the Lora radio is in.
   *
   * Implements DeviceEnergyModel::ChangeState.
   */
  void ChangeState (int newState);

  /**
   * \brief Handles energy depletion.
   *
   * Implements DeviceEnergyModel::HandleEnergyDepletion
   */
  void HandleEnergyDepletion (void);

  /**
   * \brief Handles energy recharged.
   *
   * Implements DeviceEnergyModel::HandleEnergyRecharged
   */
  void HandleEnergyRecharged (void);

  /**
   * \brief Handles energy changed.
   *
   * Implements DeviceEnergyModel::HandleEnergyChanged
   */
  void HandleEnergyChanged (void);

  /**
   * \returns Pointer to the PHY listener.
   */
  LoraEnergyModelPhyListener * GetPhyListener (void);


private:
  void DoDispose (void);

  /**
   * \returns Current draw of device, at current state.
   *
   * Implements DeviceEnergyModel::GetCurrentA.
   */
  double DoGetCurrentA (void) const;

  /**
   * \param state New state the radio device is currently in.
   *
   * Sets current state. This function is private so that only the energy model
   * can change its own state.
   */
  void SetLoraState (const LoraPhy::State state);

  Ptr<EnergySource> m_source; ///< energy source

  // Member variables for current draw in different radio modes.
  double m_txCurrentA; ///< transmit current
  double m_rxCurrentA; ///< receive current
  double m_idleCurrentA; ///< idle current
  double m_sleepCurrentA; ///< sleep current
  //Ptr<LoraTxCurrentModel> m_txCurrentModel; ///< current model //Remove

  /// This variable keeps track of the total energy consumed by this model.
  TracedValue<double> m_totalEnergyConsumption;

  // State variables.
  LoraPhy::State m_currentState;  ///< current state the radio is in
  Time m_lastUpdateTime;          ///< time stamp of previous energy update

  uint8_t m_nPendingChangeState; ///< pending state change
  bool m_isSupersededChangeState; ///< superseded change state

  /// Energy depletion callback
  LoraEnergyDepletionCallback m_energyDepletionCallback;

  /// Energy recharged callback
  LoraEnergyRechargedCallback m_energyRechargedCallback;

  /// LoraPhy listener
  LoraEnergyModelPhyListener *m_listener;
};

} // namespace ns3

#endif /* LORA_ENERGY_MODEL_H */
