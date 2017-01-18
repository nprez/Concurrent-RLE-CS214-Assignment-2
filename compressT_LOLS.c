#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>

struct RLEargstruct{
	char* fileName;
	char* fileContents;
	int partNum;
	int partSize;
	int startIndex;
};

typedef struct RLEargstruct RLEargs;

int numParts;

/*
 * Performs Run Length Encoding on chunk of a file
 * Outputs the compressed data into a seperate file
 * Called through the spawning of threads
 */
void* RLE_Part(void* args){
	//the part of the file contents this part is working with
	char* data = malloc(1 + ((RLEargs*) args)->partSize);
	data[((RLEargs*) args)->partSize] = '\0';
	int i;
	for(i=0; i<(((RLEargs*) args)->partSize); i++){
		data[i]=((RLEargs*) args)->fileContents[i+(((RLEargs*) args)->startIndex)];
	}

	//do RLE for each character listed at least 3 times in a row
	for(i=0; i<((RLEargs*) args)->partSize; i=i){
		char current = data[i];
		int count = 1;
		while((count+i)<strlen(data) && data[count+i] == current)
			count++;
		char countString[strlen(data)];
		sprintf(countString, "%d", count);
		if(count>2){
			int j;
			//difference in legnth between uncompressed data and RLE
			int charsSaved = count-(1+strlen(countString));
			//move characters backwards
			for(j=count+i; j<strlen(data); j++)
				data[(j - charsSaved)] = data[j];
			//list count of repeated character
			for(j=0; j<strlen(countString); j++)
				data[j+i] = countString[j];
			//null terminate end, garbage data left after
			data[strlen(data)-charsSaved] = '\0';
		}
		else{
			countString[0] = (count==1)? '\0':'2';
			countString[1] = '\0';
		}
		i+=(1+strlen(countString));
	}

	//Put part of compressed data into a new file named FILEPATH_EXTENTION_LOLS#
	int digits = 1;
	int temp = (((RLEargs*) args)->partNum);
	while(temp/10 != 0){
		temp/=10;
		digits++;
	}
	char partNumString[digits];
	sprintf(partNumString, "%d", ((RLEargs*) args)->partNum);
	char* newFilePath;
	if(numParts>1)
		newFilePath = malloc(6 + strlen(partNumString) + strlen(((RLEargs*) args)->fileName));
	else
		newFilePath = malloc(6 + strlen(((RLEargs*) args)->fileName));
	strcpy(newFilePath, ((RLEargs*) args)->fileName);
	for(i=strlen(newFilePath)-1; i>=0; i--){
		if(newFilePath[i]=='.'){
			newFilePath[i]='_';
			break;
		}
	}
	char* endString;
	if(numParts>1)
		endString = malloc(6+strlen(partNumString));
	else
		endString = malloc(6);
	strcpy(endString, "_LOLS");
	if(numParts>1){
		for(i=0; i<strlen(partNumString); i++){
			endString[5+i] = partNumString[i];
		}
		endString[5+strlen(partNumString)] = '\0';
	}
	strcat(newFilePath, endString);
	FILE* newFile = fopen(newFilePath, "w");
	fprintf(newFile, data);
	free(endString);
	free(newFilePath);
	free(data);
	fclose(newFile);
	return NULL;
}

/*
 * Spawns multiple consecutively running threads in order to
 * perform Run Length Encoding in multiple parts at once
 */
int main(int argc, char** argv){
	//error checking for arguments
	if(argc != 3){
		fprintf(stderr, "ERROR: Expected 2 arguments (filepath and number of parts)\n");
		return -1;
	}
	FILE* fp = fopen(argv[1], "r");
	if(fp == NULL){
		fprintf(stderr, "ERROR: Could not open file %s\n", argv[1]);
		return -1;	
	}
	numParts = atoi(argv[2]);
	if(numParts < 1){
		fprintf(stderr, "ERROR: Needs at least 1 part\n");
		fclose(fp);
		return -1;
	}
	
	char* fileContents;	//file turned into a string
	
	//find size of file, then put its contents in fileContents
	long fileSize;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);
	fileContents = malloc(1 + fileSize * (sizeof(char)));
	fread(fileContents, sizeof(char), fileSize, fp);
	fileContents[fileSize] = '\0';

	//remove non alphabetic characters from fileContents
	int i;
	for(i=0; i<strlen(fileContents); i++){
		if(!isalpha(fileContents[i])){
			int j;
			for(j=i; j<strlen(fileContents); j++)
				fileContents[j] = fileContents[j+1];
			i--;
		}
	}

	if(numParts>strlen(fileContents)){
		fprintf(stderr, "ERROR: More parts than characters in file\n");
		free(fileContents);
		fclose(fp);
		return -1;
	}

	//spawn multiple threads to do parts of RLE
	pthread_t ids[numParts];
	RLEargs* args[numParts];
	int startIndex = 0;
	for(i=0; i<numParts; i++){
		args[i] = malloc(sizeof(RLEargs));
		args[i]->fileName = argv[1];
		args[i]->fileContents = fileContents;
		args[i]->partNum = i;
		args[i]->partSize = strlen(fileContents)/numParts;
		if(i==0)
			args[i]->partSize += strlen(fileContents)%numParts;
		args[i]->startIndex = startIndex;
		startIndex+=args[i]->partSize;
		pthread_create(&(ids[i]), NULL, RLE_Part, args[i]);
	}

	//wait on each thread before ending
	void* ret = NULL;
	for(i=0; i<numParts; i++)
		pthread_join(ids[i], ret);

	free(fileContents);
	for(i=0; i<numParts; i++)
		free(args[i]);
	fclose(fp);
	return 0;
}