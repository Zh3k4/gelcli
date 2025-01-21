VERSION = 0.5.0

WARNINGS = -Wall -Wextra -Wbad-function-cast -Wcast-align -Wcast-qual \
	-Wconversion -Wmissing-declarations -Wmissing-prototypes \
	-Wmissing-variable-declarations -Wpointer-arith -Wshadow \
	-Wstrict-prototypes -Wswitch -Wundef -Wwrite-strings
CPPFLAGS = -DVERSION='"$(VERSION)"'
CFLAGS = --std=c99 -pedantic -O1 $(WARNINGS) $(CPPFLAGS)
LDFLAGS = -s
LIBS = -lcurl

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

gelcli: $(shell find src/ -name '*.[ch]')
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/main.c $(LIBS)

install: gelcli
	install -Dm755 gelcli $(DESTDIR)$(BINDIR)/gelcli

uninstall:
	rm -f -- $(DESTDIR)$(BINDIR)/gelcli

clean:
	rm -f -- gelcli
