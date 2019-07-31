# Copyright (c) 2019, Alliance for Sustainable Energy, LLC
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
import json
import sys
from pprint import pprint
from collections import OrderedDict

def modify_name(name):
    return name.replace(':','_')

class BadName(Exception):
    pass

class BadField(Exception):
    pass

class UnsupportedFeature(Exception):
    pass

class AccessorCode:
    def __init__(self, getter = None, setter = None):
        self.getter = []
        if getter != None:
            self.getter = getter
        self.setter = []
        if setter != None:
            self.setter = setter

class Field:
    def __init__(self, name, **kwargs):
        self.name = name
        if(len(self.name) == 0):
            raise BadName('Bad zero-length field name')
        self.alias = kwargs.get('alias', None)
        self.default = kwargs.get('default', None)
        self.required = kwargs.get('required', False)
    def is_POD(self):
        return True
    def to_python(self, indent='    ', member_prefix='__'):
        getter_lines = []
        setter_lines = []

        outname = self.name
        if self.alias:
            outname = self.alias

        getter_lines.append('@property')
        getter_lines.append('def %s(self):' % outname)
        getter_lines.append(indent+'return self.'+member_prefix+outname)
        return getter_lines, []
    def to_dict(self, indent = '    ', member_prefix = '__', dict_name=None):
        outname = self.name
        if self.alias:
            outname = self.alias
        if dict_name == None:
            dict_name = 'data'
        if self.required:
            return [dict_name + "['" + self.name + "'] = self." + member_prefix + outname]
        else:
            return ['if self.%s != None:' % (member_prefix + outname),
                    indent + dict_name + "['" + self.name + "'] = self." + member_prefix + outname]
    def python_init(self, member_prefix='__'):
        outname = self.name
        if self.alias:
            outname = self.alias
        if self.default:
            return 'self.'+member_prefix+outname+" = kwargs.get('"+self.name+"', "+str(self.default)+')'
        elif not self.required:
            return 'self.'+member_prefix+outname+" = kwargs.get('"+self.name+"', None)"
        else:
            return 'self.'+member_prefix+outname+" = kwargs['"+self.name+"']"
        #return None

class NumberField(Field):
    def __init__(self, name, minimum=None, maximum=None, exclusive_minimum=False,
                 exclusive_maximum=False, **kwargs):
        super().__init__(name, **kwargs)
        self.minimum = minimum
        self.maximum = maximum
        self.exclusive_minimum = exclusive_minimum
        self.exclusive_maximum = exclusive_maximum
        self.integral = False
    def to_cpp(self, parent=None, definition=False, declaration=False,
              real_type='double', integral_type='int', set_prefix = 'set',
              indent='  ', camel_case=True, member_prefix='m_'):
        getter_lines = []
        setter_lines = []
        ctype = real_type
        if self.integral:
            ctype = integral_type
        if definition and declaration: # Do a header-definition-style thing
            getter_lines.append('%s %s()' % (ctype, self.name))
            getter_lines.append('{')
            getter_lines.append(indent+'return '+member_prefix+self.name+';')
            getter_lines.append('}')
            name = self.name
            if camel_case:
                name = self.name[0].upper() + self.name[1:]
            setter_lines.append('bool %s%s%s(%s value)' % (prefix, set_prefix,
                                                           name, ctype))
            setter_lines.extend(self.cpp_setter_body(name,
                                                     indent, member_prefix))
        elif definition:
            prefix = ''
            if parent:
                prefix = parent+'::'
            getter_lines.append('%s %s%s()' % (ctype, prefix, self.name))
            getter_lines.append('{')
            getter_lines.append(indent+'return '+member_prefix+self.name+';')
            getter_lines.append('}')
            name = self.name
            if camel_case:
                name = self.name[0].upper() + self.name[1:]
            setter_lines.append('bool %s%s%s(%s value)' % (prefix, set_prefix,
                                                           name, ctype))
            setter_lines.extend(self.cpp_setter_body(name,
                                                     indent, member_prefix))
        else: # Get declaration by default
            getter_lines.append('%s %s();' % (ctype, self.name))
            name = self.name
            if camel_case:
                name = self.name[0].upper() + self.name[1:]
            setter_lines.append('bool %s%s();' % (set_prefix, name))
        return (getter_lines, setter_lines)
    def cpp_setter_body(self, name, indent, member_prefix):
        setter = ['{']
        if self.minimum != None:
            comp = '>='
            if self.exclusive_minimum:
                comp = '>'
            setter.append(indent+('if (%s %s value) {' %
                                  (self.minimum, comp)))
            setter.append(indent+indent+'return false;')
            setter.append(indent+'}')
        if self.maximum != None:
            comp = '<='
            if self.exclusive_maximum:
                comp = '<'
            setter.append(indent+('if (%s %s value) {' %
                                  (self.maximum, comp)))
            setter.append(indent+indent+'return false;')
            setter.append(indent+'}')
        setter.append(indent+member_prefix+name + ' = value;')
        setter.append(indent+'return true;')
        setter.append('}')
        return setter
    def to_python(self, indent='    ', member_prefix='__'):
        getter_lines = []
        setter_lines = []

        outname = self.name
        if self.alias:
            outname = self.alias

        getter_lines.append('@property')
        getter_lines.append('def %s(self):' % outname)
        getter_lines.append(indent+'return self.'+member_prefix+outname)

        setter_lines.append('@%s.setter' % outname)
        setter_lines.append('def %s(self, value):' % outname)
        setter_lines.extend(self.python_setter_body(outname, indent, member_prefix=member_prefix))
        return (getter_lines, setter_lines)
    def python_setter_body(self, name, indent, member_prefix = '__'):
        setter = []
        if self.minimum != None:
            comp = '>='
            if self.exclusive_minimum:
                comp = '>'
            setter.append(indent+('if %s %s value:' %
                                  (self.minimum, comp)))
            setter.append(indent+indent+'return')
        if self.maximum != None:
            comp = '<='
            if self.exclusive_maximum:
                comp = '<'
            setter.append(indent+('if %s %s value:' %
                                  (self.maximum, comp)))
            setter.append(indent+indent+'return')
        setter.append(indent+'self.'+member_prefix+name + ' = value')
        return setter

