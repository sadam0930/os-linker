simple-linker
-------------

# To compile:
`gcc -lm -o linker linker.c`

# To run:
`./linker filepath`

Dear Reader,

This code is not the best example of my work, but I am new to C and ended up trying to plow through parsing each file. I would have benefitted from creating some reuseable functions. I apologize for the spaghetti code below.

Synopsis:  
Perform two passes (while loops) over a given input file. Both loops contain logic to account for where the pointer is in the file in reference to
* definition list
* use list
* instruction list (or sometimes called program text)