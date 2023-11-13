CC = mpicc
CFLAGS = -O2 -fopenmp -std=c99

all: 
	$(CC) $(CFLAGS) -o donde donde.c 

clean:
	rm -f donde 
