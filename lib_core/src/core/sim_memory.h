/*
 * sim_memory.h
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

#ifndef __YASIMAVR_MEMORY_H__
#define __YASIMAVR_MEMORY_H__

#include "sim_types.h"
#include <stddef.h>

//=======================================================================================
/*
 * AVR_NonVolatileMemory represents a block of NVM in a AVR chip
 */

struct mem_block_t {

    size_t size;
    unsigned char* buf;

};


class DLL_EXPORT AVR_NonVolatileMemory {

public:

    AVR_NonVolatileMemory(const size_t size);
    ~AVR_NonVolatileMemory();

    size_t size() const;

    bool programmed(size_t pos) const;

    unsigned char operator[](size_t pos) const;

    mem_block_t block() const;
    mem_block_t block(size_t base, size_t size) const;

    //Programs the NVM, this is designed for loading up the memory
    //and the program at the start of a simulation
    bool program(const mem_block_t& mem_block, size_t base = 0);

    //Erases the whole memory
    void erase();
    //Erases a block in the memory
    void erase(size_t base, size_t size);
    //Erases the memory based on a bufset
    void erase(unsigned char* buf, size_t pos, size_t len);

    //Reads/writes the NVM by single byte or block. This is designed for
    //the debug probe and breakpoint mechanisms
    int dbg_read(size_t pos) const;
    size_t dbg_read(unsigned char* buf, size_t base, size_t len) const;
    void dbg_write(unsigned char v, size_t pos);
    void dbg_write(unsigned char* buf, size_t base, size_t len);

    //Writes the NVM by single byte or block. This is designed for
    //the self-programming mechanisms
    void spm_write(unsigned char v, size_t pos);
    void spm_write(unsigned char* buf, unsigned char* bufset, size_t base, size_t len);

private:

    const size_t m_size;
    unsigned char* m_memory;
    unsigned char* m_tag;

};

inline size_t AVR_NonVolatileMemory::size() const
{
    return m_size;
}

inline bool AVR_NonVolatileMemory::programmed(size_t pos) const
{
    return m_tag[pos];
}

inline uint8_t AVR_NonVolatileMemory::operator[](size_t pos) const
{
    return m_memory[pos];
}

#endif //__YASIMAVR_MEMORY_H__
