OBJS = feeder.o sem_functions.o
SOURCE = feeder.c sem_functions.c
HEADER = structs.h functions.h
OUT = feeder
CC = gcc
FLAG = -c

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT)

feeder.o: feeder.c
	$(CC) $(FLAG) feeder.c

sem_functions.o: sem_functions.c
	$(CC) $(FLAG) sem_functions.c

clean:
	rm -f $(OBJS) $(OUT) *.txt
