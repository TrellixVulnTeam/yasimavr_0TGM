/*
 * arch_avr_usart.sip
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

struct AVR_ArchAVR_USART_Config {
%TypeHeaderCode
#include "arch_avr_usart.h"
%End

    reg_addr_t reg_data;

    regbit_t rb_rx_enable;
    regbit_t rb_tx_enable;
    regbit_t rb_rxc_inten;
    regbit_t rb_rxc_flag;
    regbit_t rb_txc_inten;
    regbit_t rb_txc_flag;
    regbit_t rb_txe_inten;
    regbit_t rb_txe_flag;

    regbit_t rb_baud_2x;
    reg_addr_t reg_baud;
    uint8_t baud_bitsize;

    int_vect_t rxc_vector;
    int_vect_t txc_vector;
    int_vect_t txe_vector;

};


class AVR_ArchAVR_USART : public AVR_Peripheral, public AVR_SignalHook /NoDefaultCtors/ {
%TypeHeaderCode
#include "arch_avr_usart.h"
%End

public:

    AVR_ArchAVR_USART(uint8_t num, const AVR_ArchAVR_USART_Config&);

    virtual bool init(AVR_Device&);
    virtual void reset();
    virtual bool ctlreq(uint16_t, ctlreq_data_t*);
    virtual void ioreg_read_handler(reg_addr_t);
    virtual void ioreg_write_handler(reg_addr_t, const ioreg_write_t&);
    virtual void raised(const signal_data_t&, uint16_t);

};
