#include <time.h>

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

struct entry{
	int element;
	time_t timestamp;
};
