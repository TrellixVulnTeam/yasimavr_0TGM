/*
 * arch_avr_wdt.h
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

#ifndef __YASIMAVR_AVR_WDT_H__
#define __YASIMAVR_AVR_WDT_H__

#include "core/sim_cycle_timer.h"
#include "core/sim_interrupt.h"
#include "ioctrl_common/sim_wdt.h"


//=======================================================================================
/*
 * Implementation of a Watchdog Timer for AVR series
 */

struct AVR_ArchAVR_WDT_Config {

    uint32_t clock_frequency;
    std::vector<uint32_t> delays;
    //Assumes all the fields are on one single register
    reg_addr_t reg_wdt;
    bitmask_t bm_delay;
    bitmask_t bm_chg_enable;
    bitmask_t bm_reset_enable;
    bitmask_t bm_int_enable;
    bitmask_t bm_int_flag;
    regbit_t rb_reset_flag;

    int_vect_t vector;

};

class DLL_EXPORT AVR_ArchAVR_WDT : public AVR_WatchdogTimer, public AVR_InterruptHandler {

public:

    AVR_ArchAVR_WDT(const AVR_ArchAVR_WDT_Config& config);

    virtual bool init(AVR_Device& device) override;
    virtual void reset() override;
    virtual void ioreg_write_handler(reg_addr_t addr, const ioreg_write_t& data) override;
    virtual void interrupt_ack_handler(int_vect_t vector) override;

protected:

    virtual void timeout() override;

private:

    //Device variant configuration
    const AVR_ArchAVR_WDT_Config& m_config;
    //cycle number when the register have been unlocked for modification
    cycle_count_t m_unlock_cycle;

    void configure_timer(bool enable, uint8_t delay_index);

};

#endif //__YASIMAVR_AVR_WDT_H__
