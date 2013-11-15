/*******************************************************************************************
File Name: stable.c
Compiler: MS Visual Studio 2010
Author: Kyle Hinskens (040747198), Kwok Hong Kelvin Chan (040742238)
Course: CST 8152 - Compilers, Lab Section: 401
Assignment: 3
Date: 15 November 2013
Purpose: 
*********************************************************************************************/

/* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
* to suppress the warnings about using "unsafe" functions like fopen()
* and standard sting library functions defined in string.h.
* The define does not have any effect in Borland 5.02 projects.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>	/* standard input / output */
#include <stdlib.h> /* standard library functions and constants */
#include <string.h> /* string functions */

/* project header files */
#include "buffer.h"
#include "stable.h"

#define DEFAULT		0xFFF8  /* default status field for STVR, 1111 1111 1111 1000 */
#define INT_MASK	0xFFF2	/* turn default into int status field, 1111 1111 1111 0010 */
#define FLOAT_MASK	0xFFF4	/* turn default into float status field, 1111 1111 1111 0100 */
#define STRING_MASK 0xFFF6	/* turn default into string status field, 1111 1111 1111 0110 */
#define UPDATE_MASK 0xFFF1	/* turn the update flag on, 1111 1111 1111 0001 */
#define TYPE_MASK	0xFFF7	/* check the status field type, 1111 1111 1111 0111 */

extern STD sym_table; /* global symbol table, defined in platy_tt.c */

/*
Purpose: Creates a new (empty) symbol table. It declares a local variable of type
		STD (Symbol Table Descriptor). Then it allocates dynamic memory for an array of
		STVR with st_size number of elements. Next, it creates a self-incrementing buffer
		using the corresponding buffer function and initializes the plsBD pointer.
Author: Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: malloc(), sizeof(), b_create()
Parameters: int st_size The size of the symbol table
Return value: STD sym_table The symbol table
*/
STD st_create(int st_size) {
	STD temp_sym_table; /* local declared symbol table  */
	temp_sym_table.st_size = 0; /* set default symbol table size */
	temp_sym_table.st_offset = 0; /* set default symbol table offset */

	temp_sym_table.pstvr = (STVR *)malloc(sizeof(STVR) * st_size);  /* malloc dynamic memory for the STVR structure */
	temp_sym_table.plsBD = b_create(10, 15, 'm'); /* create Buffer for the symbol table */

	/* check if malloc and the Buffer are created successfully */
	if (temp_sym_table.pstvr != NULL && temp_sym_table.plsBD != NULL) {
		temp_sym_table.st_size = st_size; /* set the symbol table size */
	}

	return temp_sym_table;
}


/*
Purpose: Installs a new entry (VID record) in the symbol table.
Authors: Kwok Hong Kelvin Chan, Kyle Hinskens
Version: 13.11.14
Called functions: st_lookup(), b_setmark(), b_getsize(), strlen(), b_addc(), printf(),
				b_get_r_flag(), b_get_chmemloc(), b_getmark(), st_incoffset()
Parameters:
			STD sym_table The symbol table
			char *lexeme The lexeme
			int line The line number
Return value: int The offset of the lexeme or -1 on error
*/
int st_install(STD sym_table, char *lexeme, int line) {
	int offset; /* lookup() return value*/
	unsigned int i; /* loop counter for strlen */
	int j; /* loop counter */
	unsigned short buffer_offset = 0; /* buffer_offset, used for reorganize plex on the symbol table */

	offset = st_lookup(sym_table, lexeme);

	/* check if the symbol table is full */
	if (sym_table.st_offset == sym_table.st_size) return R_FAIL_1;

	/* lexeme is not found from the symbol table */ 
	if (offset == -1) {
		b_setmark(sym_table.plsBD, b_getsize(sym_table.plsBD)); /* set mark on the Buffer at the addc_offset index */
		/* store lexeme into the symbol table Buffer */
		for (i = 0; i < strlen(lexeme) + 1; ++i) {
			/* check if b_addc() executed successfully, just in case! */
			if (b_addc(sym_table.plsBD, lexeme[i]) == NULL) {
				return R_FAIL_2;
			}

			/* Buffer reallocation, reorganize plex on the symbol table */
			if (b_get_r_flag(sym_table.plsBD)) {
				/* traverse the symbol table, and reassign the correct memory address from the new Buffer */
				 for (j = 0; j < sym_table.st_offset; ++j) {
					sym_table.pstvr[j].plex = b_get_chmemloc(sym_table.plsBD, buffer_offset); /* get the character pointer from the buffer_offset */
					buffer_offset += (unsigned short)strlen(b_get_chmemloc(sym_table.plsBD, buffer_offset)) + 1; /* increment buffer_offset by the strlen from current string  */
				}
			}
		}

		sym_table.pstvr[sym_table.st_offset].plex = b_get_chmemloc(sym_table.plsBD, b_getmark(sym_table.plsBD)); /* set plex as the beginning of the lexeme from the Buffer */
		sym_table.pstvr[sym_table.st_offset].o_line = line; /* set line number */
		sym_table.pstvr[sym_table.st_offset].status_field = DEFAULT; /* set default status field */

		/* variable of type string */ 
		if (lexeme[strlen(lexeme) -1] == '#') {
			sym_table.pstvr[sym_table.st_offset].status_field |= STRING_MASK; /* set status field as string by performing a OR operation with the STRING_MASK */
			sym_table.pstvr[sym_table.st_offset].status_field |= UPDATE_MASK; /* Set update flag */
			sym_table.pstvr[sym_table.st_offset].i_value.str_offset = -1; /* set i_value */
		}
		/* variable of type int */ 
		else if (lexeme[0] == 'i' || lexeme[0] == 'o' || lexeme[0] == 'd' || lexeme[0] == 'n') {
			sym_table.pstvr[sym_table.st_offset].status_field |= INT_MASK; /* set status field as int by performing a OR operation with the INT_MASK*/
			sym_table.pstvr[sym_table.st_offset].i_value.int_val = 0; /* set i_value */
		}
		/* variable of type float */ 
		else {
			sym_table.pstvr[sym_table.st_offset].status_field |= FLOAT_MASK; /* set status field as float by performing a OR operation with the FLOAT_MASK*/
			sym_table.pstvr[sym_table.st_offset].i_value.fpl_val = 0.0f; /* set i_value */
		}

		st_incoffset(); /* increment global symbol table st_offset field */
		return sym_table.st_offset; /* The function returns the current offset of that entry from the beginning of the array of STVR (the array pointed by pstvr). */
	}

	return offset;
}


