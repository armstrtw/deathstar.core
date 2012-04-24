prefix = /usr
bindir = $(prefix)/bin

all: deathstar

clean:
	rm -f deathstar

deathstar: deathstar.cpp
	g++ -g -Wall deathstar.cpp -lzmq -o deathstar

install: all
	install deathstar $(DESTDIR)$(bindir)
	install deathstar.worker $(DESTDIR)$(bindir)
