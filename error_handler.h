void __parseerror(int linenum, int lineoffset, int errcode) {
	static char* errstr[] = {
		"NUM_EXPECTED", // Number expect 0
		"SYM_EXPECTED", // Symbol Expected 1 
		"ADDR_EXPECTED", // Addressing Expected 2
		"SYM_TOLONG", // Symbol Name is to long 
		"TO_MANY_DEF_IN_MODULE", // > 16 
		"TO_MANY_USE_IN_MODULE", // > 16 
		"TO_MANY_INSTR" // total num_instr exceeds memory size (512) 
	};
	printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
} 