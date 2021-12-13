#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>


int main(){
	struct stat st;

	const char* filename = "test.pcm";

	/*
	int fd;
	fd = open(filename, O_RDONLY);
	if(fd == -1){
		printf("open failed..");
	}

	close(fd);
	*/

	stat(filename, &st);

	printf("%s size: %d\n", filename, st.st_size);

	return 0;
}
