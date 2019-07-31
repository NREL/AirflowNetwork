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
import csv


class IncompleteFile(Exception):
    pass


class BadInput(Exception):
    pass


environment_string = '1,5,Environment Title[],Latitude[deg],Longitude[deg],Time Zone[],Elevation[m]'

timestamp_strings = '''2,6,Day of Simulation[],Month[],Day of Month[],DST Indicator[1=yes 0=no],Hour[],StartMinute[],EndMinute[],DayType
3,3,Cumulative Day of Simulation[],Month[],Day of Month[],DST Indicator[1=yes 0=no],DayType  ! When Daily Report Variables Requested
4,2,Cumulative Days of Simulation[],Month[]  ! When Monthly Report Variables Requested
5,1,Cumulative Days of Simulation[] ! When Run Period Report Variables Requested
6,1,Calendar Year of Simulation[] ! When Annual Report Variables Requested'''

class EsoEnvironment:
    def __init__(self, split, offset):
        if len(split) != 6:
            raise BadInput('Environment line has %d fields rather than 6' % len(split))
        if split[0].strip() != '1':
            raise BadInput('Environment line does not begin with the expected "1"')
        self.string = ','.join(split)
        self.title = split[1]
        self.latitude = float(split[2])
        self.longitude = float(split[3])
        self.time_zone = float(split[4])
        self.elevation = float(split[5])
        self.offset = offset
        

class EsoField:
    def __init__(self, string, comment):
        self.fullname = string.strip()
        self.units = None
        if '[]' in string:
            self.name = string.replace('[]','').strip()
        else:
            self.name,sep,self.units = string.partition('[')
            self.name = self.name.strip()
            self.units = self.units.replace(']').strip()


class EsoTimeStamp:
    def __init__(self, dayofsim, year=None, month=None, day=None, hour=None,
                 startminute=None, endminute=None, daytype=None, stamptype=None):
        self.dayofsim=dayofsim
        self.year=year
        self.month=month
        self.day=day
        self.hour=hour # Hour of the day, not clock hour
        self.startminute=startminute
        self.endminute=endminute
        self.daytype=daytype
        self.stamptype=stamptype


class EsoTimeStampFormat:
    def __init__(self,string):
        split = [el.strip() for el in string.split(',')]
        self.code = split[0]
        count = split[1]
        self.fields = split[2:]
        self.count = len(self.fields)
        
    def parse(self, row):
        pass


class EsoDictionaryEntry:
    def __init__(self,string):
        descr_str, dummy, freq_str = string.partition('!')
        split = descr_str.split(',')
        self.id = int(split[0])
        self.count = int(split[1])
        self.key = split[2]
        self.variable = split[3].strip()
        if '[' in self.variable and ']' in self.variable:
            self.variable,sep,self.units = self.variable.partition('[')
            self.units,sep,dummy = self.units.partition(']')
            self.variable = self.variable.strip()
        self.frequency = freq_str.strip()

    def __str__(self):
        return ','.join(['%d'%self.id, '%d'%self.count, self.key, self.variable + ' ['+self.units+']']) + ' !' + self.frequency

    def __repr__(self):
        return self.__str__()


class EsoDictionaryDiff:
    def __init__(self, left, right):
        self.left = left
        self.right = right
        self.mapping = {}
        self.missing_from_left = {}
        self.missing_from_right = {}
        self.mapping_left, self.multiples_left, self.missing_left = self.compare(left, right)
        self.mapping_right, self.multiples_right, self.missing_right = self.compare(right, left)

    def compare(self, left, right):
        lookup = {}
        mapping = {}
        missing_left= []
        multiples = {}
        for el in left.dictionary:
            if not el.key in lookup:
                lookup[el.key] = {}
            if not el.variable in lookup[el.key]:
                lookup[el.key][el.variable] = []
            lookup[el.key][el.variable].append((el.id, el.frequency))
        for el in right.dictionary:
            try:
                details = lookup[el.key][el.variable]
            except KeyError:
                missing_left.append(el)
                continue
            matches = []
            for item in details:
                if el.frequency == item[1]:
                    matches.append(item[0])
            if len(matches) > 1:
                multiples[el.id] = matches
            elif len(matches) == 0:
                missing_left.append(el)
            else:
                mapping[matches[0]] = el.id
        return mapping, multiples, missing_left 


def result_diff(left, right):
    diffmax = 0.0
    #print(left, right)
    try:
        for pt in zip(left,right):
            diffmax = max(abs(float(pt[0])-float(pt[1])), diffmax)
    except TypeError:
        for pts in zip(left,right):
            for pt in zip(pts[0],pts[1]):
                diffmax = max(abs(float(pt[0])-float(pt[1])), diffmax)
    return diffmax

class EsoDiff(EsoDictionaryDiff):
    def __init__(self, left, right):
        super().__init__(left, right)
        # Figure out what we should do with environments
        if len(left.environments) != len(right.environments):
            # Bail out for now
            raise BadInput('ESO files have differing numbers of environments')
        self.results = []
        for i in range(len(left.environments)):
            errors = {}
            for id_left, id_right in self.mapping_left.items():
                #print(id_left, id_right)
                result_left = left.get_by_id(left.environments[i], id_left)
                result_right = right.get_by_id(right.environments[i], id_right)
                errors[id_left] = result_diff(result_left, result_right)
            self.results.append(errors)


