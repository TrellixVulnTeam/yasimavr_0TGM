/*
 * sim_pin.h
 *
 *  Copyright 2021 Clement Savergne <csavergne@yahoo.com>

    This file is part of yasim-avr.

    yasim-avr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    yasim-avr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with yasim-avr.  If not, see <http://www.gnu.org/licenses/>.
 */

//=======================================================================================

#ifndef __YASIMAVR_PIN_H__
#define __YASIMAVR_PIN_H__

#include "sim_types.h"
#include "sim_signal.h"

class AVR_IO_Port;


//=======================================================================================
/*
 * AVR_Pin represents a external pad of the MCU used for GPIO.
 * The pin has two electrical states given by the external circuit and the internal circuit
 * It is resolved into a single electrical state. In case of conflict, the SHORTED state is
 * set.
 */
class DLL_EXPORT AVR_Pin : public AVR_SignalHook {

public:

    enum State {
        //'Weak' states
        State_Floating  = 0x00,
        State_PullUp    = 0x01,
        State_PullDown  = 0x02,
        //'Driven' states
        State_Analog    = 0x04,
        State_High      = 0x05,
        State_Low       = 0x06,
        State_Shorted   = 0xFF
    };

    //Signal IDs raised by the pin.
    //For all signals, the index is set to the pin ID.
    enum SignalId {
        //Signal raised for any change of the resolved digital state.
        //data.u is set to the new state.
        Signal_DigitalStateChange,
        //Signal raised for any change to the analog value, including
        //when induced by a digital state change.
        //data.d is set to the analog value (in interval [0;1])
        Signal_AnalogValueChange,
    };

    static const char* StateName(State state);

    //Constructor of the pin. The ID should be a unique identifier of the pad for the MCU.
    AVR_Pin(uint32_t id);

    //Interface for controlling the pad from external code
    void set_external_state(State state);
    //Set the external analog voltage value, in interval [0;1], relative to VCC
    void set_external_analog_value(double v);

    uint32_t id() const;
    //Resolved state of the pad
    State state() const;
    //Resolved state reduced to digital values: LOW, HIGH or SHORTED
    State digital_state() const;
    //Resolved analog voltage value at the pin, in interval [0;1]
    double analog_value() const;
    //Signal raised for state/value changes
    AVR_DataSignal& signal();

    //Implementation of the AVR_SignalHook interface to receive changes
    virtual void raised(const signal_data_t& sigdata, uint16_t hooktag) override;

private:

    friend class AVR_IO_Port;

    const uint32_t m_id;
    State m_ext_state;
    State m_int_state;
    State m_resolved_state;
    double m_analog_value;
    AVR_DataSignal m_signal;

    State resolve_state();

    void set_internal_state(State state);

};

inline uint32_t AVR_Pin::id() const
{
    return m_id;
};

inline AVR_Pin::State AVR_Pin::state() const
{
    return m_resolved_state;
};

inline AVR_DataSignal& AVR_Pin::signal()
{
    return m_signal;
}

#endif //__YASIMAVR_PIN_H__
