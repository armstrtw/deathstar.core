prefix = /usr/local
bindir = $(prefix)/sbin
sharedir = $(prefix)/share
mandir = $(sharedir)/man
man1dir = $(mandir)/man1

all: deathstar

clean:
	rm -f deathstar

deathstar: deathstar.cpp
	g++ -g -Wall deathstar.cpp -lzmq -o deathstar

install: all
	install deathstar $(DESTDIR)$(bindir)
	install deathstar.worker $(DESTDIR)$(bindir)
