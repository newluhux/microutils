CFLAGS += -Wall -Wextra

PREFIX ?= /usr/local

all: ufbtest umemdump ustardict

ufbtest:
	$(CC) $(CFLAGS) ufbtest.c -o ufbtest

umemdump:
	$(CC) $(CFLAGS) umemdump.c -o umemdump

ustardict:
	$(CC) $(CFLAGS) ustardict.c -o ustardict

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin

	cp -f ufbtest $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ufbtest

	cp -f umemdump $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/umemdump

	cp -f ustardict $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ustardict

clean:
	rm -fv ufbtest
	rm -fv umemdump
	rm -fv ustardict
