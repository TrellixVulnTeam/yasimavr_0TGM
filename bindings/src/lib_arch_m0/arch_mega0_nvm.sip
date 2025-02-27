/*
 * arch_mega0_nvm.sip
 *
 *  Copyright 2022 Clement Savergne <csavergne@yahoo.com>

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

class AVR_ArchMega0_USERROW : public AVR_Peripheral /NoDefaultCtors/ {
%TypeHeaderCode
#include "arch_mega0_nvm.h"
%End

public:

    AVR_ArchMega0_USERROW(reg_addr_t base);

    virtual bool init(AVR_Device&);
    virtual void ioreg_write_handler(reg_addr_t, const ioreg_write_t&);
};


class AVR_ArchMega0_Fuses : public AVR_Peripheral /NoDefaultCtors/ {
%TypeHeaderCode
#include "arch_mega0_nvm.h"
%End

public:

    AVR_ArchMega0_Fuses(reg_addr_t base);

    virtual bool init(AVR_Device&);

};


struct AVR_ArchMega0_NVM_Config {
%TypeHeaderCode
#include "arch_mega0_nvm.h"
%End

    reg_addr_t reg_base;

    mem_addr_t flash_page_size;

    unsigned int buffer_erase_delay;

    unsigned int page_write_delay;
    unsigned int page_erase_delay;

    unsigned int chip_erase_delay;
    unsigned int eeprom_erase_delay;

    int_vect_t iv_eeready;

};


class AVR_ArchMega0_NVM : public AVR_Peripheral /NoDefaultCtors/ {
%TypeHeaderCode
#include "arch_mega0_nvm.h"
%End

public:

    AVR_ArchMega0_NVM(const AVR_ArchMega0_NVM_Config&);
    virtual ~AVR_ArchMega0_NVM();

    virtual bool init(AVR_Device&);
    virtual void reset();
    virtual bool ctlreq(uint16_t, ctlreq_data_t*);
    virtual void ioreg_write_handler(reg_addr_t, const ioreg_write_t&);

};
