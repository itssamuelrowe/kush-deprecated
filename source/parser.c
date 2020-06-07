/*
 * Copyright 2017-2020 Samuel Rowe, Joel E. Rego
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jtk/collection/Pair.h>
#include <kush/parser.h>

/*******************************************************************************
 * Parser                                                                      *
 *******************************************************************************/

// Match

static void match(k_Parser_t* parser, TokenType type);
static int32_t matchEx(k_Parser_t* parser, TokenType* types, int32_t count);
static Token* matchAndYield(k_Parser_t* parser, TokenType type);
static Token* matchAndYieldEx(k_Parser_t* parser, TokenType* types, int32_t count,
    int32_t* index);

// Recovery

static void pushFollowToken(k_Parser_t* parser, TokenType type);
static void popFollowToken(k_Parser_t* parser);
static void recover(k_Parser_t* parser);

// Rules

static bool isStatementFollow(TokenType type);
static bool isSimpleStatementFollow(TokenType type);
static bool isLiteral(TokenType type);
static bool isLiteralFollow(TokenType type);
static bool isCompoundStatementFollow(TokenType type);
static bool isClassMemberFollow(TokenType type);
static bool isClassMemberModifier(TokenType type);
static bool isExpressionFollow(TokenType type);
static bool isAssignmentOperator(TokenType type);
static bool isEqualityOperator(TokenType type);
static bool isRelationalOperator(TokenType type);
static bool isShiftOperator(TokenType type);
static bool isAdditiveOperator(TokenType type);
static bool isMultiplicativeOperator(TokenType type);
static bool isUnaryExpressionFollow(TokenType type);
static bool isUnaryOperator(TokenType type);
static bool isPostfixExpressionFollow(TokenType type);
static bool isPostfixPartFollow(TokenType type);
static bool isPrimaryExpressionFollow(TokenType type);

static void parseCompilationUnit(k_Parser_t* parser, k_ASTNode_t* node);
static void parseImportDeclaration(k_Parser_t* parser, k_ASTNode_t* node);
static void parseComponentDeclaration(k_Parser_t* parser, k_ASTNode_t* node);
static void parseFunctionDeclaration(k_Parser_t* parser, k_ASTNode_t* node, uint32_t modifiers);
static void parseFunctionParameters(k_Parser_t* parser, k_ASTNode_t* node);
static void parseFunctionBody(k_Parser_t* parser, k_ASTNode_t* node);
static void parseStatementSuite(k_Parser_t* parser, k_ASTNode_t* node);
static void parseSimpleStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseEmptyStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseVariableDeclaration(k_Parser_t* parser, k_ASTNode_t* node);
static void parseStorageDeclarator(k_Parser_t* parser, k_ASTNode_t* node);
static void parseBreakStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseReturnStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseThrowStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseCompoundStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseIfStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseIfClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseElseIfClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseElseClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseIterativeStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseLabelClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseWhileStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseForStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseTryStatement(k_Parser_t* parser, k_ASTNode_t* node);
static void parseTryClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseCatchClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseCatchFilter(k_Parser_t* parser, k_ASTNode_t* node);
static void parseFinallyClause(k_Parser_t* parser, k_ASTNode_t* node);
static void parseStructureDeclaration(k_Parser_t* parser, k_ASTNode_t* node);
static void parseStructureSuite(k_Parser_t* parser, k_ASTNode_t* node);
static void parseStructureMember(k_Parser_t* parser, k_ASTNode_t* node);
static void parseExpressions(k_Parser_t* parser, k_ASTNode_t* node);
static void parseExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseAssignmentExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseConditionalExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseLogicalOrExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseLogicalAndExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseInclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseExclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseAndExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseEqualityExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseRelationalExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseShiftExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseAdditiveExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseMultiplicativeExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseUnaryExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void postfixExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseSubscript(k_Parser_t* parser, k_ASTNode_t* node);
static void parseFunctionArguments(k_Parser_t* parser, k_ASTNode_t* node);
static void parseMemberAccess(k_Parser_t* parser, k_ASTNode_t* node);
static void parsePostfixOperator(k_Parser_t* parser, k_ASTNode_t* node);
static void parsePrimaryExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseLiteral(k_Parser_t* parser, k_ASTNode_t* node);
static void parseInitializerExpression(k_Parser_t* parser, k_ASTNode_t* node);
static void parseInitializerEntries(k_Parser_t* parser, k_ASTNode_t* node);
static void parseInitializerEntry(k_Parser_t* parser, k_ASTNode_t* node);
static void parseArrayExpression(k_Parser_t* parser, k_ASTNode_t* node);

static const char ruleNames[][50] = {
    "<unknown>",
    "<terminal>",

    "compilationUnit",
    "importDeclaration",
    "functionDeclaration",
    "functionParameter",
    "blockStatement",
    "variableDeclaration",
    "variableDeclarator",
    "breakStatement",
    "returnStatement",
    "throwStatement",
    "ifStatement",
    "ifClause",
    "iterativeStatement",
    "tryStatement",
    "catchClause",
    "structureDeclaration",
    "assignmentExpression",
    "conditionalExpression",
    "logicalOrExpression",
    "logicalAndExpression",
    "inclusiveOrExpression",
    "exclusiveOrExpression",
    "andExpression",
    "equalityExpression",
    "relationalExpression",
    "shiftExpression",
    "additiveExpression",
    "multiplicativeExpression",
    "unaryExpression",
    "postfixExpression",
    "initializerExpression",
    "arrayExpression"
};

#define la(parser, count) k_TokenStream_la((parser)->tokens, (count))
#define consume(parser) k_TokenStream_consume((parser)->tokens)
#define match(parser, type) matchAndYield((parser), type)
#define lt(parser, count) k_TokenStream_lt((parser)->tokens, (count))

/* Constructor */

k_Parser_t* k_Parser_new(Compiler* compiler, k_TokenStream_t* tokens) {
    jtk_Assert_assertObject(compiler, "The specified compiler is null.");

    k_Parser_t* parser = allocate(k_Parser_t, 1);
    parser->compiler = compiler;
    parser->tokens = tokens;
    parser->followSet = jtallocate(TokenType, 128);
    parser->followSetSize = 0;
    parser->followSetCapacity = 16;
    parser->recovery = false;

    return parser;
}

/* Destructor */

void k_Parser_delete(k_Parser_t* parser) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    deallocate(parser->followSet);
    deallocate(parser);
}

/* Rule Name */

const char* k_Parser_getRuleName(k_ASTNodeType_t type) {
    return ruleNames[(int32_t)type];
}

// Reset

void k_Parser_reset(k_Parser_t* parser, k_TokenStream_t* tokens) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    parser->tokens = tokens;
    parser->followSetSize = 0;
    parser->recovery = false;
}

// Recover

/* When the parser encounters an invalid input, the current rule cannot continue,
 * so the parser recovers by skipping tokens until it a possible resynchronized
 * state is achived. The control is then returned to the calling rule.
 * This technique is known as the panic mode strategy.
 *
 * The trick here is to discard tokens only until the lookahead token is
 * something that the parent rule of the current rule expects. For example,
 * if there is a syntax error within a throw statement, the parser discards
 * tokens until a newline token or other relevant token is encountered.
 */
