#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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
	int opcode;
	int address;
	int immediate;
};

struct programText {
	int numInst;
	struct instruction instL[machine_size];
};

typedef struct module {
	int start_position;
	//struct defList dl;
	//struct useList ul;
	//struct programText pt;
} module;

struct moduleList {
	int numModules;
	module mlist[machine_size];
};

enum Entity {
	definitions,
	uses,
	instructions
	//symbol,
	//location,
	//inst_type,
	//instruction
};

struct symbol {
	char symbolDef[16];
	int numChar;
	int relvalue;
	int absvalue;
	bool isDuplicate;
	bool valToBig;
};

struct symbolTable {
	int numSymbols;
	struct symbol symbolL[256];
};

int main() {
	//get filename
	char * filename = "labsamples/input-10";
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
	struct useList ul;
	ul.numUses = 0;

	char tempSym[16];
	int tempSymSize = 0;
	int tempSymValue[512];
	int tempSymValuePos = 0;

	int defsRemaining = 0;
	int usesRemaining = 0;
	int instructionsRemaining = 0;
	int nextMemLocation = 0;

	bool isDefSym = false;
	bool isWord = false;
	bool isDuplicate = false;

	//FIRST PASS
	while(fscanf(fp, "%c", &curChar) != EOF) {
		//printf("%c\n", curChar); //debug
		
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
				
				if(defsRemaining%2 == 0){
					isDefSym = true;
				} else {
					isDefSym = false;
				}

				if(isDefSym){
					//collect each char for symbol
					tempSym[tempSymSize] = curChar;
					tempSymSize++;
				} else {
					//curChar is a relative symbol value
					tempSymValue[tempSymValuePos] = (int)(curChar - '0');
					tempSymValuePos++;
					//printf("tempSymValue = %d\n", tempSymValue[tempSymValuePos-1]); //debug
				}

				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					//put symbol in symbol table
					if(isDefSym){
						//printf("making symbol\n"); //debug

						//check for duplicate symbol already in symbol table
						int i;
						isDuplicate = false;
						for(i=0; i < st.numSymbols; i++){
							if(strcmp(tempSym, st.symbolL[i].symbolDef) == 0){
								isDuplicate = true;
								st.symbolL[i].isDuplicate = true;
								break;
							}
						}

						if(!isDuplicate){
							struct symbol new_symbol;
							new_symbol.numChar = 0;

							for(i=0; i < tempSymSize; i++){
								//printf("tempSymSize = %d\n", i); //debug
								//printf("tempSym = %c\n", tempSym[i]); //debug
								new_symbol.symbolDef[i] = tempSym[i];
								new_symbol.numChar++;
							}
							st.symbolL[st.numSymbols] = new_symbol;
						}

						//reset temp symbol size for next symbol
						tempSymSize = 0;
					} else {
						//printf("in else\n"); //debug
						//curChar is a relative symbol value
						if(!isDuplicate){
							//construct relative value from captured values in tempSymValue array
							int symValue = 0;
							int i;
							int decimalPower = 0;
							//[1.2,3,4]
							//tempSymValuePos = 4
							for(i=tempSymValuePos; i > 0; i--){
								//printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
								int multiplier = 10;
								if(decimalPower == 0){
									multiplier = 1;
								} else {
									multiplier ^= decimalPower;
								}
								//printf("multiplier = %d\n", multiplier); //debug
								symValue += tempSymValue[i-1] * multiplier;
								//printf("symValue = %d\n", symValue); //debug
								decimalPower++;
							}
							st.symbolL[st.numSymbols].relvalue = symValue;
							//st.symbolL[st.numSymbols].absvalue = ml.mlist[ml.numModules-1].start_position + symValue;

							tempSymValuePos = 0;
							//end of this symbol; increment symbol table count
							st.numSymbols++;
						}
					}

					defsRemaining--;
					//printf("DR -- %d\n", defsRemaining); //debug
				}
				
				if(defsRemaining == 0) {
					nextType = uses;
				}
			} else if(usesRemaining > 0) {
				//printf("UR > 0\n"); //debug
				//ml.mlist[numModules-1].

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

					/*
					nextMemLocation is also the size of the current module plus 1 (0 based counting)
					check symbol values do not exceed the module size
					*/
					if(st.symbolL[st.numSymbols-1].relvalue > nextMemLocation-1){
						printf("Warning: Module %d: ", ml.numModules);
						int i;
						for(i=0; i < st.symbolL[st.numSymbols-1].numChar; i++){
							printf("%c", st.symbolL[st.numSymbols-1].symbolDef[i]);
						}
						printf(" to big %d (max=%d) assume zero relative\n", st.symbolL[st.numSymbols-1].relvalue, nextMemLocation-1);
						st.symbolL[st.numSymbols-1].absvalue = ml.mlist[ml.numModules-1].start_position + 0;
					} else {
						st.symbolL[st.numSymbols-1].absvalue = ml.mlist[ml.numModules-1].start_position + st.symbolL[st.numSymbols-1].relvalue;
					}
					
					if(instructionsRemaining == 0) {
						nextType = definitions;
					}
				}
			}
			//printf("defsRemaining = %d  \n", defsRemaining); //debug
			//printf("usesRemaining = %d \n", usesRemaining); //debug
			//printf("instructionsRemaining = %d  \n", instructionsRemaining); //debug
			//printf("nextType = %d\n", nextType); //debug
			/*int i;
			for(i=0; i<st.numSymbols; i++){
				int j;
				printf("symbol value = ");
				for(j=0; j < st.symbolL[i].numChar; j++){
					printf("%c", st.symbolL[i].symbolDef[j]);
				}
				printf(" \n");
				printf("relative symbol value = %d\n", st.symbolL[i].relvalue);
				printf("absolute symbol value = %d\n", st.symbolL[i].absvalue);
			}*/
		}//end skip whitespace
		
	}//END FIRST PASS

	//print symbol table after first pass
	printf("Symbol Table\n");
	int i;
	for(i=0; i<st.numSymbols; i++){
		int j;
		for(j=0; j<st.symbolL[i].numChar; j++){
			printf("%c", st.symbolL[i].symbolDef[j]);
		}
		printf("=");
		printf("%d", st.symbolL[i].absvalue);
		if(st.symbolL[i].isDuplicate){
			printf(" Error: This variable is multiple times defined; first value used");
		}
		printf("\n");
	}
	printf("\n");

	//START SECOND PASS
	//start from the top
	rewind(fp);
	//reset variable pointers

	while(fscanf(fp, "%c", &curChar) != EOF) {
		printf("%c\n", curChar); //debug
		
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
				
				if(defsRemaining%2 == 0){
					isDefSym = true;
				} else {
					isDefSym = false;
				}

				if(isDefSym){
					
				} else {
					
				}

				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					//put symbol in symbol table
					if(isDefSym){
						
					} else {
						//printf("in else\n"); //debug

						//curChar is a relative symbol value
					}

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
					//start module
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
		}//end skip whitespace
		
	}//END SECOND PASS


	fclose(fp);

	return 0;
}	