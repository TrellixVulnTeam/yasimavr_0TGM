/*
 * sim_adc.h
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

#ifndef __YASIMAVR_IO_ADC_H__
#define __YASIMAVR_IO_ADC_H__

#include "../core/sim_peripheral.h"
#include "../core/sim_types.h"


//=======================================================================================
/*
 * CTLREQ definitions
*/
#define AVR_CTLREQ_ADC_SET_TEMP         1
#define AVR_CTLREQ_ADC_TRIGGER          2


//=======================================================================================
/*
 * Configuration enumerations and structures
*/

class DLL_EXPORT AVR_IO_ADC {

public:

    enum Channel {
        Channel_SingleEnded,
        Channel_Differential,
        Channel_Zero,
        Channel_IntRef,
        Channel_Temperature,
        Channel_AcompRef
    };

    struct channel_config_t : base_reg_config_t {
        Channel type;
        union {
            struct {
                uint32_t pin_p;
                uint32_t pin_n;
            };
            char per_num;
        };
        unsigned int gain;
    };

    enum SignalId {
        Signal_ConversionStarted,
        Signal_AboutToSample,
        Signal_ConversionComplete,
    };

};

#endif //__YASIMAVR_IO_ADC_H__
