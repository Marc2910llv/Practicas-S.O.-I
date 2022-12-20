##################################################################
###                Alberto Cugat Martín                        ###
###                Jaume Julià Vallespir                       ### 
###                Rubén Palmer Pérez                          ###
##################################################################
C=gcc
CFLAGS=-c -g -Wall -std=c99
LDFLAGS=-pthread

SOURCES=av3.c reader.c #my_lib.c #
LIBRARIES=
INCLUDES=
PROGRAMS= av3 reader
OBJS=$(SOURCES:.c=.o)

all : $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o : %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY : clean
clean :
	rm -rf *.o *~ $(PROGRAMS)