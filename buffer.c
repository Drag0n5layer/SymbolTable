/*******************************************************************************************
File Name: buffer.c
Compiler: MS Visual Studio 2010
Author: Kyle Hinskens, 040747198
Course: CST 8152 – Compilers, Lab Section: 401
Assignment: 1
Date: 25 September 2013
Purpose: Has all the function definitions
Function list:  b_create()
				b_addc()
				b_reset()
				b_destroy()
				b_isfull()
				b_getsize()
				b_getcapacity()
				b_setmark()
				b_getmark()
				b_getmode()
				b_load()
				b_isempty()
				b_eob()
				b_getc()
				b_print()
				b_pack()
				b_get_r_flag()
				b_retract()
				b_get_getc_offset()
				b_set_getc_offset()
				b_get_chmemloc()
*********************************************************************************************/
#include "buffer.h"


/*
Purpose: Creates a new Buffer in memory
Author: Kyle Hinskens
History/Versions: 13.09.24
Called functions: calloc(), malloc(), free()
Parameters:
		short init_capacity (0 to SHRT_MAX)
		char inc_factor (0 to 255)
		char o_mode ('f', 'a', 'm')
Return value: A pointer to a Buffer, or NULL on error
Algorithm: Verifies valid parameters and allocates necessary memory
			before returning a pointer to a Buffer.
*/
Buffer *b_create(short init_capacity, char inc_factor, char o_mode) {
	/* Declare local variables */
	Buffer *pBD;
	char *head;

	/* Verify o_mode and init_capacity are valid */
	if (((o_mode != 'f') && (o_mode != 'a') && (o_mode != 'm')) || (init_capacity < 0)) {
		return NULL;
	}
	
	/* Allocate memory for one Buffer */
	pBD = (Buffer *)calloc(1, sizeof(Buffer));
	
	/* Return NULL if the above calloc was not successful */
	if (pBD != NULL) {

		/* Allocate memory for the character array */
		head = (char *)malloc(init_capacity);
		
		/* Return NULL if the above malloc was not successful */
		if (head != NULL) {
			pBD->ca_head = head;
		}
		else {
			free(pBD);
			return NULL;
		}


		pBD->capacity = init_capacity;
		
		//printf("\nINIT_CAPACITY: %d | INC_FACTOR: %c | MODE: %c\n", init_capacity, inc_factor, o_mode); //TODO: REMOVE THIS LINE, IT'S FOR DEBUGGING

		/* Check the mode and inc_factor */
		if ((o_mode == 'f') || (inc_factor == 0)) {
			pBD->mode = 0;
			pBD->inc_factor = 0;
		}
		else if ((o_mode == 'f') && (inc_factor != 0)) {
			pBD->mode = 0;
			pBD->inc_factor = 0;
		}
		else if ((o_mode == 'a') && (inc_factor >= 1)) {
			pBD->mode = 1;
			pBD->inc_factor = inc_factor;
		}
		else if ((o_mode == 'm') && (inc_factor >= 1) && (inc_factor <= 100)) {
			pBD->mode = -1;
			pBD->inc_factor = inc_factor;
		}
		else {
			/* If the conditionals above were not successful, free the allocated memory and return NULL */
			free(pBD->ca_head);
			free(pBD);
			pBD = NULL;
		}
	}
	else {
		return NULL;
	}

	return pBD;
}


/*
Purpose: Adds a character symbol to the character array
Author: Kyle Hinskens
History/Versions: 13.09.24
Called functions: b_isfull(), realloc()
Parameters:
		Buffer * const pBD (Pointer to the Buffer)
		char symbol (The character symbol to add)
Return value: A pointer to a Buffer, or NULL on error
Algorithm: Verifies valid parameters, resizes character array if necessary, and puts the symbol in the array.
			If the memory location of the character array has changed, set the r_flag.
*/
Buffer *b_addc(Buffer * const pBD, char symbol) {
	char *temp_ca_head; /* Temporary character array */

	/* Declare three variables to calculate the new capacity in multiplicative mode */
	short availableSpace;
	float newIncrement;
	short newCapacity;

	if (pBD == NULL) {
		return NULL;
	}

	pBD->r_flag = 0;
	newCapacity = 0;

	/* If the buffer is full, determine the mode and calculate a new capacity */
	if (b_isfull(pBD)) {
		switch (pBD->mode) {
			case 0:
				return NULL;
			case 1:
				/* Calculate the new capacity */
				newCapacity = pBD->capacity + (short)pBD->inc_factor;

				if (newCapacity < 0) {
					return NULL;
				}

				break;
			case -1:

				/* Check if the current capacity can be incremented */
				if (pBD->capacity == SHRT_MAX) {
					return NULL;
				}

				/* Use a float for the division, less error when rounded and cast back to short */
				availableSpace = SHRT_MAX - pBD->capacity;
				newIncrement = (float)(availableSpace * (short)pBD->inc_factor / 100);
				newCapacity = (short)(pBD->capacity + newIncrement);

				/* If the current capacity can be incremented,  */
				if (pBD->capacity < SHRT_MAX && pBD->capacity == newCapacity) {
					newCapacity = SHRT_MAX;
				}

				break;
			default:
				return NULL;
		}

		/* Use realloc to get more memory for the character array, with the newly calculated capacity */
		temp_ca_head = (char *)realloc(pBD->ca_head, newCapacity);

		/* Return NULL if the above realloc failed */
		if (temp_ca_head != NULL) {

			/* Set r_flag if the memory location for ca_head has moved */
			if (temp_ca_head != pBD->ca_head) {
				pBD->r_flag = SET_R_FLAG;
				pBD->ca_head = temp_ca_head;
			}

			pBD->capacity = newCapacity;
		}
		else {
			return NULL;
		}
	}
	
	/* Place the symbol into the character array and increment addc_offset */
	pBD->ca_head[pBD->addc_offset] = symbol;
	++pBD->addc_offset;
	return pBD;
}


