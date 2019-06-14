//
//  lora-state-helper.h
//  ns3-energy
//
//  Created by Cristiano Tapparello on 12/18/13.
//
//

#ifndef LORA_STATE_HELPER_H
#define LORA_STATE_HELPER_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/lora-phy.h"

#include "ns3/traced-callback.h"


namespace ns3 {
    //class LoraPhy;

    class LoraStateHelper : public Object
    {
    public:
        static TypeId GetTypeId (void);
        
        LoraStateHelper ();

        void SetReceiveOkCallback (LoraPhy::RxOkCallback callback);
        
        void RegisterListener (LoraPhyListener *listener);
        
        bool IsStateSleep (void);
        bool IsStateTx (void);
        bool IsStateRx (void);
        bool IsStateIdle (void);
        
        void SwitchToTx (Time txDuration, double txPowerDbm);
        void SwitchToRx (Time rxDuration);
        void SwitchFromRxEndOk (Ptr<Packet> packet/*, double snr*/);
        void SwitchFromRxEndError (Ptr<Packet> packet/*, double snr*/);
        void SwitchToSleep (void);
        void SwitchFromSleep (Time duration);
        
        LoraPhy::State GetState (void);

        /**
        * The trace source fired when state is changed.
        */
        TracedCallback<Time, Time, LoraPhy::State> m_stateLogger;
        
    private:

        /**
        * Notify all WifiPhyListener that the transmission has started for the given duration.
        *
        * \param duration the duration of the transmission
        * \param txPowerDbm the nominal tx power in dBm
        */
        void NotifyTxStart (Time duration, double txPowerDbm);
        /**
          * Notify all WifiPhyListener that the reception has started for the given duration.
          *
          * \param duration the duration of the reception
          */
        void NotifyRxStart (Time duration);
        /**
          * Notify all WifiPhyListener that the reception was successful.
          */
        void NotifyRxEndOk (void);
        /**
         * Notify all WifiPhyListener that the reception was not successful.
         */
        void NotifyRxEndError (void);
        /**
         * Notify all WifiPhyListener that we are going to sleep
         */
        void NotifySleep (void);
        /**
         * Notify all WifiPhyListener that we woke up
         */
        void NotifyWakeup (void);
        /**
         * Switch the state from RX.
         */
        void DoSwitchFromRx (void);
        
        typedef std::vector<LoraPhyListener *> Listeners;

        Listeners m_listeners;
        
        LoraPhy::RxOkCallback m_rxOkCallback;

        bool m_rxing; ///< receiving
        bool m_sleeping; ///< sleeping

        Time m_endTx; ///< end transmit
        Time m_endRx; ///< end receive
        Time m_startTx; ///< start transmit
        Time m_startRx; ///< start receives
        Time m_startIdle; ///< start Idle
        Time m_startSleep; ///< start sleep
        Time m_previousStateChangeTime; ///< previous state change time
        
    };
    

    
} // namespace ns3



#endif /* defined(LORA_STATE_HELPER) */
