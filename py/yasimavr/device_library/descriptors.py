# descriptors.py
#
# Copyright 2021 Clement Savergne <csavergne@yahoo.com>
#
# This file is part of yasim-avr.
#
# yasim-avr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# yasim-avr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with yasim-avr.  If not, see <http://www.gnu.org/licenses/>.

'''
This module defines Descriptor classes that contain the variant
configuration decoded from YAML configuration files
'''

import yaml
import weakref
import os
import sys
import collections


Architectures = ['AVR', 'Mega0']


#List of path which are searched for YAML configuration files
#This can be altered by the used
ConfigRepositories = [
    os.path.join(os.path.dirname(__file__), 'configs')
]

def _find_config_file(fn):
    path_list = ConfigRepositories + sys.path
    for r in path_list:
        p = os.path.join(r, fn)
        if os.path.isfile(p):
            return p
    return None


class DeviceConfigException(Exception):
    pass

#Internal utility that returns a absolute register address based
#on its offset and the peripheral base address if it has one.
def _get_reg_address(reg_descriptor, base):

    if isinstance(reg_descriptor, ProxyRegisterDescriptor):
        return _get_reg_address(reg_descriptor.reg, base) + reg_descriptor.offset
    else:
        if reg_descriptor.address >= 0:
            return reg_descriptor.address
        elif base >= 0 and reg_descriptor.offset >= 0:
            return base + reg_descriptor.offset
        else:
            raise DeviceConfigException('Register address cannot be resolved')


MemorySegmentDescriptor = collections.namedtuple('_MemorySegmentDescriptor', ['start', 'end'])

class MemorySpaceDescriptor:
    '''Descriptor class for a memory space'''

    def __init__(self, name, mem_config):
        self.name = name
        self.segments = {}
        memend = 0

        mem_config_lc = {k.lower(): v for k, v in mem_config.items()}
        for segmarker in mem_config_lc:
            if segmarker.endswith('end') and segmarker != 'memend':
                segend = int(mem_config_lc[segmarker])

                segstartmarker = segmarker.replace('end', 'start')
                segstart = int(mem_config_lc.get(segstartmarker, 0))

                segname = segmarker.replace('end', '').lower()
                self.segments[segname] = MemorySegmentDescriptor(segstart, segend)

                memend = max(memend, segend)

        self.memend = int(mem_config.get('memend', memend))


class InterruptMapDescriptor:
    '''Descriptor class for a interrupt vector map'''

    def __init__(self, int_config):
        self.vector_size = int(int_config['vector_size'])
        self.vectors = list(int_config['vectors'])
        self.sleep_mask =dict(int_config.get('sleep_mask', {}))


class RegisterFieldDescriptor:
    '''Descriptor class for a field of a I/O register'''

    def __init__(self, field_name, field_config, reg_size):
        self.name = field_name
        self.kind = str(field_config.get('kind', 'RAW'))

        if self.kind == 'BIT':
            self.pos = int(field_config['pos'])
            self.one = field_config.get('one', 1)
            self.zero = field_config.get('zero', 0)

        elif self.kind == 'ENUM':
            self.LSB = int(field_config.get('LSB', 0))
            self.MSB = int(field_config.get('MSB', reg_size - 1))

            self.values = {}

            fvalues = field_config.get('values', None)
            if isinstance(fvalues, list):
                self.values = {i: v for i, v in enumerate(fvalues)}
            elif isinstance(fvalues, dict):
                self.values = dict(fvalues)
            else:
                self.values = None

            if self.values is not None:
                fvalues = field_config.get('values2', {})
                self.values.update(fvalues)

        elif self.kind == 'INT':
            self.LSB = int(field_config.get('LSB', 0))
            self.MSB = int(field_config.get('MSB', reg_size - 1))
            self.unit = str(field_config.get('unit', ''))

        elif self.kind == 'RAW':
            self.LSB = int(field_config.get('LSB', 0))
            self.MSB = int(field_config.get('MSB', reg_size - 1))

        else:
            raise DeviceConfigException('Field kind unknown: ' + self.kind)

        self.readonly = bool(field_config.get('readonly', False))

    def shift_mask(self):
        if self.kind == 'BIT':
            return self.pos, 1
        else:
            mask = (1 << (self.MSB - self.LSB + 1)) - 1
            return self.LSB, mask


class RegisterDescriptor:
    '''Descriptor class for a I/O register'''

    def __init__(self, reg_config):
        self.name = str(reg_config['name'])

        if 'address' in reg_config:
            self.address = int(reg_config['address'])
        elif 'offset' in reg_config:
            self.address = -1
            self.offset = int(reg_config['offset'])
        else:
            raise DeviceConfigException('No address for register ' + self.name)

        self.size = int(reg_config.get('size', 1))

        self.kind = str(reg_config.get('kind', ''))

        self.fields = {}
        for field_name, field_config in dict(reg_config.get('fields', {})).items():
            self.fields[field_name] = RegisterFieldDescriptor(field_name, field_config, self.size)

        self.readonly = bool(reg_config.get('readonly', False))
        self.supported = bool(reg_config.get('supported', True))


