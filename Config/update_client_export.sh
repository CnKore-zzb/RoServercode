#!/bin/sh

#cd bin/Debug

svn st client-export

svn st --xml client-export > client_export.xml

./update_client_export.py client_export
