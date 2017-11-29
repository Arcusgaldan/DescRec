#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef Oberon_scanner_h
#define Oberon_scanner_h

#define SCANNER_MAX_ID_LENGTH 16

typedef enum _symbol{
	symbol_null = 0,
	
	//aritméticos
	symbol_times = 1,
	symbol_division,
	symbol_mod,
	symbol_plus,
	symbol_minus,
	
	//lógicos
	symbol_or,
	symbol_and,
	symbol_not,
	symbol_single_or,
	symbol_single_and,
	
	//comparação
	symbol_equals,
	symbol_greater,
	symbol_greater_equal,
	symbol_less,
	symbol_less_equal,
	symbol_not_equal,
	
	//pontuação
	symbol_period,
	symbol_comma,
	symbol_colon,
	symbol_close_paren,
	symbol_close_bracket,
	symbol_close_braces,
	symbol_open_paren,
	symbol_open_bracket,
	symbol_open_braces,
	symbol_semicolon,
	symbol_attrib,
	
	//elemento léxico
	symbol_integer,
	symbol_real,
	symbol_id,
	symbol_string,
	
	//palavras-chave
	symbol_if,
	symbol_else,
	symbol_while,
	symbol_do,
	symbol_for,
	symbol_type_int,
	symbol_type_float,
	symbol_type_double,
	symbol_type_char,
	symbol_type_void,
}symbol_t;

#include "backend.h"

typedef char identifier_t[SCANNER_MAX_ID_LENGTH+1];

typedef struct _position{
	unsigned int line;
	unsigned int column;
	fpos_t index;
}position_t;

typedef struct _lexem {
  identifier_t id;
  symbol_t symbol;
} lexem_t;

typedef struct _token {
  lexem_t lexem;
  value_t value;
  position_t position;
} token_t;

extern token_t current_token, last_token;

extern const position_t position_zero;

char *id_for_symbol(symbol_t symbol);

symbol_t inverse_condition(symbol_t symbol);

void initialize_scanner(FILE *file);
void read_token();

#endif
