.POSIX:
.SUFFIXES:

VERSION = 0.1.0

MAINFLAGS = --std=c99 -pedantic -DVERSION='"$(VERSION)"' -Wall -Wextra -Werror
CFLAGS = -O1 -g
LDLIBS = -lcurl

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

OBJ = \
	  src/jsmn.o \
	  src/main.o \
	  src/slice.o

gelcli: $(OBJ)
	@printf 'CCLD\t%s\n' '$@'
	@$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.SUFFIXES: .c .o
.c.o:
	@printf 'CC\t%s\n' '$@'
	@$(CC) -c -o $@ $(CFLAGS) $(MAINFLAGS) $(INCLUDE) $<

install: gelcli
	install -Dm755 gelcli $(DESTDIR)$(BINDIR)/gelcli

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/gelcli

clean:
	rm -f $(OBJ) gelcli

.PHONY: install uninstall clean
