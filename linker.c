/*
Author: Steven Adam

Dear Reader,

This code is not the best example of my work, but I am new to C
and ended up trying to plow through parsing each file. 
I would have benefitted from creating some reuseable functions.
I apologize for the spaghetti code below.

Synopsis:
Perform two passes (while loops) over a given input file. Both loops
contain logic to account for where the pointer is in the file in reference to
- definition list
- use list
- instruction list (or sometimes called program text)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "error_handler.h"

#define machine_size 512
#define max_filename_size 255
#define DEF_LIMIT 16

struct symbol {
	char symbolDef[16];
	int numChar;
	int relvalue;
	int absvalue;
	bool isDuplicate;
	bool valToBig;
	bool wasUsed;
	int whichModule;
};

struct defList {
	int numDefs;
	struct symbol defL[16];
};

struct use {
	char symbol[16];
	int numChar;
	bool wasUsed;
};

struct useList {
	int numUses;
	struct use useL[16];
};

struct instruction {
	char type;
	char strInstr[16];
	int numChar;
};

struct programText {
	int numInst;
	struct instruction instL[machine_size];
};

typedef struct module {
	int start_position;
	int module_size;
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
	instructions
};

struct symbolTable {
	int numSymbols;
	struct symbol symbolL[256];
};

int main() {
	//get filename
	//char * filename = "labsamples/input-1";
	char filename[max_filename_size];
	printf("Enter filename: ");
	scanf("%s", filename);

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
	int totalInstructions = 0;

	char tempSym[16] = "";
	int tempSymSize = 0;
	int tempSymValue[512];
	int tempSymValuePos = 0;

	int defsRemaining = 0;
	int usesRemaining = 0;
	int instructionsRemaining = 0;
	int nextMemLocation = 0;

	bool isDefSym = false;
	bool isInstType = false;
	bool isDuplicate = false;
	bool found = false;

	int lineNum = 1;
	int offset = 1;

	//FIRST PASS
	while(fscanf(fp, "%c", &curChar) != EOF) {
		// printf("curChar = %c\n", curChar); //debug
		
		/*
		peek at next character
		used to group symbols and program words together
		move file pointer back to keep while loop logic consistent
		*/
		char nextChar;
		if (fscanf(fp, "%c", &nextChar) != EOF) {
			fseek(fp, -1, SEEK_CUR);
		} else {
			//nextChar == EOF
			if(curChar == '\n' && nextType == definitions && defsRemaining > 0 && defsRemaining%2 == 0){
				__parseerror(lineNum, offset, 1);
				exit(1);
			}
			if(nextType == instructions && instructionsRemaining > 0 && instructionsRemaining%2 == 0){
				__parseerror(lineNum, offset, 2);
				exit(1);
			}
		}
		
		if(curChar == '\n'){
			// printf("defsRemaining = %d\n", defsRemaining); //debug
			// printf("nextType = %d\n", nextType); //debug
			lineNum++;
			offset = 1;
		} else {
			offset++;
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
					//check next character is alpha
					if(tempSymSize == 0){
						if(!isalpha(curChar)){
							//printf("curChar = %c\n", curChar); //debug
							__parseerror(lineNum, offset-1, 1);
							exit(1);
						}
					}
					
					//collect each char for symbol
					tempSym[tempSymSize] = curChar;
					tempSymSize++;

					if(tempSymSize > DEF_LIMIT){
						__parseerror(lineNum, offset, 3);
						exit(1);
					}
				} else {
					//curChar should be a relative symbol value
					//check
					if(isalpha(curChar)){
						__parseerror(lineNum, offset-1, 0);
						exit(1);
					} else {
						tempSymValue[tempSymValuePos] = (int)(curChar - '0');
						tempSymValuePos++;
						//printf("tempSymValue = %d\n", tempSymValue[tempSymValuePos-1]); //debug
					}
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
							// printf("isDuplicate = %d\n", isDuplicate); //debug
							struct symbol new_symbol;
							new_symbol.numChar = 0;

							for(i=0; i < tempSymSize; i++){
								//printf("tempSymSize = %d\n", i); //debug
								//printf("tempSym = %c\n", tempSym[i]); //debug
								new_symbol.symbolDef[i] = tempSym[i];
								new_symbol.numChar++;
								new_symbol.whichModule = ml.numModules;
							}
							st.symbolL[st.numSymbols] = new_symbol;
							ml.mlist[ml.numModules-1].dl.defL[ml.mlist[ml.numModules-1].dl.numDefs] = new_symbol;
						}

						//reset temp symbol size for next symbol
						tempSymSize = 0;
						memset(tempSym,0,sizeof(tempSym));
					} else {
						//printf("in else\n"); //debug
						//curChar is a relative symbol value
						if(!isDuplicate){
							//construct relative value from captured values in tempSymValue array
							int symValue = 0;
							int i;
							int decimalPower = 0;
							for(i=tempSymValuePos; i > 0; i--){
								//printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
								int multiplier = 10;
								if(decimalPower == 0){
									multiplier = 1;
								} else {
									multiplier = pow(multiplier, decimalPower);
								}
								//printf("multiplier = %d\n", multiplier); //debug
								symValue += tempSymValue[i-1] * multiplier;
								//printf("symValue = %d\n", symValue); //debug
								decimalPower++;
							}
							st.symbolL[st.numSymbols].relvalue = symValue;
							ml.mlist[ml.numModules-1].dl.defL[ml.mlist[ml.numModules-1].dl.numDefs].relvalue = symValue;
							//st.symbolL[st.numSymbols].absvalue = ml.mlist[ml.numModules-1].start_position + symValue;

							//end of this symbol; increment symbol table count
							st.numSymbols++;
							ml.mlist[ml.numModules-1].dl.numDefs++;
						}
					}
					
					tempSymValuePos = 0;
					defsRemaining--;
					//printf("DR -- %d\n", defsRemaining); //debug
				}
				
				if(defsRemaining == 0) {
					nextType = uses;
				}
			} else if(usesRemaining > 0) {
				//printf("UR > 0\n"); //debug
				//construct symbol to put into uselist
				//check next character is alpha
				if(tempSymSize == 0){
					if(!isalpha(curChar)){
						//printf("curChar = %c\n", curChar); //debug
						__parseerror(lineNum, offset-1, 1);
						exit(1);
					}
				}

				tempSym[tempSymSize] = curChar;
				tempSymSize++;

				if(tempSymSize > DEF_LIMIT){
					__parseerror(lineNum, offset, 3);
					exit(1);
				}

				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					//struct module thisMod = ml.mlist[ml.numModules-1];
					int i;
					for (i=0; i < tempSymSize; i++){
						ml.mlist[ml.numModules-1].ul.useL[ml.mlist[ml.numModules-1].ul.numUses].symbol[i] = tempSym[i];
						ml.mlist[ml.numModules-1].ul.useL[ml.mlist[ml.numModules-1].ul.numUses].numChar++;
					}
					/*printf("symbol in use: "); //debug
					for(i=0; i<ml.mlist[ml.numModules-1].ul.useL[ml.mlist[ml.numModules-1].ul.numUses].numChar; i++){
						printf("%c", ml.mlist[ml.numModules-1].ul.useL[ml.mlist[ml.numModules-1].ul.numUses].symbol[i]);//debug	
					}
					printf("\n");*/
					ml.mlist[ml.numModules-1].ul.numUses++;

					//reset tempSym
					tempSymSize = 0;
					memset(tempSym,0,sizeof(tempSym));
					usesRemaining--;
				}

				if(usesRemaining == 0) {
					nextType = instructions;
				}
			} else if(instructionsRemaining > 0) {
				if(instructionsRemaining%2 == 0){
					isInstType = true;
				} else {
					isInstType = false;
				}

				//get each digit of address/immediate
				if(!isInstType){
					tempSym[tempSymSize] = curChar;
					tempSymSize++;
				}

				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					if(isInstType){	
						
					} else {
						//printf("tempSymSize = %d\n", tempSymSize); //debug
						if(tempSymSize < 4){
							__parseerror(lineNum, offset, 2);
							exit(1);
						}
					}

					tempSymSize = 0;
					memset(tempSym,0,sizeof(tempSym));
					instructionsRemaining--;
				}

				if(instructionsRemaining == 0) {
					nextType = definitions;
				}
			} else { //next char is a count of pairs or uses
				// printf("nextType = %d\n", nextType); //debug
				if(nextType == definitions) {
					tempSymValue[tempSymValuePos] = curChar;
					tempSymValuePos++;
					
					if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
						//printf("in if\n"); //debug
						module new_module;
						new_module.start_position = nextMemLocation;
						ml.mlist[ml.numModules] = new_module;
						ml.numModules++;
						//printf("start position = %d\n", new_module.start_position); //debug

						int i;
						int decimalPower = 0;
						for(i=tempSymValuePos-1; i >= 0; i--){
							// printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
							int multiplier = 10;
							if(decimalPower == 0){
								multiplier = 1;
							} else {
								multiplier = pow(multiplier, decimalPower);
							}
							//printf("multiplier = %d\n", multiplier); //debug
							defsRemaining += (int)(tempSymValue[i] - '0') * multiplier;
							// printf("defsRemaining = %d\n", defsRemaining); //debug
							decimalPower++;
						}

						if(defsRemaining > DEF_LIMIT){
							__parseerror(lineNum, offset-tempSymValuePos, 4);
							exit(1);
						} else {
							defsRemaining *= 2;
						}

						tempSymValuePos = 0;
						if(defsRemaining == 0){
							nextType = uses;
						}
					} 
				} else if(nextType == uses) {
					// printf("tempSymValuePos1 = %d\n", tempSymValuePos); //debug
					tempSymValue[tempSymValuePos] = curChar;
					tempSymValuePos++;
					
					if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
						int i;
						int decimalPower = 0;
						for(i=tempSymValuePos-1; i >= 0; i--){
							// printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
							int multiplier = 10;
							if(decimalPower == 0){
								multiplier = 1;
							} else {
								multiplier = pow(multiplier, decimalPower);
							}
							//printf("multiplier = %d\n", multiplier); //debug
							usesRemaining += (int)(tempSymValue[i] - '0') * multiplier;
							// printf("usesRemaining = %d\n", usesRemaining); //debug
							decimalPower++;
						}

						if(usesRemaining > DEF_LIMIT){
							__parseerror(lineNum, offset-tempSymValuePos, 5);
							exit(1);
						}

						tempSymValuePos = 0;

						if(usesRemaining == 0) {
							nextType = instructions;
						}
					}
				} else if(nextType == instructions) {
					tempSymValue[tempSymValuePos] = curChar;
					tempSymValuePos++;
					
					if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
						int i;
						int decimalPower = 0;
						for(i=tempSymValuePos-1; i >= 0; i--){
							// printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
							int multiplier = 10;
							if(decimalPower == 0){
								multiplier = 1;
							} else {
								multiplier = pow(multiplier, decimalPower);
							}
							//printf("multiplier = %d\n", multiplier); //debug
							instructionsRemaining += (int)(tempSymValue[i] - '0') * multiplier;
							// printf("instructionsRemaining = %d\n", instructionsRemaining); //debug
							decimalPower++;
						}
						nextMemLocation += instructionsRemaining;
						totalInstructions += nextMemLocation;

						if(totalInstructions > machine_size || (instructionsRemaining > machine_size-1)){
							__parseerror(lineNum, offset-tempSymValuePos, 6);
							exit(1);
						} else {
							instructionsRemaining *= 2;
						}

						/*
						nextMemLocation is also the size of the current module plus 1 (0 based counting)
						*/
						ml.mlist[ml.numModules-1].module_size = nextMemLocation;

						tempSymValuePos = 0;

						if(instructionsRemaining == 0) {
							nextType = definitions;
						}
					}
				}
			}
			// printf("defsRemaining = %d  \n", defsRemaining); //debug
			// printf("usesRemaining = %d \n", usesRemaining); //debug
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
			}//debug*/
		}//end skip whitespace
		
	}//END FIRST PASS

	//check symbol values do not exceed the module size
	int whichSym = 0;
	int z;
	for(z=0; z < ml.numModules; z++){
		struct defList dl = ml.mlist[z].dl;
		int j;
		for(j=0; j < dl.numDefs; j++){
			if(dl.defL[j].relvalue > ml.mlist[z].module_size-1){
				printf("Warning: Module %d: ", z);
				int k;
				for(k=0; k < dl.defL[j].numChar; k++){
					printf("%c", dl.defL[j].symbolDef[k]);
				}
				printf(" to big %d (max=%d) assume zero relative\n", dl.defL[j].relvalue, ml.mlist[z].module_size-1);
				st.symbolL[whichSym].absvalue = dl.defL[j].absvalue = ml.mlist[z].start_position + 0;
			} else {
				st.symbolL[whichSym].absvalue = dl.defL[j].absvalue = ml.mlist[z].start_position + dl.defL[j].relvalue;
			}
			whichSym++;
		}
	}

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
	printf("Memory Map\n");

	//START SECOND PASS
	//start from the top
	rewind(fp);
	struct module curMod;
	int whichModule = -1;
	bool illegal;
	bool exceeds;

	while(fscanf(fp, "%c", &curChar) != EOF) {
		// printf("curChar = %c\n", curChar); //debug
		
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

				if(instructionsRemaining%2 == 0){
					isInstType = true;
				} else {
					isInstType = false;
				}

				//get each digit of address/immediate
				if(!isInstType){
					tempSym[tempSymSize] = curChar;
					tempSymSize++;
				}

				if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
					if(isInstType){	
						curMod.pt.instL[curMod.pt.numInst].type = curChar;
					} else {
						//it's an address or immediate instruction
						int i;
						for(i=0; i < tempSymSize; i++){
							//printf("%c", tempSym[i]); //debug
							curMod.pt.instL[curMod.pt.numInst].strInstr[i] = tempSym[i];
							curMod.pt.instL[curMod.pt.numInst].numChar++;	
							//printf("%c", curMod.pt.instL[curMod.pt.numInst].strInstr[i]); //debug
						}
						//print memory map
						//printf("startPos = %d\n", curMod.start_position); //debug
						printf("%03d: ", curMod.start_position + curMod.pt.numInst);
						
						int s;
						int absaddress = 0;
						int decimalPower = 0;
						//printf("type = %c\n", curMod.pt.instL[curMod.pt.numInst].type);//debug
						switch(curMod.pt.instL[curMod.pt.numInst].type) {
							case 'I':
								//printf("in I"); //debug
								//printf("numChar = %d ", curMod.pt.instL[curMod.pt.numInst].numChar); //debug
								if(curMod.pt.instL[curMod.pt.numInst].numChar > 4){
									printf("9999 Error: Illegal immediate value; treated as 9999");
								} else {
									for(s=0; s < curMod.pt.instL[curMod.pt.numInst].numChar; s++){
	 									//printf("in for"); //debug
	 									printf("%c", curMod.pt.instL[curMod.pt.numInst].strInstr[s]);
 									}
								}
								break;
							case 'R':
								if(curMod.pt.instL[curMod.pt.numInst].numChar > 4){
									printf("9");
									absaddress = 999;
									illegal = true;
								} else {
									illegal = false;
									printf("%c", curMod.pt.instL[curMod.pt.numInst].strInstr[0]); //opcode
									//add relative address to module start position
									for(s=curMod.pt.instL[curMod.pt.numInst].numChar-1; s > 0; s--){
										// printf("char = %c", curMod.pt.instL[curMod.pt.numInst].strInstr[s]); //debug
										int multiplier = 10;
										if(decimalPower == 0){
											multiplier = 1;
										} else {
											multiplier = pow(multiplier, decimalPower);
										}
										absaddress += (int)(curMod.pt.instL[curMod.pt.numInst].strInstr[s] - '0') * multiplier;
										decimalPower++;
									}
									decimalPower = 0;
									if(absaddress > curMod.module_size){
										absaddress = 0;
										exceeds = true;
									} else {
										exceeds = false;
									}
									absaddress += curMod.start_position;
								}
								
								printf("%03d", absaddress);
								if(illegal){
									printf(" Error: Illegal opcode; treated as 9999");
								} 
								if(exceeds){
									printf(" Error: Relative address exceeds module size; zero used");
								}
								break;
							case 'A':
								printf("%c", curMod.pt.instL[curMod.pt.numInst].strInstr[0]); //opcode

								for(s=curMod.pt.instL[curMod.pt.numInst].numChar-1; s > 0; s--){
									//printf(" char = %c ", curMod.pt.instL[curMod.pt.numInst].strInstr[s]); //debug
									int multiplier = 10;
									// printf(" decP = %d ", decimalPower); //debug
									if(decimalPower == 0){
										multiplier = 1;
									} else {
										multiplier = pow(multiplier, decimalPower);
									}
									// printf(" mult = %d ", multiplier); //debug
									absaddress += (int)(curMod.pt.instL[curMod.pt.numInst].strInstr[s] - '0') * multiplier;
									decimalPower++;
								}
									decimalPower = 0;
								// printf(" absaddress = %d ", absaddress); //debug
								if(absaddress > machine_size-1){
									absaddress = 0;
									illegal = true;
								} else {
									illegal = false;
								}

								printf("%03d", absaddress);

								if(illegal){
									printf(" Error: Absolute address exceeds machine size; zero used");
								}
								// for(s=0; s < curMod.pt.instL[curMod.pt.numInst].numChar ;s++){
								// 	//printf("in for"); //debug
								// 	printf("%c", curMod.pt.instL[curMod.pt.numInst].strInstr[s]);
								// }
								break;
							case 'E':
								printf("%c", curMod.pt.instL[curMod.pt.numInst].strInstr[0]); //opcode
								//replace with symbol value where operand is index of the symbol in symbol table
								//printf("numChar = %d ", curMod.pt.instL[curMod.pt.numInst].numChar); //debug
								for(s=curMod.pt.instL[curMod.pt.numInst].numChar-1; s > 0; s--){
									// printf("char = %c", curMod.pt.instL[curMod.pt.numInst].strInstr[s]); //debug
									int multiplier = 10;
									if(decimalPower == 0){
										multiplier = 1;
									} else {
										multiplier = pow(multiplier, decimalPower);
									}
									absaddress += (int)(curMod.pt.instL[curMod.pt.numInst].strInstr[s] - '0') * multiplier;
									decimalPower++;
								}
								decimalPower = 0;
								//printf("absaddress = %d ", absaddress); //debug
								/*for(int j=0; j < st.symbolL[absaddress].numChar; j++){
									printf("%c", st.symbolL[absaddress].symbolDef[j]);
								}
								printf(" symValue = %d ", st.symbolL[absaddress].absvalue); //debug*/
								
								//get the use symbol and find it in the symbol table
								/*for(s=0; s < curMod.ul.useL[absaddress].numChar; s++){
									printf("%c", curMod.ul.useL[absaddress].symbol[s]);
								}	*/
								if(absaddress > curMod.ul.numUses){
									exceeds = true;
								} else {
									exceeds = false;
									found = false;

									if(curMod.ul.useL[absaddress].symbol[0] != 0){
										curMod.ul.useL[absaddress].wasUsed = true;
									}

									for(s=0; s < st.numSymbols; s++){
										if(curMod.ul.useL[absaddress].numChar == 1){
											if(st.symbolL[s].symbolDef[0] == curMod.ul.useL[absaddress].symbol[0]){
												// printf("found1\n"); //debug
												found = true;
												// curMod.ul.useL[absaddress].wasUsed = true;
												st.symbolL[s].wasUsed = true;
												absaddress = st.symbolL[s].absvalue;
											}
										} else {
											if(strcmp(st.symbolL[s].symbolDef, curMod.ul.useL[absaddress].symbol) == 0){
												// printf("absValue = %d", st.symbolL[s].absvalue);//debug
												// printf("found2\n"); //debug
												found = true;
												// curMod.ul.useL[absaddress].wasUsed = true;
												st.symbolL[s].wasUsed = true;
												absaddress = st.symbolL[s].absvalue;
												//printf("add = %d ", absaddress); //debug
												//printf("wasUsed1 = %d ", curMod.ul.useL[absaddress].wasUsed); //debug
												//printf(" symValue = %d ", st.symbolL[s].absvalue); //debug
											}
										}
									}
								}
								
								// if(!found){
								// 	printf("%03d", 0);
								// } else {
								// 	printf("%03d", absaddress);
								// }
								
								if(exceeds){
									printf("%03d", absaddress);
									printf(" Error: External address exceeds length of uselist; treated as immediate");
								} else if(!found){
									printf("%03d", 0);
									printf(" Error: ");
									int i;
									// printf(" numChar = %d ", curMod.ul.useL[absaddress]); //debug
									for(i=0; i < curMod.ul.useL[absaddress].numChar; i++){
										printf("%c", curMod.ul.useL[absaddress].symbol[i]);
									}
									printf(" is not defined; zero used");
								} else {
									printf("%03d", absaddress);
								}
								break;
						}
						printf("\n");

						curMod.pt.numInst++;
					}

					tempSymSize = 0;
					memset(tempSym,0,sizeof(tempSym));
					instructionsRemaining--;
				}

				if(instructionsRemaining == 0) {
					//ToDo: Make sure wasUsed is set properly above
					int t;
					for(t=0; t < curMod.ul.numUses; t++){
						if(!curMod.ul.useL[t].wasUsed){
							//printf("wasUsed2 = %d ", curMod.ul.useL[t].wasUsed); //debug
							int s;
							/*printf(" t = %d", t); //debug
							for(s=0; s < curMod.ul.useL[t].numChar; s++){
								printf("%c", curMod.ul.useL[t].symbol[s]);
							}	*/
							printf("Warning: Module %d: ", whichModule+1);
							int w;
							for(w=0; w < curMod.ul.useL[t].numChar; w++){
								printf("%c", curMod.ul.useL[t].symbol[w]);
							}
							printf(" appeared in the uselist but was not actually used\n");
						}
					}

					nextType = definitions;
				}
			} else { //next char is a count of pairs or uses
				if(nextType == definitions) {
					tempSymValue[tempSymValuePos] = curChar;
					tempSymValuePos++;
					
					if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
						//printf("in if\n"); //debug
						// module new_module;
						// new_module.start_position = nextMemLocation;
						// ml.mlist[ml.numModules] = new_module;
						// ml.numModules++;
						//printf("start position = %d\n", new_module.start_position); //debug

						//start module
						whichModule++;
						//printf("whichModule = %d ", whichModule); //debug
						curMod = ml.mlist[whichModule];

						int i;
						int decimalPower = 0;
						for(i=tempSymValuePos-1; i >= 0; i--){
							// printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
							int multiplier = 10;
							if(decimalPower == 0){
								multiplier = 1;
							} else {
								multiplier = pow(multiplier, decimalPower);
							}
							//printf("multiplier = %d\n", multiplier); //debug
							defsRemaining += (int)(tempSymValue[i] - '0') * multiplier;
							// printf("defsRemaining = %d\n", defsRemaining); //debug
							decimalPower++;
						}

						if(defsRemaining > DEF_LIMIT){
							__parseerror(lineNum, offset-tempSymValuePos, 4);
							exit(1);
						} else {
							defsRemaining *= 2;
						}

						tempSymValuePos = 0;

						nextType = uses;
					} 
				} else if(nextType == uses) {
					tempSymValue[tempSymValuePos] = curChar;
					tempSymValuePos++;
					
					if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
						int i;
						int decimalPower = 0;
						for(i=tempSymValuePos-1; i >= 0; i--){
							// printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
							int multiplier = 10;
							if(decimalPower == 0){
								multiplier = 1;
							} else {
								multiplier = pow(multiplier, decimalPower);
							}
							//printf("multiplier = %d\n", multiplier); //debug
							usesRemaining += (int)(tempSymValue[i] - '0') * multiplier;
							// printf("defsRemaining = %d\n", defsRemaining); //debug
							decimalPower++;
						}

						// if(usesRemaining > DEF_LIMIT){
						// 	__parseerror(lineNum, offset-tempSymValuePos, 5);
						// 	exit(1);
						// }

						tempSymValuePos = 0;

						if(usesRemaining == 0) {
							nextType = instructions;
						}
					}
				} else if(nextType == instructions) {
					tempSymValue[tempSymValuePos] = curChar;
					tempSymValuePos++;
					
					if(nextChar == ' ' || nextChar == '\t' || nextChar == '\n' || nextChar == EOF){
						int i;
						int decimalPower = 0;
						for(i=tempSymValuePos-1; i >= 0; i--){
							// printf("tempSymValuePos = %d\n", tempSymValuePos); //debug
							int multiplier = 10;
							if(decimalPower == 0){
								multiplier = 1;
							} else {
								multiplier = pow(multiplier, decimalPower);
							}
							//printf("multiplier = %d\n", multiplier); //debug
							instructionsRemaining += (int)(tempSymValue[i] - '0') * multiplier;
							// printf("instructionsRemaining = %d\n", instructionsRemaining); //debug
							decimalPower++;
						}
						nextMemLocation += instructionsRemaining;
						totalInstructions += nextMemLocation;

						if(totalInstructions > machine_size || (instructionsRemaining > machine_size-1)){
							__parseerror(lineNum, offset-tempSymValuePos, 6);
							exit(1);
						} else {
							instructionsRemaining *= 2;
						}

						tempSymValuePos = 0;

						if(instructionsRemaining == 0) {
							nextType = definitions;
						}
					}
				}
			}
		}//end skip whitespace
		
	}//END SECOND PASS

	//check for defined but unused symbols in symbol table
	printf("\n");
	for(i=0; i < st.numSymbols; i++){
		if(!st.symbolL[i].wasUsed){
			printf("Warning: Module %d: ", st.symbolL[i].whichModule);
			int w;
			for(w=0; w < st.symbolL[i].numChar; w++){
				printf("%c", st.symbolL[i].symbolDef[w]);
			}
			printf(" was defined but never used\n");
		}
	}

	printf("\n\n");

	fclose(fp);

	return 0;
}	