class ReferenceField(Field):
    def __init__(self, name, object_list=[], **kwargs):
        super().__init__(name, **kwargs)
        self.name = name
        self.allowed = object_list
    def is_POD(self):
        return False

class EnumerationField(Field):
    def __init__(self, name, enum=None, **kwargs):
        super().__init__(name, **kwargs)
        self.enum = enum # should really verify that this is legit
        if self.default != None:
            self.default = '"' + self.default + '"'
    def to_python(self, indent='    ', member_prefix='__'):
        getter_lines = []
        setter_lines = []

        outname = self.name
        if self.alias:
            outname = self.alias

        getter_lines.append('@property')
        getter_lines.append('def %s(self):' % outname)
        getter_lines.append(indent+'return self.'+member_prefix+outname)

        setter_lines.append('@%s.setter' % outname)
        setter_lines.append('def %s(self, value):' % outname)
        setter_lines.extend(self.python_setter_body(outname, indent, member_prefix=member_prefix))
        return (getter_lines, setter_lines)
    def python_setter_body(self, name, indent, member_prefix = '__'):
        setter = []
        if self.enum:
            #if '' in self.enum and self.default != None:
            #    setter.append(indent + "if value == '':")
            #    setter.append(indent + indent + 'self.'+member_prefix+name + " = '%s'" % self.default)
            #    opts = self.enum[:]
            #    opts.remove('')
            #    setter.append(indent+('if value in %s:' % str(opts)))
            #    setter.append(indent+indent+'self.'+member_prefix+name + ' = value')
            #else:
            setter.append(indent+('if value in %s:' % str(self.enum)))
            setter.append(indent+indent+'self.'+member_prefix+name + ' = value')
        return setter

class FreeEntryField(Field):
    def __init__(self, name, **kwargs):
        super().__init__(name, **kwargs)
    def to_python(self, indent='    ', member_prefix='__'):
        getter_lines = []
        setter_lines = []
        return (getter_lines, setter_lines)

class NameField:
    def __init__(self, **kwargs):
        pass

class FieldObject:
    def __init__(self, name, fields, alias=None):
        self.name = name
        self.fields = fields
        self.alias = alias
    def is_POD(self):
        for field in self.fields:
            if not field.is_POD():
                return False
        return True
    def to_python(self, parent=None, indent='    ', member_prefix='__'):
        # Make an init function
        lines = []
        outname = self.name
        if self.alias:
            outname = self.alias
        postfix = ':'
        if parent:
            postfix = '(%s):' % parent
        lines.append('class '+outname+postfix)
        lines.append(indent + 'def __init__(self, **kwargs):')
        if parent:
            lines.append(indent + indent + 'super().__init__(kwargs)')
        for field in self.fields:
            initializer = field.python_init(member_prefix=member_prefix)
            if initializer:
                lines.append(indent + indent + initializer)
        for field in self.fields:
            getter_lines, setter_lines = field.to_python()
            for line in getter_lines:
                lines.append(indent+line)
            for line in setter_lines:
                lines.append(indent+line)
        lines.append(indent + 'def to_dict(self):')
        lines.append(indent+indent+'obj = {}')
        for field in self.fields:
            for outline in field.to_dict(member_prefix=member_prefix, dict_name='obj'):
                lines.append(indent+indent+outline)
        lines.append(indent+indent+'return obj')
        return '\n'.join(lines)

