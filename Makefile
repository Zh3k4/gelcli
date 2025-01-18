.POSIX:
.SUFFIXES:

VERSION = 0.5.0

MAINFLAGS = --std=c99 -pedantic -DVERSION='"$(VERSION)"' -Wall -Wextra -Wconversion
CFLAGS = -O1
#CFLAGS = -g
LDFLAGS = -s
LIBS = -lcurl

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

OBJ = \
	src/gel.o \
	src/jsmn.o \
	src/main.o

gelcli: $(OBJ)
	@printf 'CCLD\t%s\n' '$@'
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

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
