CFLAGS = -O0 -g3 -Wall

all: atomdis structsizedumper

atomdis: main.o
	$(CC) -o atomdis $<
structsizedumper: structsizedumper_gen.o
	$(CC) -o structsizedumper $<

clean: _always_
	rm -f *.o
	rm -f atombios.h structsizedumper_gen.c

distclean: clean
	rm atomdis structsizedumper


atombios.h:
	test -r ../ATI/atombios.h
	ln -s ../ATI/atombios.h .

structsizedumper_gen.c: atombios.h structsizedumper_factory.pl
	perl ./structsizedumper_factory.pl < atombios.h > structsizedumper_gen.c

_always_:
	@true


main.o: atombios.h

#EOF
