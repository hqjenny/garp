
%{

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "errorMessages.h"
#include "tokens.h"
#include "names.h"
#include "tokenize.h"

bool tokenize_debug = false;

uint_fast32_t tokenize_sourceLineNum = 1;
uint_fast16_t tokenize_sourceColumnNum = 0;

uint_fast32_t blockCommentCount;
uint_fast32_t blockCommentLineNum;
uint_fast16_t blockCommentColumnNum;

extern int yyleng;

static int d( uint_fast16_t tokenCode )
{
    char * tokenName;

    if ( tokenize_debug ) {
        switch ( tokenCode ) {
         case SLASH:
            tokenName = "SLASH";
            break;
         case VERTICALBAR:
            tokenName = "VERTICALBAR";
            break;
         case BACKSLASH:
            tokenName = "BACKSLASH";
            break;
         case LEFTPARENTHESIS:
            tokenName = "LEFTPARENTHESIS";
            break;
         case RIGHTPARENTHESIS:
            tokenName = "RIGHTPARENTHESIS";
            break;
         case LEFTBRACKET:
            tokenName = "LEFTBRACKET";
            break;
         case RIGHTBRACKET:
            tokenName = "RIGHTBRACKET";
            break;
         case LEFTBRACE:
            tokenName = "LEFTBRACE";
            break;
         case RIGHTBRACE:
            tokenName = "RIGHTBRACE";
            break;
         case PLUSSIGN:
            tokenName = "PLUSSIGN";
            break;
         case MINUSSIGN:
            tokenName = "MINUSSIGN";
            break;
         case CARET:
            tokenName = "CARET";
            break;
         case LESSTHANSIGN:
            tokenName = "LESSTHANSIGN";
            break;
         case EQUALSSIGN:
            tokenName = "EQUALSSIGN";
            break;
         case EQUALSSIGN_EQUALSSIGN:
            tokenName = "EQUALSSIGN_EQUALSSIGN";
            break;
         case GREATERTHANSIGN:
            tokenName = "GREATERTHANSIGN";
            break;
         case TILDE:
            tokenName = "TILDE";
            break;
         case RIGHTQUOTATIONMARK:
            tokenName = "RIGHTQUOTATIONMARK";
            break;
         case COMMA:
            tokenName = "COMMA";
            break;
         case SEMICOLON:
            tokenName = "SEMICOLON";
            break;
         case COLON:
            tokenName = "COLON";
            break;
         case PERIOD:
            tokenName = "PERIOD";
            break;
         case EXCLAMATIONPOINT:
            tokenName = "EXCLAMATIONPOINT";
            break;
         case QUESTIONMARK:
            tokenName = "QUESTIONMARK";
            break;
         case NUMBERSIGN:
            tokenName = "NUMBERSIGN";
            break;
         case STAR:
            tokenName = "STAR";
            break;
         case DOLLARSIGN:
            tokenName = "DOLLARSIGN";
            break;
         case ATSIGN:
            tokenName = "ATSIGN";
            break;
         case AMPERSAND:
            tokenName = "AMPERSAND";
            break;
         case PERCENTSIGN:
            tokenName = "PERCENTSIGN";
            break;
         case K_ROW:
            tokenName = "K_ROW";
            break;
         case L_BINARY:
            tokenName = "L_BINARY";
            break;
         case L_NUMBER:
            tokenName = "L_NUMBER";
            break;
         case L_NAME:
            tokenName = "L_NAME";
            break;
         case L_DOTTEDNAME:
            tokenName = "L_DOTTEDNAME";
            break;
         case UNKNOWN:
            tokenName = "UNKNOWN";
            break;
        }
        fputs( "scan- ", stdout );
        fputs( tokenName, stdout );
        fputc( '\n', stdout );
        fflush( stdout );
    }
    yylval = newToken( tokenCode );
    yylval->lineNum = tokenize_sourceLineNum;
    yylval->columnNum = tokenize_sourceColumnNum;
    tokenize_sourceColumnNum += yyleng;
    return tokenCode;

}

%}

%x blockComment
%x lineComment

%%

" "+            { tokenize_sourceColumnNum += yyleng; }
"\t"            {
                  tokenize_sourceColumnNum =
                      (tokenize_sourceColumnNum | 7) + 1;
                }
"\n"            { ++tokenize_sourceLineNum; tokenize_sourceColumnNum = 0; }
"\r"            {}

<INITIAL,lineComment>"{*" {
                  blockCommentCount = 1;
                  blockCommentLineNum = tokenize_sourceLineNum;
                  blockCommentColumnNum = tokenize_sourceColumnNum;
                  tokenize_sourceColumnNum += 2;
                  BEGIN(blockComment);
                }
"*}"            {
                  markError(
                      tokenize_sourceLineNum,
                      tokenize_sourceColumnNum + yyleng - 1,
                      "Unbalanced comment bracket."
                  );
                  return d( UNKNOWN );
                }

"--"            {
                  tokenize_sourceColumnNum += 2;
                  BEGIN(lineComment);
                }

