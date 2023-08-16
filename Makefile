CC = cc
CFLAGS = -O2 -fopenmp

all: 
	$(CC) $(CFLAGS) -o donde donde.c 

clean:
	rm -f donde 
