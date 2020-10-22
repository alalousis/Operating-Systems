union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

struct entry{
	int readings;
	int writings;
};
