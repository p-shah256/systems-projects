#
# Makefile for lab 2
#

CFLAGS = -ggdb3 -Wall -pedantic -g -fstack-protector-all -fsanitize=address
shell56: shell56.c parser.c 	
	gcc shell56.c parser.c internal_commands.c external_commands.c command_loop.c -o shell56 $(CFLAGS)

clean:
	rm -f *.o shell56
