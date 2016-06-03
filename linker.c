#include <stdio.h>
#include <stdlib.h>
#include "error_handler.h"

#define machine_size 512

/*struct def {
	char symbol[16];
	int value;
}

struct defList {
	struct def defL[16];
}

struct use {
	char symbol[16];
}

struct useList {
	struct use useL[16];
}

struct instruction {
	char type;
	int address;
}

struct programText {
	struct instruction instL[machine_size];
}

typedef struct module {
	int position;
	struct defList dl;
	struct useList ul;
	struct programText pt;
}*/

enum Entity {
	definitions,
	uses,
	instructions,
	symbol,
	location,
	inst_type,
	instruction
};

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
	enum Entity nextType = definitions;

	while(fscanf(fp, "%c", &nextChar) != EOF) {
		//printf("%c", nextChar); //debug
		int defsRemaining = 0;
		int usesRemaining = 0;
		int instructionsRemaining = 0;

		//skip whitespace
		if(nextChar != ' ' && nextChar != '\t' && nextChar != '\n'){
			
			if(defsRemaining > 0) {
				
				defsRemaining--;
				
				if(defsRemaining == 0) {
					nextType = uses;
				}
			} else if(usesRemaining > 0) {

				usesRemaining--;

				if(usesRemaining == 0) {
					nextType = instructions;
				}
			} else if(instructionsRemaining > 0) {

				instructionsRemaining--;

				if(instructionsRemaining == 0) {
					nextType = definitions;
				}
			} else { //next char is a count of pairs or uses
				if(nextType == definitions) {
					//capture symbol and value for each def
					defsRemaining = (int)(nextChar - '0') * 2;
				} else if(nextType == uses) {
					usesRemaining = (int)(nextChar - '0');
				} else if(nextType == instructions) {
					instructionsRemaining = (int)(nextChar - '0') * 2;
				}
			}

		}//end skip whitespace
		
	}//end loop

	fclose(fp);


	return 0;
}	