# Makefile for cryptar. This is processed by configure to produce the final
# Makefile

prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man

LIBS=-lpopt -lresolv -lssl -lz -Lopenssl -ltomcrypt
CC=gcc
CFLAGS=-g -DHAVE_CONFIG_H -Wall -W `pkg-config --cflags glib-2.0` -DMAINTAINER_MODE ${CRYPTAR_COMPILE_CLEARTEXT}
#CFLAGS=-g -DHAVE_CONFIG_H -Wall -W -I/usr/src/glib1.2-1.2.10 -DMAINTAINER_MODE
LDFLAGS=`pkg-config --libs glib-2.0` -ldb
#LDFLAGS=-L/usr/src/glib1.2-1.2.10/.libs -lglib -ldb

INSTALLCMD=/usr/bin/install -c
INSTALLMAN=/usr/bin/install -c

srcdir=.

SHELL=/bin/sh

VERSION=@VERSION@

.SUFFIXES:
.SUFFIXES: .c .o

LIBOBJ=lib/fnmatch.o lib/compat.o lib/snprintf.o lib/mdfour.o \
	lib/permstring.o \

ZLIBOBJ=zlib/deflate.o zlib/infblock.o zlib/infcodes.o zlib/inffast.o \
	zlib/inflate.o zlib/inftrees.o zlib/infutil.o zlib/trees.o \
	zlib/zutil.o zlib/adler32.o 
OBJS1=cryptar.o generator.o receiver.o cleanup.o sender.o exclude.o util.o main.o checksum.o match.o syscall.o log.o backup.o
OBJS2=option.o options.o flist.o io.o compat.o hlink.o token.o uidlist.o socket.o fileio.o batch.o \
	clientname.o
DAEMON_OBJ = params.o loadparm.o clientserver.o access.o connection.o authenticate.o
popt_OBJS=popt/findme.o  popt/popt.o  popt/poptconfig.o \
	popt/popthelp.o popt/poptparse.o
OBJS=$(OBJS1) $(OBJS2) $(DAEMON_OBJ) $(LIBOBJ) $(ZLIBOBJ) 
# And for testing, redefine
OBJS=main.o util.o option.o fileio.o cryptar.o syscall.o options.o coprocess.o prefs.o extract.o list.o daemon.o archive.o env.o io.o protocol.o ios.o checksum.o sum.o db_misc.o remote.o covering.o hash.o queue.o workticket.o encryption.o compress.o requests.o log.o filename.o summary.o block.o key.o constants.o

DB_OBJS=cryptar-db.o db_misc.o filename.o summary.o block.o remote.o options.o prefs.o ios.o syscall.o checksum.o hash.o key.o constants.o

TLS_OBJ = tls.o syscall.o lib/permstring.o 

# Programs we must have to run the test cases
CHECK_PROGS = cryptar tls getgroups trimslash

# note that the -I. is needed to handle config.h when using VPATH
.c.o:
#
	$(CC) -I. -I$(srcdir) $(CFLAGS) -c $< -o $@
#

all: cryptar cryptar-db

# To compile without crypto, use this command line (under bash):
#    CRYPTAR_COMPILE_CLEARTEXT="-DCLEARTEXT=1" make

man: cryptar.1 cryptard.conf.5

install: all
	-mkdir -p ${DESTDIR}${bindir}
	${INSTALLCMD} ${STRIP} -m 755 cryptar ${DESTDIR}${bindir}
	-mkdir -p ${DESTDIR}${mandir}/man1
	-mkdir -p ${DESTDIR}${mandir}/man5
	${INSTALLMAN} -m 644 $(srcdir)/cryptar.1 ${DESTDIR}${mandir}/man1
	${INSTALLMAN} -m 644 $(srcdir)/cryptard.conf.5 ${DESTDIR}${mandir}/man5

install-strip:
	$(MAKE) STRIP='-s' install

cryptar: $(OBJS)
#@echo "Please ignore warnings below about mktemp -- it is used in a safe way"
	$(CC) $(CFLAGS) $(LDFLAGS) -o cryptar $(OBJS) $(LIBS)

