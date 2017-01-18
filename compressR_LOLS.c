#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "compressR_worker_LOLS.h"

void compressR_LOLS(char* file, int numParts){
	//error checking for arguments
	if(numParts < 1){
		fprintf(stderr, "ERROR: Needs at least 1 part\n");
		return;
	}

	pid_t pid = 1;
	//stores what part we're working with
	int count = 0;
	//main process forks numParts times; we wind up with a number of processes equal to numParts
	while(pid != 0 && count<numParts){
		pid = fork();
		count++;
	}
	count--;
	//main process waits on numParts children
	int i;
	if(pid!=0){
		for(i=0; i<numParts; i++){
			wait(NULL);
		}
	}
	//each child does the actual work
	else{
		char* args[5];
		//exec worker file
		args[0] = "./compressR_worker_LOLS.out";
		args[1] = file;
		int digits = 1;
		int temp = count;
		while(temp/10 != 0){
			temp/=10;
			digits++;
		}
		args[2] = malloc(digits + 1);
		sprintf(args[2], "%d", count);
		int npDigits = 1;
		int npTemp = numParts;
		while(npTemp/10 != 0){
			npTemp/=10;
			npDigits++;
		}
		char* numPartsString = malloc(1 + npDigits);
		sprintf(numPartsString, "%d", numParts);
		args[3] = numPartsString;
		args[4] = NULL;
		execv(args[0], args);

		free(args[2]);
	}
}

int main(int argc, char** argv){
	//error checking for arguments
	if(argc != 3){
		fprintf(stderr, "ERROR: Expected 2 arguments (filepath and number of parts)\n");
		return -1;
	}
	compressR_LOLS(argv[1], atoi(argv[2]));
	return 0;
}