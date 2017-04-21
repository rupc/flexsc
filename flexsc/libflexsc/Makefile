src=flexsc.c flexsc-print.c
obj=$(src:.c=.o)
exe=$(src:.c=)
inc=flexsc.h syscall_info.h
lib=libflexsc.a

lib_loc=/usr/lib/
inc_loc=/usr/include/

default:
	sudo cp $(inc) $(inc_loc)
	gcc -c $(src)
	ar -r $(lib) $(obj)
	sudo cp $(lib) $(lib_loc)
	# make -C test