/*
Purpose: Re-initialize Buffer data members to make the Buffer appear empty
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: R_FAIL_1 on error, 0 otherwise
*/
int b_reset(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	pBD->eob = 0;
	pBD->addc_offset = 0;
	pBD->getc_offset = 0;
	pBD->mark_offset = 0;

	return 0;
}


/*
Purpose: Frees memory used by the Buffer structure and its content
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: free()
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: None
*/
void b_destroy(Buffer * const pBD) {
	if (pBD != NULL) {
		free(pBD->ca_head);
		free(pBD);
	}
}


/*
Purpose: Determines if the character buffer is full
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value:
			R_FAIL_1 on error
			1 if the buffer is full
			0 otherwise
*/
int b_isfull(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	if (pBD->addc_offset == pBD->capacity) {
		return 1;
	}

	return 0;
}


/*
Purpose: Returns the size of the Buffer
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: short addc_offset, R_FAIL_1 on error
*/
short b_getsize(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->addc_offset;
}


/*
Purpose: Returns the Buffer capacity
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: short capacity, R_FAIL_1 on error
*/
short b_getcapacity(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->capacity;
}


/*
Purpose: Sets the Buffer mark_offset
Author: Kyle Hinskens
History/Versions: 13.09.24
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
			short mark
Return value: int mark or R_FAIL_1 on error
*/
int b_setmark(Buffer * const pBD, short mark) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	if ((mark >= 0) && (mark <= pBD->capacity)) {
		pBD->mark_offset = mark;
		return (int)mark;
	}

	return R_FAIL_1;
}


/*
Purpose: Returns mark_offset to the calling function
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: short mark_offset, R_FAIL_1 on error
*/
short b_getmark(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->mark_offset;
}


/*
Purpose: Returns the Buffer mode
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: int mode, R_FAIL_1 on error
*/
int b_getmode(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->mode;
}


/*
Purpose: Reads a file into the Buffer
Author: Kyle Hinskens
History/Versions: 13.09.23
Called functions: fgetc(), feof(), b_addc(), fclose()
Parameters: FILE * const fi (The file pointer)
			Buffer * const pBD (Pointer to the Buffer)
Return value: 
			R_FAIL_1 or LOAD_FAIL on error
			int count (The number of characters added to the Buffer)
Algorithm: Check for valid parameters, enter infinite loop, get the first character
			from the file, check if its the end of the file, if not then continue.
			Add the character to the array and increase the count. Return the number
			of characters added to the array.
*/
int b_load(FILE * const fi, Buffer * const pBD) {
	char ch; /* Holds the current character from the file */
	int count; /* The number of characters read */

	if (pBD == NULL) {
		return R_FAIL_1;
	}

	count = 0;
	/* This while() will generate a warning */
	while(1) {
		ch = (char)fgetc(fi);

		/* Quit loop if end of file */
		if (feof(fi)) {
			break;
		}

		/* Do not put the EOF character into the array */
		if (ch != EOF) {

			/* If b_addc failed, return error */
			if (b_addc(pBD, ch) == NULL) {
				return LOAD_FAIL;
			}

			++count;
		}
	}

	fclose(fi);

	return count;
}


/*
Purpose: Check if the Buffer is empty
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: 
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: int
				R_FAIL_1 on error
				1 if addc_offset is 0
				0 otherwise
*/
int b_isempty(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	if (pBD->addc_offset == 0) {
		return 1;
	}

	return 0;
}


/*
Purpose: Returns the Buffer eob
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: 
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: int eob, R_FAIL_1 on error
*/
int b_eob(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->eob;
}


/*
Purpose: Gets a character at getc_offset and sets eob
Author: Kyle Hinskens
History/Versions: 13.09.23
Called functions: None
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value:
			R_FAIL_2 if invalid Buffer
			R_FAIL_1 if getc_offset and addc_offset are equal
			char The character
Algorithm: Validates parameters, sets eob if the Buffer is full,
			and returns a character from the array at getc_offset
*/
char b_getc(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_2;
	}
	
	if (pBD->getc_offset == pBD->addc_offset) {
		pBD->eob = 1;
		return R_FAIL_1;
	}
	else {
		pBD->eob = 0;
	}

	return pBD->ca_head[pBD->getc_offset++];
}