/*
Purpose: This function searches for a lexeme (variable name) in 
		the symbol table.
Author: Kyle Hinskens
Version: 13.11.14
Called functions: strcmp()
Parameters: 
			STD sym_table The symbol table
			char *lexeme The lexeme
Return value: int The location of the lexeme, or -1 on error
*/
int st_lookup(STD sym_table, char *lexeme) {
	int i; /* symbol table loop counter */

	/* check if the symbol exists and the offset is greater than 0 */
	if (sym_table.st_size > 0 && sym_table.st_offset > 0) {
		
		/* loop through the symbol table */
		for (i = (sym_table.st_offset-1); i >= 0; --i) {
			/* compare to find a matching string */
			if (strcmp(lexeme, sym_table.pstvr[i].plex) == 0) return i;
		}
	}

	return -1;
}


/*
Purpose: The function updates the data type indicator in the variable 
		entry (STVR) specified by vid_offset.
Authors: Kwok Hong Kelvin Chan, Kyle Hinskens
Version: 13.11.14
Called functions: None
Parameters:
			STD sym_table The symbol table
			int vid_offset Which lexeme to check
			char v_type The type of the variable
Return value: int The vid_offset or -1 on error
*/
int st_update_type(STD sym_table, int vid_offset, char v_type) {

	/* check if the symbol table is valid */
	if (sym_table.st_size > 0) {
		/* doing an AND operation to find out if the update flag has been set*/
		if ((sym_table.pstvr[vid_offset].status_field & UPDATE_MASK) != UPDATE_MASK) {
			sym_table.pstvr[vid_offset].status_field &= DEFAULT; /* convert to default mask */

			/* find out the data type and update the status field */
			switch (v_type) {
			case 'I':
				sym_table.pstvr[vid_offset].status_field |= INT_MASK; /* convert it to int status field */
				break;
			case 'F':
				sym_table.pstvr[vid_offset].status_field |= FLOAT_MASK; /* convert it to float status field */
				break;
			}

			sym_table.pstvr[vid_offset].status_field |= UPDATE_MASK; /* turn update flag on */
			return vid_offset;
		}
	}

	return -1;
}


/*
Purpose: The function updates the i_value of the variable specified by vid_offset.
Author: Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: None
Parameters: STD sym_table, int vid_offset, InitialValue i_value
Return value: int The vid_offset or -1 on error
*/
int st_update_value(STD sym_table, int vid_offset, InitialValue i_value) {
	
	/* check if the symbol table is valid */
	if (sym_table.st_size > 0) {
		sym_table.pstvr[vid_offset].i_value = i_value; /* set the i_value */
		return vid_offset;
	}

	return -1;
}

/*
Purpose: The function returns the type of the variable specified by vid_offset.
Author: Kowk Hong Kelvin Chan
Version: 13.11.14
Called functions: None
Parameters: STD sym_table, int vid_offset
Return value: char Either S, I or F
*/
char st_get_type(STD sym_table, int vid_offset) {
	int return_type; /* variable to hold the result from the AND operation with TYPE_MASK  */

	/* check if the symbol table is valid */
	if (sym_table.st_size > 0) {
		return_type = sym_table.pstvr[vid_offset].status_field & TYPE_MASK; /* doing AND operation to find out the variable type */
		
		if (return_type == INT_MASK) return 'I';
		else if (return_type == FLOAT_MASK) return 'F';
		else return 'S';
	}

	return -1;
}



