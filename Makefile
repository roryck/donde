CC = cc
CFLAGS = -O2 -fopenmp

all: 
	$(CC) $(CFLAGS) -o donde donde.c -lmpi

clean:
	rm -f donde 
