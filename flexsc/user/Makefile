src=user-program.c
obj=$(src:.c=.o)
exe=$(src:.c=)
# inc=syscall_info.h 

default:
	gcc $(src) -o $(exe) $(inc) -I. -lflexsc -lpthread

run:
	./$(exe)

clean:
	rm $(obj) $(exe)
