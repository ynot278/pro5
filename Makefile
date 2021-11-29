CC = gcc
CFLAGS = 

all: oss user_proc

oss: main.c oss.h
	$(CC) $(CFLAGS) main.c -o oss

user: user_proc.c oss.h
	$(CC) $(CFLAGS) user_proc.c -o user_proc

clean:
	rm -rf oss user_proc *.txt