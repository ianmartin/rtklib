# makefile for dumpssr

SRC    = ../../src
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAGLO -DMT1065_BUG

all        : dumpssr diffeph

dumpssr    : rtkcmn.o rtcm.o
diffeph    : rtkcmn.o satpos.o sbas.o rinex.o preceph.o rtcm.o ppp.o

rtkcmn.o   : $(SRC)/rtklib.h $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
satpos.o   : $(SRC)/rtklib.h $(SRC)/satpos.c
	$(CC) -c $(CFLAGS) $(SRC)/satpos.c
sbas.o     : $(SRC)/rtklib.h $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
rinex.o    : $(SRC)/rtklib.h $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
preceph.o  : $(SRC)/rtklib.h $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c
rtcm.o     : $(SRC)/rtklib.h $(SRC)/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm.c
ppp.o      : $(SRC)/rtklib.h $(SRC)/ppp.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp.c

#FILE =/cygdrive/y/gpslog/20100617/cor_201006171504.log
#FILE =data/cor_201006171800.log
FILE =data/ssr_201006151017.rtcm3

clean:
	rm -f *.o *.stackdump *.trace *.out

test0:
	./dumpssr -t $(FILE) > test0.out

test1:
	./dumpssr -o -c -s 12 $(FILE) > test1.out

test2:
	./dumpssr -b $(FILE) > test2.out

#TIME0=2010/6/15 0:00:00
#TSPAN=24
#FILE1=data/*.10n
#FILE2=data/igr*.sp3
#FILE3=data/igr*.clk
#FILE4=../../data/igs05.atx
#FILE5=data/ssr_201006151017.rtcm3
TIME0=2010/6/20 18:00:00
TSPAN=6
DIR=../../../../data/gpslog/20100620
FILE1=$(DIR)/brdc17*0.10n
FILE2=$(DIR)/igr15890.sp3
FILE3=$(DIR)/igr15890.clk
FILE4=$(DIR)/cor_201006201800.log
FILE5=../../data/igs05.atx

test3:
	./diffeph -x 4 -1 3 -t0 $(TIME0) -ts $(TSPAN) "$(FILE1)" "$(FILE2)" "$(FILE3)" $(FILE4) $(FILE5) > test3.out

