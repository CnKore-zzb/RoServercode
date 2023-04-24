CC=gcc

CFLAGS=-O3 -m64 -D_LARGEFILE64_SOURCE=1

STATICLIB=libz.a

AR=ar rc
RANLIB=ranlib

OBJS = adler32.o compress.o crc32.o deflate.o gzclose.o gzlib.o gzread.o \
	gzwrite.o infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o

all: $(STATICLIB)

libz.a: $(OBJS)
	$(AR) $@ $(OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

clean:
	rm -f *.o libz.* 

depend:
	makedepend -- $(CFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE -- make depend depends on it.

adler32.o zutil.o: zutil.h zlib.h zconf.h
gzclose.o gzlib.o gzread.o gzwrite.o: zlib.h zconf.h gzguts.h
compress.o uncompr.o: zlib.h zconf.h
crc32.o: zutil.h zlib.h zconf.h crc32.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
infback.o inflate.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h

