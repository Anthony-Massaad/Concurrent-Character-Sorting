struct shared_use_st {
	char AR[7];
	int ordered[3];
	int complete;
};

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short array;
};

