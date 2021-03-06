prefix = /usr
bindir = $(prefix)/sbin

all: deathstar

clean:
	rm -f deathstar

deathstar: deathstar.cpp
	g++ -O2 -g -Wall deathstar.cpp -lzmq -o deathstar

install: all
	install deathstar $(DESTDIR)$(bindir)
	install deathstar.worker $(DESTDIR)$(bindir)
