/*******************************************************************************************
File Name: scanner.c
Compiler: MS Visual Studio 2010
Author: Kyle Hinskens (040747198), Kwok Hong Kelvin Chan (040742238)
Course: CST 8152 - Compilers, Lab Section: 401
Assignment: 2
Date: 25 October 2013
Purpose: Functions implementing a Lexical Analyzer (Scanner). scanner_init() must be called 
before using the scanner.
*********************************************************************************************/

/* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
* to suppress the warnings about using "unsafe" functions like fopen()
* and standard sting library functions defined in string.h.
* The define does not have any effect in Borland 5.02 projects.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>	/* standard input / output */
#include <ctype.h>	/* conversion functions */
#include <stdlib.h>	/* standard library functions and constants */
#include <string.h>	/* string functions */
#include <limits.h>	/* integer types constants */
#include <float.h>	/* floating-point types constants */
#include <math.h>	/* math functions, like pow() */

/* #define NDEBUG to suppress assert() call */
#include <assert.h>  /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "table.h"
#include "stable.h"

#define DEBUG  /* for conditional processing */
#undef DEBUG

/* Global objects - variables */
/* This buffer is used as a repository for string literals. It is defined in platy_st.c */
extern Buffer * str_LTBL; /* String literal table */
int line; /* current line number of the source code */
extern int scerrnum; /* defined in platy_st.c - run-time error number */
extern STD sym_table; /* defined in platy_tt.c - symbol table */

/* Local(file) global objects - variables */
static Buffer *lex_buf;/*pointer to temporary lexeme buffer*/

/* No other global variable declarations/definitiond are allowed */

/* scanner.c static(local) function  prototypes */ 
static int char_class(char c); /* character class function */
static int get_next_state(int, char, int *); /* state machine function */
static int isKeyword(char * kw_lexeme); /* keywords lookup functuion */
static long atool(char * lexeme); /* converts octal string to decimal value */
Token runtimeError(); /* handle a potential runtime error */


/*
Purpose: Initialize the Lexical Analyzer
Author: Svillen Ranev
Version: 1
Called functions: b_isempty(), b_set_getc_offset(), b_reset()
Parameters: Buffer * sc_buf
Return value: int
*/
int scanner_init(Buffer * sc_buf) {
	if (b_isempty(sc_buf)) return EXIT_FAILURE; /*1*/
	b_set_getc_offset(sc_buf,0); /* incase the buffer has been read previously  */
	b_reset(str_LTBL);
	line = 1;
	return EXIT_SUCCESS; /*0*/
}


