# need to install subunit, check
#
# file:    Makefile
#
LDLIBS = -lcheck -lsubunit
CFLAGS = -g

# default build rules:
# .c to .o: $(CC) $(CFLAGS) file.c -c -o file.o
# multiple .o to exe. : $(CC) $(LDFLAGS) file.o [file.o..] $(LDLIBS) -o exe

test: test.o qthread.o qthread_manager.o switch.o

switch.o: switch.S
	gcc -g -c switch.S -o switch.o

clean:
	rm -f *.o test
