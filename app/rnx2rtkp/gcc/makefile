# makefile for rnx2rtkp

BINDIR  = /usr/local/bin
SRC     = ../../../src

CFLAGS  = -Wall -O3 -ansi -pedantic -I$(SRC) -DLAPACK -DTRACE -DENAGLO -DNFREQ=3
LDLIBS  = -lm -llapack -lblas
#CFLAGS  = -Wall -O3 -ansi -pedantic -I$(SRC) -DLAPACK -DTRACE -DENAGLO -DENAGAL -DENAQZS -DNFREQ=6
#LDLIBS  = -lm -llapack -lblas
#CFLAGS  = -Wall -O3 -ansi -pedantic -I$(SRC) -DLAPACK -DTRACE -DENAGLO -pg
#LDLIBS  = -lm -llapack -lblas -pg

# for mkl
#MKLDIR  = /opt/intel
#CFLAGS  = -O3 -ansi -pedantic -I$(SRC) -DMKL -DTRACE -DENAGLO -DNFREQ=3 -I$(MKLDIR)/include
#LDLIBS  = -L$(MKLDIR)/lib/32 -lm -lmkl_core -lmkl_intel -lmkl_gnu_thread -liomp5 -lpthread

all        : rnx2rtkp
rnx2rtkp   : rnx2rtkp.o rtkcmn.o rinex.o rtkpos.o postpos.o solution.o
rnx2rtkp   : lambda.o geoid.o sbas.o preceph.o pntpos.o ephemeris.o options.o
rnx2rtkp   : ppp.o

rnx2rtkp.o : ../rnx2rtkp.c
	$(CC) -c $(CFLAGS) ../rnx2rtkp.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
rinex.o    : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
rtkpos.o   : $(SRC)/rtkpos.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkpos.c
postpos.o  : $(SRC)/postpos.c
	$(CC) -c $(CFLAGS) $(SRC)/postpos.c
solution.o : $(SRC)/solution.c
	$(CC) -c $(CFLAGS) $(SRC)/solution.c
lambda.o   : $(SRC)/lambda.c
	$(CC) -c $(CFLAGS) $(SRC)/lambda.c
geoid.o    : $(SRC)/geoid.c
	$(CC) -c $(CFLAGS) $(SRC)/geoid.c
sbas.o     : $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
preceph.o  : $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c
pntpos.o   : $(SRC)/pntpos.c
	$(CC) -c $(CFLAGS) $(SRC)/pntpos.c
ephemeris.o: $(SRC)/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/ephemeris.c
options.o  : $(SRC)/options.c
	$(CC) -c $(CFLAGS) $(SRC)/options.c
ppp.o      : $(SRC)/ppp.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp.c

rnx2rtkp.o : $(SRC)/rtklib.h
rtkcmn.o   : $(SRC)/rtklib.h
rinex.o    : $(SRC)/rtklib.h
rtkpos.o   : $(SRC)/rtklib.h
postpos.o  : $(SRC)/rtklib.h
solution.o : $(SRC)/rtklib.h
lambda.o   : $(SRC)/rtklib.h
geoid.o    : $(SRC)/rtklib.h
sbas.o     : $(SRC)/rtklib.h
preceph.o  : $(SRC)/rtklib.h
pntpos.o   : $(SRC)/rtklib.h
ephemeris.o: $(SRC)/rtklib.h
options.o  : $(SRC)/rtklib.h
ppp.o      : $(SRC)/rtklib.h

CMD1    = ./rnx2rtkp
INPUT11 = ../../../test/data/07590920.05o ../../../test/data/30400920.05n
INPUT12 = ../../../test/data/30400920.05o
OPTS1   = -r -3978241.958 3382840.234 3649900.853

test : test1 test2 test3 test4 test5 test6 test7 test8 test9 test10
test : test11 test12 test13 test14 test15 test16 test17 test18 test19 test20
test : test21 test22 test23 test24

test1 :
	$(CMD1) $(INPUT11) -x 5 -o test1.out
test2 :
	$(CMD1) -t -e $(OPTS1) $(INPUT11) > test2.out
test3 :
	$(CMD1) -t -p 1 -e $(OPTS1) $(INPUT11) $(INPUT12) > test3.out
test4 :
	$(CMD1) -t -p 3 -e $(OPTS1) $(INPUT11) $(INPUT12) > test4.out
test5 :
	$(CMD1) -t -m 15 -e $(OPTS1) $(INPUT11) $(INPUT12) > test5.out
test6 :
	$(CMD1) -t -f 1 -e $(OPTS1) $(INPUT11) $(INPUT12) > test6.out
test7 :
	$(CMD1) -t -v 5 -e $(OPTS1) $(INPUT11) $(INPUT12) > test7.out
test8 :
	$(CMD1) -t -i -e $(OPTS1) $(INPUT11) $(INPUT12) > test8.out
test9 :
	$(CMD1) -t -p 0 $(OPTS1) $(INPUT11) > test9.out
test10 :
	$(CMD1) -t -p 0 $(OPTS1) $(INPUT11) -o test10.out
test11 :
	$(CMD1) -t -p 0 -n $(OPTS1) $(INPUT11) > test11.out
test12 :
	$(CMD1) -t -p 0 -g $(OPTS1) $(INPUT11) > test12.out
test13 :
	$(CMD1) -t -p 0 $(OPTS1) $(INPUT11) > test13.out
test14 :
	$(CMD1) -t -p 0 -u $(OPTS1) $(INPUT11) > test14.out
test15 :
	$(CMD1) -t -p 0 -d 9 $(OPTS1) $(INPUT11) > test15.out
test16 :
	$(CMD1) -t -p 0 -s , $(OPTS1) $(INPUT11) > test16.out
test17 :
	$(CMD1) -t -b -e $(OPTS1) $(INPUT11) $(INPUT12) > test17.out
test18 :
	$(CMD1) -t -c -e $(OPTS1) $(INPUT11) $(INPUT12) > test18.out
test19 :
	$(CMD1) -t -h -e $(OPTS1) $(INPUT11) $(INPUT12) > test19.out
test20 :
	$(CMD1) -t -p 4 -a $(OPTS1) $(INPUT11) $(INPUT12) > test20.out
test21 :
	$(CMD1) $(INPUT11) $(INPUT12) > test21.out
test22 :
	$(CMD1) -k opts1.conf $(INPUT11) $(INPUT12) > test22.out
test23 :
	$(CMD1) -k opts2.conf $(INPUT11) $(INPUT12) > test23.out
test24 :
	$(CMD1) -k opts3.conf $(INPUT11) $(INPUT12) > test24.out

clean :
	rm -f rnx2rtkp rnx2rtkp.exe *.o *.out *.trace

prof :
	gprof rnx2rtkp > prof.txt

install :
	cp rnx2rtkp $(BINDIR)