void recover(k_Parser_t* parser) {
    /* The parser is now in recovery mode; flag other parts of the parser. */
    parser->recovery = true;

    if (parser->followSetSize > 0) {
        Token* lt1 = lt(parser, 1);
        /* The parser tries to recover until a token from the follow set or
         * the end-of-stream token is encountered.
         */
        while (lt1->type != TOKEN_END_OF_STREAM) {
            /* When searching for a follow token, the parser prioritizes tokens
             * that are expected by the nearest rule in the rule invocation
             * stack. This is why, the linear search algorithm is applied in a
             * reverse fashion over the follow set.
             */
            int32_t i;
            for (i = parser->followSetSize - 1; i >= 0; i--) {
                if (lt1->type == parser->followSet[i]) {
                    /* A token from the follow set was encountered. The parser
                     * may have resynchronized with the input.
                     */
                    goto afterDiscard;
                }
            }
            /* Consume and discard the current token. */
            consume(parser);
            /* Update the lookahead token. */
            lt1 = lt(parser, 1);
        }
        afterDiscard:
            ;
    }
}

void reportAndRecover(k_Parser_t* parser, TokenType expected) {
    /* Do not report the error if the parser is in recovery mode. Otherwise,
    * duplicate syntax errors will be reported to the end user.
    */
    if (!parser->recovery) {
        Token* lt1 = lt(parser, 1);
        Compiler* compiler = parser->compiler;
        k_ErrorHandler_t* errorHandler = compiler->errorHandler;
        k_ErrorHandler_handleSyntacticalError(errorHandler, parser,
            KUSH_ERROR_CODE_UNEXPECTED_TOKEN, lt1, expected);
    }

    /* Try to resychronize the parser with the input. */
    recover(parser);
}

bool ensureFollowSetSpace(k_Parser_t* parser, int32_t capacity) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    bool result = false;
    if (capacity > 0) {
        int32_t currentCapacity = parser->followSetCapacity;
        int32_t minimumCapacity = (parser->followSetSize + capacity);
        int32_t requiredCapacity = minimumCapacity - currentCapacity;
        if (requiredCapacity > 0) {
            int32_t newCapacity = (currentCapacity * 2) + 2;
            if ((newCapacity - minimumCapacity) < 0) {
                newCapacity = minimumCapacity;
            }

            if (newCapacity < 0) {
                if (minimumCapacity < 0) {
                    /* Report an error, the requested capacity is too big. */
                    printf("[internal error] The requested capacity is too big. The parser has run out of memory.\n");
                }
                else {
                    /* Fall back, the new capacity was recommened by this function. */
                    newCapacity = JTK_INTEGER_MAX_VALUE;
                }
            }

            /* Do not allocate a new buffer if an out-of-memory error was
             * reported.
             */
            if (newCapacity > 0) {
                uint8_t* temporary = parser->followSet;
                parser->followSet = (uint8_t*)jtk_Arrays_copyOfEx_b(
                    parser->followSet, parser->followSet, newCapacity, 0,
                    false);
                parser->followSetCapacity = newCapacity;
                deallocate(temporary);
            }
        }
    }
    return result;
}

void pushFollowToken(k_Parser_t* parser, TokenType type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    /* Make sure that the set is large enough to hold another token type. */
    ensureFollowSetSpace(parser, parser->followSetSize + 1);
    /* Insert the follow token to the set. */
    parser->followSet[parser->followSetSize] = type;
    /* Increment the size of the follow set. */
    parser->followSetSize++;
}

void popFollowToken(k_Parser_t* parser) {
    jtk_Assert_assertTrue(parser->followSetSize > 0, "The follow set is empty.");

    parser->followSetSize--;
}

/* Consume */

Token* consumeAndYield(k_Parser_t* parser) {
    Token* lt1 = lt(parser, 1);
    consume(parser);

    return lt1;
}

/* Match */

int32_t matchEx(k_Parser_t* parser, TokenType* types, int32_t count) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    int32_t index;
    matchAndYieldEx(parser, types, count, &index);
    return index;
}

Token* matchAndYieldEx(k_Parser_t* parser, TokenType* types, int32_t count,
    int32_t* index) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");
    jtk_Assert_assertTrue(count > 0, "The specified count is invalid.");

    Token* lt1 = lt(parser, 1);
    *index = -1;
    int32_t i;
    for (i = 0; i < count; i++) {
        if (lt1->type == types[i]) {
            *index = i;

            /* The token expected by the parser was found. If we the parser is
            * in error recovery, turn it off.
            */
            parser->recovery = false;

            /* The token stream prohibts consumption of end-of-stream
             * token.
             */
            if (lt1->type != TOKEN_END_OF_STREAM) {
                consume(parser);
            }
        }
    }

    if (*index == -1) {
        reportAndRecover(parser, types[0]);
        lt1 = NULL;
    }

    return lt1;
}

Token* matchAndYield(k_Parser_t* parser, TokenType type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    Token* lt1 = lt(parser, 1);
    if (lt1->type == type) {
        /* The token expected by the parser was found. If we the parser is
         * in error recovery, turn it off.
         */
        parser->recovery = false;


        /* The token stream prohibts consumption of end-of-stream
         * token.
         */
        if (lt1->type != TOKEN_END_OF_STREAM) {
            consume(parser);
        }
    }
    else {
        reportAndRecover(parser, type);
    }
    return lt1;
}

bool isReturnType(TokenType token) {
    return (token == TOKEN_KEYWORD_VOID) ||
        (token == TOKEN_KEYWORD_I8) ||
        (token == TOKEN_KEYWORD_I16) ||
        (token == TOKEN_KEYWORD_I32) ||
        (token == TOKEN_KEYWORD_I64) ||
        (token == TOKEN_KEYWORD_F32) ||
        (token == TOKEN_KEYWORD_F64) ||
        (token == TOKEN_IDENTIFIER);
}

bool isComponentFollow(TokenType token) {
    return (token == TOKEN_KEYWORD_STRUCT) || isReturnType(token);
}

bool isType(TokenType token) {
    return (token == TOKEN_KEYWORD_I8) ||
        (token == TOKEN_KEYWORD_I16) ||
        (token == TOKEN_KEYWORD_I32) ||
        (token == TOKEN_KEYWORD_I64) ||
        (token == TOKEN_KEYWORD_F32) ||
        (token == TOKEN_KEYWORD_F64) ||
        (token == TOKEN_IDENTIFIER);
}

bool isSimpleStatementFollow(TokenType type) {
    return (type == TOKEN_SEMICOLON) || // emptyStatement
        (type == TOKEN_KEYWORD_VAR) || // variableDeclaration
        (type == TOKEN_KEYWORD_LET) || // variableDeclaration
        (type == TOKEN_KEYWORD_BREAK) || // breakStatement
        (type == TOKEN_KEYWORD_RETURN) || // returnStatement
        (type == TOKEN_KEYWORD_THROW) || // throwStatement
        isExpressionFollow(type); // expressionStatement (includes IDENTIFIER, which may lead to variableDeclaration, too!)
}

