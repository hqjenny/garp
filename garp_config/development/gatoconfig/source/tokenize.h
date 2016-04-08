
#ifndef tokenize_h
#define tokenize_h 1

#include <stdbool.h>
#include <stdint.h>

struct tokenS;

extern bool tokenize_debug;

typedef struct tokenS *YYSTYPE;
#define YYSTYPE YYSTYPE

#include PARSE_TAB_H

extern uint_fast32_t tokenize_sourceLineNum;
extern uint_fast16_t tokenize_sourceColumnNum;

int yylex( void );

#endif

