/*
 * sim_ioreg.h
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

#ifndef __YASIMAVR_IOREG_H__
#define __YASIMAVR_IOREG_H__

#include "sim_types.h"
#include <vector>


//=======================================================================================

//This structure is used for 'ioreg_write_handler' callbacks to hold pre-calculated
//information on the new register value
struct ioreg_write_t {
    uint8_t value;   //contains the new value in the register
    uint8_t posedge; //indicates bits transiting from '0' to '1'
    uint8_t negedge; //indicates bits transiting from '1' to '0'
    uint8_t old;     //contains the old value of the register
};


//=======================================================================================
/*
 * Abstract interface for I/O register handlers
 * The handler is notified when the register is accessed by the CPU
 * It is meant to be implemented by I/O peripherals
 */
class DLL_EXPORT AVR_IO_RegHandler {

public:
    virtual ~AVR_IO_RegHandler() {}

    virtual void ioreg_read_handler(reg_addr_t addr) = 0;

    virtual void ioreg_write_handler(reg_addr_t addr, const ioreg_write_t& data) = 0;

};


//=======================================================================================
/*
 *  Represents one 8-bits I/O register that is a vehicle for data transfer
 *  between the CPU and I/O peripherals
 *  Peripherals can be added as handlers to a register to be notified of
 *  accesses (read or write) by the CPU
 */
class DLL_EXPORT AVR_IO_Register {

public:

    AVR_IO_Register(bool core_reg=false);
    ~AVR_IO_Register();

    //Simple inline interface to access the value
    uint8_t value() const;
    void set(uint8_t value);

    //Add a handler to this register
    void set_handler(AVR_IO_RegHandler* handler, uint8_t use_mask, uint8_t ro_mask);

    //CPU interface for read/write operation on this register
    uint8_t cpu_read(reg_addr_t addr);
    //return true if the read-only rule has been violated, i.e. attempting to write
    //a read-only or unused bit with '1'
    //Note that if the register has no handler, all 8 bits are read-only except if
    //core_reg was true at construction
    bool cpu_write(reg_addr_t addr, uint8_t value);

    //I/O peripheral interface for read/write operation on this register
    uint8_t ioctl_read(reg_addr_t addr);
    void ioctl_write(reg_addr_t addr, uint8_t value);

private:

    //Contains the current 8-bits value of this register
    uint8_t m_value;
    //Pointer to the register handler, which is called notified when the register is accessed by the CPU
    AVR_IO_RegHandler *m_handler;
    //Flag set
    uint8_t m_flags;
    //8-bits mask indicating which bits of the register are used
    uint8_t m_use_mask;
    //8-bits mask indicating which bits of the register are read-only for the CPU
    uint8_t m_ro_mask;

};

inline uint8_t AVR_IO_Register::value() const
{
    return m_value;
}

inline void AVR_IO_Register::set(uint8_t value)
{
    m_value = value;
}

#endif //__YASIMAVR_IOREG_H__
