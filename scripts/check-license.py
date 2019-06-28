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
import glob
import os

def checkpath(path):
    if path.startswith('..'+os.path.sep+'dependencies'):
        return False
    if path.startswith('..'+os.path.sep+'build'):
        return False
    return True

cpps = [el for el in glob.glob('../**/*.cpp', recursive=True) if checkpath(el)]
hpps = [el for el in glob.glob('../**/*.hpp', recursive=True) if checkpath(el)]
pys  = [el for el in glob.glob('../**/*.py', recursive=True) if checkpath(el)]

hpps = [el for el in hpps if not el.endswith(os.path.sep+'catch.hpp')]
# Check the first line for something we want
epstart = '// EnergyPlus, Copyright (c) 1996-'
# This one is not great, but whatever
afnstart = '// Copyright (c) 2019, Alliance for Sustainable Energy, LLC'
# This one is not great, but whatever
pystart = '# Copyright (c) 2019, Alliance for Sustainable Energy, LLC'

found = 0
not_found = []
total = 0

for file in cpps + hpps:
    fp = open(file, 'r')
    txt = fp.read()
    fp.close()
    total += 1
    if not txt.startswith(epstart):
        if not txt.startswith(afnstart):
            not_found.append(file)
            continue
    found += 1

print('Found %d C++ files with a license' % found)
print('Found %d C++ files without a license' % len(not_found))
for el in not_found:
    print('\t'+el)
print('Looked at %d files' % total)
print('Crosscheck total: %d' % (found+len(not_found)))
print()

found = 0
not_found = []
total = 0

for file in pys:
    fp = open(file, 'r')
    txt = fp.read()
    fp.close()
    total += 1
    if not txt.startswith(pystart):
        not_found.append(file)
        continue
    found += 1

print('Found %d Python files with a license' % found)
print('Found %d Python files without a license' % len(not_found))
for el in not_found:
    print('\t'+el)
print('Looked at %d files' % total)
print('Crosscheck total: %d' % (found+len(not_found)))
