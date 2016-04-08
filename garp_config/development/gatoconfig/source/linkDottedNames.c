
#include <stdint.h>
#include "platform.h"
#include "errorMessages.h"
#include "tokens.h"
#include "names.h"
#include "tokenize.h"
#include "linkDottedNames.h"

enum {
    multiplyDefined = 1,
    definitionIgnored
};

void
 installDottedNames( tokenT *tree, uint_fast16_t nameTokenCode, tokenT *ref )
{
    nameT *namePtr;
    tokenT *declaration;

    if ( tree == 0 ) return;
    switch ( tree->code ) {
     case L_DOTTEDNAME:
        namePtr = tree->v.name.namePtr;
        declaration = namePtr->declaration;
        if ( declaration ) {
            markErrorAtToken( tree, "Name multiply defined." );
            declaration->v.name.status = multiplyDefined;
            tree->v.name.status = definitionIgnored;
        } else {
            namePtr->declaration = tree;
        }
        tree->code = nameTokenCode;
        tree->ref = ref;
        break;
     case L_NUMBER:
        installDottedNames(
            tree->a, nameTokenCode ? nameTokenCode : OP_NAMED_CONSTANT, tree );
        break;
     case OP_LOGICBLOCK:
        installDottedNames( tree->a, OP_NAMED_COL, tree );
        installDottedNames( tree->b, 0, 0 );
        break;
     case OP_ROW:
        installDottedNames( tree->a, OP_NAMED_ROW, tree );
        break;
    }
    installDottedNames( tree->t, nameTokenCode, ref );

}

void linkDottedNamesZ( tokenT *tree )
{
    tokenT *declaration, *ref;
    uint_fast16_t tokenCode;

    if ( tree == 0 ) return;
    switch ( tree->code ) {
     case L_DOTTEDNAME:
        declaration = tree->v.name.namePtr->declaration;
        if ( declaration ) {
            tokenCode = declaration->code;
            tree->code = tokenCode;
            tree->ref =
                (tokenCode == OP_NAMED_CONSTANT) ? declaration
                    : declaration->ref;
        } else {
            markErrorAtToken( tree, "Name not defined." );
        }
        break;
     case L_NUMBER:
        break;
     case OP_LOGICBLOCK:
        linkDottedNamesZ( tree->b );
        break;
     case OP_ROW:
        linkDottedNames( tree->b );
        break;
     default:
        linkDottedNamesZ( tree->a );
        linkDottedNamesZ( tree->b );
        break;
    }
    linkDottedNamesZ( tree->t );

}

void removeDottedNames( tokenT *tree )
{

    if ( tree == 0 ) return;
    removeDottedNames( tree->t );
    switch ( tree->code ) {
     case OP_NAMED_CONSTANT:
     case OP_NAMED_COL:
     case OP_NAMED_ROW:
        switch ( tree->v.name.status ) {
         case 0:
            tree->v.name.namePtr->declaration = 0;
            break;
         case multiplyDefined:
            markErrorAtToken( tree, "Name multiply defined." );
            break;
        }
        tree->v.name.status = 0;
        break;
     case L_NUMBER:
     case OP_ROW:
        removeDottedNames( tree->a );
        break;
     case OP_LOGICBLOCK:
        removeDottedNames( tree->b );
        removeDottedNames( tree->a );
        break;
    }

}


void linkDottedNames( tokenT *tree )
{

    installDottedNames( tree, 0, 0 );
    linkDottedNamesZ( tree );
    removeDottedNames( tree );

}

