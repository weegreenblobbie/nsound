# Makefile

# :mode=Makefile:

LIBDIR  := /home/nhilton/development/nsound-0.6.1/lib
INCLUDE := /home/nhilton/development/nsound-0.6.1/src

all: filter1.so guitarBass.so

%.o: %.cc
	g++ $(CXXFLAGS) -c $< -fPIC -o $@ -I$(INCLUDE)

%.so: %.o
	g++ -shared $< -o $@ -Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -lNsound

.PHONY: clean

clean:
	rm -f *.so *.o

install: filter1.so guitarBass.so
	cp -f *.so ~/.ladspa
