CFLAGS += -Wall -Wextra -static -O3

PREFIX ?= /usr/local

all: ufbtest umemdump ustardict umemtest utermhello usnake ufbtop ufbterm unc utftp umips32dis ukermit

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

usnake:
	$(CC) $(CFLAGS) usnake.c -o usnake

ufbtop:
	$(CC) $(CFLAGS) ufbtop.c -o ufbtop

ufbterm:
	$(CC) $(CFLAGS) ufbterm.c -o ufbterm

unc:
	$(CC) $(CFLAGS) unc.c -o unc

utftp:
	$(CC) $(CFLAGS) utftp.c -o utftp

umips32dis:
	$(CC) $(CFLAGS) umips32dis.c -o umips32dis

ukermit:
	$(CC) $(CFLAGS) ukermit.c -o ukermit

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

	cp -f usnake $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/usnake

	cp -f ufbtop $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ufbtop

	cp -f ufbterm $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ufbterm

	cp -f unc $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/unc

	cp -f utftp $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/utftp

	cp -f umips32dis $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/umips32dis

	cp -f ukermit $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/ukermit

check:
	$(CC) *.h

clean:
	rm -fv *.out
	rm -fv *.gch
	rm -fv ufbtest
	rm -fv umemdump
	rm -fv ustardict
	rm -fv umemtest
	rm -fv utermhello
	rm -fv usnake
	rm -fv ufbtop
	rm -fv ufbterm
	rm -fv unc
	rm -fv utftp
	rm -fv umips32dis
	rm -fv ukermit