bool isCompoundStatementFollow(TokenType type) {
    return (type == TOKEN_KEYWORD_IF) || // ifStatement
		(type == TOKEN_HASH) || // iterativeStatement
		(type == TOKEN_KEYWORD_WHILE) || // whileStatement
		(type == TOKEN_KEYWORD_FOR) || // forStatement
		(type == TOKEN_KEYWORD_TRY); // tryStatement
}

/* The parser needs to look ahead 3 tokens to differentiate between variable
 * declarations and expressions, recognizing an LL(3) grammar.
 *
 * followVariableDeclaration
 * :    'let'
 * |    'var'
 * |    IDENTIFIER (('[' ']') | IDENTIFIER)
 * ;
 */
bool followVariableDeclaration(k_Parser_t* parser) {
    TokenType la1 = la(parser, 1);
    return (la1 == TOKEN_KEYWORD_LET) ||
           (la1 == TOKEN_KEYWORD_VAR) ||
           ((la1 == TOKEN_IDENTIFIER) &&
            (((la(parser, 2) == TOKEN_LEFT_SQUARE_BRACKET) && (la(parser, 3) == TOKEN_RIGHT_SQUARE_BRACKET)) ||
            (la(parser, 2) == TOKEN_IDENTIFIER)));
}

bool isStatementFollow(TokenType type) {
    return isSimpleStatementFollow(type) || isCompoundStatementFollow(type);
}

bool isExpressionFollow(TokenType type) {
    return isUnaryExpressionFollow(type);
}

/*
 * assignmentOperator
 * :    '='
 * |    '*='
 * |    '/='
 * |    '%='
 * |    '+='
 * |    '-='
 * |    '<<='
 * |    '>>='
 * |    '>>>='
 * |    '&='
 * |    '^='
 * |    '|='
 * ;
 */
bool isAssignmentOperator(TokenType type) {
    return (type == TOKEN_EQUAL) ||
           (type == TOKEN_ASTERISK_EQUAL) ||
           (type == TOKEN_FORWARD_SLASH_EQUAL) ||
           (type == TOKEN_MODULUS_EQUAL) ||
           (type == TOKEN_PLUS_EQUAL) ||
           (type == TOKEN_DASH_EQUAL) ||
           (type == TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL) ||
           (type == TOKEN_AMPERSAND_EQUAL) ||
           (type == TOKEN_CARET_EQUAL) ||
           (type == TOKEN_VERTICAL_BAR_EQUAL);
}

/*
 * equalityOperator
 * :	'=='
 * |	'!='
 * ;
 */
bool isEqualityOperator(TokenType type) {
    return (type == TOKEN_EQUAL_2) ||
           (type == TOKEN_EXCLAMATION_MARK_EQUAL);
}

/*
 * relationalOperator
 * :	'<'
 * |	'>'
 * |	'<='
 * |	'>='
 * |	'is'
 * ;
 */
bool isRelationalOperator(TokenType type) {
    return (type == TOKEN_LEFT_ANGLE_BRACKET) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET) ||
           (type == TOKEN_LEFT_ANGLE_BRACKET_EQUAL) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET_EQUAL);
}

/*
 * shiftOperator
 * :	'<<'
 * |	'>>'
 * |	'>>>'
 * ;
 */
bool isShiftOperator(TokenType type) {
    return (type == TOKEN_LEFT_ANGLE_BRACKET_2) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET_2) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET_3);
}

/*
 * additiveOperator
 * :	'+'
 * |	'-'
 * ;
 */
bool isAdditiveOperator(TokenType type) {
    return (type == TOKEN_PLUS) ||
           (type == TOKEN_DASH);
}

/*
 * multiplicativeOperator
 * :	'*'
 * |	'/'
 * |	'%'
 * ;
 */
bool isMultiplicativeOperator(TokenType type) {
    return (type == TOKEN_ASTERISK) ||
           (type == TOKEN_FORWARD_SLASH) ||
           (type == TOKEN_MODULUS);
}

bool isUnaryExpressionFollow(TokenType type) {
    return isUnaryOperator(type) || isPostfixExpressionFollow(type);
}

bool isPostfixExpressionFollow(TokenType type) {
    return isPrimaryExpressionFollow(type);
}

/*
 * unaryOperator
 * :    '+'
 * |    '-'
 * |    '~'
 * |    '!'
 * // |    '++'
 * // |    '--'
 * ;
 */
bool isUnaryOperator(TokenType type) {
    return (type == TOKEN_PLUS) ||
           (type == TOKEN_DASH) ||
           (type == TOKEN_TILDE) ||
           (type == TOKEN_EXCLAMATION_MARK);
           /*
           ||
           (type == TOKEN_PLUS_2) ||
           (type == TOKEN_DASH_2);
           */
}

bool isPostfixPartFollow(TokenType type) {
    return (type == TOKEN_LEFT_SQUARE_BRACKET) ||
           (type == TOKEN_LEFT_PARENTHESIS) ||
           (type == TOKEN_DOT) ||
           (type == TOKEN_PLUS_2) ||
           (type == TOKEN_DASH_2);
}

bool isPrimaryExpressionFollow(TokenType type) {
    bool result = false;
    if (isLiteralFollow(type)) {
        result = true;
    }
    else {
        switch (type) {
            case TOKEN_KEYWORD_THIS:
            case TOKEN_IDENTIFIER:
            case TOKEN_LEFT_PARENTHESIS:
            case TOKEN_LEFT_BRACE:
            case TOKEN_LEFT_SQUARE_BRACKET:
            case TOKEN_LEFT_ANGLE_BRACKET:
            case TOKEN_KEYWORD_NEW: {
                result = true;
                break;
            }
        }
    }
    return result;
}

bool isLiteral(TokenType type) {
    bool result = false;
    switch (type) {
        case TOKEN_INTEGER_LITERAL:
        case TOKEN_FLOATING_POINT_LITERAL:
        case TOKEN_KEYWORD_TRUE:
        case TOKEN_KEYWORD_FALSE:
        case TOKEN_STRING_LITERAL:
        case TOKEN_KEYWORD_NULL: {
            result = true;
            break;
        }
    }
    return result;
}

bool isLiteralFollow(TokenType type) {
    return isLiteral(type);
}

/*
 * -----------------------------------------------------------------------------
 * How do we construct the abstract syntax tree?
 * -----------------------------------------------------------------------------
 *
 * Every parent rule allocates and passes an AST node to a child
 * rule, except for the entry rules. In this case, the user has to
 * manually allocate an AST node and pass. A node is associated
 * with a context. A context has specific data related to a given
 * rule. It is recognized by the type attribute within the
 * k_ASTNode_t structure.
 *
 * -----------------------------------------------------------------------------
 * How are abstract syntax trees, contexts, and rule specific data
 * destroyed?
 * -----------------------------------------------------------------------------
 *
 * Every rule receives a node. Note that within the abstract
 * syntax tree this node represents the current rule and not
 * the parent rule. Any allocated object should be immediately
 * attached to the node or the rules context. Because the parser
 * long jumps when a syntax error is discovered. Failing to
 * attach a resource may result in dangling pointers, or worst
 * case scenario: undefined behaviour.
 *
 * A rule first creates its context and attachs it to the
 * given node. Further, any object allocated within a
 * rule is immediately attached to its context. Thus,
 * all allocated objects are destroyed even if the parser
 * long jumps using the contextDestructor function within
 * the k_ASTNode_t structure.
 *
 * The special function k_ASTNode_delete() should be invoked
 * on the root node. k_ASTNode_delete() travereses down the
 * AST destroying each node it encounters and all its descendants.
 * The context of a node is destroyed by invoking the
 * contextDestructor, provided that it is non-null. Otherwise,
 * the context is ignored.
 */