class ProxyRegisterDescriptor:
    '''Descriptor class for a register proxy, used to represent the
    high and low parts of a 16-bits register'''

    def __init__(self, r, offset):
        self.reg = r
        self.offset = offset


class PeripheralClassDescriptor:
    '''Descriptor class for a peripheral type'''

    def __init__(self, per_config):
        self.registers = {}
        for reg_config in list(per_config.get('registers', [])):
            r = RegisterDescriptor(reg_config)
            self.registers[r.name] = r

            if r.size == 2:
                self.registers[r.name + 'L'] = ProxyRegisterDescriptor(r, 0)
                self.registers[r.name + 'H'] = ProxyRegisterDescriptor(r, 1)


        self.config = per_config.get('config', {})


class PeripheralInstanceDescriptor:
    '''Descriptor class for the instanciation of a peripheral type'''

    def __init__(self, name, loader, f, device):
        self.name = name
        self.per_class = f.get('class', name)
        self.reg_base = f.get('base', -1)
        self.class_descriptor = loader.load_peripheral(self.per_class, f['file'])
        self.device = device
        self.config = f.get('config', {})

    def reg_address(self, reg_name, default=None):
        if '.' in reg_name:
            per_name, reg_name = reg_name.split('.', 1)
            per = self.device.peripherals[per_name]
        else:
            per = self

        try:
            reg_descriptor = per.class_descriptor.registers[reg_name]
            return _get_reg_address(reg_descriptor, per.reg_base)
        except:
            if default is not None:
                return default
            else:
                raise


#Utility class that manages caches of peripheral configurations and
#loaded YAML configuration files
#This is only used at configuration loading time and discarded once
#the configuration loading is complete
class _DeviceDescriptorLoader:

    def __init__(self, yml_cfg):
        self.cfg = yml_cfg
        self._yml_cache = {}
        self._per_cache = {}

    def load_peripheral(self, per_name, per_filepath):
        if per_name in self._per_cache:
            return self._per_cache[per_name]

        if per_filepath in self._yml_cache:
            per_yml_doc = self._yml_cache[per_filepath]
        else:
            per_path = _find_config_file(per_filepath)
            if per_path is None:
                raise DeviceConfigException('Config file not found: ' + per_filepath)

            f = open(per_path)
            per_yml_doc = yaml.safe_load(f)
            self._yml_cache[per_filepath] = per_yml_doc
            f.close()

        per_config = per_yml_doc[per_name]
        per_descriptor = PeripheralClassDescriptor(per_config)
        self._per_cache[per_name] = per_descriptor
        return per_descriptor


class DeviceDescriptor:
    '''Top-level descriptor for a device variant, storing the configuration
    from a YAML configuration file.
    '''

    #Instance cache to speed up the loading of a device several times
    _cache = weakref.WeakValueDictionary()

    def __new__(cls, devname):
        lower_devname = devname.lower()
        if lower_devname in cls._cache:
            return cls._cache[lower_devname]
        else:
            desc = super().__new__(cls)
            desc._load_config(devname)
            cls._cache[lower_devname] = desc
            return desc

    def _load_config(self, devname):
        yml_cfg = self._read_config(devname)

        self.name = str(yml_cfg['name'])

        if 'aliasof' in yml_cfg:
            yml_cfg = self._read_config(str(yml_cfg['aliasof']).lower())

        dev_loader = _DeviceDescriptorLoader(yml_cfg)

        self.architecture = str(yml_cfg['architecture'])
        if self.architecture not in Architectures:
            raise DeviceConfigException('Unsupported architecture: ' + self.architecture)

        self.mem_spaces = {}
        for name, f in dict(yml_cfg['memory']).items():
            self.mem_spaces[name] = MemorySpaceDescriptor(name, f)

        self.core_attributes = dict(yml_cfg['core'])

        self.fuses = dict(yml_cfg.get('fuses', {}))

        self.access_config = dict(yml_cfg.get('access', {}))

        self.pins = list(yml_cfg['pins'])

        self.interrupt_map = InterruptMapDescriptor(dict(yml_cfg['interrupts']))

        self.peripherals = {}
        for per_name, f in dict(yml_cfg['peripherals']).items():
            self.peripherals[per_name] = PeripheralInstanceDescriptor(per_name, dev_loader, f, self)

    @classmethod
    def _read_config(cls, devname):
        fn = _find_config_file(devname + '.yml')
        if fn is None:
            raise DeviceConfigException('No configuration found for variant ' + devname)

        f = None
        try:
            f = open(fn)
            yml_cfg = yaml.safe_load(f)
        except Exception as exc:
            msg = 'Error reading the configuration file for ' + devname
            raise DeviceConfigException(msg) from exc
        else:
            return yml_cfg
        finally:
            if f:
                f.close()
