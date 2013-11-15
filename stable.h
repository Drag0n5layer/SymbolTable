/*******************************************************************************************
File Name: stable.h
Compiler: MS Visual Studio 2010
Author: Kyle Hinskens (040747198), Kwok Hong Kelvin Chan (040742238)
Course: CST 8152 - Compilers, Lab Section: 401
Assignment: 3
Date: 15 November 2013
Purpose: 
*********************************************************************************************/

#ifndef STABLE_H_
#define STABLE_H_

typedef union InitialValue {
	int int_val; /* integer variable initial value */
	float fpl_val; /* floating-point variable initial value */
	int str_offset; /* string variable initial value (offset) */
} InitialValue;

typedef struct SymbolTableVidRecord {
	unsigned short status_field; /* variable record status field*/
	char * plex; /* pointer to lexeme (VID name) in CA */
	int o_line; /* line of first occurrence */
	InitialValue i_value; /* variable initial value */
	size_t ds_offset;/*offset from the beginning of data segment*/
}STVR;

typedef struct SymbolTableDescriptor {
	STVR *pstvr; /* pointer to array of STVR */
	int st_size; /* size in number of STVR elements */
	int st_offset; /*offset in number of STVR elements */
	Buffer *plsBD; /* pointer to the lexeme storage buffer descriptor */
} STD;


/* Function prototypes */
STD st_create(int);
int st_install(STD, char *, int);
int st_lookup(STD, char *);
int st_update_type(STD,int, char);
int st_update_value(STD, int, InitialValue);
void st_destroy(STD);
int st_print(STD);
static void st_setsize(void);
static void st_incoffset(void);
int st_store(STD);
int st_sort(STD, char);
static int compareAscending(const void *, const void *);
static int compareDescending(const void *, const void *);

#endif