/*
 * compilationUnit
 * :	NEWLINE*
 *      importDeclaration*
 *      componentDeclaration*
 *      EOF
 * ;
 *
 * componentDeclaration
 * :    functionDeclaration
 * |    structureDeclaration
 * ;
 */
k_CompilationUnit_t* parseCompilationUnit(k_Parser_t* parser) {
	/* Create the context of this rule. */
	k_CompilationUnit_t* context = k_CompilationUnit_new();

	/* Zero or more import declarations may occur in the source code.
	 * Therefore, repeatedly parse import declarations as long as
	 * LA(1) is the 'import' keyword.
	 *
	 * The following code represents: importDeclaration*
	 */
    while (la(parser, 1) == TOKEN_KEYWORD_IMPORT) {
		ImportDeclaration* importDeclaration = parseImportDeclaration(parser);
		jtk_ArrayList_add(context->imports, importDeclaration);
    }

    while (isComponentFollow(la(parser, 1))) {
        if (la(parser, 1) == TOKEN_KEYWORD_STRUCT) {
            k_StructureDeclaration_t* structure = parseStructureDeclaration(parser);
            jtk_ArrayList_add(context->structures, structure);
        }
        else {
            Function* function = parseFunctionDeclaration(parser, 0);
            jtk_ArrayList_add(context->structures, function);
        }
    }

	/* We are expecting the 'end of stream' token here. */
    match(parser, TOKEN_END_OF_STREAM);

    return context;
}

/*
 * importDeclaration
 * :    'import' importTarget ';'
 * ;
 *
 * TODO:
 *
 * importTarget
 * :    IDENTIFIER ('.' IDENTIFIER)* ('.' '*' | ('as' IDENTIFIER))? ';'
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
ImportDeclaration* parseImportDeclaration(k_Parser_t* parser) {
	ImportDeclaration* context = k_ImportDeclaration_new();

	/* An import statement begins with the 'import' keyword. Therefore,
	 * we are expecting it.
	 */
    match(parser, TOKEN_KEYWORD_IMPORT);

    /* If importTarget fails, discard tokens until the newline token is
     * encountered.
     */
    pushFollowToken(parser, TOKEN_SEMICOLON);

	/* The user is expected to specify at least, one identifier.
	 * Consume it. The consumed identifier saved for later inspection.
	 */
    Token* identifier = matchAndYield(parser, TOKEN_IDENTIFIER);
	jtk_ArrayList_add(context->identifiers, identifier);

	/* Optionally, the user may specify more identifiers (with each identifier
	 * separated by the '.' token. Therefore, we repeatedly consume the '.' and
	 * identifier tokens as long as LA(1) is the '.' token and LA(2) is the
	 * identifier token.
	 */
    while ((la(parser, 1) == TOKEN_DOT) &&
           (k_TokenStream_la(parser->tokens, 2) == TOKEN_IDENTIFIER)) {
		/* Consume and discard the '.' token. */
        consume(parser);

		/* The consumed identifier is saved for later inspection. */
        identifier = matchAndYield(parser, TOKEN_IDENTIFIER);
		jtk_ArrayList_add(context->identifiers, identifier);
    }

	/* Optionally, the user may specify a wildcard; recognized when
	 * LA(1) is the '.' token and LA(2) is the '*' token.
	 */
    if (la(parser, 1) == TOKEN_DOT) {
		consume(parser);
		match(parser, TOKEN_ASTERISK);
        context->wildcard = true;
    }

    /* Pop the semicolon token from the follow set. */
    popFollowToken(parser);
	/* The import declaration is terminated with a semicolon.
	 * Therefore, we are expecting it here.
	 */
    match(parser, TOKEN_SEMICOLON);

    return context;
}

Token* parseTypeEx(k_Parser_t* parser, int32_t* dimensions, bool includeVoid) {
    int32_t index;
    static const TokenType returnTypes[] = {
        TOKEN_KEYWORD_BOOLEAN,
        TOKEN_KEYWORD_I8,
        TOKEN_KEYWORD_I16,
        TOKEN_KEYWORD_I32,
        TOKEN_KEYWORD_I64,
        TOKEN_KEYWORD_F32,
        TOKEN_KEYWORD_F64,
        TOKEN_KEYWORD_VOID,
        TOKEN_IDENTIFIER
    };
    static const TokenType types[] = {
        TOKEN_KEYWORD_BOOLEAN,
        TOKEN_KEYWORD_I8,
        TOKEN_KEYWORD_I16,
        TOKEN_KEYWORD_I32,
        TOKEN_KEYWORD_I64,
        TOKEN_KEYWORD_F32,
        TOKEN_KEYWORD_F64,
        TOKEN_IDENTIFIER
    };
    Token* token = matchAndYieldEx(parser, includeVoid? returnTypes : types,
        includeVoid? 9 : 8, &index);

    *dimensions = 0;
    while (la(parser, 1) == TOKEN_LEFT_SQUARE_BRACKET) {
        *dimensions++;
        match(parser, TOKEN_RIGHT_SQUARE_BRACKET);
    }

    return token;
}

/**
 * componentType
 * :    'i8'
 * |    'i16'
 * |    'i32'
 * |    'i64'
 * |    'f32'
 * |    'f64'
 * |    'boolean'
 * ;
 *
 * type
 * :    componentType ('[' ']')*
 * ;
 */
Token* parseType(k_Parser_t* parser, int32_t* dimensions) {
    return parseTypeEx(parser, dimensions, false);
}

/**
 * returnType
 * :    type
 * |    'void'
 * ;
 */
Token* parseReturnType(k_Parser_t* parser, int32_t* dimensions) {
    return parseTypeEx(parser, dimensions, true);
}

/*
 * functionDeclaration
 * :    returnType IDENTIFIER functionParameters (functionBody | SEMICOLON)
 * ;
 */
Function* parseFunctionDeclaration(k_Parser_t* parser, k_ASTNode_t* node,
    uint32_t modifiers) {
	/* If function parameters fails, skip tokens until ';', '{', or '}' is found. */
    pushFollowToken(parser, TOKEN_SEMICOLON);
    pushFollowToken(parser, TOKEN_LEFT_BRACE);
    pushFollowToken(parser, TOKEN_RIGHT_BRACE);

    Function* context = k_FunctionDeclaration_new();
    context->returnType = parseReturnType(parser, &context->returnTypeDimensions);
    context->identifier = matchAndYield(parser, TOKEN_IDENTIFIER);
    parseFunctionParameters(parser, context->fixedParameters,
        &context->variableParameter);

    /* Pop the ';', '{', and '}' tokens from the follow set. */
    popFollowToken(parser);
    popFollowToken(parser);
    popFollowToken(parser);

    if (k_Modifier_hasNative(modifiers)) {
        match(parser, TOKEN_SEMICOLON);
    }
    else {
	    context->body = parseBlockStatement(parser);
    }
}

