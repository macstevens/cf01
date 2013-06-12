# Make file
#
# based on http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

IDIR =include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =lib

#LIBS=-lm
LIBS=-lm

_DEPS = cf.h cf_unit_test.h cf.c cf_unit_test.c main.c
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS = $(_DEPS)

_OBJ = main.o cf.o cf_unit_test.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main_exe: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
