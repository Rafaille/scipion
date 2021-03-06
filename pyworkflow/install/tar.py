#!/usr/bin/env python

import sys
import os
from os.path import dirname, realpath, join


SCIPION_HOME = dirname(dirname(dirname(realpath(__file__))))


def usage(error):
    print """
    ERROR: %s

    Usage: scripts/tar.py TARNAME
        TARNAME: the type of tar that is going to be generated from 
        the scipion folder. Must be 'source' or 'linux64'.
    """ % error
    sys.exit(1)


def _parseVersionDate():
    """ Parse the version and release date from scipion script. """
    scipionScript = open(join(SCIPION_HOME, 'scipion'))
    version, date = None, None

    for line in scipionScript:
        l = line.strip()
        if '__version__ =' in l:
            version = line.split("'")[1]
        elif '__releasedate__ =' in line:
            date = line.split("'")[1]
            break

    return version, date


if len(sys.argv) != 2:
    usage("Incorrect number of input parameters")


version, date = _parseVersionDate()
label = sys.argv[1]
if label == 'source':
    excludeTgz = ''
elif label == 'linux64':
    excludeTgz = "--exclude='*.tgz' --exclude='*.h' --exclude='*.cpp' --exclude='*.java'"

else:
    usage("incorrect TARNAME'")

args = {'label': label,
        'version': version,
        'date': date,
        'excludeTgz': excludeTgz
        }

# We will generate the tar from the same level of scipion git folder
cwd = os.getcwd()
os.chdir(dirname(SCIPION_HOME))

cmdStr = """ tar czf scipion_%(version)s_%(date)s_%(label)s.tgz \\
--exclude=.git --exclude='*.o' --exclude='*.os' --exclude='*pyc' \\
--exclude='*.mrc' --exclude='*.stk' --exclude='*.gz' %(excludeTgz)s \\
--exclude='software/tmp/*' --exclude='*.scons*' --exclude='config/*.conf' scipion
"""

cmd = cmdStr % args

print cmd
os.system(cmd)

# Restore current working dir
os.chdir(cwd)
