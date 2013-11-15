/*******************************************************************************************
File Name: table.h
Compiler: MS Visual Studio 2010
Author: Kyle Hinskens (040747198), Kwok Hong Kelvin Chan (040742238)
Course: CST 8152 - Compilers, Lab Section: 401
Assignment: 2
Date: 25 October 2013
Purpose: Transition Table and function declarations necessary for the scanner implementation
*********************************************************************************************/

#ifndef  TABLE_H_
#define  TABLE_H_ 

#ifndef BUFFER_H_
#include "buffer.h"
#endif

#ifndef NULL
#include <_null.h> /* NULL pointer constant is defined there */
#endif

#define SEOF '\0' /* End-Of-File symbol */
#define SEOF_HEX 0xFF /* End-Of-File symbol */
#define INC_FACTOR 15 /* Increment factor for buffer */
#define TWO_BYTE_INT_MAX 32767 /* 2-byte int max in c */
#define ES  12 /* Error state */
#define IS  -1 /* Inavalid state */

/* State transition table definition */  
#define TABLE_COLUMNS 7

/*transition table - type of states defined in separate table */
int  st_table[ ][TABLE_COLUMNS] = {
/* State 0 */  {1, 6, 4, 4, IS, IS, IS},
/* State 1 */  {1, 1, 1, 1, 2, 3, 2},
/* State 2 */  {IS, IS, IS, IS, IS, IS, IS},
/* State 3 */  {IS, IS, IS, IS, IS, IS, IS},
/* State 4 */  {ES, 4, 4, 4, 7, 5, 5},
/* State 5 */  {IS, IS, IS, IS, IS, IS, IS},
/* State 6 */  {ES, 10, 9, ES, 7, ES, 5},
/* State 7 */  {8, 7, 7, 7, 8, 8, 8},
/* State 8 */  {IS, IS, IS, IS, IS, IS, IS},
/* State 9 */  {ES, 9, 9, ES, ES, ES, 11},
/* State 10 */ {ES, ES, ES, ES, ES, ES, 11},
/* State 11 */ {IS, IS, IS, IS, IS, IS, IS},
/* State 12 */ {IS, IS, IS, IS, IS, IS, IS}
};

/* Accepting state table definition */
#define ASWR     1  /* accepting state with retract */
#define ASNR     2  /* accepting state with no retract */
#define NOAS     0  /* not accepting state */

int as_table[ ] = {NOAS, NOAS, ASWR, ASNR, NOAS, ASWR, NOAS, NOAS, ASWR, NOAS, NOAS, ASWR, ASNR};

/* Accepting action function declarations */
Token aa_func02(char *lexeme);
Token aa_func03(char *lexeme);
Token aa_func05(char *lexeme);
Token aa_func08(char *lexeme);
Token aa_func11(char *lexeme);
Token aa_func12(char *lexeme);

/* defining a new type: pointer to function (of one char * argument) returning Token */  
typedef Token (*PTR_AAF)(char *lexeme);

/* Accepting function (action) callback table (array) definition */
PTR_AAF aa_table[ ] ={NULL, NULL, aa_func02, aa_func03, NULL, aa_func05, NULL, NULL, aa_func08, NULL, NULL, aa_func11, aa_func12};

/* Keyword lookup table (.AND. and .OR. are not keywords) */
#define KWT_SIZE  8

char * kw_table []= {
    "ELSE",
    "IF",
    "INPUT",
    "OUTPUT",
    "PLATYPUS",
    "REPEAT",
    "THEN",
    "USING"   
};

#endif