/*********************************************************************
*	Purpose: Read the next character and begin processing the special characters that are defined 
by the grammar. When a digit or an alpha character is found, the state machine starts,
and the output state determine which accepting function the character goes to. Finally,
if the character is not recognized, the error token is set.

*	Author: Kwok Hong Kelvin Chan / Kyle Hinskens
*	History/Versions: 1.0, October 25th, 2013
*	Called functions: 
memset()
b_getc()
b_addc()
b_retract()
b_getmark()
b_setmark()
b_get_getc_offset()
b_set_getc_offset()
b_getsize()
b_getcapacity()
b_destroy()
runtimeError()

*	Parameters: 
Type:		parameter:
Buffer 		sc_buf 

*	Return value: 
type:		value:
Token 		see token.h to see all possible types of token

*	Algorithm: 		The input parameter provides a pointer to a buffer which holds the input file characters.
The function begin with parsing and filtering the special characters and 
set the corresponding tokens. Afterwards, the characters that are known to 
be a digital or an alpha are then process through the finite state machine to determine which state
it belongs to, and finally the accepting functions are called. When a unrecognizable character is found,
the function will set a error token with the error token attribute to be the character itself. 
*********************************************************************/
Token mlwpar_next_token(Buffer * sc_buf) {
	Token t;			/* token to return after recognition */
	unsigned char c;	/* input symbol */
	int state = 0;		/* initial state of the FSM */
	short lexstart;		/* start offset of a lexeme in the input buffer */
	short lexend;		/* end offset of a lexeme in the input buffer */
	int accept = NOAS;	/* type of state - initially not accepting */
	int i;				/* Loop counter */
	int lexLength;		/* lexeme length */

	/* endless loop broken by token returns it will generate a warning */
	while(1) {
		memset(t.attribute.err_lex, '\0', ERR_LEN + 1);	/* memset err_lex char array with \0 to cleanup initial values */
		c = b_getc(sc_buf); /* get next character */

		/* special cases or token driven processing */
		switch(c) {
			/* check for empty space, and tab */
		case ' ':
		case '\t':
			continue;

			/* check for comment token */
		case '!':
			b_setmark(sc_buf, b_get_getc_offset(sc_buf) - 1); /* set mark at the first character including ! */
			c = b_getc(sc_buf); /* get next character */

			if (c == '<') {
				/* valid comment, check if it reaches the end of line */
				while(c != '\n') {
					c = b_getc(sc_buf); /* get next character */
				}

				++line; /* Need to increment the line number */
				continue;
			}
			/* check for relational operator token */
			else if (c == '=') {
				t.code = REL_OP_T; /* set relational operator token */
				t.attribute.rel_op = NE; /* set relational operator token attribute */
				return t;
			}
			else {
				/* continue until it reaches new line, neither a valid comment or a relational token is found */  
				while (c != '\n') {
					c = b_getc(sc_buf); /* get next character */
				}

				++line; /* Need to increment the line number */

				lexend = b_get_getc_offset(sc_buf); /* set mark at the end character */
				b_set_getc_offset(sc_buf, b_getmark(sc_buf)); /* set getc_offset back to the beginning of character  */
				t.code = ERR_T; /* set error token */
				t.attribute.err_lex[0] = b_getc(sc_buf); /* set error token attribute */
				t.attribute.err_lex[1] = b_getc(sc_buf); /* set error token attribute */
				b_set_getc_offset(sc_buf, lexend); /* set getc_offset to the end of character */

				return t;
			}

			/* check for source end of file */
		case SEOF:
		case SEOF_HEX:
			t.code = SEOF_T; /* set source end-of-file token */
			return t;

			/* check for string concatenation operator token */
		case '<':
			c = b_getc(sc_buf); /* get next character */
			if (c == '>'){
				t.code = SCC_OP_T; /* set string concatenation operator token */
			}
			/* check for relational operator token */
			else {
				b_retract(sc_buf); /* retract to the relational operator symbol */
				t.code = REL_OP_T; /* set relational operator token */
				t.attribute.rel_op = LT; /* set relational operator token attribute */
			}
			return t;

			/* check for relational operator token */
		case '>':
			t.code = REL_OP_T; /* set relational operator token */
			t.attribute.rel_op = GT; /* set relational operator token attribute */
			return t;

			/* check for assignment operator token */
		case '=':
			c = b_getc(sc_buf); /* get next character */
			/* check for relational operator token */
			if (c == '='){	
				t.code = REL_OP_T; /* set relational operator token */
				t.attribute.rel_op = EQ; /* set relational operator token attribute */
			}
			else {
				b_retract(sc_buf); /* retract to the assignment operator symbol */
				t.code = ASS_OP_T; /* set assignment operator token */
			}
			return t;

			/* check for arithmetic variable identifier token */
		case '+':
			t.code = ART_OP_T; 
			t.attribute.arr_op = PLUS;
			return t;
		case '-':
			t.code = ART_OP_T; 
			t.attribute.arr_op = MINUS;
			return t;
		case '*':
			t.code = ART_OP_T; 
			t.attribute.arr_op = MULT;
			return t;
		case '/':
			t.code = ART_OP_T; 
			t.attribute.arr_op = DIV;
			return t; 

			/* check for logical operator token */
		case '.':
			b_setmark(sc_buf, b_get_getc_offset(sc_buf) - 1); /* set mark to the current getc_offset -1 */
			c = b_getc(sc_buf);  /* get next character */

			/* switch case to check for logical operator token */
			switch(c) {
			case 'A':
				c = b_getc(sc_buf);

				if (c == 'N') {
					c = b_getc(sc_buf);

					if (c == 'D') {
						c = b_getc(sc_buf);

						/* a valid logical operator */
						if (c == '.') {
							t.code = LOG_OP_T; /* set logical operator token */
							t.attribute.log_op = AND; /* set logical operator attribute */
							return t;
						}
					}
				}
			case 'O':
				c = b_getc(sc_buf);

				if (c == 'R') {
					c = b_getc(sc_buf);

					/* a valid logical operator */
					if (c == '.') {
						t.code = LOG_OP_T; /* set logical operator token */
						t.attribute.log_op = OR; /* set logical operator attribute */
						return t;
					}
				}
			}

			/* a non-valid logical operator */
			b_set_getc_offset(sc_buf, b_getmark(sc_buf)); /* set getc_offset back to the first character */
			t.code = ERR_T; /* set error token */
			t.attribute.err_lex[0] = b_getc(sc_buf); /* set error token attribute */
			return t;

			/* check for left parenthesis token */
		case '(':
			t.code = LPR_T; /* set left parenthesis token */
			return t;

			/* check for right parenthesis token */
		case ')':
			t.code = RPR_T; /* set right parenthesis token */
			return t;

			/* check for left brace token */
		case '{':
			t.code = LBR_T; /* set left brace token */
			return t;

			/* check for right brace token */
		case '}':
			t.code = RBR_T; /* set right brace token */
			return t;

			/* check for string literal token */
		case '"':
			lexstart = b_get_getc_offset(sc_buf); /* set lexstart to the first character */
			b_setmark(sc_buf, lexstart); /* set mark to the first character */

			/* endless loop that will generate a warning */
			while(1) {
				c = b_getc(sc_buf); /* get next character */

				if (c == '\n') {
					++line; /* increment line number when it reaches new line symbol */
				}

				/* it reaches to the end of file before finding the closing ", invalid string " */
				if (c == SEOF || c == SEOF_HEX) {
					lexend = b_get_getc_offset(sc_buf) - 1; /* set lexend to the end character */
					t.code = ERR_T; /* set error token */
					b_set_getc_offset(sc_buf, lexstart - 1); /* set getc_offset to the first character including " */
					lexLength = lexend - lexstart; /* calculate the lexeme length */
					if (lexLength > ERR_LEN) lexLength = ERR_LEN; /* set the correct lexeme length if it is great than ERR_LEN */
					i = 0; /* initialize loop counter */

					/* while i is less than lexLength, copy characters into err_lex character array */
					while (i < lexLength) {
						c = b_getc(sc_buf);
						/* set the first 17 character into the err_lex array regardless */
						if (i < 17) {
							t.attribute.err_lex[i] = c; 
						}
						else {
							/* if the invalid string is greater than ERR_LEN, replace the last 3 character to . */
							if ((lexend - lexstart) > ERR_LEN) {
								t.attribute.err_lex[i] = '.';
							}
							else {
								t.attribute.err_lex[i] = c; /* normal assigning character into err_lex array */
							}
						}
						++i;
					}
					b_set_getc_offset(sc_buf, lexend); /* set the getc_offset back to the end of character */
					return t;
				}

				/* if it finds the ending ", it is a valid string */
				if (c == '"') {
					b_set_getc_offset(sc_buf, b_getmark(sc_buf)); /* set getc_offset back to first character */
					t.attribute.str_offset = b_getsize(str_LTBL); /* set string literal token attribute to the current str_LTBL buffer size */

					c = b_getc(sc_buf);
					/* while loop to assign characters into str_LTBL buffer */
					while(c != '"') {
						/* if b_addc() fail, runtime error occured */
						if (b_addc(str_LTBL, c) == NULL) {
							t = runtimeError();
							return t;
						}

						c = b_getc(sc_buf);
					}

					/* if b_addc() fail, runtime error occured */
					if (b_addc(str_LTBL, '\0') == NULL) {
						t = runtimeError();
						return t;
					}

					t.code = STR_T; /* set string literal token */
					return t;
				}
			}

			/* check for comma token */
		case ',':
			t.code = COM_T;
			return t;

			/* check for end of statement token */
		case ';':
			t.code = EOS_T;
			return t;

			/* increment line number when it reaches to new line */
		case '\n':
			++line;
			continue;
		}


		/* Process state transition table */
		if (isdigit(c) || isalpha(c)) {
			b_setmark(sc_buf, b_get_getc_offset(sc_buf) - 1); /* Set the mark at the beginning of the lexeme */

			/* The finite state machine, produces a warning */
			while (1) {
				state = get_next_state(state, c, &accept);
				if (accept != NOAS) { break; }
				c = b_getc(sc_buf);
			}

			lexstart = b_getmark(sc_buf); /* Set the start of the lexeme */
			lex_buf = b_create(b_getcapacity(sc_buf), INC_FACTOR, 'f'); /* Create a temporary buffer */

			/* Generate a runtime error if the lex_buf wasn't created */
			if (lex_buf == NULL) {
				t = runtimeError();
				return t;
			}

			/* Retract getc_offset if the final state is a retracting state */
			if (accept == ASWR) {
				b_retract(sc_buf);
			}

			lexend = b_get_getc_offset(sc_buf); /* Set lexend to getc_offset */
			b_set_getc_offset(sc_buf, lexstart);

			/* use memset to initialize the ca_head to \0 so that theres no garbage characters */
			memset(lex_buf->ca_head, '\0', b_getcapacity(lex_buf));

			/* Copy the lexeme between lexstart and lexend from the input buffer into lex_buf */
			for (i = lexstart; i < lexend; i++) {
				c = b_getc(sc_buf);

				/* Make sure b_addc() worked, if not return a runtime error */
				if (b_addc(lex_buf, c) == NULL) {
					t = runtimeError();
					return t;
				}
			}

			/* Call the accepting action function and pass the lexeme to it */
			t = aa_table[state](b_get_chmemloc(lex_buf, b_getmark(lex_buf)));

			b_destroy(lex_buf);
			return t;
		}
		else {
			/* Anything other than a digit or letter; set an error token */
			t.code = ERR_T;
			t.attribute.err_lex[0] = c;

			return t;
		}
	} /* end while(1) */
}