/*
 * functionParameters
 * :   '(' functionParameterList? ')'
 * ;
 *
 * functionParameterList
 * :   functionParameter (',' functionParameter)* (',' variableFunctionParameter)?
 * |   variableFunctionParameter
 * ;
 *
 * functionParameter
 * :   type IDENTIFIER
 * ;
 *
 * variableFunctionParameter
 * :   type '...' IDENTIFIER
 * ;
 *
 * The following function combines the above mentioned rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void parseFunctionParameters(k_Parser_t* parser,
    jtk_ArrayList_t* fixedParameters, FunctionParameter** variableParameter) {
    match(parser, TOKEN_LEFT_PARENTHESIS);
    pushFollowToken(parser, TOKEN_RIGHT_PARENTHESIS);

    TokenType la1 = la(parser, 1);
    if (isType(la1)) {
        bool first = true;
        do {
            if (!first) {
                match(parser, TOKEN_COMMA);
            }

            FunctionParameter* parameter = k_FunctionParameter_new();
            parameter->baseType = parseType(parser, &parameter->dimensions);
            if (la(parser, 1) == TOKEN_ELLIPSIS) {
                match(parser, TOKEN_ELLIPSIS);
                *variableParameter = parameter;
                break;
            }
            else {
                jtk_ArrayList_add(fixedParameters, parameter);
            }
            parameter->identifier = matchAndYield(parser, TOKEN_IDENTIFIER);
            first = false;
        }
        while (la(parser, 1) == TOKEN_COMMA);
    }

    popFollowToken(parser);
    match(parser, TOKEN_RIGHT_PARENTHESIS);
}

/*
 * blockStatement
 * |    '{' statement+ '}'
 * ;
 *
 * statement
 * :    simpleStatement
 * |    compoundStatement
 * ;
 *
 * The following function combines the above mentioned rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
Block* parseBlockStatement(k_Parser_t* parser) {
	Block* context = k_StatementSuite_new();

    /* Consume and discard the '{' token. */
    match(parser, TOKEN_LEFT_BRACE);
    /* If statement fails, discard tokens until the '}' token is
     * encountered.
     */
    pushFollowToken(parser, TOKEN_RIGHT_BRACE);

    do {
        TokenType la1 = la(parser, 1);
        if (isSimpleStatementFollow(la1)) {
            Context* statement = parseSimpleStatement(parser);
            jtk_ArrayList_add(context->statements, statement);
        }
        else if (isCompoundStatementFollow(la1)) {
            Context* statement = parseCompoundStatement(parser);
            jtk_ArrayList_add(context->statements, statement);
        }
        else {
            // TODO: Expected simple or compound statement
            reportAndRecover(parser, TOKEN_KEYWORD_VAR);
        }
    }
    while (isStatementFollow(la(parser, 1)));

    /* Pop the '}' token from the follow set. */
    popFollowToken(parser);
    /* Consume and discard the '}' token. */
    match(parser, TOKEN_RIGHT_BRACE);
}

/*
 * simpleStatement
 * :    unterminatedSimpleStatement SEMICOLON
 * ;
 *
 * unterminatedSimpleStatement
 * :   expressionStatement
 * |   emptyStatement
 * |   storageDeclaration
 * |   breakStatement
 * |   returnStatement
 * |   throwStatement
 * ;
 *
 * expressionStatement
 * :    expression
 * ;
 *
 * The following function combines the above mentioned rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
Context* parseSimpleStatement(k_Parser_t* parser) {
    Context* result = NULL;

    /* If expressionStatement, emptyStatement, variableDeclaration,
     * breakStatement, returnStatement, or throwStatement fails, discard tokens
     * until the semicolon token is encountered.
     */
    pushFollowToken(parser, TOKEN_SEMICOLON);

    TokenType la1 = la(parser, 1);
    if (followVariableDeclaration(parser)) {
        result = parseVariableDeclaration(parser);
    }
    else if (isExpressionFollow(la1)) {
        result = parseExpression(parser);
    }
    else {
        switch (la1) {
            case TOKEN_SEMICOLON: {
                /* Match and discard the ';' token. An empty statement is not part
                 * of the AST.
                 */
                match(parser, TOKEN_SEMICOLON);
                break;
            }

            case TOKEN_KEYWORD_BREAK: {
                result = parseBreakStatement(parser);
                break;
            }

            case TOKEN_KEYWORD_RETURN: {
                result = parseReturnStatement(parser);
                break;
            }

            case TOKEN_KEYWORD_THROW: {
                result = parseThrowStatement(parser);
                break;
            }
        }
    }

    /* Pop the ';' token from the follow set. */
    popFollowToken(parser);
    /* Match and discard the newline token. */
	match(parser, TOKEN_SEMICOLON);
}

/*
 * variableDeclaration
 * :    ('var' | 'let' | type) variableDeclarator (',' variableDeclarator)*
 * ;
 */
VariableDeclaration* parseVariableDeclaration(k_Parser_t* parser) {
	VariableDeclaration* context = k_VariableDeclaration_new();

    TokenType la1 = la(parser, 1);
    bool infer = (la1 == TOKEN_KEYWORD_VAR);
    bool constant = (la1 == TOKEN_KEYWORD_LET);
    Token* typeName = NULL;
    int32_t dimensions = -1;

    if (!infer && !constant) {
        dimensions = 0;
        typeName = parseType(parser, &dimensions);
    }

	Variable* declarator = k_StorageDeclarator_new(infer, constant,
        typeName, dimensions, NULL);
    jtk_ArrayList_add(context->declarators, declarator);
	parseStorageDeclarator(parser, declarator);

	while (la(parser, 1) == TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        consume(parser);

		declarator = k_StorageDeclarator_new(infer, constant,
        typeName, dimensions, NULL);
        jtk_ArrayList_add(context->declarators, declarator);
		parseStorageDeclarator(parser, declarator);
	}

    return context;
}

/*
 * variableDeclarator
 * :    IDENTIFIER ('=' expression)?
 * ;
 */
void parseStorageDeclarator(k_Parser_t* parser, Variable* declarator) {
    declarator->identifier = matchAndYield(parser, TOKEN_IDENTIFIER);

	if (la(parser, 1) == TOKEN_EQUAL) {
        /* Consume and discard the '=' token. */
		consume(parser);

		k_ASTNode_t* expression = k_ASTNode_new(NULL);
        declarator->expression = expression;
		parseExpression(parser, expression);
	}
}

/*
 * breakStatement
 * :    'break' IDENTIFIER?
 * ;
 */
k_BreakStatement_t* parseBreakStatement(k_Parser_t* parser) {
    k_BreakStatement_t* context = k_BreakStatement_new();

    /* Match and discard the 'break' token. */
    match(parser, TOKEN_KEYWORD_BREAK);

    if (la(parser, 1) == TOKEN_IDENTIFIER) {
        Token* identifier = lt(parser, 1);
        context->identifier = newTerminalNode(node, identifier);
        consume(parser);
    }

    return context;
}