/*
Purpose: Prints the content of the Buffer
Author: Kyle Hinskens
History/Versions: 13.09.23
Called functions:
				b_isempty()
				printf()
				b_set_getc_offset()
				b_getc()
				b_eob()
				b_get_getc_offset()
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: int getc_offset (the number of characters printed)
			R_FAIL_1 on error
Algorithm: Verifies the buffer is not empty, prints out each character in
			the buffer array and returns the number of characters printed
*/
int b_print(Buffer * const pBD) {
	char ch;

	if (pBD == NULL) {
		return R_FAIL_1;
	}

	/* Make sure the Buffer is not empty before printing the character array */
	if (b_isempty(pBD)) {
		printf("The buffer is empty.\n");
		return R_FAIL_1;
	}
	else {
		/* Set getc_offset to 0 to start at the beginning of the array, quit on error */
		if (b_set_getc_offset(pBD, 0) == R_FAIL_1) {
			return R_FAIL_1;
		}

		/* This while() will generate a warning */
		while(1) {
			ch = b_getc(pBD);

			/* Keep looping until we are at the end of the buffer */
			if ((b_eob(pBD) == 1)) {
				break;
			}

			printf("%c", ch);
		}

		printf("\n");
		return pBD->getc_offset;
	}
}


/*
Purpose: To increment the capacity by 1
Author: Kyle Hinskens
History/Versions: 13.09.23
Called functions: realloc()
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: NULL on error, otherwise a Buffer pointer
Algorithm: Check for valid parameters, calculate new capacity, 
			realloc to get memory and save
*/
Buffer *b_pack(Buffer * const pBD) {
	short newCapacity; /* Variable to hold new capacity */
	char *temp_ca_head; /* Temporary character array */

	if (pBD == NULL) {
		return NULL;
	}
	
	pBD->r_flag = 0;
	newCapacity = pBD->addc_offset + 1;

	if (newCapacity < SHRT_MAX && newCapacity > 0) {
		/* Use realloc to get more memory for the character array, with the newly calculated capacity */
		temp_ca_head = (char *)realloc(pBD->ca_head, newCapacity);

		/* Return NULL if realloc failed */
		if (temp_ca_head == NULL) {
			return NULL;
		}
		else {
			/* Set r_flag if the memory location for ca_head has moved */
			if (temp_ca_head != pBD->ca_head) {
				pBD->r_flag = SET_R_FLAG;
				pBD->ca_head = temp_ca_head;
			}

			pBD->capacity = newCapacity;
		}
	}

	return pBD;
}


/*
Purpose: Returns the Buffer r_flag
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: 
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: char r_flag, R_FAIL_1 on error
*/
char b_get_r_flag(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->r_flag;
}


/*
Purpose: Decrements getc_offset by 1
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: 
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: R_FAIL_1 on error, 0 otherwise
*/
int b_retract(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	/* getc_offset cannot be less than 0 */
	if (pBD->getc_offset > 0) {
		pBD->getc_offset -= 1;
		return 0;
	}

	return R_FAIL_1;
}


/*
Purpose: Returns getc_offset to the calling function
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: 
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: short getc_offset, R_FAIL_1 on error
*/
short b_get_getc_offset(Buffer * const pBD) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}

	return pBD->getc_offset;
}


/*
Purpose: Sets getc_offset to offset
Author: Kyle Hinskens
History/Versions: 13.09.21
Called functions: 
Parameters: Buffer * const pBD (Pointer to the Buffer)
Return value: R_FAIL_1 on error, 0 if offset was sucessfully set
*/
int b_set_getc_offset(Buffer * const pBD, short offset) {
	if (pBD == NULL) {
		return R_FAIL_1;
	}
	
	/* Make sure offset is within the bounds of the character array */
	if ((offset >= 0) && (offset < pBD->addc_offset)) {
		pBD->getc_offset = offset;
		return 0;
	}
	
	return R_FAIL_1;
}


/*
Purpose: Returns a pointer to the location of a character in the Buffer
Author: Kyle Hinskens
History/Versions: 13.09.24
Called functions: None
Parameters:
			Buffer * const pBD (Pointer to the Buffer)
			short offset (0 to addc_offset)
Return value: char *ch, NULL on error
*/
char *b_get_chmemloc(Buffer * const pBD, short offset) {
	char *ch;
	ch = NULL;

	/* Return NULL if the buffer is NULL */
	if (pBD == NULL) {
		return ch;
	}

	/* Make sure offset is within the bounds of the character array */
	if ((offset >= 0) && (offset < pBD->addc_offset)) {
		ch = &(pBD->ca_head[offset]);
	}

	return ch;
}