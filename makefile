CFLAGS += -Wall -Wextra -static -O3

PREFIX ?= /usr/local

all: ufbtest umemdump ustardict umemtest utermhello ufbtop urime2stardict

ufbtest:
	$(CC) $(CFLAGS) ufbtest.c -o ufbtest

umemdump:
	$(CC) $(CFLAGS) umemdump.c -o umemdump

ustardict:
	$(CC) $(CFLAGS) ustardict.c -o ustardict

umemtest:
	$(CC) $(CFLAGS) umemtest.c -o umemtest

utermhello:
	$(CC) $(CFLAGS) utermhello.c -o utermhello

ufbtop:
	$(CC) $(CFLAGS) ufbtop.c -o ufbtop

urime2stardict:
	$(CC) $(CFLAGS) urime2stardict.c -o urime2stardict

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin

	cp -f ufbtest $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ufbtest

	cp -f umemdump $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/umemdump

	cp -f ustardict $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ustardict

	cp -f umemtest $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/umemtest

	cp -f utermhello $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/utermhello

	cp -f ufbtop $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ufbtop

	cp -f urime2stardict $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/urime2stardict

clean:
	rm -fv *.gch
	rm -fv ufbtest
	rm -fv umemdump
	rm -fv ustardict
	rm -fv umemtest
	rm -fv utermhello
	rm -fv ufbtop
	rm -fv urime2stardict
