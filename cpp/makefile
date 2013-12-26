# Make file
#
# based on http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

IDIR =include
#CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =lib

#LIBS=-lm
LIBS=-lm

_DEPS = \
$(ODIR) \
../cf.h \
../cf_basic_test_suite.h \
../cf_unit_test.h \
../cf.c \
../cf_basic_test_suite.c \
../cf_unit_test.c \
main.cpp

#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS = $(_DEPS)

_OBJ = \
cpp/main.o \
c/cf.o \
c/cf_basic_test_suite.o \
c/cf_unit_test.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

main_exe: $(OBJ)
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR):
	mkdir $(ODIR)
	mkdir $(ODIR)/c
	mkdir $(ODIR)/cpp

$(ODIR)/c/%.o: ../%.c $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)

$(ODIR)/cpp/%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm main_exe



