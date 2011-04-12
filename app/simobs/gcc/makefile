# makefile for simobs

BINDIR = /usr/local/bin
SRC    = ../../../src
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAGLO -DENAGAL -DENAQZS -DNFREQ=4 -DMAXOBS=128
LDLIBS  = -lm

simobs     : simobs.o rinex.o rtkcmn.o preceph.o

simobs.o   : ../simobs.c
	$(CC) -c $(CFLAGS) ../simobs.c
rinex.o    : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
preceph.o  : $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c

simobs.o   : $(SRC)/rtklib.h
rtkcmn.o   : $(SRC)/rtklib.h
preceph.o  : $(SRC)/rtklib.h

install:
	cp simobs $(BINDIR)

clean:
	rm -f simobs simobs.exe *.o

OPT0=-ts 2009/4/1 -te 2009/4/2 -ti 10
#OPT1=../sim/brdc0910.09n ../sim/brdc0910.09g ../sim/brdc0910.09l ../sim/brdc0910.09q
OPT1=../sim/brdc0910.09n ../sim/brdc0910_q7.09q
#OPT2=-r 36.106114294 140.087190410 70.3010
#OPT3=-r 36.103635125 140.086307150 69.7442
#OPT4=-r 36.031339503 140.202443500 70.9029
#OPT5=-r 36.263268936 140.174264100 94.2843

OPT2=-r 35.68017 139.76635 0
OPT3=-r 37.53369 127.07062 0
OPT4=-r 39.90552 116.40014 0
OPT5=-r 31.20341 121.48682 0
OPT6=-r 13.72071 100.50293 0

OPT7=-m ../sim/elmask.txt

OPT8 =-r 35.77000 139.76635 0
OPT9 =-r 37.62352 127.07062 0
OPT10=-r 39.99535 116.40014 0
OPT11=-r 31.29324 121.48682 0
OPT12=-r 13.81054 100.50293 0

sim1:
	./simobs $(OPT0) $(OPT1) $(OPT2) -o ../sim/tokyo_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT3) -o ../sim/seoul_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT4) -o ../sim/peking_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT5) -o ../sim/shanghai_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT6) -o ../sim/bangkok_20080509.obs

sim2:
	./simobs $(OPT0) $(OPT1) $(OPT2) $(OPT7) -o ../sim/tokyo_20080509_m.obs
	./simobs $(OPT0) $(OPT1) $(OPT3) $(OPT7) -o ../sim/seoul_20080509_m.obs
	./simobs $(OPT0) $(OPT1) $(OPT4) $(OPT7) -o ../sim/peking_20080509_m.obs
	./simobs $(OPT0) $(OPT1) $(OPT5) $(OPT7) -o ../sim/shanghai_20080509_m.obs
	./simobs $(OPT0) $(OPT1) $(OPT6) $(OPT7) -o ../sim/bangkok_20080509_m.obs

sim3:
	./simobs $(OPT0) $(OPT1) $(OPT8) -o ../sim/tokyo_base_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT9) -o ../sim/seoul_base_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT10) -o ../sim/peking_base_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT11) -o ../sim/shanghai_base_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT12) -o ../sim/bangkok_base_20080509.obs
