OBJS = coordinator.o read_write_functions.o sem_functions.o
SOURCE = coordinator.c read_write_functions.c sem_functions.c
HEADER = semun.h functions.h #structs.h 
OUT = coordinator
CC = gcc
FLAG = -c 

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) 

coordinator.o: coordinator.c
	$(CC) $(FLAG) coordinator.c

sem_functions.o: sem_functions.c
	$(CC) $(FLAG) sem_functions.c

read_write_functions.o: read_write_functions.c
	$(CC) $(FLAG) read_write_functions.c

clean:
	rm -f $(OBJS) $(OUT)
