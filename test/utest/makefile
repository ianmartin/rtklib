# makefile for rtklib unit test

SRC    = ../../src
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DENAGLO
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAGLO
LDLIBS = -lm -llapack -lblas

BIN    = t_matrix t_time t_coord t_rinex t_lambda t_atmos t_misc t_preceph t_gloeph t_geoid t_ppp

all        : $(BIN)
t_matrix   : t_matrix.o rtkcmn.o
t_time     : t_time.o rtkcmn.o
t_coord    : t_coord.o rtkcmn.o geoid.o
t_rinex    : t_rinex.o rtkcmn.o rinex.o preceph.o
t_lambda   : t_lambda.o rtkcmn.o lambda.o
t_atmos    : t_atmos.o rtkcmn.o
t_misc     : t_misc.o rtkcmn.o
t_preceph  : t_preceph.o rtkcmn.o preceph.o rinex.o ephemeris.o sbas.o
t_gloeph   : t_gloeph.o rtkcmn.o rinex.o ephemeris.o sbas.o preceph.o
t_geoid    : t_geoid.o rtkcmn.o geoid.o
t_ppp      : t_ppp.o rtkcmn.o ephemeris.o preceph.o sbas.o ppp.o

rtkcmn.o   : $(SRC)/rtklib.h $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
rinex.o    : $(SRC)/rtklib.h $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
rtkpos.o   : $(SRC)/rtklib.h $(SRC)/rtkpos.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkpos.c
lambda.o   : $(SRC)/rtklib.h $(SRC)/lambda.c
	$(CC) -c $(CFLAGS) $(SRC)/lambda.c
geoid.o    : $(SRC)/rtklib.h $(SRC)/geoid.c
	$(CC) -c $(CFLAGS) $(SRC)/geoid.c
ephemeris.o: $(SRC)/rtklib.h $(SRC)/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/ephemeris.c
sbas.o     : $(SRC)/rtklib.h $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
preceph.o  : $(SRC)/rtklib.h $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c
ppp.o      : $(SRC)/rtklib.h $(SRC)/ppp.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp.c

utest : utest1 utest2 utest3 utest4 utest5 utest6 utest7 utest8
utest : utest9 utest10 utest11

utest1 :
	./t_matrix  > utest1.out
utest2 :
	./t_time    > utest2.out
utest3 :
	./t_coord   > utest3.out
utest4 :
	./t_rinex   > utest4.out
utest5 :
	./t_lambda  > utest5.out
utest6 :
	./t_atmos   > utest6.out
utest7 :
	./t_misc    > utest7.out
utest8 :
	./t_preceph > utest8.out
utest9 :
	./t_gloeph  > utest9.out
utest10 :
	./t_geoid   > utest10.out
utest11 :
	./t_ppp     > utest11.out

clean :
	rm -f *.o *.out *.exe $(BIN) *.stackdump gmon.out

