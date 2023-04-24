#!/usr/bin/env python2
# -*- coding: utf8 -*-

import sys,os
from xml.etree import ElementTree as ET

def parse_version_xml(filename):
    reload(sys)
    sys.setdefaultencoding('utf8')
    per = ET.parse("%s.xml" % (filename))
    p = per.findall('./target/entry')

    for oneper in p:
        path = oneper.get('path')
        for child in oneper.getchildren():
            if "wc-status" == child.tag:
    #        cmd = "cp --parents '%s' %s" % (text[44:], filename)
    #        print cmd
    #        os.system(cmd)
    #        print child.tag,':',child.text
    #        print child.attrib
                item = child.get('item')
                if "unversioned" == item:
                    os.system("svn add %s" % (path))

                cmd = "svn ci %s -m '[opt] : 提交obj'" % (path)
                print cmd
                os.system(cmd)

def main(argv):
    if (len(argv) < 2):
        print '''
-- parse_version_xml --
param1: file name,  e.g. 1000_2000.xml
USAGE EXAMPLE:
./parse_version_xml.py 1000_2000.xml
'''
        sys.exit(2)

    parse_version_xml(argv[1])

if __name__ == '__main__':
    main(sys.argv)
