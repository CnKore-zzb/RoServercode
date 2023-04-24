import os
import glob
import codecs

#Server
for a in glob.glob('.\\*\\*\\*'):
    b = a[a.rindex('.') + 1:]
    if b == 'json':
        fp = open(a, 'rb')
        source = fp.read()
        fp.close()

        if source[:3] == codecs.BOM_UTF8:
            source = source[3:]

            fp = open(a, "wb")
            #source.replace(b'\r\n', b'\n')
            fp.write(source)
            fp.close()

            print a

os.system("pause")