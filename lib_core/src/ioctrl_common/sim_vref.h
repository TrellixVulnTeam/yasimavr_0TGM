/*
 * sim_vref.h
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

#ifndef __YASIMAVR_IO_VREF_H__
#define __YASIMAVR_IO_VREF_H__

#include "../core/sim_peripheral.h"
#include "../core/sim_types.h"


//=======================================================================================
/*
 * CTLREQ definitions
*/

//AVR_CTLREQ_GET_SIGNAL is not implemented

//Request sent by the ADC to the VREF controller to obtain the VREF.
//The index shall be set to the required source (one of AVR_IO_VREF::Source enum values)
//On returning, the value 'd' contains the voltage value
//Except for VCC which is returned in absolute volts, all values are returned as a ratio of VCC
#define AVR_CTLREQ_VREF_GET             1

//Request sent by Device classes to set the VCC or AREF values when loading the firmware.
//The index shall be set to the required source (VREF_Ext_VCC or VREF_Ext_AREF)
//The value 'd' shall be set to the voltage value :
//  - VCC shall be an absolute positive value in volts
//  - AREF shall be a ratio of VCC and is clipped to [0.0; 1.0]
#define AVR_CTLREQ_VREF_SET             2


//=======================================================================================
/*
 * Generic I/O controller for managing VREF for analog peripherals (ADC, analog comparator)
 * Note that setting VCC in the firmware is required for using any analog feature of a MCU.
 * Failing to do so will trigger a device crash
 */
class DLL_EXPORT AVR_IO_VREF : public AVR_Peripheral {

public:

    enum Source {
        Source_VCC,             //VCC voltage value
        Source_AVCC,            //AVCC voltage value (always equal to VCC for now)
        Source_AREF,            //AREF voltage value
        Source_Internal,        //Internal reference voltage value
    };

    enum SignalId {
        Signal_ARefChange,
        Signal_IntRefChange,
        Signal_VCCChange,
    };

    AVR_IO_VREF(uint32_t ref_count);

    bool active() const;

    virtual bool ctlreq(uint16_t req, ctlreq_data_t* data) override;

protected:

    void set_reference(uint32_t index, Source source, double voltage=1.0);
    double reference(uint32_t index) const;

private:

    double m_vcc;
    double m_aref;
    AVR_DataSignal m_signal;

    struct ref_t {
        double value;
        bool relative;
    };

    std::vector<ref_t> m_references;

};

inline bool AVR_IO_VREF::active() const
{
    return m_vcc;
}

#endif //__YASIMAVR_IO_VREF_H__