class Translator:
    def __init__(self):
        self.references = {}
        self.objects = {}
        self.warnings = []
    def warning(self,message):
        print(message)
        self.warnings.append(message)
    def translate(self, data):
        for key, value in data.items():
            fields = []
            print(key)
            field_data = value['patternProperties']['.*']['properties']
            required_fields = value['patternProperties']['.*'].get('required', [])
            for field_name, field_details in field_data.items():
                print(field_name)
                if field_name in required_fields:
                    field_details['required'] = True
                field_type = 'string'
                if 'type' in field_details:
                    field_type = field_details['type']
                if field_type == 'number':
                    fields.append(NumberField(field_name, **field_details))
                    continue
                elif field_type == 'string':
                    if 'data_type' in field_details:
                        data_type = field_details['data_type']
                        if data_type == 'object_list':
                            fields.append(ReferenceField(field_name, **field_details))
                            continue
                    elif 'enum' in field_details:
                        fields.append(EnumerationField(field_name, **field_details))
                        continue
                    else:
                        self.warning('Field "%s" in %s is free entry' % (field_name, key))
                        fields.append(FreeEntryField(field_name, **field_details))
                        continue
                elif field_type == 'array':
                    raise UnsupportedFeature('Field type "array" in %s not supported' % key)
                # Oops, failed
                string = ''
                for k,v in field_details.items():
                    string += '%s : %s\n' % (k,v)
                raise(BadField('Failed to map field "%s" of "%s"\n%s' % (field_name,
                                                                     key,string)))
            modname = modify_name(key)
            self.objects[modname] = FieldObject(modname, fields)
    def add_references(self, name, data):
        name = modify_name(name)
        #self.objects[name] = data
        #kind = data['patternProperties']['.*']['type']
        #props = data['patternProperties']['.*']['properties']
        #for key,value in data.items():
        #    print('\t',key)
        #pprint(props)
        #for field_name,attrs in props.items():
        #    print(field_name, '=>', attrs['type'])
        # Does the object have a name?
        if 'name' in data:
            if 'reference' in data['name']:
                #reference_name = data['name']['reference']
                for reference_name in data['name']['reference']:
                    if reference_name in self.references:
                        self.references[reference_name].append(name)
                    else:
                        self.references[reference_name] = [name]
        #pprint(data['name'])
    def build(self):
        self.fields = {}
        for name,data in self.objects.items():
            self.fields[name] = []
            if 'name' in data:
                self.fields[name].append(NameField(data['name']))
            for prop_name,prop in data['patternProperties']['.*']['properties'].items():
                kind = prop['type']
                if kind == 'number':
                    self.fields[name].append(NumberField(prop_name,prop))
                elif kind == 'string':
                    data_type = prop['data_type']
                    if data_type == 'object_list':
                        self.fields[name].append(Referencefield(prop_name, prop))
    def merge_alias_file(self, filename):
        try:
            fp = open(filename, 'r')
            for line in fp:
                data = line.split(',')
##                if len(data) == 2:
##                    # No alias selected
##                    if data[0] in aliases:
##                        if not data[1] in aliases[data[0]]:
##                            aliases[data[0]][data[1]] = None
##                    else:
##                        aliases[data[0]] = {}
##                        aliases[data[0]][data[1]] = None
                if len(data) == 3:
                    # Alias selected
                    data = [el.strip() for el in data]
                    if data[2] == '':
                        continue
                    if data[0] in self.objects:
                        for field in self.objects[data[0]].fields:
                            if field.name == data[1]:
                                field.alias = data[2]
                                break
                        else:
                            print('Failed to find field %s in %s' % (data[1], data[0]))
                    else:
                        print('Failed to find object %s' % data[0])
            fp.close()
        except FileNotFoundError:
            pass
        fp = open(filename, 'w')
        for name, obj in self.objects.items():
            for field in obj.fields:
                data = [name, field.name]
                if field.alias:
                    data.append(field.alias)
                fp.write(','.join(data)+'\n')
        fp.close()
    def write_alias_file(self, filename):
        fp = open(filename, 'w')
        for name, obj in self.objects.items():
            for field in obj.fields:
                data = [name, field.name]
                if field.alias:
                    data.append(field.alias)
                fp.write(','.join(data)+'\n')
        fp.close()
    def to_python(self, filename=None, indent='    ', member_prefix='__'):
        txt = ''
        for name,obj in self.objects.items():
            print('#',name)
            txt += obj.to_python(indent=indent, member_prefix=member_prefix) + '\n\n' 
        fp = sys.stdout
        if filename != None:
            fp = open(filename,'w')
        fp.write(txt)
        if fp != sys.stdout:
            fp.close()
        
