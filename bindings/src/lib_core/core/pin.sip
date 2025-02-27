/*
 * pin.sip
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


class AVR_Pin : public AVR_SignalHook /NoDefaultCtors/ {
%TypeHeaderCode
#include "core/sim_pin.h"
%End

public:

    enum State /BaseType=IntEnum/ {
        State_Floating  /PyName=Floating/,
        State_PullUp    /PyName=PullUp/,
        State_PullDown  /PyName=PullDown/,
        State_Analog    /PyName=Analog/,
        State_High      /PyName=High/,
        State_Low       /PyName=Low/,
        State_Shorted   /PyName=Shorted/,
    };

    enum SignalId /BaseType=IntEnum/ {
        Signal_DigitalStateChange   /PyName=DigitalStateChange/,
        Signal_AnalogValueChange    /PyName=AnalogValueChange/,
    };

    static const char* StateName(AVR_Pin::State state);

    AVR_Pin(uint32_t);

    void set_external_state(AVR_Pin::State state);
    void set_external_analog_value(double v);
    uint32_t id() const;
    AVR_Pin::State state() const;
    AVR_Pin::State digital_state() const;
    double analog_value() const;

    AVR_DataSignal& signal();

    virtual void raised(const signal_data_t&, uint16_t);

};
