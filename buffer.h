/*******************************************************************************************
File Name: buffer.h
Compiler: MS Visual Studio 2010
Author: Kyle Hinskens, 040747198
Course: CST 8152 – Compilers, Lab Section: 401
Assignment: 1
Date: 25 September 2013
Purpose: Has all the macro and function declarations
*/
#ifndef BUFFER_H_
#define BUFFER_H_

/*#pragma warning(1:4001)*//*to enforce C89 type comments  - to make //comments an warning */
/*#pragma warning(error:4001)*//* to enforce C89 comments - to make // comments an error */

/* standard header files */
#include <stdio.h>  /* standard input/output */
#include <malloc.h> /* for dynamic memory allocation*/
#include <limits.h> /* implementation-defined data type ranges and limits */

/* constant definitions */
/* You may add your own constant definitions here */
#define R_FAIL_1 -1         /* fail return value */
#define R_FAIL_2 -2         /* fail return value */
#define LOAD_FAIL -2       /* load fail error */
#define SET_R_FLAG 1       /* realloc flag set value */


/* user data type declarations */
typedef struct BufferDescriptor {
    char *ca_head;   /* pointer to the beginning of character array (character buffer) */
    short capacity;    /* current dynamic memory size (in bytes) allocated to character buffer */
    short addc_offset;  /* the offset (in chars) to the add-character location */
    short getc_offset;  /* the offset (in chars) to the get-character location */
	short mark_offset; /* the offset (in chars) to the mark location */
	char  inc_factor; /* character array increment factor */
	char  r_flag;     /* reallocation flag */
    char  mode;       /* operational mode indicator*/
	int   eob;       /* end-of-buffer flag */
} Buffer;


/* function declarations */
Buffer *b_create(short init_capacity, char inc_factor, char o_mode);
Buffer *b_addc(Buffer * const pBD, char symbol);
int b_reset(Buffer * const pBD);
void b_destroy(Buffer * const pBD);
int b_isfull(Buffer * const pBD);
short b_getsize(Buffer * const pBD);
short b_getcapacity(Buffer * const pBD);
int b_setmark(Buffer * const pBD, short mark);
short b_getmark(Buffer * const pBD);
int b_getmode(Buffer * const pBD);
int b_load(FILE * const fi, Buffer * const pBD);
int b_isempty(Buffer * const pBD);
int b_eob(Buffer * const pBD);
char b_getc(Buffer * const pBD);
int b_print(Buffer * const pBD);
Buffer *b_pack(Buffer * const pBD);
char b_get_r_flag(Buffer * const pBD);
int b_retract(Buffer * const pBD);
short b_get_getc_offset(Buffer * const pBD);
int b_set_getc_offset(Buffer * const pBD, short offset);
char *b_get_chmemloc(Buffer * const pBD, short offset);

#endif