<blockComment>{
 "{*"           {
                  ++blockCommentCount;
                  tokenize_sourceColumnNum += 2;
                }
 "*}"           {
                  --blockCommentCount;
                  tokenize_sourceColumnNum += 2;
                  if ( blockCommentCount == 0 ) BEGIN(INITIAL);
                }
 <<EOF>>        {
                  markError(
                      blockCommentLineNum,
                      blockCommentColumnNum,
                      "Unbalanced comment bracket."
                  );
                  return d( UNKNOWN );
                }
 "\n"           { ++tokenize_sourceLineNum; tokenize_sourceColumnNum = 0; }
}

<lineComment>{
 "\n"           {
                  ++tokenize_sourceLineNum;
                  tokenize_sourceColumnNum = 0;
                  BEGIN(INITIAL);
                }
}

<blockComment,lineComment>{
 ["][^\n"]*["]  |
 [^{*"\n]+      |
 .              { tokenize_sourceColumnNum += yyleng; }
}

<INITIAL,blockComment>["] {
                  markError(
                      tokenize_sourceLineNum,
                      tokenize_sourceColumnNum,
                      "Unmatched quotation mark."
                  );
                  return d( UNKNOWN );
                }
<INITIAL,blockComment,lineComment>"{*}" {
                  markError(
                      tokenize_sourceLineNum,
                      tokenize_sourceColumnNum,
                      "Ambiguous comment bracket."
                  );
                  return d( UNKNOWN );
                }

["][^\n"]*["]   { return d( UNKNOWN ); }

"/"             { return d( SLASH ); }
"|"             { return d( VERTICALBAR ); }
"("             { return d( LEFTPARENTHESIS ); }
")"             { return d( RIGHTPARENTHESIS ); }
"["             { return d( LEFTBRACKET ); }
"]"             { return d( RIGHTBRACKET ); }
"{"             { return d( LEFTBRACE ); }
"}"             { return d( RIGHTBRACE ); }

"+"             { return d( PLUSSIGN ); }
"-"             { return d( MINUSSIGN ); }
"^"             { return d( CARET ); }
"<"             { return d( LESSTHANSIGN ); }
"="             { return d( EQUALSSIGN ); }
"=="            { return d( EQUALSSIGN_EQUALSSIGN ); }
">"             { return d( GREATERTHANSIGN ); }
"~"             { return d( TILDE ); }

"'"             { return d( RIGHTQUOTATIONMARK ); }
","             { return d( COMMA ); }
";"             { return d( SEMICOLON ); }
":"             { return d( COLON ); }
"."             { return d( PERIOD ); }
"!"             { return d( EXCLAMATIONPOINT ); }
"?"             { return d( QUESTIONMARK ); }

"#"             { return d( NUMBERSIGN ); }
"*"             { return d( STAR ); }

"$"             { return d( DOLLARSIGN ); }
"@"             { return d( ATSIGN ); }
"&"             { return d( AMPERSAND ); }

"%"             { return d( PERCENTSIGN ); }

"row"           { return d( K_ROW ); }

"00"            {
                  d( L_BINARY );
                  yylval->v.m = 0;
                  return L_BINARY;
                }
"01"            {
                  d( L_BINARY );
                  yylval->v.m = 1;
                  return L_BINARY;
                }
"10"            {
                  d( L_BINARY );
                  yylval->v.m = 2;
                  return L_BINARY;
                }
"11"            {
                  d( L_BINARY );
                  yylval->v.m = 3;
                  return L_BINARY;
                }

"+"?[0-9]+      {
                  uint_fast64_t n;
                  d( L_NUMBER );
                  errno = 0;
                  n = strtoull( yytext, 0, 10 );
                  if ( errno ) {
                      markError(
                          tokenize_sourceLineNum,
                          yylval->columnNum,
                          "Literal too large."
                      );
                      n = 0;
                  }
                  yylval->v.n = n;
                  return L_NUMBER;
                }

"+"?"0x"[0-9A-Fa-f]+ {
                  uint_fast64_t n;
                  d( L_NUMBER );
                  errno = 0;
                  n = strtoull( yytext, 0, 16 );
                  if ( errno ) {
                      markError(
                          tokenize_sourceLineNum,
                          yylval->columnNum,
                          "Literal too large."
                      );
                      n = 0;
                  }
                  yylval->v.n = n;
                  return L_NUMBER;
                }

([a-zA-Z]|"_")(([a-zA-Z0-9]|"-"|"_")*([a-zA-Z0-9]|"_"))? {
                  d( L_NAME );
                  yylval->v.name.namePtr = stringToName( yyleng, yytext );
                  return L_NAME;
                }

"."([a-zA-Z]|"_")(([a-zA-Z0-9]|"-"|"_")*([a-zA-Z0-9]|"_"))? {
                  d( L_DOTTEDNAME );
                  yylval->v.name.namePtr =
                      stringToName( yyleng - 1, yytext + 1 );
                  return L_DOTTEDNAME;
                }

.               { return d( UNKNOWN ); }

%%

#ifndef yywrap

yywrap()
{

    return 1;

}

#endif

