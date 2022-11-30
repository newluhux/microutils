CFLAGS += -Wall -Wextra

PREFIX ?= /usr/local

all: ufbtest umemdump ustardict umemtest

ufbtest:
	$(CC) $(CFLAGS) ufbtest.c -o ufbtest

umemdump:
	$(CC) $(CFLAGS) umemdump.c -o umemdump

ustardict:
	$(CC) $(CFLAGS) ustardict.c -o ustardict

umemtest:
	$(CC) $(CFLAGS) umemtest.c -o umemtest

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

clean:
	rm -fv ufbtest
	rm -fv umemdump
	rm -fv ustardict
	rm -fv umemtest
