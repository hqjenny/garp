
#ifndef parse_h
#define parse_h 1

struct tokenS;

extern struct tokenS *parseTokenTree;

int yyparse( void );

#endif

