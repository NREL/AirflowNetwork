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

class Translator:
    def __init__(self, model):
        self.model = model
        #self.errors = []
        #self.warnings = []
        self.json = {'errors':[], 'warnings':[]}
    def error(self, mesg):
        #self.errors.append(mesg)
        self.json['errors'].append(mesg)
    def warning(self, mesg):
        #self.warnings.append(mesg)
        self.json['warnings'].append(mesg)
    def translate(self):
        return None

def xs(name):
    return '{http://www.w3.org/2001/XMLSchema}' + name

#import xml.etree.ElementTree as et
from lxml import etree as et
import glob

if __name__ == '__main__':
    
    xsd = et.parse('../input/airflownetwork.xsd').getroot()

    tree = et.parse('../input/airflownetwork.xsd')
    schema = et.XMLSchema(tree)

    files = glob.glob('../input/*.xml')

    for file in files:
        instance = et.parse(file)
        try:
            schema.assertValid(instance)
        except et.DocumentInvalid as exc:
            print(file)
            print(exc)