/*
Purpose: Get the next state
Author: Svillen Ranev
Version: 1
Called functions: assert(), printf()
Parameters: Buffer * sc_buf
Return value:
int state The current state
char c The character
int *accept The next state
*/
int get_next_state(int state, char c, int *accept) {
	int col;
	int next;
	col = char_class(c);
	next = st_table[state][col];

#ifdef DEBUG
	printf("Input symbol: %c Row: %d Column: %d Next: %d \n",c,state,col,next);
#endif

	/*
	The assert(int test) macro can be used to add run-time diagnostic to programs
	and to "defend" from producing unexpected results.
	assert() is a macro that expands to an if statement;
	if test evaluates to false (zero) , assert aborts the program
	(by calling abort()) and sends the following message on stderr:

	Assertion failed: test, file filename, line linenum

	The filename and linenum listed in the message are the source file name
	and line number where the assert macro appears.
	If you place the #define NDEBUG directive ("no debugging")
	in the source code before the #include <assert.h> directive,
	the effect is to comment out the assert statement.
	*/

	assert(next != IS);

	/*
	The other way to include diagnostics in a program is to use
	conditional preprocessing as shown bellow. It allows the programmer
	to send more details describing the run-time problem. 
	Once the program is tested thoroughly #define DEBUG is commented out
	or #undef DEBUF is used - see the top of the file.
	*/ 
#ifdef DEBUG
	if(next == IS){
		printf("Scanner Error: Illegal state:\n");
		printf("Input symbol: %c Row: %d Column: %d\n",c,state,col);
		exit(1);
	}
#endif

	*accept = as_table[next];
	return next;
}


