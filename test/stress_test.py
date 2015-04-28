#!/usr/bin/env python

import difflib
import getopt
import os
import random
import re
import shutil
import subprocess
import sys

from pprint import pprint

MAX_LEVEL = 5
MAX_FILES = 10
MAX_LINKS = 30
MAX_SIZE = 50 * 1024 * 1024

links = []
link_count = 0

MIN_FILES_TOTAL = 30
file_count = 0

""" Display help """
def print_help():
    print 'stress_test.py [options]'
    print
    print 'Builds a directory structure both inside a secure container and outside it'
    print 'Then, it tests that both sets of files have the same attributes'
    print
    print 'Options:'
    print '\t--max-files = maximum number of files on each level of the directory structure'
    print '\t--max-level = the maximum number of levels in the directory structure'
    print '\t--max-size  = maximum size of a single file in B (default), KB or MB'
    print '\t              e.g. 1 = 1 Byte, 10B(no space between) = 10Bytes, 20MB = 20 Megabytes'
    print '\t--max-links = maximum number of links in the entire directory structure'
    print

""" Try to make an int from a String """
def str2int(s):
    try:
        return int(s)
    except ValueError:
        return None

""" Wrap command in a list to be used by the subprocess module """
def adb_command(command):
    return ['adb', 'shell', command]

""" Get a random filename """
def rand_filename():
    return '%06x' % random.randrange(16**6)

""" Generate a set of permissions """
def rand_perms():
    gp = random.randrange(0, 8)
    op = random.randrange(0, 8)
    return 7 * 0o100 + gp * 0o10 + op

""" Make a directory in both the unencrypted and encrypted paths """
def make_dir(basepaths):

    dir = rand_filename()
    dirpathn = basepaths[0] + '/' + dir

    """ Get a filename that is available """
    while os.path.exists(dirpathn):
        dir = rand_filename()
        dirpathn = basepaths[0] + '/' + dir
    dirpathe = basepaths[1] + '/' + dir

    subprocess.Popen(adb_command('mkdir ' + dirpathn)).communicate()
    subprocess.Popen(adb_command('mkdir ' + dirpathe)).communicate()

    return [dirpathn, dirpathe]

""" Make a file in both the unencrypted and encrypted paths """
def make_file(basepaths):

    file = rand_filename()
    filepathn = basepaths[0] + '/' + file

    while os.path.exists(filepathn):
        file = rand_filename()
        filepathn = basepaths[0] + '/' + file
    filepathe = basepaths[1] + '/' + file

    count = random.randrange(1, MAX_SIZE) / 512
    subprocess.Popen(adb_command
    ('dd if=/dev/urandom of=' + filepathn + ' count=' + str(count)    + ' &> /dev/null')
    ).communicate()
    perms = rand_perms()
    subprocess.Popen(adb_command('chmod ' + oct(perms) + ' ' + filepathn)).communicate()

    subprocess.Popen(adb_command('cp -p ' + filepathn + ' ' + filepathe)).communicate()

    return [filepathn, filepathe]

""" Make a link to a previously marked file, both in the unencrypted and encrypted paths """
def make_link(basepaths, targets):

    link = rand_filename()
    linkpathn = basepaths[0] + '/' + link
    while os.path.exists(linkpathn):
        link = rand_filename()
        linkpathn = basepaths[0] + '/' + link
    linkpathe = basepaths[1] + '/' + link

    subprocess.Popen(adb_command('ln -s ' + targets[0] + ' ' + linkpathn)).communicate()
    subprocess.Popen(adb_command('ln -s ' + targets[1] + ' ' + linkpathe)).communicate()

""" Build the file strucute used for testing """
def build_fs(basepaths, level = 0):

    global link_count, file_count

    for i in range(1, random.randrange(MAX_FILES)):

        type = random.randint(0, 1)
        link = random.randint(0, 2)

        """ Make a new directory """
        if type == 1 and level < MAX_LEVEL:
            dirpaths = make_dir(basepaths)
            build_fs(dirpaths, level + 1)
            """ Store as target for future symblink """
            if link > 0 and len(links) < MAX_LINKS:
                links.append(dirpaths)
        else:
            """ Make a symlink to a file """
            if link == 2 and link_count < MAX_LINKS and link_count < len(links):
                make_link(basepaths, links[link_count])
                link_count = link_count + 1
            else:
                """ Make a regular file """
                filepaths = make_file(basepaths)
                file_count = file_count + 1
                if link == 1 and len(links) < MAX_LINKS:
                    links.append(filepaths)

