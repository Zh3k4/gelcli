.POSIX:
.PRAGMA: target_name
.SUFFIXES:

VERSION = 0.2.0

MAINFLAGS = --std=c99 -pedantic -DVERSION='"$(VERSION)"' -Wall -Wextra -Werror
CFLAGS = -O1 -g
LDLIBS = -lcurl

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

OBJ = \
	src/gel.o \
	src/jsmn.o \
	src/main.o

gelcli: $(OBJ)
	@printf 'CCLD\t%s\n' '$@'
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

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