/*
Purpose: Get the column number in the transition table
for the given character c
Author: Kwok Hong Kelvin Chan
History/Versions: 13.10.22
Called functions: None
Parameters: char c
Return value: int The column number in the transition table
*/
int char_class(char c) {
	int val;

	if (isalpha(c)) {
		val = 0;
	}
	else if (c == '0') {
		val = 1;
	}
	else if (c >= '1' && c <= '7') {
		val = 2;
	}
	else if (c == '8' || c == '9') {
		val = 3;
	}
	else if (c == '.') {
		val = 4;
	}
	else if (c == '#') {
		val = 5;
	}
	else {
		val = 6;
	}

	return val;
}


/*
Purpose: Accepting function for arithmetic variable (AVID) and keywords
Author: Kyle Hinskens
History/Versions: 13.11.14
Called functions: isKeyword()
Parameters: char lexeme[]
Return value: Token
Algorithm: Check if the lexeme is a keyword, if yes set the token code
and keyword attribute to its respective index. If not a
keyword, set an AVID token code, put the lexeme into
vid_lex with at most VID_LEN characters and append \0
*/
Token aa_func02(char lexeme[]) {
	Token t;		/* The Token to return */
	int keyword;	/* To hold the keyword index */
	int r_install;	/* Return value of st_install */

	keyword = isKeyword(lexeme);

	/* Check if the lexeme is a Keyword */
	if (keyword != R_FAIL_1) {
		t.code = KW_T;
		t.attribute.kwt_idx = keyword;
	}
	else {
		/* It wasn't a keyword, so we have an AVID */
		t.code = AVID_T; /* Set the token attribute */

		/* Truncate the lexeme to VID_LEN */
		if (strlen(lexeme) > VID_LEN) {
			lexeme[VID_LEN] = '\0';
		}

		r_install = st_install(sym_table, lexeme, line); /* Install the symbol into the table */

		/* Check if the symbol table is full */
		if (r_install == R_FAIL_1) {
			printf("\nError: The Symbol Table is full - install failed.\n");
			st_store(sym_table);
			exit(1);
		}
		else if (r_install == R_FAIL_2) { /* If b_addc failed generate a runtime error to increase scerrnum */
			runtimeError();
		}
	}

	return t;
}


