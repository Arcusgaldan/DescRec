#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "errors.h"
#include "scanner.h"

FILE *input_file;

token_t current_token, last_token;
const position_t position_zero = { .line = 0, .column = 0, .index = 0 };

char current_char, last_char;
position_t current_position;

lexem_t keywords[] = {
	{.id = "if", 		.symbol = symbol_if},
	{.id = "else", 		.symbol = symbol_else},
	{.id = "do", 		.symbol = symbol_do},
	{.id = "while",		.symbol = symbol_while},
	{.id = "for", 		.symbol = symbol_for},
	{.id = "int", 		.symbol = symbol_type_int},
	{.id = "float", 	.symbol = symbol_type_float},
	{.id = "double",	.symbol = symbol_type_double},
	{.id = "char", 		.symbol = symbol_type_char},
	{.id = "void", 		.symbol = symbol_type_void}	
};

const unsigned int keywords_count = sizeof(keywords) / sizeof(lexem_t);

lexem_t operators[] = {
  { .id = "*",    		.symbol = symbol_times},
  { .id = "/",    		.symbol = symbol_division},
  { .id = "%",    		.symbol = symbol_mod},
  { .id = "+",    		.symbol = symbol_plus},
  { .id = "-",    		.symbol = symbol_minus},
  { .id = "||",    		.symbol = symbol_or},
  { .id = "|",    		.symbol = symbol_single_or},
  { .id = "&",    		.symbol = symbol_single_and},
  { .id = "&&",    		.symbol = symbol_and},
  { .id = "!",    		.symbol = symbol_not},
  { .id = "==",    		.symbol = symbol_equals},
  { .id = ">",    		.symbol = symbol_greater},
  { .id = ">=",    		.symbol = symbol_greater_equal},
  { .id = "<",    		.symbol = symbol_less},
  { .id = "<=",    		.symbol = symbol_less_equal},
  { .id = "!=",    		.symbol = symbol_not_equal}
};

const unsigned int operators_count = sizeof(keywords) / sizeof(lexem_t);

lexem_t punctuation[] = {
	{ .id = ".",  .symbol = symbol_period},
	{ .id = ",",  .symbol = symbol_comma},
	{ .id = ":",  .symbol = symbol_colon},
	{ .id = ")",  .symbol = symbol_close_paren},
	{ .id = "]",  .symbol = symbol_close_bracket},
	{ .id = "}",  .symbol = symbol_close_braces},
	{ .id = "(",  .symbol = symbol_open_paren},
	{ .id = "[",  .symbol = symbol_open_bracket},
	{ .id = "{",  .symbol = symbol_open_braces},
	{ .id = "=",  .symbol = symbol_attrib},
	{ .id = ";",  .symbol = symbol_semicolon}
};

const unsigned int punctuation_count = sizeof(keywords) / sizeof(lexem_t);

bool is_letter(char c)
{
  return isalpha(c);
}

bool is_digit(char c)
{
  return isdigit(c);
}

bool is_blank(char c)
{
  return (c == ' ' || c == '\t');
}

bool is_newline(char c, char p)
{
  return (c == '\n' && p != '\r') || c == '\r';
}

bool is_keyword(identifier_t id, symbol_t *symbol)
{
  unsigned int index = 0;
  while (index < keywords_count && strcmp(keywords[index].id, id) != 0)
    index++;
  if (index < keywords_count) {
    if (symbol)
      *symbol = keywords[index].symbol;
    return true;
  }
  if (symbol)
    *symbol = symbol_null;
  return false;
}


char *id_for_symbol(symbol_t symbol)
{
  if (symbol == symbol_integer)
    return "inteiro";
  else if (symbol == symbol_real)
    return "real";
  else if (symbol == symbol_string)
    return "cadeia";
  for (unsigned int index = 0; index < keywords_count; index++)
    if (keywords[index].symbol == symbol)
      return keywords[index].id;
  for (unsigned int index = 0; index < operators_count; index++)
    if (operators[index].symbol == symbol)
      return operators[index].id;
  for (unsigned int index = 0; index < punctuation_count; index++)
    if (punctuation[index].symbol == symbol)
      return punctuation[index].id;
  return "unknown";
}