""" Check if everything went as expected """
def check(basepaths):
    pn = subprocess.Popen(adb_command('ls -lR ' + basepaths[0]),
                          stdout = subprocess.PIPE)
    outn, errn = pn.communicate()

    pe = subprocess.Popen(adb_command('ls -lR ' + basepaths[1]),
                          stdout = subprocess.PIPE)
    oute, erre = pe.communicate()

    date = '\d\d\d\d-\d\d-\d\d'
    time = '\d\d:\d\d'

    outn = re.sub(date, '', re.sub(time, '', outn.replace(basepaths[0], '')))
    oute = re.sub(date, '', re.sub(time, '', oute.replace(basepaths[1], '')))

    d = difflib.Differ()

    result = list(d.compare(outn.splitlines(), oute.splitlines()))

    filter_result = [s for s in result if s[:2] != '  ']
    if len(filter_result) > 0:
        sys.stdout.write('\n');
        pprint(filter_result)
        return False
    else:
        return True

""" Prepare our directories """
def prepare(basepaths):
    subprocess.Popen(adb_command('setenforce 0')).communicate()

    """ We want a clean directory """
    subprocess.Popen(adb_command('rm -rf ' + basepaths[0])).communicate()
    subprocess.Popen(adb_command('mkdir ' + basepaths[0])).communicate()
    subprocess.Popen(adb_command('rm -rf ' + basepaths[1])).communicate()
    subprocess.Popen(adb_command('mkdir ' + basepaths[1])).communicate()

    subprocess.Popen(adb_command('efs-tools storage create ' + basepaths[1] + ' 0000')).communicate()
    subprocess.Popen(adb_command('efs-tools storage unlock ' + basepaths[1] + ' 0000')).communicate()

""" Leave everyting as it was """
def cleanup(basepaths):
    subprocess.Popen(adb_command('efs-tools storage lock ' + basepaths[1])).communicate()
    subprocess.Popen(adb_command('efs-tools storage remove ' + basepaths[1])).communicate()

    subprocess.Popen(adb_command('rm -rf ' + basepaths[1])).communicate()
    subprocess.Popen(adb_command('rm -rf ' + basepaths[0])).communicate()

    subprocess.Popen(adb_command('setenforce 1')).communicate()

def parse_args(argv):
    global MAX_LEVEL, MAX_FILES, MAX_LINKS, MAX_SIZE
    try:
        opts, args = getopt.getopt(argv, '', ['max-level=', 'max-files=', 'max-links=', 'max-size='])
    except getopt.GetoptError:
        print_help()
        sys.exit()

    try:
        """ Ugly """
        for opt, arg in opts:
            arg = arg.upper()
            if opt == '--max-size':
                if arg[-1] == 'B':
                    arg = arg[:-1]
                if arg[-1] == 'K':
                    argvalue = str2int(arg[:-1])
                    if argvalue == None:
                        print_help()
                        sys.exit()
                    MAX_SIZE = argvalue * 1024
                elif arg[-1] == 'M':
                    argvalue = str2int(arg[:-1])
                    if argvalue == None:
                        print_help()
                        sys.exit()
                    MAX_SIZE = argvalue * 1024 * 1024
                else:
                    argvalue = str2int(arg)
                    if argvalue == None:
                        print_help()
                        sys.exit()
                    MAX_SIZE = argvalue
            else:
                argvalue = str2int(arg)
                if argvalue == None:
                    print_help()
                    sys.exit()
                if opt == '--max-level':
                    MAX_LEVEL = argvalue
                elif opt == '--max-files':
                    MAX_FILES = argvalue
                elif opt == '--max-links':
                    MAX_LINKS = argvalue
    except IndexError:
        print_help()
        sys.exit()

if __name__ == '__main__':
    basepaths = ['/data/data/testn', '/data/data/teste']

    parse_args(sys.argv[1:])

    sys.stdout.write('Checking device ........................................................... ')
    sys.stdout.flush()
    out, err = subprocess.Popen(
                            ['adb', 'shell', 'exit'], stderr = subprocess.PIPE
                            ).communicate()
    if err != '':
        sys.stdout.write('FAILED\n')
        sys.exit()
    sys.stdout.write('DONE\n')

    sys.stdout.write('Preparing test ............................................................ ')
    sys.stdout.flush()
    prepare(basepaths)
    sys.stdout.write('DONE\n')

    sys.stdout.write('Building file structure ................................................... ')
    sys.stdout.flush()
    while file_count < MIN_FILES_TOTAL:
        build_fs(basepaths)
    sys.stdout.write('DONE\n')

    sys.stdout.write('Checking .................................................................. ')
    sys.stdout.flush()
    if check(basepaths) == True:
        sys.stdout.write('DONE\n')

    sys.stdout.write('Cleaning up ............................................................... ')
    sys.stdout.flush()
    cleanup(basepaths)
    sys.stdout.write('DONE\n')

