VERSION = 0.5.0

CFLAGS = --std=c99 -pedantic \
	-Wall -Wextra -Wbad-function-cast -Wcast-align -Wcast-qual \
	-Wconversion -Wmissing-declarations -Wmissing-prototypes \
	-Wmissing-variable-declarations -Wpointer-arith -Wshadow \
	-Wstrict-prototypes -Wswitch -Wundef -Wwrite-strings \
	-O1 \
	-DVERSION='"$(VERSION)"'
LDFLAGS = -s
LIBS = -lcurl

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

gelcli: $(shell find src/ -name '*.[ch]')
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/main.c $(LIBS)

install: gelcli
	install -Dm755 gelcli $(DESTDIR)$(BINDIR)/gelcli

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/gelcli

clean:
	rm -f $(OBJ) gelcli
