/*
 * arch_mega0_port.sip
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

struct AVR_ArchMega0_PortConfig {
%TypeHeaderCode
#include "arch_mega0_port.h"
%End

    reg_addr_t reg_base_port;
    reg_addr_t reg_base_vport;
    int_vect_t iv_port;

};


class AVR_ArchMega0_Port : public AVR_IO_Port /NoDefaultCtors/ {
%TypeHeaderCode
#include "arch_mega0_port.h"
%End

public:

    AVR_ArchMega0_Port(char name, const AVR_ArchMega0_PortConfig&);

    virtual bool init(AVR_Device&);
    virtual void reset();
    virtual void ioreg_read_handler(reg_addr_t);
    virtual void ioreg_write_handler(reg_addr_t, const ioreg_write_t&);

protected:

    virtual void pin_state_changed(uint8_t, AVR_Pin::State);

};
