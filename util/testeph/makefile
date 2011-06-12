# makefile for diffeph dumpssr

SRC    = ../../src
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DEXTLEX
LDFLAGS = -lm

all        : diffeph dumpssr

diffeph    : rtkcmn.o ephemeris.o sbas.o rinex.o preceph.o rtcm.o ionex.o pntpos.o ppp.o
#diffeph    : qzslex.o
dumpssr    : rtkcmn.o rtcm.o

rtkcmn.o   : $(SRC)/rtklib.h $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
ephemeris.o: $(SRC)/rtklib.h $(SRC)/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/ephemeris.c
sbas.o     : $(SRC)/rtklib.h $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
rinex.o    : $(SRC)/rtklib.h $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
preceph.o  : $(SRC)/rtklib.h $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c
rtcm.o     : $(SRC)/rtklib.h $(SRC)/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm.c
ionex.o    : $(SRC)/rtklib.h $(SRC)/ionex.c
	$(CC) -c $(CFLAGS) $(SRC)/ionex.c
pntpos.o   : $(SRC)/rtklib.h $(SRC)/pntpos.c
	$(CC) -c $(CFLAGS) $(SRC)/pntpos.c
ppp.o      : $(SRC)/rtklib.h $(SRC)/ppp.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp.c
#qzslex.o   : $(SRC)/rtklib.h $(SRC)/../extlex/qzslex.c
#	$(CC) -c $(CFLAGS) $(SRC)/../extlex/qzslex.c

install:
	cp -f diffeph.exe /usr/local/bin
	cp -f dumpssr.exe /usr/local/bin

clean:
	rm -f *.o *.stackdump *.trace *.out *.exe