def jsonToObject(name, data):
    kind = data['patternProperties']['.*']['type']
    props = data['patternProperties']['.*']['properties']
    #for key,value in data.items():
    #    print('\t',key)
    #pprint(props)
    for name,attrs in props.items():
        print(name, '=>', attrs['type'])
    # Does the object have a name?
    if 'name' in data:
        if 'reference' in data['name']:
            reference = data['name']['reference']
    pprint(data['name'])

fp = open('Energy+.schema.epJSON')
data = json.load(fp, object_pairs_hook=OrderedDict)
fp.close()

translator = Translator()

#pprint(data)

#for key,val in data['properties'].items():
#    print(key)

#translator.translate('RunPeriod', data['properties']['RunPeriod'])

# Add all the objects
for name, val in data['properties'].items():
    translator.add_references(name, val)
    
#print(translator.references)

objects = data['properties']

fields = objects['AirflowNetwork:MultiZone:Surface:Crack']['patternProperties']['.*']['properties']



for key,value in fields.items():
    print(key,value['type'])

name = 'air_mass_flow_coefficient_at_reference_conditions'
field = fields[name]

obj = NumberField(name, **field)
r = obj.to_cpp(parent='Crack', camel_case=False, definition=True, set_prefix='set_')

print('\n'.join(r[0]))
print()
print('\n'.join(r[1]))

keys = list(fields.keys())
print(keys)

k = 'AirflowNetwork:MultiZone:Surface:Crack'
k = 'AirflowNetwork:MultiZone:Component:DetailedOpening'
k = 'AirflowNetwork:MultiZone:Component:SimpleOpening'
#objs = { k: objects[k]}

#obj_list = ['AirflowNetwork:MultiZone:Surface:Crack',
#            'AirflowNetwork:MultiZone:Component:DetailedOpening',
#            'AirflowNetwork:MultiZone:Component:SimpleOpening']
obj_list = [#'RoomAir:Node:AirflowNetwork:AdjacentSurfaceList',
            #'RoomAir:Node:AirflowNetwork:InternalGains',
            #'RoomAir:Node:AirflowNetwork:HVACEquipment',
            'AirflowNetwork:SimulationControl',
            'AirflowNetwork:MultiZone:Zone',
            'AirflowNetwork:MultiZone:Surface',
            'AirflowNetwork:MultiZone:ReferenceCrackConditions',
            'AirflowNetwork:MultiZone:Surface:Crack',
            'AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea',
            #'AirflowNetwork:MultiZone:Component:DetailedOpening',
            'AirflowNetwork:MultiZone:Component:SimpleOpening',
            'AirflowNetwork:MultiZone:Component:HorizontalOpening',
            'AirflowNetwork:MultiZone:Component:ZoneExhaustFan',
            'AirflowNetwork:MultiZone:ExternalNode',
            #'AirflowNetwork:MultiZone:WindPressureCoefficientArray',
            #'AirflowNetwork:MultiZone:WindPressureCoefficientValues',
            'AirflowNetwork:ZoneControl:PressureController',
            'AirflowNetwork:Distribution:Node',
            'AirflowNetwork:Distribution:Component:Leak',
            'AirflowNetwork:Distribution:Component:LeakageRatio',
            'AirflowNetwork:Distribution:Component:Duct',
            'AirflowNetwork:Distribution:Component:Fan',
            'AirflowNetwork:Distribution:Component:Coil',
            'AirflowNetwork:Distribution:Component:HeatExchanger',
            'AirflowNetwork:Distribution:Component:TerminalUnit',
            'AirflowNetwork:Distribution:Component:ConstantPressureDrop',
            'AirflowNetwork:Distribution:Component:OutdoorAirFlow',
            'AirflowNetwork:Distribution:Component:ReliefAirFlow',
            'AirflowNetwork:Distribution:Linkage',
            #'AirflowNetwork:Distribution:DuctViewFactors',
            'AirflowNetwork:OccupantVentilationControl',
            'AirflowNetwork:IntraZone:Node',
            'AirflowNetwork:IntraZone:Linkage']            
objs = {}
for name in obj_list:
    objs[name] = objects[name]

translator.translate(objs)
translator.merge_alias_file('aliases.csv')

print(translator.objects['AirflowNetwork_MultiZone_Component_SimpleOpening'].to_python())
print(translator.objects['AirflowNetwork_MultiZone_Surface_Crack'].to_python())

#translator.write_alias_file('aliases.csv')

translator.to_python('afn_auto.py')