/*
 * returnStatement
 * :    'return' expression
 * ;
 */
k_ReturnStatement_t* parseReturnStatement(k_Parser_t* parser) {
    k_ReturnStatement_t* context = k_ReturnStatement_new();

    /* Match and discard the 'return' token. */
    match(parser, TOKEN_KEYWORD_RETURN);
    /* An expression is mandatory after the 'return' keyword. */
    context->expression = parseExpression(parser);

    return context;
}

/*
 * throwStatement
 * :    'throw' expression?
 * ;
 */
ThrowStatement* parseThrowStatement(k_Parser_t* parser) {
    ThrowStatement* context = k_ThrowStatement_new();

    /* Match and discard the 'throw' token. */
    match(parser, TOKEN_KEYWORD_THROW);

    if (isExpressionFollow(la(parser, 1))) {
        context->expression = parseExpression(parser);
    }

    return context;
}

/*
 * compoundStatement
 * :    ifStatement
 * |    iterativeStatement
 * |    tryStatement
 * ;
 */
Context* parseCompoundStatement(k_Parser_t* parser) {
    Context* context = NULL;

	switch (la(parser, 1)) {
		case TOKEN_KEYWORD_IF: {
            context = parseIfStatement(parser);
			break;
		}

		case TOKEN_HASH:
		case TOKEN_KEYWORD_WHILE:
		case TOKEN_KEYWORD_FOR: {
            context = parseIterativeStatement(parser);
			break;
		}

		case TOKEN_KEYWORD_TRY: {
			context = parseTryStatement(parser);
			break;
		}

		default: {
			printf("[internal error] Control should not reach here.\n");
			break;
		}
	}

    return context;
}

/*
 * ifStatement
 * :    ifClause elseIfClause* elseClause?
 * ;
 *
 * elseClause
 * :    'else' blockStatement
 * ;
 */
IfStatement* parseIfStatement(k_Parser_t* parser) {
    IfStatement* context = k_IfStatement_new();

    // ifClause
    context->ifClause = parseIfClause(parser, ifClause);

    // elseIfClause*
	while ((la(parser, 1) == TOKEN_KEYWORD_ELSE) &&
	       (k_TokenStream_la(parser->tokens, 2) == TOKEN_KEYWORD_IF)) {
		IfClause* elseIfClause = parseElseIfClause(parser);
        jtk_ArrayList_add(context->elseIfClauses, elseIfClause);
	}

    // elseClause?
    if (la(parser, 1) == TOKEN_KEYWORD_ELSE) {
        consume(parser);
        context->elseClause = parseBlockStatement(parser);
    }

    return context;
}

/*
 * ifClause
 * :    'if' expression blockStatement
 * ;
 */
IfClause* parseIfClause(k_Parser_t* parser) {
    IfClause* context = k_IfClause_new();

	match(parser, TOKEN_KEYWORD_IF);
    context->expression = parseExpression(parser);
    context->body = parseBlockStatement(parser);

    return context;
}

/* elseIfClause
 * :    'else' 'if' expression blockStatement
 * ;
 */
IfClause* parseElseIfClause(k_Parser_t* parser) {
    IfClause* context = k_IfClause_new();

	match(parser, TOKEN_KEYWORD_ELSE);
	match(parser, TOKEN_KEYWORD_IF);
    context->expression = parseExpression(parser);
    context->body = parseBlockStatement(parser);

    return context;
}

/*
 * iterativeStatement
 * :    labelClause? (whileStatement | forStatement)
 * ;
 *
 * labelClause
 * :    '#' IDENTIFIER
 * ;
 *
 * whileStatement
 * :    'while' expression blockStatement
 * ;
 *
 * forStatement
 * :    'for' forParameter ':' expression blockStatement
 * ;
 *
 * forParameter
 * :    'let' IDENTIFIER
 * ;
 */
k_IterativeStatement_t* parseIterativeStatement(k_Parser_t* parser) {
    k_IterativeStatement_t* context = k_IterativeStatement_new();

	if (la(parser, 1) == TOKEN_HASH) {
        consume(parser);
	    context->label = matchAndYield(parser, TOKEN_IDENTIFIER);
	}

	switch (la(parser, 1)) {
		case TOKEN_KEYWORD_WHILE: {
            context->while = true;
            consume(parser);
            context->expression = parseExpression(parser);
            context->blockStatement = parseBlockStatement(parser);

			break;
		}

		case TOKEN_KEYWORD_FOR: {
            context->while = false;
            consume(parser);
            match(parser, TOKEN_KEYWORD_LET);
            context->parameter = matchAndYield(parser, TOKEN_IDENTIFIER);
            match(parser, TOKEN_COLON);
            context->expression = parseExpression(parser);
            context->blockStatement = parseBlockStatement(parser);

			break;
		}

        default: {
            // Error: Expected 'for' or 'while'
            reportAndRecover(parser, TOKEN_KEYWORD_WHILE);
            break;
        }
	}

    return context;
}

/*
 * tryStatement
 * :    tryClause catchClause* finallyClause?
 * ;
 *
 * tryClause
 * :    'try' blockStatement
 * ;
 *
 * finallyClause
 * :    'finally' blockStatement
 * ;
 */
k_TryStatement_t* parseTryStatement(k_Parser_t* parser) {
    k_TryStatement_t* context = k_TryStatement_new();
	bool hasCatch = false;
	bool hasFinally = false;

    match(parser, TOKEN_KEYWORD_TRY);
    context->tryClause = parseBlockStatement(parser);

	while ((la(parser, 1) == TOKEN_KEYWORD_CATCH)) {
		hasCatch = true;
		CatchClause* catchClause = parseCatchClause(parser);
        jtk_ArrayList_add(context->catchClauses, catchClause);
	}

	if (la(parser, 1) == TOKEN_KEYWORD_FINALLY) {
		hasFinally = true;
        consume(parser);
        context->finallyClause = parseBlockStatement(parser);
	}

	if (!hasCatch && !hasFinally) {
		/* Try clause without catch or finally. According to the grammar of KUSH,
         * this is not an error. However, the KUSH specification requires a try
         * clause to be followed by at least a catch or finally clause.
         */
        k_ErrorHandler_handleSyntacticalError(parser->compiler->errorHandler,
            parser, KUSH_ERROR_CODE_TRY_STATEMENT_EXPECTS_CATCH_OR_FINALLY,
            tryKeyword, TOKEN_UNKNOWN);
	}

    return context;
}

/*
 * catchClause
 * :	'catch' catchFilter? IDENTIFIER blockStatement
 * ;
 *
 * catchFilter
 * :	(STRING_LITERAL | IDENTIFIER) ('|' (STRING_LITERAL | IDENTIFIER))*
 * ;
 */
