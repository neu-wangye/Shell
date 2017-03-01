objects = main.o command.o 
CFLAGS = -std=gnu99
shell: $(objects)
	gcc -o shell $(CFLAGS) $(objects) -l:libreadline.so.6
main.o: command.h 
	gcc -c $(CFLAGS) main.c
command.o:
	gcc -c $(CFLAGS) command.c

.PHONY: clean

clean:
	rm *.o
	rm shell
