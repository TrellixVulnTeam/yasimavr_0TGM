/*
 * vref.sip
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


%ModuleCode

#include "ioctrl_common/sim_vref.h"

const uint16_t CTLREQ_VREF_GET = AVR_CTLREQ_VREF_GET;
const uint16_t CTLREQ_VREF_SET = AVR_CTLREQ_VREF_SET;

%End

const uint16_t CTLREQ_VREF_GET;
const uint16_t CTLREQ_VREF_SET;


class AVR_IO_VREF : public AVR_Peripheral /Abstract/ {
%TypeHeaderCode
#include "ioctrl_common/sim_vref.h"
%End

public:

    enum Source /BaseType=IntEnum/ {
        Source_VCC                  /PyName=VCC/,
        Source_AVCC                 /PyName=AVCC/,
        Source_AREF                 /PyName=AREF/,
        Source_Internal             /PyName=Internal/,
    };

    enum SignalId /BaseType=IntEnum/ {
        Signal_ARefChange           /PyName=ARefChange/,
        Signal_IntRefChange         /PyName=IntRefChange/,
        Signal_VCCChange            /PyName=VCCChange/,
    };

    AVR_IO_VREF(size_t ref_count);

    bool active() const;

    virtual bool ctlreq(uint16_t, ctlreq_data_t*);

protected:

    void set_reference(uint32_t, Source, double=1.0);
    double reference(uint32_t) const;

};