cryptar-db: $(DB_OBJS)
#@echo "Please ignore warnings below about mktemp -- it is used in a safe way"
	$(CC) $(CFLAGS) $(LDFLAGS) -o cryptar-db $(DB_OBJS) $(LIBS)

$(OBJS): config.h

tls: $(TLS_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TLS_OBJ) $(LIBS)

getgroups: getgroups.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ getgroups.o $(LIBS)

TRIMSLASH_OBJ = trimslash.o syscall.o
trimslash: $(TRIMSLASH_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TRIMSLASH_OBJ) $(LIBS)

Makefile: Makefile.in configure config.status
	echo "WARNING: You need to run ./config.status --recheck"

# don't actually run autoconf, just issue a warning
configure: configure.in
	echo "WARNING: you need to rerun autoconf"

cryptar.1: cryptar.yo
	yodl2man -o cryptar.1 cryptar.yo

cryptard.conf.5: cryptard.conf.yo
	yodl2man -o cryptard.conf.5 cryptard.conf.yo

proto:
	cat $(srcdir)/*.c | awk -f $(srcdir)/mkproto.awk > $(srcdir)/proto.h

clean: cleantests
	rm -f *~ $(OBJS) cryptar $(DB_OBJS) cryptar-db $(TLS_OBJ) $(CHECK_PROGS)

cleantests:
	rm -rf ./testtmp*

# We try to delete built files from both the source and build
# directories, just in case somebody previously configured things in
# the source directory.
distclean: clean
	rm -f Makefile config.h config.status 
	rm -f $(srcdir)/Makefile $(srcdir)/config.h $(srcdir)/config.status 

	rm -f config.cache config.log
	rm -f $(srcdir)/config.cache $(srcdir)/config.log

	rm -f shconfig $(srcdir)/shconfig

# this target is really just for my use. It only works on a limited
# range of machines and is used to produce a list of potentially
# dead (ie. unused) functions in the code. (tridge)
finddead:
	nm *.o */*.o |grep 'U ' | awk '{print $$2}' | sort -u > nmused.txt
	nm *.o */*.o |grep 'T ' | awk '{print $$3}' | sort -u > nmfns.txt
	comm -13 nmused.txt nmfns.txt 

# 'check' is the GNU name, 'test' is the name for everybody else :-)
.PHONY: check test

test: check


# There seems to be no standard way to specify some variables as
# exported from a Makefile apart from listing them like this.

# TODO: Tests that depend on built test aide programs like tls need to
# know where the build directory is.

# This depends on building cryptar; if we need any helper programs it
# should depend on them too.

# We try to run the scripts with POSIX mode on, in the hope that will
# catch Bash-isms earlier even if we're running on GNU.  Of course, we
# might lose in the future where POSIX diverges from old sh.

check: all $(CHECK_PROGS)
	POSIXLY_CORRECT=1 TOOLDIR=`pwd` cryptar_bin=`pwd`/cryptar srcdir="$(srcdir)" $(srcdir)/runtests.sh

# This does *not* depend on building or installing: you can use it to
# check a version installed from a binary or some other source tree,
# if you want.

installcheck: $(CHECK_PROGS)
	POSIXLY_CORRECT=1 TOOLDIR=`pwd` cryptar_bin="$(bindir)/cryptar" srcdir="$(srcdir)" $(srcdir)/runtests.sh

# TODO: Add 'dist' target; need to know which files will be included

# Run the SPLINT (Secure Programming Lint) tool.  <www.splint.org>
.PHONY: splint
splint: 
	splint +unixlib +gnuextensions -weak cryptar.c


cryptar.dvi: doc/cryptar.texinfo
	texi2dvi -o $@ $<

cryptar.ps: cryptar.dvi	
	dvips -ta4 -o $@ $<

cryptar.pdf: doc/cryptar.texinfo
	texi2dvi -o $@ --pdf $<

test-comm: util.o options.o comm.o log.o cleanup.o main.o
	$(CC) $(CFLAGS) `pkg-config --cflags glib-2.0`  `pkg-config --libs glib-2.0`  $^ -o test-comm