/*
Purpose: Frees the memory occupied by the symbol table 
		dynamic areas and sets st_size to 0.
Author: Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: free(), b_destroy()
Parameters: STD sym_table
Return value: void
*/
void st_destroy(STD sym_table) {
	/* check if the symbol table is valid */
	if (sym_table.st_size > 0) {
		free(sym_table.pstvr); /* free the STVR array allocated memory */
		b_destroy(sym_table.plsBD); /* free the Buffer allocated memory by calling b_destroy() */
		sym_table.st_size = 0; /* reset symbol table st_size */
	}
}


/*
Purpose: Prints the contents of the symbol table to the standard output.
Author: Kyle Hinskens
Version: 13.11.14
Called functions: printf()
Parameters: STD sym_table The symbol table
Return value: int The number of entries printed, or -1 on failure.
*/
int st_print(STD sym_table) {
	int i; /* Loop counter */

	if (sym_table.st_size > 0) {
		printf("\nSymbol Table\n____________\n\nLine Number Variable Identifier\n");

		/* Print the entries in the symbol table */
		for (i = 0; i < sym_table.st_offset; ++i) {
			printf("%2i          %s\n", sym_table.pstvr[i].o_line, sym_table.pstvr[i].plex);
		}

		return i; /* Return the number of entries printed */
	}

	return -1;
}


/*
Purpose: Sets st_size to 0.
Author: Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: None
Parameters: void
Return value: void
*/
static void st_setsize(void) {
	sym_table.st_size = 0;
}


/*
Purpose: Increments st_offset by 1.
Author: Kyle Hinskens
Version: 13.11.14
Called functions: None
Parameters: void
Return value: void
*/
static void st_incoffset(void) {
	sym_table.st_offset++;
}


/*
Purpose: Stores the symbol table into a file named $stable.ste
Author: Kyle Hinskens
Version: 13.11.14
Called functions: fopen(), fprintf(), fclose(), printf()
Parameters: STD sym_table The symbol table
Return value: int The number of records stored, or -1 on failure.
*/
int st_store(STD sym_table) {
	FILE *fp; /* File pointer */
	int i; /* Loop counter */
	char *fname = "$stable.ste"; /* Define a file name */

	/* Open a text file for writing ("wt") and check for sym_table validity */ 
	if ((fp = fopen(fname,"wt")) == NULL || sym_table.st_size <= 0) {
		return -1;
	}

	/* Put the symbol table size into the file */
	fprintf(fp, "%d", sym_table.st_size);

	/* Traverse the entire symbol table and put data into the file */
	for (i = 0; i < sym_table.st_offset; ++i) {
		fprintf(fp, " %X", sym_table.pstvr[i].status_field);
		fprintf(fp, " %i", strlen(sym_table.pstvr[i].plex));
		fprintf(fp, " %s", sym_table.pstvr[i].plex);
		fprintf(fp, " %i", sym_table.pstvr[i].o_line);

		/* Determine the type and print accordingly */
		switch(st_get_type(sym_table, i)) {
		case 'I':
			fprintf(fp, " %i", sym_table.pstvr[i].i_value.int_val);
			break;
		case 'F':
			fprintf(fp, " %.2f", sym_table.pstvr[i].i_value.fpl_val);
			break;
		case 'S':
			fprintf(fp, " %i", sym_table.pstvr[i].i_value.str_offset);
			break;
		default:
			return -1;
		}
	}

	fclose(fp); /* Close the file */
	printf("\nSymbol Table stored.\n");

	return i; /* Return the number of entries in the table */
}


/*
Purpose: Sorts the array of symbol table entries by variable 
		name in ascending or descending order.
Authors: Kyle Hinskens, Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: qsort(), compareAscending(), compareDescending()
Parameters: 
			STD sym_table The symbol table
			char s_order The sort order (A or D)
Return value: int On success return 1, otherwise return –1
*/
int st_sort(STD sym_table, char s_order) {

	if (sym_table.st_size > 0) {
		/* Determine the sort order */
		switch(s_order) {
		case 'A':
			qsort(sym_table.pstvr, sym_table.st_offset, sizeof(STVR), compareAscending);
			return 1;
		case 'D':
			qsort(sym_table.pstvr, sym_table.st_offset, sizeof(STVR), compareDescending);
			return 1;
		default:
			return -1;
		}
	}

	return -1;
}


/*
Purpose: Helper function for qsort in ascending order
Authors: Kyle Hinskens, Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: strcmp()
Parameters: void * ptr0, void * ptr0
Return value: int Strings are equal if 0,
			positive indicates first string is bigger,
			negative indicates second string is bigger
*/
static int compareAscending(const void *ptr0, const void *ptr1) {
	return strcmp(((STVR *)ptr0)->plex, ((STVR *)ptr1)->plex);
}


/*
Purpose: Helper function for qsort in descending order
Authors: Kyle Hinskens, Kwok Hong Kelvin Chan
Version: 13.11.14
Called functions: strcmp()
Parameters: void * ptr0, void * ptr0
Return value: int Strings are equal if 0,
			positive indicates first string is bigger,
			negative indicates second string is bigger
*/
static int compareDescending(const void *ptr0, const void *ptr1) {
	return strcmp(((STVR *)ptr1)->plex, ((STVR *)ptr0)->plex);
}