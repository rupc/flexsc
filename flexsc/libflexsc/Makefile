src=flexsc.c flexsc-print.c
obj=$(src:.c=.o)
exe=$(src:.c=)


default:
	sudo cp flexsc.h /usr/include/
	gcc -c flexsc.c flexsc-print.c
	ar -r libflexsc.a flexsc.o flexsc-print.o
	sudo cp libflexsc.a /usr/lib/
	# make -C test
