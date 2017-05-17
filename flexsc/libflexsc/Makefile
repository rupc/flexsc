src=flexsc.c flexsc-print.c flexsc_syscalls.c
obj=$(src:.c=.o)
exe=$(src:.c=)
inc=flexsc.h syscall_info.h flexsc_syscalls.h flexsc_cpu.h flexsc_types.h
lib=libflexsc.a

lib_loc=/usr/lib/
inc_loc=/usr/include/

default:
	sudo cp $(inc) $(inc_loc)
	gcc -c $(src)
	ar -r $(lib) $(obj)
	sudo cp $(lib) $(lib_loc)
	# make -C test
