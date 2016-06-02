#include <stdio.h>

int main() {
	//get filename
	char filename[255];
	prinf("Enter filename: ");
	scanf("%s", filename);
	
	//read file character by character
	prinf("%s\n", filename); //test

	return 0;
}