CatchClause* parseCatchClause(k_Parser_t* parser) {
    CatchClause* context = k_CatchClause_new();

    match(parser, TOKEN_KEYWORD_CATCH);

    if ((la(parser, 1) == TOKEN_STRING_LITERAL) ||
        ((la(parser, 1) == TOKEN_IDENTIFIER) && (la(parser, 2) == TOKEN_IDENTIFIER))) {
        TokenType validTokens = {
            TOKEN_STRING_LITERAL,
            TOKEN_IDENTIFIER
        };
        int32_t index;
        Token* capture = matchAndYieldEx(parser, validTokens, 2, &index);
        jtk_ArrayList_add(context->captures, capture);

        while (la(parser, 1) == TOKEN_VERTICAL_BAR) {
            /* Consume and discard the '|' token. */
            consume(parser);

		    capture = matchAndYieldEx(parser, validTokens, 2, &index);
            jtk_ArrayList_add(filter->captures, capture);
        }
	}

    context->parameter = matchAndYield(parser, TOKEN_IDENTIFIER);
    context->body = parseBlockStatement(parser);

    return context;
}

/*
 * structureDeclaration
 * :	'struct' IDENTIFIER structureBody
 * ;
 *
 * structureBody
 * :	'{' structureMember+ '}'
 * ;
 *
 * structureMember
 * :	variableDeclaration ';'
 * ;
 */
k_StructureDeclaration_t* parseStructureDeclaration(k_Parser_t* parser) {
    k_StructureDeclaration_t* context = k_StructureDeclaration_new();

    match(parser, TOKEN_KEYWORD_STRUCT);
    context->identifier = matchAndYield(parser, TOKEN_IDENTIFIER);

    match(parser, TOKEN_LEFT_BRACE);
    pushFollowToken(parser, TOKEN_RIGHT_BRACE);

    do {
        pushFollowToken(parser, TOKEN_SEMICOLON);
        VariableDeclaration* declaration = parseVariableDeclaration(parser, true, false); // typed, expression
        match(parser, TOKEN_SEMICOLON);
        popFollowToken(parser);
        jtk_ArrayList_add(context->variables, declaration);
    }
    while (isType(la(parser, 1)));

    popFollowToken(parser);
    match(parser, TOKEN_RIGHT_BRACE);

    return context;
}

/**
 * expressions
 * :    expression (',' expression)*
 * ;
 */
jtk_ArrayList_t* parseExpressions(k_Parser_t* parser) {
    jtk_ArrayList_t* expressions = jtk_ArrayList_new();

    k_Expression_t* expression = parseExpression(parser);
    jtk_ArrayList_add(expressions, expression);

    while (la(parser, 1) == TOKEN_COMMA) {
        consume(parser);
        expression = parseExpression(parser);
        jtk_ArrayList_add(expressions, expression);
    }

    return expressions;
}

/*
 * expression
 * :	assignmentExpression
 * ;
 */
BinaryExpression* parseExpression(k_Parser_t* parser) {
    return parseAssignmentExpression(parser);
}

/*
 * assignmentExpression
 * :	conditionalExpression (assignmentOperator assignmentExpression)?
 * ;
 */
BinaryExpression* parseAssignmentExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_AssignmentExpression_new();

    context->left = parseConditionalExpression(parser);

    if (isAssignmentOperator(la(parser, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->others, pair);

        pair->left = consumeAndYield(parser);
        pair->right = parseAssignmentExpression(parser);
    }

    return context;
}

/*
 * conditionalExpression
 * :	logicalOrExpression ('then' expression 'else' conditionalExpression)?
 * ;
 */
ConditionalExpression* parseConditionalExpression(k_Parser_t* parser) {
    ConditionalExpression* context = k_ConditionalExpression_new();
    context->logicalOrExpression = parseLogicalOrExpression(parser);

    if (la(parser, 1) == TOKEN_HOOK) {
        consume(parser);
        context->thenExpression = parseExpression(parser, expression);
        match(parser, TOKEN_COLON);
        context->elseExpression = parseConditionalExpression(parser);
    }

    return context;
}

/*
 * logicalOrExpression
 * :	logicalAndExpression ('||' logicalAndExpression)*
 * ;
 */
BinaryExpression* parseLogicalOrExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseLogicalAndExpression(parser);

    while (la(parser, 1) == TOKEN_VERTICAL_BAR_2) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseLogicalAndExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * logicalAndExpression
 * :	inclusiveOrExpression ('and' logicalAndExpression)?
 * ;
 */
BinaryExpression parseLogicalAndExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseInclusiveOrExpression(parser);

    while (la(parser, 1) == TOKEN_AMPERSAND_2) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseInclusiveOrExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * inclusiveOrExpression
 * :	exclusiveOrExpression ('|' exclusiveOrExpression)*
 * ;
 */
BinaryExpression* parseInclusiveOrExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseExclusiveOrExpression(parser);

    if (la(parser, 1) == TOKEN_VERTICAL_BAR) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseExclusiveOrExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * exclusiveOrExpression
 * :	andExpression ('^' andExpression)*
 * ;
 */
BinaryExpression* parseExclusiveOrExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseAndExpression(parser);

    while (la(parser, 1) == TOKEN_CARET) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseAndExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * andExpression
 * :	equalityExpression ('&' equalityExpression)*
 * ;
 */
BinaryExpression* parseAndExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseEqualityExpression(parser);

    while (la(parser, 1) == TOKEN_AMPERSAND) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseEqualityExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * equalityExpression
 * :	relationalExpression (equalityOperator relationalExpression)*
 * ;
 */
BinaryExpression* parseEqualityExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseRelationalExpression(parser, relationalExpression);

    while (isEqualityOperator(la(parser, 1))) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseRelationalExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * relationalExpression
 * :	shiftExpression (relationalOperator shiftExpression)*
 * ;
 */
BinaryExpression* parseRelationalExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseShiftExpression(parser);

    while (isRelationalOperator(la(parser, 1))) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseShiftExpression(parser, shiftExpression0);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * shiftExpression
 * :	additiveExpression (shiftOperator additiveExpression)*
 * ;
 */
