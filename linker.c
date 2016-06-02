#include <stdio.h>
#include <stdlib.h>

int main() {
	//get filename
	char filename[255];
	printf("Enter filename: ");
	scanf("%s", filename);
	//printf("%s\n", filename); //debug

	//read file character by character
	FILE *fp;
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("Cannot open file: %s\n", filename);
		exit(1);
	}

	char nextChar;
	while(fscanf(fp, "%c", &nextChar) != EOF) {
		//printf("%c", nextChar); //debug
	}

	fclose(fp);


	return 0;
}	