symbol_t inverse_condition(symbol_t symbol)
{
  switch (symbol) {
    case symbol_equal: return symbol_not_equal; break;
    case symbol_not_equal: return symbol_equal; break;
    case symbol_less: return symbol_greater_equal; break;
    case symbol_less_equal: return symbol_greater; break;
    case symbol_greater: return symbol_less_equal; break;
    case symbol_greater_equal: return symbol_less; break;
    default: break;
  }
  return symbol_null;
}

bool read_char()
{
  last_char = current_char;
  if (fread(&current_char, sizeof(char), 1, input_file) == sizeof(char)) {
    if (is_newline(current_char, last_char)) {
      current_position.line++;
      current_position.column = 0;
    } else current_position.column++;
    current_position.index++;
    return true;
  }
  return false;
}

void id()
{
  unsigned int index = 0;
  current_token.position = current_position;
  while (index < SCANNER_MAX_ID_LENGTH && (is_letter(current_char) || is_digit(current_char) || current_char == '_')) {
    current_token.lexem.id[index++] = current_char;
    if (!read_char())
      break;
  }
  
  current_token.lexem.id[index] = '\0';
  if (!is_keyword(current_token.lexem.id, &current_token.lexem.symbol))
    current_token.lexem.symbol = symbol_id;
}

void number()
{
  unsigned int index = 0;
  current_token.position = current_position;
  current_token.value = 0;
  identifier_t id; 
  while (index < SCANNER_MAX_ID_LENGTH && is_digit(current_char)) { 
    id[index] = current_char;
    current_token.lexem.id[index] = current_char;
    index++;    
    current_token.value = 10 * current_token.value + (current_char - '0');
    if (!read_char())
      break;
  }
  current_token.lexem.symbol = symbol_integer;
  if (current_char == '.' || current_char == ',') { 
    id[index] = current_char;
    current_token.lexem.id[index] = current_char;
    index++;
    read_char();
    float factor = 0.1;
    while (index < SCANNER_MAX_ID_LENGTH && is_digit(current_char)) {
      id[index] = current_char;
      current_token.lexem.id[index] = current_char;
      index++;
      current_token.value = current_token.value + (current_char - '0') * factor;
      factor *= 0.1;
      if (!read_char())
        break;
    }
    current_token.lexem.symbol = symbol_real;
  }
  current_token.lexem.id[index] = '\0';
  bool invalid_ending = false;
  while (index < SCANNER_MAX_ID_LENGTH && (is_letter(current_char) || current_char == '_')) {
    id[index++] = current_char;
    invalid_ending  = true;
    if (!read_char())
      break;
  }
  if (invalid_ending)
    mark(error_warning, "\"%s\" is not a number. Assuming \"%s\".", id, current_token.lexem.id);
}

void string()
{
  unsigned int index = 0; 
  read_char(); 
  while (index < SCANNER_MAX_ID_LENGTH && current_char != '\"') { 
    current_token.lexem.id[index] = current_char; 
    index++; 
    read_char(); 
  }
  if (current_char == '\"'){
    current_token.lexem.id[index] = '\0';
    read_char();

  }
}

void comment()
{
  current_token.position = current_position;
  while (read_char()) {
    if (current_char == '/' && last_char == '*') {
      read_char();
      return;
    }
  }
  mark(error_fatal, "Endless comment detected.");
  current_token.lexem.symbol = symbol_eof;
}

void singleLineComment()
{
	current_token.position = current_position;
	while(!is_newline(current_char, last_char)){
		if(!read_char())
			return;
	}
}