class EsoFile:
    def __init__(self,filename):
        self.fp = open(filename, 'r')
        self.version = next(self.fp)
        self.incomplete = False
        self.dictionary = None
        self.offset = None
        self.messages = []
        dd = []
        count = 1
        for line in self.fp:
            count += 1
            if line.startswith('End'):
                break
            dd.append(line)
        else:
            self.incomplete = True
            self.messages.append('Encountered end of file in data dictionary')
            raise IncompleteFile('Encountered end of file in data dictionary')
        # Need at least the location information, a timestamp, and a variable
        if len(dd) < 3:
            raise IncompleteFile('Insufficient entries in data dictionary')
        self.offset = count
        self.timestamps = []
        self.dictionary = []
        self.dd = dd
        # The first item should be the location information
        env = dd.pop(0).strip()
        if not env == environment_string:
            raise IncompleteFile('Failed to find environment entry in data dictionary')
        # Skip the timestamps for the moment
        dd.pop(0)
        dd.pop(0)
        dd.pop(0)
        dd.pop(0)
        dd.pop(0)
        for el in dd:
            self.dictionary.append(EsoDictionaryEntry(el))
        # Now have all the dictionary handled, gather up the environments
        self.environments = self.find_environments()

    def rewind(self):
        self.fp.seek(0)
        self.reader = csv.reader(self.fp)
        for i in range(self.offset):
            next(self.reader)
        self.messages.append('Rewind to start of data')

    def rewindto(self, offset):
        self.rewind()
        for i in range(offset):
            next(self.reader)

    def find_environments(self):
        self.rewind()
        results = []
        count = 1
        for row in self.reader:
            if row[0] == '1':
                results.append(EsoEnvironment(row, count))
            count += 1
        self.messages.append('Found %d environment(s)' % len(results))
        return results

    def get_by_id(self, environment, id):
        for el in self.dictionary:
            if id == el.id:
                break
        else:
            return None
        # OK! Do it!
        result = []
        self.rewindto(environment.offset)
        idstring = str(id)
        for row in self.reader:
            if row[0] == idstring:
                result.append(row[1:])
            elif row[0] == '1':
                break
        return result

    def get(self, environment, key, variable, frequency):
        id = None
        for el in self.dictionary:
            print('"%s" "%s"' % (variable,el.variable))
            if variable == el.variable:
                if key == el.key:
                    id = el.id
                break
        else:
            return None
        lookup = {'Daily':'3',
                  'Monthly':'4',
                  'Run Period':'5'}
        if frequency in lookup:
            timecode = lookup[frequency]
        else:
            timecode = '2'
        for line in self.fp:
            if line.startswith(timecode):
                dt = line
            elif line.startswith(code):
                print('\t',line)
            elif line.startswith('1'):
                rp = line
        # OK! Do it!
        self.rewindto(environment.offset)

    def getEntryByRunPeriod(self,entry,runperiod):
        lookup = {'Daily':'3',
                  'Monthly':'4',
                  'Run Period':'5'}
        if entry.frequency in lookup:
            timecode = lookup[entry.frequency]
        else:
            timecode = '2'
        # Find the start of the run period
        for line in self.fp:
            if line.startswith('1,'):
                print('X',line)
                split = line.split(',')
                print(split[1])
                print(runperiod)
                if split[1] == runperiod:
                    break
        # Now the data
        for line in self.fp:
            if line.startswith(timecode):
                dt = line
            elif line.startswith(entry.code):
                print('\t',line)
            elif line.startswith('1,'):
                break

if __name__ == '__main__':
    filename1 = 'c:/Users/jdegraw/Desktop/afn-coil/new-coil/eplusout.eso'
    filename2 = 'c:/Users/jdegraw/Desktop/afn-coil/old-coil/eplusout.eso'

#file = 'RefBldgMediumOfficeNew2004_v1.4_8.7_5A_USA_IL_CHICAGO-OHARE-CustomRange.eso'

    #fp1 = open(filename1,'r')
    reader1 = EsoFile(filename1)

    #fp2 = open(filename2,'r')
    reader2 = EsoFile(filename2)

    #dict_diff = EsoDictionaryDiff(reader1,reader2)
    #reader1.compare_dictionaries(reader2)

    #print(reader.dictionary[4])
    
    #reader.get('Electricity:Facility','Hourly')
    #reader.getEntryByRunPeriod(reader.dictionary[5],'Chicago Ohare Intl Ap IL USA TMY3 WMO#=725300')

    for env in reader1.environments:
        print(env.title, env.offset + 9 + len(reader1.dictionary))

    for entry in reader1.dictionary:
        print(entry.id, entry.key, entry.variable)

    v636 = reader1.get_by_id(reader1.environments[1], 636)

    diff = EsoDiff(reader1,reader2)
