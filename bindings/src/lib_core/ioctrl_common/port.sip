/*
 * port.sip
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


class AVR_IO_Port : public AVR_Peripheral, public AVR_SignalHook /NoDefaultCtors/ {
%TypeHeaderCode
#include "ioctrl_common/sim_port.h"
%End

public:

    AVR_IO_Port(char name);

    virtual bool init(AVR_Device&);
    virtual void reset();
    virtual bool ctlreq(uint16_t, ctlreq_data_t*);
    virtual void raised(const signal_data_t&, uint16_t);

protected:

    uint8_t pin_mask() const;
    void set_pin_internal_state(uint8_t, AVR_Pin::State);
    virtual void pin_state_changed(uint8_t, AVR_Pin::State);

};
