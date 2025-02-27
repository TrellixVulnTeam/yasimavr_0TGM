/*
 * arch_mega0_rtc.sip
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

struct AVR_ArchMega0_RTC_Config {
%TypeHeaderCode
#include "arch_mega0_rtc.h"
%End

    enum RTC_ClockSource {
        Clock_32kHz,
        Clock_1kHz
    };

    struct clksel_config_t : base_reg_config_t {
        RTC_ClockSource source;
    };

    std::vector<AVR_ArchMega0_RTC_Config::clksel_config_t> clocks;
    reg_addr_t reg_base;
    int_vect_t iv_rtc;
    int_vect_t iv_pit;

};

class AVR_ArchMega0_RTC : public AVR_Peripheral /NoDefaultCtors/ {
%TypeHeaderCode
#include "arch_mega0_rtc.h"
%End

public:

    AVR_ArchMega0_RTC(const AVR_ArchMega0_RTC_Config&);
    virtual ~AVR_ArchMega0_RTC();

    virtual bool init(AVR_Device&);
    virtual void reset();
    virtual void ioreg_read_handler(reg_addr_t);
    virtual void ioreg_write_handler(reg_addr_t, const ioreg_write_t&);
    virtual void sleep(bool, AVR_SleepMode);

};
