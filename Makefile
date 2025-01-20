VERSION = 0.5.0

CFLAGS = --std=c99 -pedantic \
	-Wall -Wextra -Wconversion \
	-O1 \
	-DVERSION='"$(VERSION)"'
LDFLAGS = -s
LIBS = -lcurl

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

gelcli: $(shell find src/ -name '*.c')
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

install: gelcli
	install -Dm755 gelcli $(DESTDIR)$(BINDIR)/gelcli

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/gelcli

clean:
	rm -f $(OBJ) gelcli
