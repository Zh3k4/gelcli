.POSIX:
.SUFFIXES:

VERSION = 0.1.0

MAINFLAGS := -DVERSION='"$(VERSION)"' -Wall -Wextra -Werror
CFLAGS ?= -g
INCLUDE += -Iinclude
LIBS += -lcurl

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
OUTDIR = .build

OBJ = \
	  $(OUTDIR)/json.o \
	  $(OUTDIR)/main.o \
	  $(OUTDIR)/slice.o

gelcli: $(OBJ)
	@printf 'LD\t%s\n' $@
	@$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	@printf 'CC\t%s\n' $@
	@$(CC) -std=c99 -pedantic -c -o $@ $(CFLAGS) $(MAINFLAGS) $(INCLUDE) $<

install: gelcli
	mkdir -p $(DESTDIR)/$(BINDIR)
	install -m755 gelcli $(DESTDIR)/$(BINDIR)/gelcli

uninstall:
	rm -f $(DESTDIR)/$(BINDIR)/gelcli

clean:
	@rm -f $(OBJ) gelcli

.PHONY: install uninstall clean