/*
Purpose: Accepting function for string variable (SVID)
Author: Kyle Hinskens
History/Versions: 13.11.14
Called functions: strlen()
Parameters: char lexeme[]
Return value: Token
Algorithm: Set token code puts the first VID_LEN
characters into vid_lex. Appends a #
before the \0 if the lexeme length
was longer than VID_LEN
*/
Token aa_func03(char lexeme[]) {
	Token t;		/* The Token to return */
	int r_install;	/* Return value of st_install */

	t.code = SVID_T; /* Set the token attribute */

	/* Truncate the lexeme to VID_LEN */
	if (strlen(lexeme) > VID_LEN) {
		lexeme[VID_LEN - 1] = '#';
		lexeme[VID_LEN] = '\0';
	}

	r_install = st_install(sym_table, lexeme, line); /* Install the symbol into the table */

	/* Check if the symbol table is full */
	if (r_install == R_FAIL_1) {
		printf("\nError: The Symbol Table is full - install failed.\n");
		st_store(sym_table);
		exit(1);
	}
	else if (r_install == R_FAIL_2) { /* If b_addc failed generate a runtime error to increase scerrnum */
		runtimeError();
	}

	return t;
}


/*
Purpose: Accepting function for decimal constant (DIL)
Author: Kwok Hong Kelvin Chan
History/Versions: 13.10.23
Called functions: atoi(), aa_func12()
Parameters: char lexeme[]
Return value: Token
Algorithm: Set token code and convert lexeme to long. Verify
the range and set the int_value of the token to
return
*/
Token aa_func05(char lexeme[]) {
	Token t;	/* The Token to return */
	long temp;	/* Temporary variable, larger than an integer */

	t.code = INL_T; /* Set the token attribute */
	temp = atoi(lexeme);

	/* If integer out of range, return error token */
	if (temp > TWO_BYTE_INT_MAX) {
		t = aa_func12(lexeme);
		return t;
	}

	t.attribute.int_value = (int)temp;

	return t;
}


/*
Purpose: Accepting function for floating point literal (FPL)
Author: Kwok Hong Kelvin Chan
History/Versions: 13.10.23
Called functions: atof(), aa_func12()
Parameters: char lexeme[]
Return value: Token
Algorithm: Set token code and convert lexeme to a double.
Verify it's within bounds and set the float
value of the token
*/
Token aa_func08(char lexeme[]) {
	Token t;		/* The Token to return */
	double temp;	/* Temporary variable, larger than an integer */

	t.code = FPL_T; /* Set the token attribute */
	temp = atof(lexeme);

	/* If float out of range, return error token */
	if ((temp > FLT_MAX || temp < FLT_MIN) && temp != 0.0) {
		t = aa_func12(lexeme);
		return t;
	}

	t.attribute.flt_value = (float)temp;

	return t;
}


