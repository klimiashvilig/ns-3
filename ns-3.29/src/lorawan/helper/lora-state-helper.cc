//
//  lora-state-helper.cc
//  ns3-energy
//
//  Created by Cristiano Tapparello on 12/18/13.
//
//

#include "lora-state-helper.h"

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"

NS_LOG_COMPONENT_DEFINE ("LoraStateHelper");

namespace ns3 {
    
    TypeId
    LoraStateHelper::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::LoraStateHelper")
            .SetParent<Object> ()
            .SetGroupName ("Lora")
            .AddConstructor<LoraStateHelper> ()
            .AddTraceSource ("State",
                                "The state of the Lora",
                                MakeTraceSourceAccessor (&LoraStateHelper::m_stateLogger),
                                "ns3::LoraPhyStateHelper::StateTracedCallback")
//            .AddTraceSource ("Reading", "Reading is starting.",
//                             MakeTraceSourceAccessor (&LoraStateHelper::m_readingTrace))
        ;
        return tid;
    }
    
    LoraStateHelper::LoraStateHelper ()
    : m_rxing (false),
      m_sleeping (false),
      m_endTx (Seconds (0)),
      m_endRx (Seconds (0)),
      m_startTx (Seconds (0)),
      m_startRx (Seconds (0)),
      m_startIdle (Seconds (0)),
      m_startSleep (Seconds (0))
    {
        NS_LOG_FUNCTION (this);
    }
    
    bool LoraStateHelper::IsStateIdle (void)
    {
        return (GetState () == LoraPhy::IDLE);
    }
    
    bool LoraStateHelper::IsStateSleep (void)
    {
        return (GetState () == LoraPhy::SLEEP);
    }
    
    bool LoraStateHelper::IsStateTx (void)
    {
        return (GetState () == LoraPhy::TX);
    }
    
    bool LoraStateHelper::IsStateRx (void)
    {
        return (GetState () == LoraPhy::RX);
    }
    
    void
    LoraStateHelper::SetReceiveOkCallback (LoraPhy::RxOkCallback callback)
    {
        m_rxOkCallback = callback;
    }
    
    void LoraStateHelper::RegisterListener (LoraPhyListener *listener)
    {
        m_listeners.push_back (listener);
        /*for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            NS_LOG_UNCOND("-----In the Loop-----");
            (*i)->NotifySleep();
        }*/
    }
    
    void
    LoraStateHelper::SwitchToTx (Time txDuration, double txPowerDbm)
    {
        NS_LOG_FUNCTION (this << txDuration <<txPowerDbm);
        Time now = Simulator::Now ();
        switch (GetState ())
        {
            case LoraPhy::RX:
                /* The packet which is being received as well
                 * as its endRx event are cancelled by the caller.
                 */
                m_rxing = false;
                m_stateLogger (m_startRx, now - m_startRx, LoraPhy::RX);
                m_endRx = now;
                break;
            case LoraPhy::IDLE:
                break;
            case LoraPhy::SLEEP:
                m_sleeping = false;
                break;
            case LoraPhy::TX:
                break;
            default:
                NS_FATAL_ERROR ("Invalid LoraPhy state.");
                break;
        }
        m_stateLogger (now, txDuration, LoraPhy::TX);
        m_previousStateChangeTime = now;
        m_endTx = now + txDuration;
        m_startTx = now;
        NotifyTxStart (txDuration, txPowerDbm);
    }
    
    void
    LoraStateHelper::SwitchToRx (Time rxDuration)
    {
        NS_LOG_FUNCTION (this << rxDuration);
        //NS_ASSERT(!m_rxing);
        //NS_ASSERT (IsStateIdle ());
        Time now = Simulator::Now ();
        if (GetState() == LoraPhy::SLEEP)
            NS_FATAL_ERROR ("Invalid LoraPhy state.");
        m_previousStateChangeTime = now;
        m_rxing = true;
        m_startRx = now;
        m_endRx = now + rxDuration;
        NotifyRxStart (rxDuration);
        NS_ASSERT (IsStateRx ());
    }

    void
    LoraStateHelper::SwitchFromRxEndOk (Ptr<Packet> packet/*, double snr*/)
    {
        NS_LOG_FUNCTION (this << packet/* << snr*/);
        NotifyRxEndOk ();
        DoSwitchFromRx ();
        if (!m_rxOkCallback.IsNull ())
        {
            m_rxOkCallback (packet/*, snr*/);
        }
    }

    void
    LoraStateHelper::SwitchFromRxEndError (Ptr<Packet> packet/*, double snr*/)
    {
      NS_LOG_FUNCTION (this << packet/* << snr*/);
      NotifyRxEndError ();
      DoSwitchFromRx ();
    }
    
    void
    LoraStateHelper::SwitchToSleep (void)
    {
        NS_LOG_FUNCTION (this);
        Time now = Simulator::Now ();
        if (GetState() == LoraPhy::SLEEP)
            NS_FATAL_ERROR ("Invalid LoraPhy state.");
        m_previousStateChangeTime = now;
        m_startSleep = now;
        m_sleeping = true;
        NotifySleep ();
        NS_ASSERT (IsStateSleep ());
    }

    void
    LoraStateHelper::SwitchFromSleep (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
        NS_ASSERT (IsStateSleep ());
        Time now = Simulator::Now ();
        m_stateLogger (m_startSleep, now - m_startSleep, LoraPhy::SLEEP);
        m_previousStateChangeTime = now;
        m_sleeping = false;
        NotifyWakeup ();
    }
    
    
    LoraPhy::State
        LoraStateHelper::GetState (void)
    {
        if (m_sleeping)
        {
            //NS_LOG_UNCOND("Current State is SLEEP");
            return LoraPhy::SLEEP;
        }
        else if (m_endTx > Simulator::Now ())
        {
            //NS_LOG_UNCOND("Current State is TX");
            return LoraPhy::TX;
        }
        else if (m_rxing)
        {
            //NS_LOG_UNCOND("Current State is RX");
            return LoraPhy::RX;
        }
        else
        {
            //NS_LOG_UNCOND("Current State is IDLE");
            return LoraPhy::IDLE;
        }
    }

    void
    LoraStateHelper::NotifyTxStart (Time duration, double txPowerDbm)
    {
      NS_LOG_FUNCTION (this);
      for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyTxStart (duration, txPowerDbm);
        }
    }

    void
    LoraStateHelper::NotifyRxStart (Time duration)
    {
      NS_LOG_FUNCTION (this);
      for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
          (*i)->NotifyRxStart (duration);
        }
    }

    void
    LoraStateHelper::NotifyRxEndOk (void)
    {
      NS_LOG_FUNCTION (this);
      for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
          (*i)->NotifyRxEndOk ();
        }
    }

    void
    LoraStateHelper::NotifyRxEndError (void)
    {
      NS_LOG_FUNCTION (this);
      for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
          (*i)->NotifyRxEndError ();
        }
    }

    void
    LoraStateHelper::NotifySleep (void)
    {
      NS_LOG_FUNCTION (this);
      for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
          (*i)->NotifySleep ();
        }
    }

    void
    LoraStateHelper::NotifyWakeup (void)
    {
      NS_LOG_FUNCTION (this);
      for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
          (*i)->NotifyWakeup ();
        }
    }

    void
    LoraStateHelper::DoSwitchFromRx (void)
    {
      NS_LOG_FUNCTION (this);
      //NS_ASSERT (IsStateRx ());
      //NS_ASSERT (m_rxing);

      Time now = Simulator::Now ();
      m_stateLogger (m_startRx, now - m_startRx, LoraPhy::RX);
      m_previousStateChangeTime = now;

      m_rxing = false;

      NS_ASSERT (IsStateIdle ());
    }
    
    
} // namespace ns3