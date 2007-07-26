CFLAGS = -O0 -g3 -Wall

all: atomdis

atomdis: main.o datastructs.o
	$(CC) -o atomdis $^

clean: _always_
	-rm -f *.o
	-rm -f atombios.h datastructs_gen.c

distclean: clean
	-rm -f *~
	-rm atomdis structsizedumper


atombios.h:
	test -r ../ATI/atombios.h
	ln -s ../ATI/atombios.h .

datastructs_gen.c: atombios.h datastructs_factory.pl
	cpp atombios.h | perl ./datastructs_factory.pl > datastructs_gen.c

_always_:
	@true


main.o: atombios.h
datastructs.o: CFLAGS += -Wno-unused
datastructs.o: atombios.h datastructs_gen.c

#EOF
