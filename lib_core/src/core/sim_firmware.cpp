/*
 * sim_firmware.cpp
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

#include "sim_firmware.h"
#include "sim_device.h"
#include <libelf.h>
#include <gelf.h>
#include <stdio.h>
#include <cstring>


//=======================================================================================

AVR_Firmware::AVR_Firmware()
:variant("")
,frequency(0)
,vcc(0.0)
,aref(0.0)
,console_register(0)
,m_datasize(0)
,m_bsssize(0)
{}

AVR_Firmware::~AVR_Firmware()
{
    for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
        for (Block b : it->second)
            if (b.mem_block.size)
                free(b.mem_block.buf);
    }
}

static Elf32_Phdr* elf_find_phdr(Elf32_Phdr* phdr_table, size_t phdr_count, GElf_Shdr* shdr)
{
    for (size_t i = 0; i < phdr_count; ++i) {
        Elf32_Phdr* phdr = phdr_table + i;
        if (phdr->p_type != PT_LOAD) continue;
        if (shdr->sh_offset < phdr->p_offset) continue;
        if ((shdr->sh_offset + shdr->sh_size) > (phdr->p_offset + phdr->p_filesz)) continue;
        return phdr;
    }
    return nullptr;
}

AVR_Firmware* AVR_Firmware::read_elf(const std::string& filename)
{
    Elf32_Ehdr elf_header;          // ELF header
    Elf *elf = nullptr;                // Our Elf pointer for libelf
    std::FILE *file;                // File Descriptor

    if ((file = fopen(filename.c_str(), "rb")) == nullptr) {
        AVR_global_logger().err("Unable to open ELF file '%s'", filename.c_str());
        return nullptr;
    }

    if (fread(&elf_header, sizeof(elf_header), 1, file) == 0) {
        AVR_global_logger().err("Unable to read ELF file '%s'", filename.c_str());
        fclose(file);
        return nullptr;
    }

    AVR_Firmware *firmware = new AVR_Firmware();
    int fd = fileno(file);

    /* this is actually mandatory !! otherwise elf_begin() fails */
    if (elf_version(EV_CURRENT) == EV_NONE) {
            /* library out of date - recover from error */
    }

    elf = elf_begin(fd, ELF_C_READ, nullptr);

    //Obtain the Program Header table pointer and size
    size_t phdr_count;
    elf_getphdrnum(elf, &phdr_count);
    Elf32_Phdr* phdr_table = elf32_getphdr(elf);

    //Iterate through all sections
    Elf_Scn* scn = nullptr;
    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        //Get the section header and its name
        GElf_Shdr shdr;
        gelf_getshdr(scn, &shdr);
        char * name = elf_strptr(elf, elf_header.e_shstrndx, shdr.sh_name);

        //For bss and data sections, store the size
        if (!strcmp(name, ".bss")) {
            Elf_Data* s = elf_getdata(scn, nullptr);
            firmware->m_bsssize = s->d_size;
        }
        else if (!strcmp(name, ".data")) {
            Elf_Data* s = elf_getdata(scn, nullptr);
            firmware->m_datasize = s->d_size;
        }

        //The rest of the loop is for retrieving the binary data of the section,
        //skip it if the section is non-loadable or empty.
        if (((shdr.sh_flags & SHF_ALLOC) == 0) || (shdr.sh_type != SHT_PROGBITS))
            continue;
        if (shdr.sh_size == 0)
            continue;

        //Find the Program Header segment containing this section
        Elf32_Phdr* phdr = elf_find_phdr(phdr_table, phdr_count, &shdr);
        if (!phdr) continue;

        Elf_Data* scn_data = elf_getdata(scn, nullptr);

        //Load Memory Address calculation
        unsigned int lma = phdr->p_paddr + shdr.sh_offset - phdr->p_offset;

        //Create the memory block
        uint8_t* buf;
        if (scn_data->d_size) {
            buf = (uint8_t*) malloc(scn_data->d_size);
            memcpy(buf, scn_data->d_buf, scn_data->d_size);
        } else {
            buf = nullptr;
        }

        //Construct the firmware chunk (essentially a memory block and an offset)
        Block b;
        b.mem_block.size = scn_data->d_size;
        b.mem_block.buf = buf;

        //Add the firmware chunk to the corresponding memory area (Flash, etc...)
        if (!strcmp(name, ".text") || !strcmp(name, ".data") || !strcmp(name, ".rodata")) {
            b.base = lma;
            firmware->m_blocks["flash"].push_back(b);
        }
        else if (!strcmp(name, ".eeprom")) {
            b.base = lma - 0x810000;
            firmware->m_blocks["eeprom"].push_back(b);
        }
        else if (!strcmp(name, ".fuse")) {
            b.base = lma - 0x820000;
            firmware->m_blocks["fuse"].push_back(b);
        }
        else if (!strcmp(name, ".lock")) {
            b.base = lma - 0x830000;
            firmware->m_blocks["lock"].push_back(b);
        }
        else if (!strcmp(name, ".signature")) {
            b.base = lma - 0x840000;
            firmware->m_blocks["signature"].push_back(b);
        }
        else if (!strcmp(name, ".user_signatures")) {
            b.base = lma - 0x850000;
            firmware->m_blocks["user_signatures"].push_back(b);
        }
        else {
            AVR_global_logger().err("Firmware section unknown: '%s'", name);
        }
    }

    elf_end(elf);
    fclose(file);

    AVR_global_logger().dbg("Firmware read from ELF file '%s'", filename.c_str());

    return firmware;
}

void AVR_Firmware::add_block(const std::string& name, const mem_block_t& block, size_t base)
{
    //Make a deep copy of the memory block
    mem_block_t mb;
    mb.size = block.size;
    mb.buf = (uint8_t*) malloc(block.size);
    memcpy(mb.buf, block.buf, block.size);
    //Construct a firmware block and add it to the map
    Block b = { .mem_block = mb, .base = base };
    m_blocks[name].push_back(b);
}

bool AVR_Firmware::has_memory(const std::string& name) const
{
    return m_blocks.find(name) != m_blocks.end();
}


size_t AVR_Firmware::memory_size(const std::string& name) const
{
    auto it = m_blocks.find(name);
    if (it == m_blocks.end())
        return 0;

    size_t s = 0;
    for (auto block : it->second)
        s += block.mem_block.size;

    return s;
}


std::vector<AVR_Firmware::Block> AVR_Firmware::blocks(const std::string& name) const
{
    auto it = m_blocks.find(name);
    if (it == m_blocks.end())
        return std::vector<Block>();
    else
        return it->second;
}

bool AVR_Firmware::load_memory(const std::string& name, AVR_NonVolatileMemory& memory) const
{
    bool status = true;

    for (Block fb : blocks(name))
        status &= memory.program(fb.mem_block, fb.base);

    return status;
}
