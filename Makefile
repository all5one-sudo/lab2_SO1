CC = gcc
CFLAGS = -Wall -pedantic -Werror

myshell: myshell.c
	$(CC) $(CFLAGS) -o myshell myshell.c

clean:
	rm -f myshell