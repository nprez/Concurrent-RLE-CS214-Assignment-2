#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * Does RLE on a given file
 * Expects 4 arguments
 * argv[0] is "./compressR_worker_LOLS.out"
 * argv[1] is the name of the file to be compressed
 * argv[2] is what part this is
 * argv[3] is the total number of parts
 */
int main(int argc, char** argv){
	//error checking
	if(argc!=4){
		fprintf(stderr, "ERROR: Expected 4 arguments\n");
		return -1;
	}
	if(atoi(argv[3])<1){
		fprintf(stderr, "ERROR: Needs at least 1 part\n");
		return -1;
	}
	if(atoi(argv[2])<0 || atoi(argv[2])>(atoi(argv[3])-1) ){
		fprintf(stderr, "ERROR: Invalid part number\n");
		return -1;
	}

	FILE* fp = fopen(argv[1], "r");
	if(fp == NULL){
		fprintf(stderr, "ERROR: Could not open file %s\n", argv[1]);
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

	if(atoi(argv[3])>strlen(fileContents)){
		fprintf(stderr, "ERROR: More parts than characters in file\n");
		return -1;
	}

	int startIndex = 0;
	int partSize = 0;
	if(atoi(argv[2])==0)
		partSize = strlen(fileContents)%atoi(argv[3]) + strlen(fileContents)/atoi(argv[3]);
	else{
		for(i=0; i<atoi(argv[2]); i++){
			if(i==0){
				startIndex += strlen(fileContents)%atoi(argv[3]);
			}
			startIndex += strlen(fileContents)/atoi(argv[3]);
		}
		partSize = strlen(fileContents)/atoi(argv[3]);
	}

	//store the part of the file we're working with
	char* data = malloc(1 + partSize);
	data[partSize] = '\0';
	for(i=0; i<partSize; i++)
		data[i]=fileContents[i+startIndex];

	//do RLE for each character listed at least 3 times in a row
	for(i=0; i<strlen(data); i=i){
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
	int temp = atoi(argv[2]);
	while(temp/10 != 0){
		temp/=10;
		digits++;
	}
	char* partNumString = malloc(digits + 1);
	sprintf(partNumString, "%d", atoi(argv[2]));
	char* newFilePath;
	if(atoi(argv[3])>1)
		newFilePath = malloc(6 + strlen(partNumString) + strlen(argv[1]));
	else
		newFilePath = malloc(6 + strlen(argv[1]));
	strcpy(newFilePath, argv[1]);
	for(i=strlen(newFilePath)-1; i>=0; i--){
		if(newFilePath[i]=='.'){
			newFilePath[i]='_';
			break;
		}
	}
	char* endString;
	if(atoi(argv[3])>1)
		endString = malloc(6+strlen(partNumString));
	else
		endString = malloc(6);
	strcpy(endString, "_LOLS");
	if(atoi(argv[3])>1){
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
	return 0;
}