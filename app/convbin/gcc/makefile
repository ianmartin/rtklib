# makefile for convbin

BINDIR = /usr/local/bin
SRC    = ../../../src
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DENAGLO -DNFREQ=3
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DENAGLO -DENAGAL -DENAQZS -DNFREQ=6
LDLIBS = -lm

convbin    : convbin.o rtkcmn.o rinex.o sbas.o rcvraw.o convrnx.o rtcm.o
convbin    : novatel.o ss2.o ublox.o crescent.o skytraq.o

convbin.o  : ../convbin.c
	$(CC) -c $(CFLAGS) ../convbin.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
rinex.o    : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
sbas.o     : $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
rcvraw.o   : $(SRC)/rcvraw.c
	$(CC) -c $(CFLAGS) $(SRC)/rcvraw.c
convrnx.o  : $(SRC)/convrnx.c
	$(CC) -c $(CFLAGS) $(SRC)/convrnx.c
rtcm.o     : $(SRC)/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm.c
novatel.o  : $(SRC)/rcv/novatel.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/novatel.c
ss2.o      : $(SRC)/rcv/ss2.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ss2.c
ublox.o    : $(SRC)/rcv/ublox.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ublox.c
crescent.o : $(SRC)/rcv/crescent.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/crescent.c
skytraq.o : $(SRC)/rcv/skytraq.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/skytraq.c

convbin.o  : $(SRC)/rtklib.h
rtkcmn.o   : $(SRC)/rtklib.h
rinex.o    : $(SRC)/rtklib.h
sbas.o     : $(SRC)/rtklib.h
rcvraw.o   : $(SRC)/rtklib.h
convrnx.o  : $(SRC)/rtklib.h
rtcm.o     : $(SRC)/rtklib.h
novatel.o  : $(SRC)/rtklib.h
ss2.o      : $(SRC)/rtklib.h
ublox.o    : $(SRC)/rtklib.h
crescent.o : $(SRC)/rtklib.h
skytraq.o  : $(SRC)/rtklib.h

install:
	cp convbin $(BINDIR)

clean:
	rm -f convbin convbin.exe *.o *.obs *.nav *.gnav *.hnav *.sbs *.stackdump

test : test1 test2 test3 test4 test5 test6 test7 test8 test9 test10

test1:
	./convbin -r nov ../../../test/data/nov0080526.gps -ti 10 -d . -os
test2:
	./convbin -r hemis ../../../test/data/cres080526.bin -ti 10 -d . -f 1 -od -os
test3:
	./convbin ../../../test/data/ubx0080526.ubx -o ubx_test.obs -d . -f 1 -ts 2008/5/26 6:00 -te 2008/5/26 6:10
test4:
	./convbin ../../../test/data/ubx0080526.ubx -n ubx_test.nav -d .
test5:
	./convbin ../../../test/data/ubx0080526.ubx -h ubx_test.hnav -s ubx_test.sbs -d . -x 129
test6:
	./convbin ../../../test/data/novtestglo.gps -od -os -d .
test7:
	./convbin ../../../test/data/testglo.rtcm2 -tr 2009/12/18 23:20 -d .
test8:
	./convbin ../../../test/data/testglo.rtcm3 -os -tr 2009/12/18 23:20 -d .
test9:
	./convbin -v 3 -f 6 -r nov ../../../test/data/nov0080526.gps -od -os -o rnx3_test.obs -n rnx3_test.nav -d .
test10:
	./convbin -v 2 -f 3 -r rinex "rnx3_test.*" -o rnx2_test.obs -n rnx2_test.nav -g rnx2_test.gnav -h rnx2_test.hnav
test11:
	stty raw < /dev/ttyACM0
	./convbin -r ubx -o ubx.obs -n ubx.nav -s ubx.sbs -h ubx.hnav /dev/ttyACM0


