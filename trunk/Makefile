SRCS = bannergrab.c
BINPATH = /usr/bin/
MANPATH = /usr/share/man/

all:
	gcc -lssl -o bannergrab $(SRCS) $(LDFLAGS) $(CFLAGS)

install:
	cp bannergrab $(BINPATH)
	cp bannergrab.1 $(MANPATH)man1

uninstall:
	rm -f $(BINPATH)bannergrab
	rm -f $(MANPATH)man1/bannergrab.1

clean:
	rm -f bannergrab