/*
Purpose: Accepting function for octal constant (OIL)
Author: Kyle Hinskens
History/Versions: 13.10.23
Called functions: atool(), aa_func12()
Parameters: char lexeme[]
Return value: Token
Algorithm: Set token code and convert lexeme from octal string
to long int. Verify within bounds and assign temp
to the token's int_value
*/
Token aa_func11(char lexeme[]) {
	Token t;	/* The Token to return */
	long temp;	/* Temporary variable, larger than an integer */

	t.code = INL_T; /* Set the token attribute */
	temp = atool(lexeme);

	/* If integer out of range, return error token */
	if (temp > TWO_BYTE_INT_MAX) {
		t = aa_func12(lexeme);
		return t;
	}

	t.attribute.int_value = (int)temp;

	return t;
}


/*
Purpose: Set the error token
Author: Kyle Hinskens
History/Versions: 13.10.22
Called functions: memset()
Parameters: char lexeme[]
Return value: Token
Algorithm: Sets the error token and puts the first ERR_LEN
characters into err_lex
*/
Token aa_func12(char lexeme[]) {
	Token t;		/* The Token to return */
	int i;			/* Loop counter */
	int lexLength;	/* Length of the lexeme */

	t.code = ERR_T; /* Set the token attribute */

	lexLength = strlen(lexeme);
	if (lexLength > ERR_LEN) lexLength = ERR_LEN; /* Only put ERR_LEN characters into the err_lex */

	/* Use memset to initialize the err_lex to \0 so that there are no garbage characters */
	memset(t.attribute.err_lex, '\0', ERR_LEN + 1);

	/* Copy lexeme into err_lex */
	for (i = 0; i < lexLength; i++) {
		t.attribute.err_lex[i]= lexeme[i];
	}

	t.attribute.err_lex[i]= '\0';

	return t;
}


/*
Purpose: Convert an ASCII string representing OIL to an IL
Author: Kyle Hinskens
History/Versions: 13.10.22
Called functions: atol(), pow()
Parameters: char * lexeme
Return value: long The decimal representation of the octal string
Algorithm: Decimal plus last digit of octal multiplied by the
power of 8 to however many digits have been read.
Then divide octal by 10 to discard the last digit
and return a decimal number.
*/
long atool(char * lexeme) {
	long octal;			/* Temporary variable for calculations */
	long decimal = 0;	/* Decimal value to return */
	int i = 0;			/* Loop counter */

	octal = atol(lexeme);
	while (octal != 0) {
		/* Multiply the last digit of octal by next power of 8 and add to decimal */
		decimal = decimal + (octal % 10) * pow((long double)8, i++); /* this line has a warning, possible loss of data */
		octal = octal / 10; /* Divide by 10 to remove the last digit */
	}

	return decimal;
}


/*
Purpose: To determine if the given lexeme is a keyword.
Author: Kyle Hinskens
History/Versions: 13.10.22
Called functions: strcmp()
Parameters: char * kw_lexeme
Return value: int The index of the found keyword, or -1 if not found.
Algorithm: Initializes the return variable to -1 and uses strcmp()
in a loop to determine if the lexeme is a keyword. If
found, the return value is set to the keyword index.
*/
int isKeyword(char * kw_lexeme) {
	int i;						 /* Loop counter */
	int keywordIndex = R_FAIL_1; /* To hold the index of the found keyword */

	for (i = 0; i < KWT_SIZE; i++) {

		/* Check to see if the lexeme is a Keyword from the table */
		if (strcmp(kw_lexeme, kw_table[i]) == 0) {
			keywordIndex = i; /* Set the index of the table as our return value */
			break; /* The lexeme was a Keyword, get out of the loop */
		}
	}

	return keywordIndex;
}


/*
Purpose: To handle potential runtime errors
Author: Kyle Hinskens, Kwok Hong Kelvin Chan
History/Versions: 13.10.24
Called functions: memset()
Parameters: None
Return value: Token
Algorithm: Increments scerrnum and sets the err_lex
attribute
*/
Token runtimeError() {
	Token t;		/* The token to return */
	int i;			/* Loop counter */
	char * error;	/* Error message */

	t.code = ERR_T;
	scerrnum++;

	error = "RUN TIME ERROR: ";

	/* Use memset to initialize the err_lex to \0 so that there are no garbage characters */
	memset(t.attribute.err_lex, '\0', ERR_LEN + 1);
	for (i = 0; i < ERR_LEN; ++i) {
		t.attribute.err_lex[i] = error[i];
	}

	return t;
}