#include <stdio.h>
#include <stdlib.h>
#include "error_handler.h"

#define machine_size 512
#define max_filename_size 255

struct def {
	char symbol[16];
	int value;
};

struct defList {
	int numDefs;
	struct def defL[16];
};

struct use {
	char symbol[16];
};

struct useList {
	int numUses;
	struct use useL[16];
};

struct instruction {
	char type;
	int address;
};

struct programText {
	int numInst;
	struct instruction instL[machine_size];
};

typedef struct module {
	int start_position;
	struct defList dl;
	struct useList ul;
	struct programText pt;
} module;

struct moduleList {
	int numModules;
	module mlist[machine_size];
};

enum Entity {
	definitions,
	uses,
	instructions,
	symbol,
	location,
	inst_type,
	instruction
};

struct symbol {
	char symbolDef[16];
	int value;
};

struct symbolTable {
	int numSymbols;
	struct symbol symbolL[256];
};

int main() {
	//get filename
	char filename[max_filename_size];
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
	
	/*
	moduleList and symbolTable 
	should be accessible to both passes through the file
	*/
	struct moduleList ml;
	ml.numModules = 0;
	struct symbolTable st;
	st.numSymbols = 0;

	while(fscanf(fp, "%c", &nextChar) != EOF) {
		//printf("%c", nextChar); //debug
		int defsRemaining = 0;
		int usesRemaining = 0;
		int instructionsRemaining = 0;
		int nextMemLocation = 0;

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
					module new_module;
					new_module.start_position = nextMemLocation;
					ml.mlist[ml.numModules] = new_module;
					ml.numModules++;

					//capture symbol and value for each def
					defsRemaining = (int)(nextChar - '0') * 2;

					if(defsRemaining == 0){
						nextType = uses;
					}
				} else if(nextType == uses) {
					usesRemaining = (int)(nextChar - '0');

					if(usesRemaining == 0) {
						nextType = instructions;
					}
				} else if(nextType == instructions) {
					//capture type and 'word' for each instruction
					instructionsRemaining = (int)(nextChar - '0') * 2;
					nextMemLocation += (int)(nextChar - '0');
					
					if(instructionsRemaining == 0) {
						nextType = definitions;
					}
				}
			}

		}//end skip whitespace
		
	}//end loop

	fclose(fp);


	return 0;
}	