BinaryExpression* parseShiftExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseAdditiveExpression(parser);

    while (isShiftOperator(la(parser, 1))) {
        // The following line doesn't work! You need to capture the operator in a pair.
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseAdditiveExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * additiveExpression
 * :	multiplicativeExpression (multiplicativeOperator multiplicativeExpression)*
 * ;
 */
BinaryExpression* parseAdditiveExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseMultiplicativeExpression(parser);

    while (isAdditiveOperator(la(parser, 1))) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseMultiplicativeExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * multiplicativeExpression
 * :	unaryExpression (multiplicativeOperator unaryExpression)*
 * ;
 */
BinaryExpression* parseMultiplicativeExpression(k_Parser_t* parser) {
    BinaryExpression* context = k_BinaryExpression_new();

    context->left = parseUnaryExpression(parser);

    while (isMultiplicativeOperator(la(parser, 1))) {
        context->operator = consumeAndYield(parser);
        BinaryExpression* right = parseUnaryExpression(parser);
        jtk_ArrayList_add(context->others, right);
    }

    return context;
}

/*
 * unaryExpression
 * :	unaryOperator unaryExpression
 * |	postfixExpression
 * ;
 */
UnaryExpression* parseUnaryExpression(k_Parser_t* parser) {
    UnaryExpression* context = k_UnaryExpression_new();

    TokenType la1 = la(parser, 1);
    if (isUnaryOperator(la1)) {
        context->operator = consumeAndYield(parser);
        context->expression = parseUnaryExpression(parser);
    }
    else if (isPostfixExpressionFollow(la1)) {
        context->expression = parsePostfixExpression(parser);
    }
    else {
        // Error: Expected unary operator or postfix expression follow
        reportAndRecover(parser, TOKEN_INTEGER_LITERAL);
    }

    return context;
}

/*
 * postfixExpression
 * :	primaryExpression postfixPart*
 * ;
 *
 * postfixPart
 * :    subscript
 * |    functionArguments
 * |    memberAccess
 * |    postfixOperator         -- future
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
PostfixExpression* parsePostfixExpression(k_Parser_t* parser) {
    PostfixExpression* context = k_PostfixExpression_new();

    context->primary = parsePrimaryExpression(parser,
        &context->primaryToken);

    TokenType la1 = la(parser, 1);
    while (isPostfixPartFollow(la1)) {
        switch (la1) {
            case TOKEN_LEFT_SQUARE_BRACKET: {
                void* subscript = parseSubscript(parser);
                jtk_ArrayList_add(context->postfixParts, subscript);
                break;
            }

            case TOKEN_LEFT_PARENTHESIS: {
                void* functionArguments = parseFunctionArguments(parser);
                jtk_ArrayList_add(context->postfixParts, functionArguments);
                break;
            }

            case TOKEN_DOT: {
                void* memberAccess = parseMemberAccess(parser);
                jtk_ArrayList_add(context->postfixParts, memberAccess);
                break;
            }
        }
        la1 = la(parser, 1);
    }

    return context;
}

/*
 * This function returns a pair of the form:
 * ( '[' token, assignment expression )
 *
 * subscript
 * :	'[' expression ']'
 * ;
 */
k_Subscript_t* parseSubscript(k_Parser_t* parser) {
    k_Subscript_t* context = k_Subscript_new();

    context->bracket = matchAndYield(parser, TOKEN_LEFT_SQUARE_BRACKET);
    pushFollowToken(parser, TOKEN_RIGHT_SQUARE_BRACKET);
    context->expression = parseExpression(parser);
    popFollowToken(parser);
    match(parser, TOKEN_RIGHT_SQUARE_BRACKET);

    return pair;
}

/*
 * This function returns a pair of the form:
 * [ '(' token, list of assignment expressions ].
 *
 * functionArguments
 * :	'(' expressions? ')'
 * ;
 */
FunctionArguments* parseFunctionArguments(k_Parser_t* parser) {
    FunctionArguments* context = k_FunctionArguments_new();

    match(context, TOKEN_LEFT_PARENTHESIS);

    if (isExpressionFollow(la(parser, 1))) {
        pushFollowToken(parser, TOKEN_RIGHT_PARENTHESIS);
        context->expressions = parseExpressions(parser);
        popFollowToken(parser);
    }
    match(parser, TOKEN_RIGHT_PARENTHESIS);

    return pair;
}

/*
 * This function returns a pair of the form:
 * ( '.' token, identifier token ).
 *
 * memberAccess
 * :	'.' IDENTIFIER
 * ;
 */
MemberAccess* parseMemberAccess(k_Parser_t* parser) {
    MemberAccess* context = k_MemberAccess_new();
    match(parser, TOKEN_DOT);
    context->identifier = matchAndYield(parser, TOKEN_IDENTIFIER);
    return context;
}

/*
 * This function returns either a context or a token.
 *
 * primaryExpression
 * :	IDENTIFIER
 * |	literal
 * |	'(' expression ')'
 * |	mapExpression
 * |	arrayExpression
 * ;
 *
 * literal
 * :	INTEGER_LITERAL
 * |	FLOATING_POINT_LITERAL
 * |	'true'
 * |	'false'
 * |	STRING_LITERAL
 * |	'null'
 * ;
 *
 * NOTE: The primaryExpression rule has no context. It simply forwards the
 * AST node it receives to the best matching child rule.
 */
void* parsePrimaryExpression(k_Parser_t* parser, bool* token) {
    void* result = NULL;
    *result = false;

    TokenType la1 = la(parser, 1);
    if (isLiteralFollow(la1)) {
        result = consumeAndYield(parser);
        *token = true;
    }
    else {
        switch (la1) {
            case TOKEN_IDENTIFIER: {
                result = consumeAndYield(parser);
                *token = true;
                break;
            }

            case TOKEN_LEFT_PARENTHESIS: {
                consume(parser);
                pushFollowToken(parser, TOKEN_RIGHT_PARENTHESIS);
                result = parseExpression(parser);
                popFollowToken(parser);
                match(parser, TOKEN_RIGHT_PARENTHESIS);
                break;
            }

            case TOKEN_LEFT_BRACE: {
                consume(parser);
                pushFollowToken(parser, TOKEN_RIGHT_BRACE);
                result = parseInitializerExpression(parser);
                popFollowToken(parser);
                break;
            }

            case TOKEN_LEFT_SQUARE_BRACKET: {
                result = parseArrayExpression(parser);
                break;
            }

            default: {
                printf("[internal error] Control should not reach here.\n");
                break;
            }
        }
    }

    return result;
}

/*
 * initializerExpression
 * :	'{' initializerEntries? '}'
 * ;
 *
 * initializerEntries
 * :	initializerEntry (',' initializerEntry)*
 * ;
 *
 * TODO: We can add an arbitary ',' in the end of a map, list, or an array.
 *		 Simply use the isExpressionFollow() function.
 */
InitializerExpression* parseInitializerExpression(k_Parser_t* parser) {
	InitializerExpression* context = k_InitializerExpression_new();

    match(parser, TOKEN_LEFT_BRACE);
    pushFollowToken(parser, TOKEN_RIGHT_BRACE);

    if (isExpressionFollow(la(parser, 1))) {
        jtk_Pair_t* entry = parseInitializerEntry(parser);
        jtk_ArrayList_add(context->entries, entry);

        while (la(parser, 1) == TOKEN_COMMA) {
            consume(parser);
            entry = parseInitializerEntry(parser);
            jtk_ArrayList_add(context->entries, entry);
        }
    }

    popFollowToken(parser);
    match(parser, TOKEN_RIGHT_BRACE);

    return context;
}

/*
 * initializerEntry
 * :	IDENTIFIER ':' expression
 * ;
 */
jtk_Pair_t* parseInitializerEntry(k_Parser_t* parser) {
    jtk_Pair_t* pair = jtk_Pair_new();
    pair->left = matchAndYield(parser, TOKEN_IDENTIFIER);
    match(parser, TOKEN_COLON);
    pair->right = parseExpression(parser);

    return pair;
}

/*
 * arrayExpression
 * :    '[' expressions ']'
 * ;
 */
ArrayExpression* parseArrayExpression(k_Parser_t* parser) {
    ArrayExpression* context = k_ArrayExpression_new();

    match(parser, TOKEN_LEFT_SQUARE_BRACKET);
    if (isExpressionFollow(la(parser, 1))) {
        pushFollowToken(parser, TOKEN_RIGHT_SQUARE_BRACKET);
        parseExpressions(parser, context->expressions);
        popFollowToken(parser);
    }
    match(parser, TOKEN_RIGHT_SQUARE_BRACKET);

    return context;
}