void read_token() 
{ 
  if (feof(input_file)) {
    if (current_token.lexem.symbol != symbol_eof) {
      strcpy(current_token.lexem.id, "EOF");
      current_token.lexem.symbol = symbol_eof;
    }
    return;
  }
  
  while (is_blank(current_char))
    read_char();
    
  if (is_letter(current_char)) {
    id();
    return;
  }
  else if (is_digit(current_char)) {
    number();
    return;
  }
  else if (current_char == '\"') {
    string();
    return;
  }
  else if (is_newline(current_char, last_char)){
    current_token.position = current_position;
    current_token.lexem.symbol = symbol_newline;
  }
  current_token.position = current_position;
  current_token.lexem.id[0] = current_char;
  switch (current_token.lexem.id[0]) {
    case '*': current_token.lexem.symbol = symbol_times;          	break;
    case '/': current_token.lexem.symbol = symbol_division;       	break;
    case '%': current_token.lexem.symbol = symbol_mod;           	break;
    case '+': current_token.lexem.symbol = symbol_plus;          	break;
    case '-': current_token.lexem.symbol = symbol_minus;          	break;
    case '|': current_token.lexem.symbol = symbol_single_or;      	break;
    case '&': current_token.lexem.symbol = symbol_single_and;       break;
    case '!': current_token.lexem.symbol = symbol_not;        		break;
    case '<': current_token.lexem.symbol = symbol_less;      		break;
    case '>': current_token.lexem.symbol = symbol_greater;          break;
    case '.': current_token.lexem.symbol = symbol_period;          	break;
    case ',': current_token.lexem.symbol = symbol_comma;         	break;
    case ':': current_token.lexem.symbol = symbol_colon;     		break;
    case ')': current_token.lexem.symbol = symbol_close_paren;    	break;
    case ']': current_token.lexem.symbol = symbol_close_bracket;   	break;
    case '}': current_token.lexem.symbol = symbol_close_braces;  	break;
    case '(': current_token.lexem.symbol = symbol_open_paren;       break;
    case '[': current_token.lexem.symbol = symbol_open_bracket;     break;
    case '{': current_token.lexem.symbol = symbol_open_braces;      break;
    case '=': current_token.lexem.symbol = symbol_attrib;           break;
    case ';': current_token.lexem.symbol = symbol_semicolon;        break;
    case '\r': current_token.lexem.symbol = symbol_newline;       	break;
    case '\n': current_token.lexem.symbol = symbol_newline;       	break;
    default:  current_token.lexem.symbol = symbol_null;           	break;
  }
  current_token.lexem.id[1] = '\0';
  read_char();
  if (current_token.lexem.symbol == symbol_null) {
    mark(error_scanner, "\"%s\" is not a valid symbol.", current_token.lexem.id);
    return;
  }
  
  if (current_token.lexem.symbol == symbol_less && current_char == '=') {//<=
    current_token.lexem.id[1] = '=';
    current_token.lexem.id[2] = '\0';
    read_char();
    current_token.lexem.symbol = symbol_less_equal;
  }
  else if (current_token.lexem.symbol == symbol_greater && current_char == '=') {//>=
    current_token.lexem.id[1] = '=';
    current_token.lexem.id[2] = '\0';
    read_char();
    current_token.lexem.symbol = symbol_greater_equal;
  }
  else if (current_token.lexem.symbol == symbol_single_and && current_char == '&') {//&&
    current_token.lexem.id[1] = '&';
    current_token.lexem.id[2] = '\0';
    read_char();
    current_token.lexem.symbol = symbol_and;
  }
  else if (current_token.lexem.symbol == symbol_single_or && current_char == '|') {//||
    current_token.lexem.id[1] = '|';
    current_token.lexem.id[2] = '\0';
    read_char();
    current_token.lexem.symbol = symbol_or;
  }
  else if (current_token.lexem.symbol == symbol_attrib && current_char == '=') {//==
    current_token.lexem.id[1] = '=';
    current_token.lexem.id[2] = '\0';
    read_char();
    current_token.lexem.symbol = symbol_equals;
  }
  else if (current_token.lexem.symbol == symbol_not && current_char == '=') {//!=
    current_token.lexem.id[1] = '=';
    current_token.lexem.id[2] = '\0';
    read_char();
    current_token.lexem.symbol = symbol_not_equal;
  }
  else if (current_token.lexem.symbol == symbol_division && current_char == '*') {// /**/
	read_char();
    comment();
    read_token();
  }else if (current_token.lexem.symbol == symbol_division && current_char == '/') {// //
	read_char();
    singleLineComment();
    read_token();
  }
}

void initialize_scanner(FILE *file)
{
  input_file = file;
  strcpy(current_token.lexem.id, "");
  current_token.position = position_zero;
  current_token.lexem.symbol = symbol_null;
  current_token.value = 0;
  current_position.line = 1;
  current_position.column = 0;
  current_position.index = 0;
  current_char = '\0';
  read_char();
}
  
