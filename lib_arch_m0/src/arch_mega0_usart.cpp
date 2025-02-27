/*
 * arch_mega0_usart.cpp
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

#include "arch_mega0_usart.h"
#include "arch_mega0_io.h"
#include "arch_mega0_io_utils.h"
#include "core/sim_sleep.h"

//=======================================================================================

#define REG_ADDR(reg) \
    reg_addr_t(m_config.reg_base + offsetof(USART_t, reg))

#define REG_OFS(reg) \
    offsetof(USART_t, reg)


AVR_ArchMega0_USART::AVR_ArchMega0_USART(uint8_t num, const AVR_ArchMega0_USART_Config& config)
:AVR_Peripheral(AVR_IOCTL_UART(0x30 + num))
,m_config(config)
,m_rxc_intflag(false)
,m_txc_intflag(false)
,m_txe_intflag(false)
{
    m_endpoint = { &m_uart.signal(), &m_uart };
}

bool AVR_ArchMega0_USART::init(AVR_Device& device)
{
    bool status = AVR_Peripheral::init(device);

    add_ioreg(REG_ADDR(RXDATAL), USART_DATA_gm, true);
    add_ioreg(REG_ADDR(RXDATAH), 0, true);
    add_ioreg(REG_ADDR(TXDATAL), USART_DATA_gm);
    add_ioreg(REG_ADDR(TXDATAH), USART_DATA8_bm);
    add_ioreg(REG_ADDR(STATUS), USART_RXCIF_bm | USART_DREIF_bm, true); // R/O part
    add_ioreg(REG_ADDR(STATUS), USART_TXCIF_bm | USART_RXSIF_bm); // R/W part
    add_ioreg(REG_ADDR(CTRLA));
    add_ioreg(REG_ADDR(CTRLB));
    add_ioreg(REG_ADDR(CTRLC));
    add_ioreg(REG_ADDR(BAUDL));
    add_ioreg(REG_ADDR(BAUDH));
    //CTRLD not implemented
    //DBGCTRL not supported
    //EVCTRL not implemented
    //TXPLCTRL not implemented
    //RXPLCTRL not implemented

    status &= m_rxc_intflag.init(device,
                                 regbit_t(REG_ADDR(CTRLA), 0, USART_RXCIE_bm | USART_RXSIE_bm),
                                 regbit_t(REG_ADDR(STATUS), 0, USART_RXCIF_bm | USART_RXSIF_bm),
                                 m_config.iv_rxc);
    status &= m_txc_intflag.init(device,
                                 DEF_REGBIT_B(CTRLA, USART_TXCIE),
                                 DEF_REGBIT_B(STATUS, USART_TXCIF),
                                 m_config.iv_txc);
    status &= m_txe_intflag.init(device,
                                 DEF_REGBIT_B(CTRLA, USART_DREIE),
                                 DEF_REGBIT_B(STATUS, USART_DREIF),
                                 m_config.iv_txe);

    m_uart.init(device.cycle_manager(), logger());
    m_uart.set_tx_buffer_limit(2);
    m_uart.set_rx_buffer_limit(3);
    m_uart.signal().connect_hook(this);

    return status;
}

void AVR_ArchMega0_USART::reset()
{
    m_uart.reset();
    SET_IOREG(STATUS, USART_DREIF);
    update_framerate();
}

bool AVR_ArchMega0_USART::ctlreq(uint16_t req, ctlreq_data_t* data)
{
    if (req == AVR_CTLREQ_GET_SIGNAL) {
        data->data = &m_uart.signal();
        return true;
    }
    else if (req == AVR_CTLREQ_UART_ENDPOINT) {
        data->data = &m_endpoint;
        return true;
    }

    return false;
}

void AVR_ArchMega0_USART::ioreg_read_handler(reg_addr_t addr)
{
    reg_addr_t reg_ofs = addr - m_config.reg_base;

    if (reg_ofs == REG_OFS(RXDATAL)) {
        uint8_t frame = m_uart.pop_rx();
        write_ioreg(REG_ADDR(RXDATAL), frame);
        if (!m_uart.rx_available())
            m_rxc_intflag.clear_flag(USART_RXCIF_bm);
    }
}

void AVR_ArchMega0_USART::ioreg_write_handler(reg_addr_t addr, const ioreg_write_t& data)
{
    reg_addr_t reg_ofs = addr - m_config.reg_base;

    //Writing to TXDATA emits the value, if TX is enabled
    if (reg_ofs == REG_OFS(TXDATAL)) {
        if (TEST_IOREG(CTRLB, USART_TXEN)) {
            m_uart.push_tx(data.value);
            if (m_uart.tx_pending())
                m_txe_intflag.clear_flag();

            logger().dbg("Data pushed: 0x%02x", data.value);
        }
    }

    else if (reg_ofs == REG_OFS(STATUS)) {
        //Writing one to RXSIF clears the bit and cancels the interrupt
        if (data.value & USART_RXSIF_bm)
            m_rxc_intflag.clear_flag(USART_RXSIF_bm);
        //Writing one to TXCIF clears the bit and cancels the interrupt
        if (data.value & USART_TXCIF_bm)
            m_txc_intflag.clear_flag();
    }

    else if (reg_ofs == REG_OFS(CTRLA)) {
        m_txc_intflag.update_from_ioreg();
        m_txe_intflag.update_from_ioreg();
        m_rxc_intflag.update_from_ioreg();
    }

    else if (reg_ofs == REG_OFS(CTRLB)) {
        //Processing of TXEN. If it is cleared, we flush the TX buffer
        if (data.negedge & USART_TXEN_bm) {
            m_uart.cancel_tx_pending();
            m_txe_intflag.set_flag();
        }

        //Processing of RXEN changes
        if (data.posedge & USART_RXEN_bm) {
            m_uart.set_rx_enabled(true);
        }
        else if (data.negedge & USART_RXEN_bm) {
            m_uart.set_rx_enabled(false);
            m_rxc_intflag.clear_flag(USART_RXCIF_bm);
        }

        update_framerate();
    }

    else if (reg_ofs == REG_OFS(BAUD) || reg_ofs == (REG_OFS(BAUD) + 1)) {
        update_framerate();
    }
}

void AVR_ArchMega0_USART::raised(const signal_data_t& sigdata, uint16_t __unused)
{
    if (sigdata.sigid == AVR_IO_UART::Signal_TX_Start) {
        //Notification that the pending frame has been pushed to the shift register
        //to be emitted. The TX buffer is now empty so raise the DRE interrupt.
        m_txe_intflag.set_flag();
        logger().dbg("TX started, raising DRE");
    }

    else if (sigdata.sigid == AVR_IO_UART::Signal_TX_Complete && sigdata.data.as_uint()) {
        //Notification that the frame in the shift register has been emitted
        //Raise the TXC interrupt.
        m_txc_intflag.set_flag();
        logger().dbg("TX complete, raising TXC");
    }

    else if (sigdata.sigid == AVR_IO_UART::Signal_RX_Start) {
        //If the Start-of-Frame detection is enabled, raise the RXS flag
        if (TEST_IOREG(CTRLB, USART_SFDEN) && device()->sleep_mode() == AVR_SleepMode::Standby) {
            m_rxc_intflag.set_flag(USART_RXSIF_bm);
            logger().dbg("RX start, raising RXS");
        }
    }

    else if (sigdata.sigid == AVR_IO_UART::Signal_RX_Complete && sigdata.data.as_uint()) {
        //Raise the RX completion flag
        m_rxc_intflag.set_flag(USART_RXCIF_bm);
        logger().dbg("RX complete, raising RXC");
    }
}

void AVR_ArchMega0_USART::update_framerate()
{
    //From datasheet (normal speed mode) : (Fbaud = Fclk * 64 / (16 * reg_baud))
    //Expressed in delay rather than frequency: bit_delay = Tbaud / Tclk = reg_baud / 4
    //With 10 bits per frame (8 data, 1 start, 1 stop) : frame_delay = 10 * reg_baud / 4
    uint16_t brr = (read_ioreg(REG_ADDR(BAUD) + 1) << 8) | read_ioreg(REG_ADDR(BAUD));
    if (brr < 64) brr = 64;
    cycle_count_t delay = 10 * brr / 4;
    m_uart.set_frame_delay(delay);
}

/*
* The USART is paused for modes above Standby and in Standby if the Start-Frame Detection feature is not enabled
*/
void AVR_ArchMega0_USART::sleep(bool on, AVR_SleepMode mode)
{
    if (mode > AVR_SleepMode::Standby || (mode == AVR_SleepMode::Standby && !TEST_IOREG(CTRLB, USART_SFDEN))) {
        if (on)
            logger().dbg("Pausing");
        else
            logger().dbg("Resuming");

        m_uart.set_paused(on);
    }
}
