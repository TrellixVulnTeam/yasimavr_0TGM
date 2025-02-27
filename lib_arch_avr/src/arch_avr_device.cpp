/*
 * arch_avr_device.cpp
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

#include "arch_avr_device.h"
#include "core/sim_debug.h"
#include "core/sim_peripheral.h"
#include "core/sim_firmware.h"
#include <cstring>


//=======================================================================================

AVR_ArchAVR_Core::AVR_ArchAVR_Core(const AVR_ArchAVR_CoreConfig& config)
:AVR_Core(config)
,m_eeprom(config.eepromend ? (config.eepromend + 1) : 0)
{}

uint8_t AVR_ArchAVR_Core::cpu_read_data(mem_addr_t data_addr)
{
    uint8_t value = 0;

    if (data_addr < 32) {
        value = m_regs[data_addr];
    }
    else if (data_addr <= m_config.ioend) {
        value = cpu_read_ioreg(data_addr - 32);
    }
    else if (data_addr >= m_config.ramstart && data_addr <= m_config.ramend) {
        value = m_sram[data_addr - m_config.ramstart];
    }
    else if (!m_device->test_option(AVR_Device::Option_IgnoreBadCpuIO)) {
        m_device->logger().err("CPU reading an invalid data address: 0x%04x", data_addr);
        m_device->crash(CRASH_BAD_CPU_IO, "Bad data address");
    }

    if (m_debug_probe)
        m_debug_probe->_cpu_notify_data_read(data_addr);

    return value;
}

void AVR_ArchAVR_Core::cpu_write_data(mem_addr_t data_addr, uint8_t value)
{
    if (data_addr < 32) {
        m_regs[data_addr] = value;
    }
    else if (data_addr <= m_config.ioend) {
        cpu_write_ioreg(data_addr - 32, value);
    }
    else if (data_addr >= m_config.ramstart && data_addr <= m_config.ramend) {
        m_sram[data_addr - m_config.ramstart] = value;
    }
    else if (!m_device->test_option(AVR_Device::Option_IgnoreBadCpuIO)) {
        m_device->logger().err("CPU writing an invalid data address: 0x%04x", data_addr);
        m_device->crash(CRASH_BAD_CPU_IO, "Bad data address");
    }

    if (m_debug_probe)
        m_debug_probe->_cpu_notify_data_write(data_addr);
}

void AVR_ArchAVR_Core::dbg_read_data(mem_addr_t addr, uint8_t* buf, mem_addr_t len)
{
    std::memset(buf, 0x00, len);

    mem_addr_t bufofs, blockofs;
    uint32_t n;

    if (data_space_map(addr, len, 0, 32, &bufofs, &blockofs, &n))
        std::memcpy(buf + bufofs, m_regs + blockofs, n);

    if (data_space_map(addr, len, 32, m_config.ioend, &bufofs, &blockofs, &n)) {
        for (reg_addr_t i = 0; i < n; ++i)
            buf[bufofs + i] = cpu_read_ioreg(blockofs + i);
    }

    if (data_space_map(addr, len, m_config.ramstart, m_config.ramend, &bufofs, &blockofs, &n))
        std::memcpy(buf + bufofs, m_sram + blockofs, n);

}

void AVR_ArchAVR_Core::dbg_write_data(mem_addr_t addr, uint8_t* buf, mem_addr_t len)
{
    mem_addr_t bufofs, blockofs;
    uint32_t n;

    if (data_space_map(addr, len, 0, 32, &bufofs, &blockofs, &n))
        std::memcpy(m_regs + blockofs, buf + bufofs, n);

    if (data_space_map(addr, len, 32, m_config.ioend, &bufofs, &blockofs, &n)) {
        for (reg_addr_t i = 0; i < n; ++i)
            cpu_write_ioreg(blockofs + i, buf[bufofs + i]);
    }

    if (data_space_map(addr, len, m_config.ramstart, m_config.ramend, &bufofs, &blockofs, &n))
        std::memcpy(m_sram + blockofs, buf + bufofs, n);
}


//=======================================================================================

AVR_ArchAVR_Device::AVR_ArchAVR_Device(const AVR_ArchAVR_DeviceConfig& config)
:AVR_Device(m_core_impl, config)
,m_core_impl(*(config.core))
{}


AVR_ArchAVR_Device::~AVR_ArchAVR_Device()
{
    erase_peripherals();
}


bool AVR_ArchAVR_Device::core_ctlreq(uint16_t req, ctlreq_data_t* reqdata)
{
    if (req == AVR_CTLREQ_CORE_NVM) {
        if (reqdata->index == AVR_ArchAVR_Core::NVM_EEPROM)
            reqdata->data = &(m_core_impl.m_eeprom);
        else
            return AVR_Device::core_ctlreq(req, reqdata);

        return true;
    } else {
        return AVR_Device::core_ctlreq(req, reqdata);
    }
}

bool AVR_ArchAVR_Device::program(const AVR_Firmware& firmware)
{
    if (!AVR_Device::program(firmware))
        return false;

    if (firmware.has_memory("eeprom")) {
        if (firmware.load_memory("eeprom", m_core_impl.m_eeprom)) {
            logger().dbg("Firmware load: EEPROM loaded");
        } else {
            logger().err("Firmware load: Error loading the EEPROM");
            return false;
        }
    }

    return true;
}
