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
