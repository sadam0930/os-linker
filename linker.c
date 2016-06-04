#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
	char * filename = "labsamples/input-1";
	// char filename[max_filename_size];
	// printf("Enter filename: ");
	// scanf("%s", filename);

	//read file character by character
	FILE *fp;
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("Cannot open file: %s\n", filename);
		exit(1);
	}

	char curChar;
	enum Entity nextType = definitions;
	
	/*
	moduleList and symbolTable 
	should be accessible to both passes through the file
	*/
	struct moduleList ml;
	ml.numModules = 0;
	struct symbolTable st;
	st.numSymbols = 0;
	char tempSym[16];
	int tempSymSize = 0;

	int defsRemaining = 0;
	int usesRemaining = 0;
	int instructionsRemaining = 0;
	int nextMemLocation = 0;

	bool isDefValue = false;
	bool isWord = false;

	while(fscanf(fp, "%c", &curChar) != EOF) {
		//printf("%c", curChar); //debug
		
		/*
		peek at next character
		used to group symbols and program words together
		move file pointer back to keep while loop logic consistent
		*/
		char nextChar;
		if (fscanf(fp, "%c", &nextChar) != EOF) {
			fseek(fp, -1, SEEK_CUR);
		}
		

		//skip whitespace
		if(curChar != ' ' && curChar != '\t' && curChar != '\n'){
			//printf("*****curChar = %c \n", curChar); //debug
			
			if(defsRemaining > 0) {
				//printf("DR > 0\n"); //debug
				
				/*//collect each char for symbol
				tempSym[0] = curChar;
				tempSymSize++;*/

				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					/*struct symbol;
					st.numSymbols++;
					//reset temp symbol size for next symbol
					tempSymSize = 0;*/

					defsRemaining--;
					//printf("DR -- %d\n", defsRemaining); //debug
				}
				
				if(defsRemaining == 0) {
					nextType = uses;
				}
			} else if(usesRemaining > 0) {
				//printf("UR > 0\n"); //debug
				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					usesRemaining--;
				}

				if(usesRemaining == 0) {
					nextType = instructions;
				}
			} else if(instructionsRemaining > 0) {
				//printf("IR > 0\n"); //debug
				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					instructionsRemaining--;
				}

				if(instructionsRemaining == 0) {
					nextType = definitions;
				}
			} else { //next char is a count of pairs or uses
				if(nextType == definitions) {
					module new_module;
					new_module.start_position = nextMemLocation;
					ml.mlist[ml.numModules] = new_module;
					ml.numModules++;
					//printf("start position = %d\n", new_module.start_position); //debug

					//capture symbol and value for each def
					defsRemaining = (int)(curChar - '0') * 2;

					if(defsRemaining == 0){
						nextType = uses;
					}
				} else if(nextType == uses) {
					usesRemaining = (int)(curChar - '0');

					if(usesRemaining == 0) {
						nextType = instructions;
					}
				} else if(nextType == instructions) {
					//capture type and 'word' for each instruction
					instructionsRemaining = (int)(curChar - '0') * 2;
					nextMemLocation += (int)(curChar - '0');
					
					if(instructionsRemaining == 0) {
						nextType = definitions;
					}
				}
			}
			//printf("defsRemaining = %d  \n", defsRemaining); //debug
			//printf("usesRemaining = %d \n", usesRemaining); //debug
			//printf("instructionsRemaining = %d  \n", instructionsRemaining); //debug
			//printf("nextType = %d\n", nextType); //debug
		}//end skip whitespace
		
	}//end loop

	fclose(fp);


	return 0;
}	