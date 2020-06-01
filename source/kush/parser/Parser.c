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

#include <kush/ast/context/Context.h>
#include <kush/parser/Parser.h>
#include <kush/lexer/TokenStream.h>
#include <kush/support/StackTrace.h>

/*******************************************************************************
 * Parser                                                                      *
 *******************************************************************************/

#define la(parser, count) k_TokenStream_la(parser->m_tokens, count)

// Match

static void match(k_Parser_t* parser, k_TokenType_t type);
static int32_t matchEx(k_Parser_t* parser, k_TokenType_t* types, int32_t count);
static k_Token_t* matchAndYield(k_Parser_t* parser, k_TokenType_t type);
static k_Token_t* matchAndYieldEx(k_Parser_t* parser, k_TokenType_t* types, int32_t count,
    int32_t* index);

// Recovery

static void pushFollowToken(k_Parser_t* parser, k_TokenType_t type);
static void popFollowToken(k_Parser_t* parser);
static void recover(k_Parser_t* parser);

// Rules

static bool isStatementFollow(k_TokenType_t type);
static bool isSimpleStatementFollow(k_TokenType_t type);
static bool isLiteral(k_TokenType_t type);
static bool isLiteralFollow(k_TokenType_t type);
static bool isCompoundStatementFollow(k_TokenType_t type);
static bool isClassMemberFollow(k_TokenType_t type);
static bool isClassMemberModifier(k_TokenType_t type);
static bool isExpressionFollow(k_TokenType_t type);
static bool isAssignmentOperator(k_TokenType_t type);
static bool isEqualityOperator(k_TokenType_t type);
static bool isRelationalOperator(k_TokenType_t type);
static bool isShiftOperator(k_TokenType_t type);
static bool isAdditiveOperator(k_TokenType_t type);
static bool isMultiplicativeOperator(k_TokenType_t type);
static bool isUnaryExpressionFollow(k_TokenType_t type);
static bool isUnaryOperator(k_TokenType_t type);
static bool isPostfixExpressionFollow(k_TokenType_t type);
static bool isPostfixPartFollow(k_TokenType_t type);
static bool isPrimaryExpressionFollow(k_TokenType_t type);

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
    "componentDeclaration",
    "functionDeclaration",
    "functionParameters",
    "functionBody",
    "blockStatement",
    "simpleStatement",
    "statement",
    "emptyStatement",
    "variableDeclaration",
    "variableDeclarator",
    "breakStatement",
    "returnStatement",
    "throwStatement",
    "compoundStatement",
    "ifStatement",
    "ifClause",
    "elseIfClause",
    "elseClause",
    "iterativeStatement",
    "labelClause",
    "whileStatement",
    "forStatement",
    "forParameters",
    "tryStatement",
    "tryClause",
    "catchClause",
    "catchFilter",
    "finallyClause",
    "structureDeclaration",
    "structureSuite",
    "structureMember",
    "expressions",
    "expression",
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
    "subscript",
    "functionArguments",
    "memberAccess",
    "postfixOperator",
    "primaryExpression",
    "initializerExpression",
    "initializerEntries",
    "initializerEntry",
    "arrayExpression"
};

/* Constructor */

k_Parser_t* k_Parser_new(k_Compiler_t* compiler, k_TokenStream_t* tokens) {
    jtk_Assert_assertObject(compiler, "The specified compiler is null.");

    k_Parser_t* parser = k_Memory_allocate(k_Parser_t, 1);
    parser->m_compiler = compiler;
    parser->m_tokens = tokens;
    parser->m_followSet = jtk_Memory_allocate(k_TokenType_t, 128);
    parser->m_followSetSize = 0;
    parser->m_followSetCapacity = 128;
    parser->m_recovery = false;

    return parser;
}

/* Destructor */

void k_Parser_delete(k_Parser_t* parser) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    jtk_Memory_deallocate(parser->m_followSet);
    jtk_Memory_deallocate(parser);
}

/* Rule Name */

const char* k_Parser_getRuleName(k_ASTNodeType_t type) {
    return ruleNames[(int32_t)type];
}

// Reset

void k_Parser_reset(k_Parser_t* parser, k_TokenStream_t* tokens) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    parser->m_tokens = tokens;
    parser->m_followSetSize = 0;
    parser->m_recovery = false;
}

/* Terminal Node */

static k_ASTNode_t* newTerminalNode(k_ASTNode_t* node, k_Token_t* token) {
    k_ASTNode_t* terminalNode = k_ASTNode_new();
    terminalNode->m_type = KUSH_AST_NODE_TYPE_TERMINAL;
    terminalNode->m_context = token;
    terminalNode->m_contextDestructor = NULL;
    terminalNode->m_enumerateContextChildren = NULL;

    return terminalNode;
}

/* Recover */

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
    parser->m_recovery = true;

    if (parser->m_followSetSize > 0) {
        k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
        /* The parser tries to recover until a token from the follow set or
         * the end-of-stream token is encountered.
         */
        while (lt1->m_type != KUSH_TOKEN_END_OF_STREAM) {
            /* When searching for a follow token, the parser prioritizes tokens
             * that are expected by the nearest rule in the rule invocation
             * stack. This is why, the linear search algorithm is applied in a
             * reverse fashion over the follow set.
             */
            int32_t i;
            for (i = parser->m_followSetSize - 1; i >= 0; i--) {
                if (lt1->m_type == parser->m_followSet[i]) {
                    /* A token from the follow set was encountered. The parser
                     * may have resynchronized with the input.
                     */
                    goto afterDiscard;
                }
            }
            /* Consume and discard the current token. */
            k_TokenStream_consume(parser->m_tokens);
            /* Update the lookahead token. */
            lt1 = k_TokenStream_lt(parser->m_tokens, 1);
        }
        afterDiscard:
            ;
    }
}

bool ensureFollowSetSpace(k_Parser_t* parser, int32_t capacity) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    bool result = false;
    if (capacity > 0) {
        int32_t currentCapacity = parser->m_followSetCapacity;
        int32_t minimumCapacity = (parser->m_followSetSize + capacity);
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
                uint8_t* temporary = parser->m_followSet;
                parser->m_followSet = (uint8_t*)jtk_Arrays_copyOfEx_b(
                    parser->m_followSet, parser->m_followSet, newCapacity, 0,
                    false);
                parser->m_followSetCapacity = newCapacity;
                jtk_Memory_deallocate(temporary);
            }
        }
    }
    return result;
}

void pushFollowToken(k_Parser_t* parser, k_TokenType_t type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    /* Make sure that the set is large enough to hold another token type. */
    ensureFollowSetSpace(parser, parser->m_followSetSize + 1);
    /* Insert the follow token to the set. */
    parser->m_followSet[parser->m_followSetSize] = type;
    /* Increment the size of the follow set. */
    parser->m_followSetSize++;
}

void popFollowToken(k_Parser_t* parser) {
    jtk_Assert_assertTrue(parser->m_followSetSize > 0, "The follow set is empty.");

    parser->m_followSetSize--;
}

/* Match */

void match(k_Parser_t* parser, k_TokenType_t type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    matchAndYield(parser, type);
}

int32_t matchEx(k_Parser_t* parser, k_TokenType_t* types, int32_t count) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    int32_t index;
    matchAndYieldEx(parser, types, count, &index);
}

void reportAndRecover(k_Parser_t* parser, k_TokenType_t expected) {
    /* Do not report the error if the parser is in recovery mode. Otherwise,
    * duplicate syntax errors will be reported to the end user.
    */
    if (!parser->m_recovery) {
        k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
        k_Compiler_t* compiler = parser->m_compiler;
        k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
        k_ErrorHandler_handleSyntacticalError(errorHandler, parser,
            KUSH_ERROR_CODE_UNEXPECTED_TOKEN, lt1, expected);
    }

    /* Try to resychronize the parser with the input. */
    recover(parser);
}


k_Token_t* matchAndYieldEx(k_Parser_t* parser, k_TokenType_t* types, int32_t count,
    int32_t* index) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");
    jtk_Assert_assertTrue(count > 0, "The specified count is invalid.");

    k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
    *index = -1;
    int32_t i;
    for (i = 0; i < count; i++) {
        if (lt1->m_type == types[i]) {
            *index = i;

            /* The token expected by the parser was found. If we the parser is
            * in error recovery, turn it off.
            */
            parser->m_recovery = false;

            /* The token stream prohibts consumption of end-of-stream
             * token.
             */
            if (lt1->m_type != KUSH_TOKEN_END_OF_STREAM) {
                k_TokenStream_consume(parser->m_tokens);
            }
        }
    }

    if (*index == -1) {
        reportAndRecover(parser, types[0]);
        lt1 = NULL;
    }

    return lt1;
}

k_Token_t* matchAndYield(k_Parser_t* parser, k_TokenType_t type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);

    if (lt1->m_type == type) {
        /* The token expected by the parser was found. If we the parser is
         * in error recovery, turn it off.
         */
        parser->m_recovery = false;


        /* The token stream prohibts consumption of end-of-stream
         * token.
         */
        if (lt1->m_type != KUSH_TOKEN_END_OF_STREAM) {
            k_TokenStream_consume(parser->m_tokens);
        }
    }
    else {
        reportAndRecover(parser, type);
    }
    return lt1;
}


static bool isReturnType(k_TokenType_t token) {
    return (token == KUSH_TOKEN_KEYWORD_VOID) ||
        (token == KUSH_TOKEN_KEYWORD_I8) ||
        (token == KUSH_TOKEN_KEYWORD_I16) ||
        (token == KUSH_TOKEN_KEYWORD_I32) ||
        (token == KUSH_TOKEN_KEYWORD_I64) ||
        (token == KUSH_TOKEN_KEYWORD_F32) ||
        (token == KUSH_TOKEN_KEYWORD_F64) ||
        (token == KUSH_TOKEN_IDENTIFIER);
}

bool isComponentFollow(k_TokenType_t token) {
    return (token == KUSH_TOKEN_KEYWORD_STRUCT) || isReturnType(token);
}

bool isType(k_TokenType_t token) {
    return (token == KUSH_TOKEN_KEYWORD_I8) ||
        (token == KUSH_TOKEN_KEYWORD_I16) ||
        (token == KUSH_TOKEN_KEYWORD_I32) ||
        (token == KUSH_TOKEN_KEYWORD_I64) ||
        (token == KUSH_TOKEN_KEYWORD_F32) ||
        (token == KUSH_TOKEN_KEYWORD_F64) ||
        (token == KUSH_TOKEN_IDENTIFIER);
}

bool isSimpleStatementFollow(k_TokenType_t type) {
    return (type == KUSH_TOKEN_SEMICOLON) || // emptyStatement
        (type == KUSH_TOKEN_KEYWORD_VAR) || // variableDeclaration
        (type == KUSH_TOKEN_KEYWORD_LET) || // variableDeclaration
        (type == KUSH_TOKEN_KEYWORD_BREAK) || // breakStatement
        (type == KUSH_TOKEN_KEYWORD_RETURN) || // returnStatement
        (type == KUSH_TOKEN_KEYWORD_THROW) || // throwStatement
        isExpressionFollow(type); // expressionStatement (includes IDENTIFIER, which may lead to variableDeclaration, too!)
}

bool isCompoundStatementFollow(k_TokenType_t type) {
    return (type == KUSH_TOKEN_KEYWORD_IF) || // ifStatement
		(type == KUSH_TOKEN_HASH) || // iterativeStatement
		(type == KUSH_TOKEN_KEYWORD_WHILE) || // whileStatement
		(type == KUSH_TOKEN_KEYWORD_FOR) || // forStatement
		(type == KUSH_TOKEN_KEYWORD_TRY); // tryStatement
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
    k_TokenType_t la1 = la(parser, 1);
    return (la1 == KUSH_TOKEN_KEYWORD_LET) ||
           (la1 == KUSH_TOKEN_KEYWORD_VAR) ||
           ((la1 == KUSH_TOKEN_IDENTIFIER) &&
            (((la(parser, 2) == KUSH_TOKEN_LEFT_SQUARE_BRACKET) && (la(parser, 3) == KUSH_TOKEN_RIGHT_SQUARE_BRACKET)) ||
            (la(parser, 2) == KUSH_TOKEN_IDENTIFIER)));
}

bool isStatementFollow(k_TokenType_t type) {
    return isSimpleStatementFollow(type) || isCompoundStatementFollow(type);
}

static k_TokenType_t returnTypes[] = {
    KUSH_TOKEN_KEYWORD_BOOLEAN,
    KUSH_TOKEN_KEYWORD_I8,
    KUSH_TOKEN_KEYWORD_I16,
    KUSH_TOKEN_KEYWORD_I32,
    KUSH_TOKEN_KEYWORD_I64,
    KUSH_TOKEN_KEYWORD_F32,
    KUSH_TOKEN_KEYWORD_F64,
    KUSH_TOKEN_KEYWORD_VOID,
    KUSH_TOKEN_IDENTIFIER
};

static k_TokenType_t types[] = {
    KUSH_TOKEN_KEYWORD_BOOLEAN,
    KUSH_TOKEN_KEYWORD_I8,
    KUSH_TOKEN_KEYWORD_I16,
    KUSH_TOKEN_KEYWORD_I32,
    KUSH_TOKEN_KEYWORD_I64,
    KUSH_TOKEN_KEYWORD_F32,
    KUSH_TOKEN_KEYWORD_F64,
    KUSH_TOKEN_IDENTIFIER
};

/*
 * -----------------------------------------------------------------------------
 * How do we construct the abstract syntax tree?
 * -----------------------------------------------------------------------------
 *
 * Every parent rule allocates and passes an AST node to a child
 * rule, except for the entry rules. In this case, the user has to
 * manually allocate an AST node and pass. A node is associated
 * with a context. A context has specific data related to a given
 * rule. It is recognized by the m_type attribute within the
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
 * long jumps using the m_contextDestructor function within
 * the k_ASTNode_t structure.
 *
 * The special function k_ASTNode_delete() should be invoked
 * on the root node. k_ASTNode_delete() travereses down the
 * AST destroying each node it encounters and all its descendants.
 * The context of a node is destroyed by invoking the
 * m_contextDestructor, provided that it is non-null. Otherwise,
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
    while (la(parser, 1) == KUSH_TOKEN_KEYWORD_IMPORT) {
		k_ImportDeclaration_t* importDeclaration = parseImportDeclaration(parser);
		jtk_ArrayList_add(context->m_imports, importDeclaration);
    }

    while (isComponentFollow(la(parser, 1))) {
        if (la(parser, 1) == KUSH_TOKEN_KEYWORD_STRUCT) {
            k_StructureDeclaration_t* structure = parseStructureDeclaration(parser);
            jtk_ArrayList_add(context->m_structures, structure);
        }
        else {
            k_FunctionDeclaration_t* function = parseFunctionDeclaration(parser, 0);
            jtk_ArrayList_add(context->m_structures, function);
        }
    }

	/* We are expecting the 'end of stream' token here. */
    match(parser, KUSH_TOKEN_END_OF_STREAM);

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
k_ImportDeclaration_t* parseImportDeclaration(k_Parser_t* parser) {


	k_ImportDeclaration_t* context = k_ImportDeclaration_new();

	/* An import statement begins with the 'import' keyword. Therefore,
	 * we are expecting it.
	 */
    match(parser, KUSH_TOKEN_KEYWORD_IMPORT);

    /* We are building an abstract syntax tree (AST). Therefore, we
	 * discard the '.' '*' tokens and use a boolean variable to mark
	 * wildcard imports. Thus, we uphold abstraction.
	 *
	 * By default, we assume that we have no wildcard.
	 */
	context->m_wildcard = false;

    /* If importTarget fails, discard tokens until the newline token is
     * encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_SEMICOLON);

	/* The user is expected to specify at least, one identifier.
	 * Consume it. The consumed identifier saved for later inspection.
	 */
    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
	jtk_ArrayList_add(context->m_identifiers, identifier);

	/* Optionally, the user may specify more identifiers (with each identifier
	 * separated by the '.' token. Therefore, we repeatedly consume the '.' and
	 * identifier tokens as long as LA(1) is the '.' token and LA(2) is the
	 * identifier token.
	 */
    while ((la(parser, 1) == KUSH_TOKEN_DOT) &&
           (k_TokenStream_la(parser->m_tokens, 2) == KUSH_TOKEN_IDENTIFIER)) {
		/* Consume and discard the '.' token. */
        k_TokenStream_consume(parser->m_tokens);

		/* The consumed identifier is saved for later inspection. */
        identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
		jtk_ArrayList_add(context->m_identifiers, identifier);
    }

	/* Optionally, the user may specify a wildcard; recognized when
	 * LA(1) is the '.' token and LA(2) is the '*' token.
	 */
    if (la(parser, 1) == KUSH_TOKEN_DOT) {
		k_TokenStream_consume(parser->m_tokens);
		match(parser, KUSH_TOKEN_ASTERISK);
        context->m_wildcard = true;
    }

    /* Pop the semicolon token from the follow set. */
    popFollowToken(parser);
	/* The import declaration is terminated with a semicolon.
	 * Therefore, we are expecting it here.
	 */
    match(parser, KUSH_TOKEN_SEMICOLON);

    return context;
}

k_Token_t* parseTypeEx(k_Parser_t* parser, int32_t* dimensions, bool includeVoid) {
    int32_t index;
    k_Token_t* token = matchAndYieldEx(parser, includeVoid? returnTypes : types,
        9, &index);

    *dimensions = 0;
    while (la(parser, 1) == KUSH_TOKEN_LEFT_SQUARE_BRACKET) {
        *dimensions++;
        match(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);
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
k_Token_t* parseType(k_Parser_t* parser, int32_t* dimensions) {
    return parseTypeEx(parser, dimensions, false);
}

/**
 * returnType
 * :    type
 * |    'void'
 * ;
 */
k_Token_t* parseReturnType(k_Parser_t* parser, int32_t* dimensions) {
    return parseTypeEx(parser, dimensions, true);
}

/*
 * functionDeclaration
 * :    returnType IDENTIFIER functionParameters (functionBody | SEMICOLON)
 * ;
 */
void parseFunctionDeclaration(k_Parser_t* parser, k_ASTNode_t* node,
    uint32_t modifiers) {
	/* If function parameters fails, skip tokens until ';', '{', or '}' is found. */
    pushFollowToken(parser, KUSH_TOKEN_SEMICOLON);
    pushFollowToken(parser, KUSH_TOKEN_LEFT_BRACE);
    pushFollowToken(parser, KUSH_TOKEN_RIGHT_BRACE);

    k_FunctionDeclaration_t* context = k_FunctionDeclaration_new();
    context->m_returnType = parseReturnType(parser, &context->m_returnTypeDimensions);
    context->m_identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_functionParameters = parseFunctionParameters(parser);

    /* Pop the ';', '{', and '}' tokens from the follow set. */
    popFollowToken(parser);
    popFollowToken(parser);
    popFollowToken(parser);

    if (k_Modifier_hasNative(modifiers)) {
        match(parser, KUSH_TOKEN_SEMICOLON);
    }
    else {
	    context->m_functionBody = parseBlockStatement(parser);
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
void parseFunctionParameters(k_Parser_t* parser) {
	k_FunctionParameters_t* context = k_FunctionParameters_new();

    /* Match and discard the '(' token. */
    match(parser, KUSH_TOKEN_LEFT_PARENTHESIS);

    k_TokenType_t la1 = la(parser, 1);
    if (isType(la1)) {
        bool first = true;
        do {
            if (!first) {
                match(parser, KUSH_TOKEN_COMMA);
            }

            k_FunctionParameter_t* parameter = k_FunctionParameter_new();
            parameter->m_type = parseType(parser, &parameter->m_dimensions);
            if (la(parser, 1) == KUSH_TOKEN_ELLIPSIS) {
                parameter->m_variable = true;
                match(parser, KUSH_TOKEN_ELLIPSIS);
            }
            parameter->m_identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
            jtk_ArrayList_add(context->m_fixedParameters, parameter);

            first = false;
        }
        while (la(parser, 1) == KUSH_TOKEN_COMMA);
    }

    /* Match and discard the ')' token. */
    match(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);


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
void parseBlockStatementk_Parser_t* parser) {
	k_StatementSuite_t* context = k_StatementSuite_new();

    /* Consume and discard the '{' token. */
    match(parser, KUSH_TOKEN_LEFT_BRACE);
    /* If statement fails, discard tokens until the '}' token is
     * encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_RIGHT_BRACE);

    do {
        k_TokenType_t la1 = la(parser, 1);
        if (isSimpleStatementFollow(la1)) {
            k_Context_t* statement = parseSimpleStatement(parser);
            jtk_ArrayList_add(context->m_statements, statement);
        }
        else if (isCompoundStatementFollow(la1)) {
            k_Context_t* statement = parseCompoundStatement(parser);
            jtk_ArrayList_add(context->m_statements, statement);
        }
        else {
            // TODO: Expected simple or compound statement
            reportAndRecover(parser, KUSH_TOKEN_KEYWORD_VAR);
        }
    }
    while (isStatementFollow(la(parser, 1)));

    /* Pop the '}' token from the follow set. */
    popFollowToken(parser);
    /* Consume and discard the '}' token. */
    match(parser, KUSH_TOKEN_RIGHT_BRACE);
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
k_Context_t* parseSimpleStatement(k_Parser_t* parser) {
    k_Context_t* result = NULL;

    /* If expressionStatement, emptyStatement, variableDeclaration,
     * breakStatement, returnStatement, or throwStatement fails, discard tokens
     * until the semicolon token is encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_SEMICOLON);

    k_TokenType_t la1 = la(parser, 1);
    if (followVariableDeclaration(parser)) {
        result = parseVariableDeclaration(parser);
    }
    else if (isExpressionFollow(la1)) {
        result = parseExpression(parser);
    }
    else {
        switch (la1) {
            case KUSH_TOKEN_SEMICOLON: {
                /* Match and discard the ';' token. An empty statement is not part
                 * of the AST.
                 */
                match(parser, KUSH_TOKEN_SEMICOLON);
                break;
            }

            case KUSH_TOKEN_KEYWORD_BREAK: {
                result = parseBreakStatement(parser);
                break;
            }

            case KUSH_TOKEN_KEYWORD_RETURN: {
                result = parseReturnStatement(parser);
                break;
            }

            case KUSH_TOKEN_KEYWORD_THROW: {
                result = parseThrowStatement(parser);
                break;
            }
        }
    }

    /* Pop the ';' token from the follow set. */
    popFollowToken(parser);
    /* Match and discard the newline token. */
	match(parser, KUSH_TOKEN_SEMICOLON);
}

/*
 * variableDeclaration
 * :    ('var' | 'let' | type) variableDeclarator (',' variableDeclarator)*
 * ;
 */
k_VariableDeclaration_t* parseVariableDeclaration(k_Parser_t* parser) {
	k_VariableDeclaration_t* context = k_VariableDeclaration_new();

    k_TokenType_t la1 = la(parser, 1);
    bool infer = (la1 == KUSH_TOKEN_KEYWORD_VAR);
    bool constant = (la1 == KUSH_TOKEN_KEYWORD_LET);
    k_Token_t* typeName = NULL;
    int32_t dimensions = -1;

    if (!infer && !constant) {
        dimensions = 0;
        typeName = parseType(parser, &dimensions);
    }

	k_StorageDeclarator_t* declarator = k_StorageDeclarator_new(infer, constant,
        typeName, dimensions, NULL);
    jtk_ArrayList_add(context->m_declarators, declarator);
	parseStorageDeclarator(parser, declarator);

	while (la(parser, 1) == KUSH_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

		declarator = k_StorageDeclarator_new(infer, constant,
        typeName, dimensions, NULL);
        jtk_ArrayList_add(context->m_declarators, declarator);
		parseStorageDeclarator(parser, declarator);
	}

    return context;
}

/*
 * variableDeclarator
 * :    IDENTIFIER ('=' expression)?
 * ;
 */
void parseStorageDeclarator(k_Parser_t* parser, k_StorageDeclarator_t* declarator) {
    declarator->m_identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);

	if (la(parser, 1) == KUSH_TOKEN_EQUAL) {
        /* Consume and discard the '=' token. */
		k_TokenStream_consume(parser->m_tokens);

		k_ASTNode_t* expression = k_ASTNode_new(NULL);
        declarator->m_expression = expression;
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
    match(parser, KUSH_TOKEN_KEYWORD_BREAK);

    if (la(parser, 1) == KUSH_TOKEN_IDENTIFIER) {
        k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_identifier = newTerminalNode(node, identifier);
        k_TokenStream_consume(parser->m_tokens);
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
    match(parser, KUSH_TOKEN_KEYWORD_RETURN);
    /* An expression is mandatory after the 'return' keyword. */
    context->m_expression = parseExpression(parser);

    return context;
}

/*
 * throwStatement
 * :    'throw' expression?
 * ;
 */
k_ThrowStatement_t* parseThrowStatement(k_Parser_t* parser) {
    k_ThrowStatement_t* context = k_ThrowStatement_new();

    /* Match and discard the 'throw' token. */
    match(parser, KUSH_TOKEN_KEYWORD_THROW);

    if (isExpressionFollow(la(parser, 1))) {
        context->m_expression = parseExpression(parser);
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
k_Context_t* parseCompoundStatement(k_Parser_t* parser) {
    k_Context_t* context = NULL;

	switch (la(parser, 1)) {
		case KUSH_TOKEN_KEYWORD_IF: {
            context = parseIfStatement(parser);
			break;
		}

		case KUSH_TOKEN_HASH:
		case KUSH_TOKEN_KEYWORD_WHILE:
		case KUSH_TOKEN_KEYWORD_FOR: {
            context = parseIterativeStatement(parser);
			break;
		}

		case KUSH_TOKEN_KEYWORD_TRY: {
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
 */
k_IfStatement_t* parseIfStatement(k_Parser_t* parser) {
    k_IfStatement_t* context = k_IfStatement_new();

    // ifClause
    context->m_ifClause = parseIfClause(parser, ifClause);

    // elseIfClause*
	while ((la(parser, 1) == KUSH_TOKEN_KEYWORD_ELSE) &&
	       (k_TokenStream_la(parser->m_tokens, 2) == KUSH_TOKEN_KEYWORD_IF)) {
		k_IfClause_t* elseIfClause = parseElseIfClause(parser);
        jtk_ArrayList_add(context->m_elseIfClauses, elseIfClause);
	}

    // elseClause?
    if (la(parser, 1) == KUSH_TOKEN_KEYWORD_ELSE) {
        context->m_elseClause = parseElseClause(parser, elseClause);
    }

    return context;
}

/*
 * ifClause
 * :    'if' expression blockStatement
 * ;
 */
k_IfClause_t* parseIfClause(k_Parser_t* parser) {
    k_IfClause_t* context = k_IfClause_new();

	match(parser, KUSH_TOKEN_KEYWORD_IF);
    context->m_expression = parseExpression(parser);
    context->m_blockStatement = parseBlockStatement(parser);

    return context;
}

/* elseIfClause
 * :    'else' 'if' expression blockStatement
 * ;
 */
k_IfClause_t* parseElseIfClause(k_Parser_t* parser) {
    k_IfClause_t* context = k_IfClause_new();

	match(parser, KUSH_TOKEN_KEYWORD_ELSE);
	match(parser, KUSH_TOKEN_KEYWORD_IF);
    context->m_expression = parseExpression(parser);
    context->m_blockStatement = parseBlockStatement(parser);

    return context;
}

/*
 * elseClause
 * :    'else' blockStatement
 * ;
 */
k_IfClause_t parseElseClause(k_Parser_t* parser) {
    k_ElseClause_t* context = k_IfClause_new();

	match(parser, KUSH_TOKEN_KEYWORD_ELSE);
    context->m_blockStatement = parseBlockStatement(parser);

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

	if (la(parser, 1) == KUSH_TOKEN_HASH) {
        consume(parser);
	    context->m_label = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
	}

	switch (la(parser, 1)) {
		case KUSH_TOKEN_KEYWORD_WHILE: {
            context->m_while = true;
            consume(parser);
            context->m_expression = parseExpression(parser);
            context->m_blockStatement = parseBlockStatement(parser);

			break;
		}

		case KUSH_TOKEN_KEYWORD_FOR: {
            context->m_while = false;
            consume(parser);
            match(parser, KUSH_TOKEN_KEYWORD_LET);
            context->m_parameter = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
            match(parser, KUSH_TOKEN_COLON);
            context->m_expression = parseExpression(parser);
            context->m_blockStatement = parseBlockStatement(parser);

			break;
		}

        default: {
            // Error: Expected 'for' or 'while'
            reportAndRecover(parser, KUSH_TOKEN_KEYWORD_WHILE);
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
 * finallyClause
 * :    'finally' blockStatement
 * ;
 */
k_TryStatement_t* parseTryStatement(k_Parser_t* parser) {
    k_TryStatement_t* context = k_TryStatement_new();
	bool hasCatch = false;
	bool hasFinally = false;

    match(parser->m_tokens, KUSH_TOKEN_KEYWORD_TRY);
    context->m_tryClause = parseTryClause(parser, tryClause);

	while ((la(parser, 1) == KUSH_TOKEN_KEYWORD_CATCH)) {
		hasCatch = true;
		k_CatchClause_t* catchClause = parseCatchClause(parser);
        jtk_ArrayList_add(context->m_catchClauses, catchClause);
	}

	if (la(parser, 1) == KUSH_TOKEN_KEYWORD_FINALLY) {
		hasFinally = true;
        consume(parser);
        context->m_finallyClause = parseBlockStatement(parser);
	}

	if (!hasCatch && !hasFinally) {
		/* Try clause without catch or finally. According to the grammar of KUSH,
         * this is not an error. However, the KUSH specification requires a try
         * clause to be followed by at least a catch or finally clause.
         */
        k_ErrorHandler_handleSyntacticalError(parser->m_compiler->m_errorHandler,
            parser, KUSH_ERROR_CODE_TRY_STATEMENT_EXPECTS_CATCH_OR_FINALLY,
            tryKeyword, KUSH_TOKEN_UNKNOWN);
	}

    return context;
}

/*
 * tryClause
 * :    'try' blockStatement
 * ;
 */
void parseTryClause(k_Parser_t* parser) {


    k_TryClause_t* context = k_TryClause_new();

    /* Match and discard the 'try' token. */
	match(parser, KUSH_TOKEN_KEYWORD_TRY);

	k_ASTNode_t* blockStatement = k_ASTNode_new();
    context->m_blockStatement = blockStatement;
	parseBlockStatement(parser);


}

struct k_CatchFilter_t {
    jtk_ArrayList_t* m_captures;
    k_Token_t* m_identifier;
};

typedef struct k_CatchFilter_t k_CatchFilter_t;

/*
 * catchClause
 * :	'catch' catchFilter? IDENTIFIER blockStatement
 * ;
 *
 * catchFilter
 * :	(STRING_LITERAL | IDENTIFIER) ('|' (STRING_LITERAL | IDENTIFIER))*
 * ;
 */
void parseCatchClause(k_Parser_t* parser) {


    k_CatchClause_t* context = k_CatchClause_new();

    /* Match and discard the 'catch' token. */
    match(parser, KUSH_TOKEN_KEYWORD_CATCH);

    k_CatchFilter_t* filter = k_CatchFilter_new();
    if ((la(parser, 1) == KUSH_TOKEN_STRING_LITERAL) ||
        ((la(parser, 1) == KUSH_TOKEN_IDENTIFIER) && (la(parser, 2) == KUSH_TOKEN_IDENTIFIER))) {
        k_TokenType_t validTokens = {
            KUSH_TOKEN_STRING_LITERAL,
            KUSH_TOKEN_IDENTIFIER
        };
        int32_t index;
        k_Token_t* capture = matchAndYieldEx(parser, validTokens, 2, &index);
        jtk_ArrayList_add(filter->m_captures, capture);

        while (la(parser, 1) == KUSH_TOKEN_VERTICAL_BAR) {
            /* Consume and discard the '|' token. */
            k_TokenStream_consume(parser->m_tokens);

		    capture = matchAndYieldEx(parser, validTokens, 2, &index);
            jtk_ArrayList_add(filter->m_captures, capture);
        }
	}

    filter->m_identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_catchFilter = filter;

    k_ASTNode_t* blockStatement = k_ASTNode_new();
    context->m_blockStatement = blockStatement;
    parseBlockStatement(parser);


}

/*
 * finallyClause
 * :	'finally' blockStatement
 * ;
 */
void parseFinallyClause(k_Parser_t* parser) {


    k_FinallyClause_t* context = k_FinallyClause_new();

    /* Match and discard the 'finally' token. */
	match(parser, KUSH_TOKEN_KEYWORD_FINALLY);

	k_ASTNode_t* blockStatement = k_ASTNode_new();
    context->m_blockStatement = blockStatement;
	parseBlockStatement(parser);


}

/*
 * structureDeclaration
 * :	'struct' IDENTIFIER structureBody
 * ;
 */
void parseStructureDeclaration(k_Parser_t* parser) {


    k_ClassDeclaration_t* context = k_ClassDeclaration_new();

    /* Match and discard the 'struct' token. */
    match(parser, KUSH_TOKEN_KEYWORD_STRUCT);

    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);

    k_ASTNode_t* structureSuite = k_ASTNode_new();
    context->m_structureSuite = structureSuite;
    parseStructureSuite(parser, structureSuite);


}

/*
 * structureSuite
 * :	'{' structureMember+ '}'
 * ;
 */
void parseStructureSuite(k_Parser_t* parser) {


    k_ClassSuite_t* context = k_ClassSuite_new();

    /* Match and discard the left brace token. */
    match(parser, KUSH_TOKEN_LEFT_BRACE);

    do {
        k_ASTNode_t* structureMember = k_ASTNode_new();
        jtk_ArrayList_add(context->m_classMembers, structureMember);
        parseStructureMember(parser, structureMember);
    }
    while (isType(la(parser, 1)));

    /* Match and discard the right brace token. */
    match(parser, KUSH_TOKEN_RIGHT_BRACE);


}

/*
 * structureMember
 * :	variableDeclaration ';'
 * ;
 */
void parseStructureMember(k_Parser_t* parser) {


    k_ClassMember_t* context = k_ClassMember_new();

    /* If parseVariableDeclaration( ) fails, discard tokens until the semicolon token is encountered. */
    pushFollowToken(parser, KUSH_TOKEN_SEMICOLON);

    // TODO: variableDeclaration should not accept initializers!
    k_ASTNode_t* variableDeclaration = k_ASTNode_new();
    context->m_declaration = variableDeclaration;
    parseVariableDeclaration(parser, variableDeclaration);

    /* Match and discard the newline token. */
    match(parser, KUSH_TOKEN_SEMICOLON);

    /* Pop the newline token from the follow set. */
    popFollowToken(parser);


}

/*
 * expressions
 * :    expression (',' expression)*
 * ;
 */
void parseExpressions(k_Parser_t* parser) {


    k_Expressions_t* context = k_Expressions_new();

    k_ASTNode_t* expression = k_ASTNode_new();
    jtk_ArrayList_add(context->m_expressions, expression);
    parseExpression(parser, expression);

    while (la(parser, 1) == KUSH_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        expression = k_ASTNode_new();
        jtk_ArrayList_add(context->m_expressions, expression);
        parseExpression(parser, expression);
    }


}

/*
 * expression
 * :	parseAssignmentExpression(
 * ;
 *
 */
void parseExpression(k_Parser_t* parser) {


    k_Expression_t* context = k_Expression_new();

    k_ASTNode_t* assignmentExpression = k_ASTNode_new();
    context->m_assignmentExpression = assignmentExpression;
    parseAssignmentExpression(parser, parseAssignmentExpression);
}

bool isExpressionFollow(k_TokenType_t type) {
    return isUnaryExpressionFollow(type);
}

/*
 * parseAssignmentExpression(
 * :	conditionalExpression (assignmentOperator parseAssignmentExpression()?
 * ;
 */
void parseAssignmentExpression(k_Parser_t* parser) {


    k_AssignmentExpression_t* context = k_AssignmentExpression_new();

    k_ASTNode_t* conditionalExpression = k_ASTNode_new();
    context->m_conditionalExpression = conditionalExpression;
    parseConditionalExpression(parser, conditionalExpression);

    if (isAssignmentOperator(la(parser, 1))) {
        k_Token_t* assignmentOperator = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_assignmentOperator = newTerminalNode(node, assignmentOperator);
        /* Consume the assignment operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* assignmentExpression = k_ASTNode_new();
        context->m_assignmentExpression = parseAssignmentExpression;
        parseAssignmentExpression(parser, parseAssignmentExpression);
    }


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
bool isAssignmentOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_EQUAL) ||
           (type == KUSH_TOKEN_ASTERISK_EQUAL) ||
           (type == KUSH_TOKEN_FORWARD_SLASH_EQUAL) ||
           (type == KUSH_TOKEN_MODULUS_EQUAL) ||
           (type == KUSH_TOKEN_PLUS_EQUAL) ||
           (type == KUSH_TOKEN_DASH_EQUAL) ||
           (type == KUSH_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL) ||
           (type == KUSH_TOKEN_AMPERSAND_EQUAL) ||
           (type == KUSH_TOKEN_CARET_EQUAL) ||
           (type == KUSH_TOKEN_VERTICAL_BAR_EQUAL);
}

/*
 * conditionalExpression
 * :	logicalOrExpression ('then' expression 'else' conditionalExpression)?
 * ;
 */
void parseConditionalExpression(k_Parser_t* parser) {


    k_ConditionalExpression_t* context = k_ConditionalExpression_new();

    k_ASTNode_t* logicalOrExpression = k_ASTNode_new();
    context->m_logicalOrExpression = logicalOrExpression;
    parseLogicalOrExpression(parser, logicalOrExpression);

    if (la(parser, 1) == KUSH_TOKEN_HOOK) {
        /* Consume and discard the '?' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* expression = k_ASTNode_new();
        context->m_thenExpression = expression;
        parseExpression(parser, expression);

		/* Consume and discard the ':' token. */
        match(parser, KUSH_TOKEN_COLON);

        k_ASTNode_t* conditionalExpression = k_ASTNode_new();
        context->m_elseExpression = conditionalExpression;
        parseConditionalExpression(parser, conditionalExpression);
    }


}

/*
 * logicalOrExpression
 * :	logicalAndExpression ('or' logicalAndExpression)*
 * ;
 */
void parseLogicalOrExpression(k_Parser_t* parser) {


    k_LogicalOrExpression_t* context = k_LogicalOrExpression_new();

    k_ASTNode_t* logicalAndExpression = k_ASTNode_new();
    context->m_logicalAndExpression = logicalAndExpression;
    parseLogicalAndExpression(parser, logicalAndExpression);

    while (la(parser, 1) == KUSH_TOKEN_VERTICAL_BAR_2) {
        /* Consume and discard the 'or' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* logicalAndExpression0 = k_ASTNode_new();
        jtk_ArrayList_add(context->m_logicalAndExpressions, logicalAndExpression0);
        parseLogicalAndExpression(parser, logicalAndExpression0);
    }


}

/*
 * logicalAndExpression
 * :	inclusiveOrExpression ('and' logicalAndExpression)?
 * ;
 */
void parseLogicalAndExpression(k_Parser_t* parser) {


    k_LogicalAndExpression_t* context = k_LogicalAndExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* inclusiveOrExpression = k_ASTNode_new();
    context->m_inclusiveOrExpression = inclusiveOrExpression;
    parseInclusiveOrExpression(parser, inclusiveOrExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (la(parser, 1) == KUSH_TOKEN_AMPERSAND_2) {
        /* Consume and discard the 'and' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* inclusiveOrExpression0 = k_ASTNode_new();
        jtk_ArrayList_add(context->m_inclusiveOrExpressions, inclusiveOrExpression0);
        parseInclusiveOrExpression(parser, inclusiveOrExpression0);
    }


}

/*
 * inclusiveOrExpression
 * :	exclusiveOrExpression ('|' exclusiveOrExpression)*
 * ;
 */
void parseInclusiveOrExpression(k_Parser_t* parser) {


    k_InclusiveOrExpression_t* context = k_InclusiveOrExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* exclusiveOrExpression = k_ASTNode_new();
    context->m_exclusiveOrExpression = exclusiveOrExpression;
    parseExclusiveOrExpression(parser, exclusiveOrExpression);

    /* Parse the expression to the right of the operator, if any. */
    if (la(parser, 1) == KUSH_TOKEN_VERTICAL_BAR) {
        /* Consume and discard the '|' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* exclusiveOrExpression0 = k_ASTNode_new();
        jtk_ArrayList_add(context->m_exclusiveOrExpressions, exclusiveOrExpression0);
        parseExclusiveOrExpression(parser, exclusiveOrExpression0);
    }


}

/*
 * exclusiveOrExpression
 * :	andExpression ('^' andExpression)*
 * ;
 */
void parseExclusiveOrExpression(k_Parser_t* parser) {


    k_ExclusiveOrExpression_t* context = k_ExclusiveOrExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* andExpression = k_ASTNode_new();
    context->m_andExpression = andExpression;
    parseAndExpression(parser, andExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (la(parser, 1) == KUSH_TOKEN_CARET) {
        /* Consume and discard the '^' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* andExpression0 = k_ASTNode_new();
        jtk_ArrayList_add(context->m_andExpressions, andExpression0);
        parseAndExpression(parser, andExpression0);
    }


}

/*
 * andExpression
 * :	equalityExpression ('&' equalityExpression)*
 * ;
 */
void parseAndExpression(k_Parser_t* parser) {


    k_AndExpression_t* context = k_AndExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* equalityExpression = k_ASTNode_new();
    context->m_equalityExpression = equalityExpression;
    parseEqualityExpression(parser, equalityExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (la(parser, 1) == KUSH_TOKEN_AMPERSAND) {

        /* Consume and discard the '&' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* equalityExpression0 = k_ASTNode_new();
        jtk_ArrayList_add(context->m_equalityExpressions, equalityExpression0);
        parseEqualityExpression(parser, equalityExpression0);
    }


}

/*
 * equalityExpression
 * :	relationalExpression (equalityOperator relationalExpression)*
 * ;
 */
void parseEqualityExpression(k_Parser_t* parser) {


    k_EqualityExpression_t* context = k_EqualityExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* relationalExpression = k_ASTNode_new();
    context->m_relationalExpression = relationalExpression;
    parseRelationalExpression(parser, relationalExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isEqualityOperator(la(parser, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_relationalExpressions, pair);

        k_Token_t* equalityOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* equalityOperator = newTerminalNode(node, equalityOperatorToken);
        pair->m_left = equalityOperator;
        /* Consume the equality operator. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* relationalExpression0 = k_ASTNode_new();
        pair->m_right = relationalExpression0;
        parseRelationalExpression(parser, relationalExpression0);
    }


}

/*
 * equalityOperator
 * :	'=='
 * |	'!='
 * ;
 */
bool isEqualityOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_EQUAL_2) ||
           (type == KUSH_TOKEN_EXCLAMATION_MARK_EQUAL);
}

/*
 * relationalExpression
 * :	shiftExpression (relationalOperator shiftExpression)*
 * ;
 */
void parseRelationalExpression(k_Parser_t* parser) {


    k_RelationalExpression_t* context = k_RelationalExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* shiftExpression = k_ASTNode_new();
    context->m_shiftExpression = shiftExpression;
    parseShiftExpression(parser, shiftExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isRelationalOperator(la(parser, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_shiftExpressions, pair);

        k_Token_t* relationalOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* relationalOperator = newTerminalNode(node, relationalOperatorToken);
        pair->m_left = relationalOperator;
        /* Consume the relational operator. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* shiftExpression0 = k_ASTNode_new();
        pair->m_right = shiftExpression0;
        parseShiftExpression(parser, shiftExpression0);
    }


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
bool isRelationalOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_LEFT_ANGLE_BRACKET) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET) ||
           (type == KUSH_TOKEN_LEFT_ANGLE_BRACKET_EQUAL) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_EQUAL);
}

/*
 * shiftExpression
 * :	additiveExpression (shiftOperator additiveExpression)*
 * ;
 */
void parseShiftExpression(k_Parser_t* parser) {


    k_ShiftExpression_t* context = k_ShiftExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* additiveExpression = k_ASTNode_new();
    context->m_additiveExpression = additiveExpression;
    parseAdditiveExpression(parser, additiveExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isShiftOperator(la(parser, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_additiveExpressions, pair);

        k_Token_t* shiftOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* shiftOperator = newTerminalNode(node, shiftOperatorToken);
        pair->m_left = shiftOperator;
        /* Consume the shift operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* additiveExpression0 = k_ASTNode_new();
        pair->m_right = additiveExpression0;
        parseAdditiveExpression(parser, additiveExpression0);
    }


}

/*
 * shiftOperator
 * :	'<<'
 * |	'>>'
 * |	'>>>'
 * ;
 */
bool isShiftOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_LEFT_ANGLE_BRACKET_2) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_2) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_3);
}

/*
 * additiveExpression
 * :	multiplicativeExpression (multiplicativeOperator multiplicativeExpression)*
 * ;
 */
void parseAdditiveExpression(k_Parser_t* parser) {


    k_AdditiveExpression_t* context = k_AdditiveExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* multiplicativeExpression = k_ASTNode_new();
    context->m_multiplicativeExpression = multiplicativeExpression;
    parseMultiplicativeExpression(parser, multiplicativeExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isAdditiveOperator(la(parser, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_multiplicativeExpressions, pair);

        k_Token_t* additiveOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* additiveOperator = newTerminalNode(node, additiveOperatorToken);
        pair->m_left = additiveOperator;
        /* Consume the additive operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* multiplicativeExpression0 = k_ASTNode_new();
        pair->m_right = multiplicativeExpression0;
        parseMultiplicativeExpression(parser, multiplicativeExpression0);
    }


}

/*
 * additiveOperator
 * :	'+'
 * |	'-'
 * ;
 */
bool isAdditiveOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_PLUS) ||
           (type == KUSH_TOKEN_DASH);
}

/*
 * multiplicativeExpression
 * :	unaryExpression (multiplicativeOperator unaryExpression)*
 * ;
 */
void parseMultiplicativeExpression(k_Parser_t* parser) {


    k_MultiplicativeExpression_t* context = k_MultiplicativeExpression_new();

    /* Parse the operand left of the operator. */
    k_ASTNode_t* unaryExpression = k_ASTNode_new();
    context->m_unaryExpression = unaryExpression;
    parseUnaryExpression(parser, unaryExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isMultiplicativeOperator(la(parser, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_unaryExpressions, pair);

        k_Token_t* multiplicativeOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* multiplicativeOperator = newTerminalNode(node, multiplicativeOperatorToken);
        pair->m_left = multiplicativeOperator;
        /* Consume the multiplicative operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* unaryExpression0 = k_ASTNode_new();
        pair->m_right = unaryExpression0;
        parseUnaryExpression(parser, unaryExpression0);
    }


}

/*
 * multiplicativeOperator
 * :	'*'
 * |	'/'
 * |	'%'
 * ;
 */
bool isMultiplicativeOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_ASTERISK) ||
           (type == KUSH_TOKEN_FORWARD_SLASH) ||
           (type == KUSH_TOKEN_MODULUS);
}

/*
 * unaryExpression
 * :	unaryOperator unaryExpression
 * |	postfixExpression
 * ;
 */
void parseUnaryExpression(k_Parser_t* parser) {


    k_UnaryExpression_t* context = k_UnaryExpression_new();

    k_TokenType_t la1 = la(parser, 1);
    if (isUnaryOperator(la1)) {
        k_Token_t* unaryOperator = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_unaryOperator = newTerminalNode(node, unaryOperator);
        /* Consume the unary operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* unaryExpression = k_ASTNode_new();
        context->m_unaryExpression = unaryExpression;
        parseUnaryExpression(parser, unaryExpression);
    }
    else if (isPostfixExpressionFollow(la1)) {
        k_ASTNode_t* postfixExpression = k_ASTNode_new();
        context->m_postfixExpression = postfixExpression;
        parsePostfixExpression(parser, postfixExpression);
    }
    else {
        // Error: Expected unary operator or postfix expression follow
        reportAndRecover(parser, KUSH_TOKEN_INTEGER_LITERAL);
    }


}

bool isUnaryExpressionFollow(k_TokenType_t type) {
    return isUnaryOperator(type) || isPostfixExpressionFollow(type);
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
bool isUnaryOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_PLUS) ||
           (type == KUSH_TOKEN_DASH) ||
           (type == KUSH_TOKEN_TILDE) ||
           (type == KUSH_TOKEN_EXCLAMATION_MARK);
           /*
           ||
           (type == KUSH_TOKEN_PLUS_2) ||
           (type == KUSH_TOKEN_DASH_2);
           */
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
 * |    parsePostfixOperator(
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void parsePostfixExpression(k_Parser_t* parser) {


    k_PostfixExpression_t* context = k_PostfixExpression_new();

    k_ASTNode_t* primaryExpression = k_ASTNode_new();
    context->m_primaryExpression = primaryExpression;
    parsePrimaryExpression(parser, primaryExpression);

    k_TokenType_t la1 = la(parser, 1);
    while (isPostfixPartFollow(la1)) {
        switch (la1) {
            case KUSH_TOKEN_LEFT_SQUARE_BRACKET: {
                k_ASTNode_t* subscript = k_ASTNode_new();
                jtk_ArrayList_add(context->m_postfixParts, subscript);
                parseSubscript(parser, subscript);
                break;
            }

            case KUSH_TOKEN_LEFT_PARENTHESIS: {
                k_ASTNode_t* functionArguments = k_ASTNode_new();
                jtk_ArrayList_add(context->m_postfixParts, functionArguments);

                parseFunctionArguments(parser, functionArguments);

                break;
            }

            case KUSH_TOKEN_DOT: {
                k_ASTNode_t* memberAccess = k_ASTNode_new();
                jtk_ArrayList_add(context->m_postfixParts, memberAccess);
                parseMemberAccess(parser, memberAccess);
                break;
            }

            // case KUSH_TOKEN_PLUS_2:
            // case KUSH_TOKEN_DASH_2: {
            //     k_ASTNode_t* parsePostfixOperator( = k_ASTNode_new();
            //     jtk_ArrayList_add(context->m_postfixParts, parsePostfixOperator();
            //     parsePostfixOperator((parser, parsePostfixOperator();
            //     break;
            // }
        }
        la1 = la(parser, 1);
    }


}

bool isPostfixExpressionFollow(k_TokenType_t type) {
    return isPrimaryExpressionFollow(type);
}

bool isPostfixPartFollow(k_TokenType_t type) {
    return (type == KUSH_TOKEN_LEFT_SQUARE_BRACKET) ||
           (type == KUSH_TOKEN_LEFT_PARENTHESIS) ||
           (type == KUSH_TOKEN_DOT) ||
           (type == KUSH_TOKEN_PLUS_2) ||
           (type == KUSH_TOKEN_DASH_2);
}

/*
 * subscript
 * :	'[' expression ']'
 * ;
 */
void parseSubscript(k_Parser_t* parser) {


    k_Subscript_t* context = k_Subscript_new();

    /* Match and discard the '[' token. */
    match(parser, KUSH_TOKEN_LEFT_SQUARE_BRACKET);

    /* If expression fails, discard tokens until the ']' token is
     * encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);

    k_ASTNode_t* expression = k_ASTNode_new();
    context->m_expression = expression;
    parseExpression(parser, expression);

    /* Pop the ']' token from the follow set. */
    popFollowToken(parser);

    /* Match and discard the ']' token. */
    match(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);


}

/*
 * functionArguments
 * :	'(' expressions? ')'
 * ;
 */
void parseFunctionArguments(k_Parser_t* parser) {


    k_FunctionArguments_t* context = k_FunctionArguments_new();

    /* Match and discard the '(' token. */
    match(parser, KUSH_TOKEN_LEFT_PARENTHESIS);

    if (isExpressionFollow(la(parser, 1))) {
        /* If expression fails, discard tokens until the ')' token is
        * encountered.
        */
        pushFollowToken(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

        k_ASTNode_t* expressions = k_ASTNode_new();
        context->m_expressions = expressions;
        parseExpressions(parser, expressions);

        /* Pop the ')' token from the follow set. */
        popFollowToken(parser);
    }

    /* Match and discard the ')' token. */
    match(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);


}

/*
 * memberAccess
 * :	'.' IDENTIFIER
 * ;
 */
void parseMemberAccess(k_Parser_t* parser) {


    k_MemberAccess_t* context = k_MemberAccess_new();

    /* Match and discard the '.' token. */
    match(parser, KUSH_TOKEN_DOT);

    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);


}

/*
 * parsePostfixOperator(
 * :	'++'
 * |	'--'
 * ;
 */
void parsePostfixOperator(k_Parser_t* parser) {


    k_PostfixOperator_t* context = k_PostfixOperator_new();

    switch (la(parser, 1)) {
        case KUSH_TOKEN_PLUS_2:
        case KUSH_TOKEN_DASH_2: {
            k_Token_t* postfixOperator = k_TokenStream_lt(parser->m_tokens, 1);
            context->m_postfixOperator = newTerminalNode(node, postfixOperator);
            /* Consume the '++' or '--' token. */
            k_TokenStream_consume(parser->m_tokens);

            break;
        }

        default: {
            // Syntax error
        }
    }


}

/*
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
 * |    'this'
 * ;
 *
 * NOTE: The primaryExpression rule has no context. It simply forwards the
 * AST node it receives to the best matching child rule.
 *
 */
void parsePrimaryExpression(k_Parser_t* parser) {


    k_PrimaryExpression_t* context = k_PrimaryExpression_new();
    k_TokenType_t la1 = la(parser, 1);
    if (isLiteralFollow(la1)) {
        k_Token_t* literal = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_expression = newTerminalNode(node, literal);
        /* Consume the literal token. */
        k_TokenStream_consume(parser->m_tokens);
    }
    else {
        switch (la1) {
            case KUSH_TOKEN_IDENTIFIER: {
                k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
                context->m_expression = newTerminalNode(node, identifier);
                /* Consume the identifier token. */
                k_TokenStream_consume(parser->m_tokens);

                break;
            }

            case KUSH_TOKEN_LEFT_PARENTHESIS: {
                /* Consume and discard the '(' token. */
                k_TokenStream_consume(parser->m_tokens);

                /* If expression fails, discard tokens until the ')' token is
                 * encountered.
                 */
                pushFollowToken(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

                k_ASTNode_t* expression = k_ASTNode_new();
                context->m_expression = expression;
                parseExpression(parser, expression);

                /* Pop the ')' token from the follow set. */
                popFollowToken(parser);

                /* Match and discard the ')' token. */
                match(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

                break;
            }

            case KUSH_TOKEN_LEFT_BRACE: {
                k_ASTNode_t* mapExpression = k_ASTNode_new();
                context->m_expression = mapExpression;
                parseInitializerExpression(parser, mapExpression);

                break;
            }

            case KUSH_TOKEN_LEFT_SQUARE_BRACKET: {
                k_ASTNode_t* arrayExpression = k_ASTNode_new();
                context->m_expression = arrayExpression;
                parseArrayExpression(parser, arrayExpression);

                break;
            }

            case KUSH_TOKEN_KEYWORD_THIS: {
                k_Token_t* keyword = k_TokenStream_lt(parser->m_tokens, 1);
                context->m_expression = newTerminalNode(node, keyword);
                /* Consume the this keyword. */
                k_TokenStream_consume(parser->m_tokens);

                break;
            }

            default: {
                printf("[internal error] Control should not reach here.\n");

                break;
            }
        }
    }


}

bool isPrimaryExpressionFollow(k_TokenType_t type) {
    bool result = false;
    if (isLiteralFollow(type)) {
        result = true;
    }
    else {
        switch (type) {
            case KUSH_TOKEN_KEYWORD_THIS:
            case KUSH_TOKEN_IDENTIFIER:
            case KUSH_TOKEN_LEFT_PARENTHESIS:
            case KUSH_TOKEN_LEFT_BRACE:
            case KUSH_TOKEN_LEFT_SQUARE_BRACKET:
            case KUSH_TOKEN_LEFT_ANGLE_BRACKET:
            case KUSH_TOKEN_KEYWORD_NEW: {
                result = true;
                break;
            }
        }
    }
    return result;
}

bool isLiteral(k_TokenType_t type) {
    bool result = false;
    switch (type) {
        case KUSH_TOKEN_INTEGER_LITERAL:
        case KUSH_TOKEN_FLOATING_POINT_LITERAL:
        case KUSH_TOKEN_KEYWORD_TRUE:
        case KUSH_TOKEN_KEYWORD_FALSE:
        case KUSH_TOKEN_STRING_LITERAL:
        case KUSH_TOKEN_KEYWORD_NULL: {
            result = true;
            break;
        }
    }
    return result;
}

bool isLiteralFollow(k_TokenType_t type) {
    return isLiteral(type);
}

/*
 * mapExpression
 * :	'{' mapEntries? '}'
 * ;
 */
/*
 * TODO: We can add an arbitary ',' in the end of a map, list, or an array.
 *		 Simply use the isExpressionFollow() function.
 */
void parseInitializerExpression(k_Parser_t* parser) {


	k_MapExpression_t* context = k_MapExpression_new();

    /* Match and discard the '{' token. */
    match(parser, KUSH_TOKEN_LEFT_BRACE);

    /* If expression fails, discard tokens until the '}' token is
     * encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_RIGHT_BRACE);

    if (isExpressionFollow(la(parser, 1))) {
        k_ASTNode_t* mapEntries = k_ASTNode_new();
        context->m_mapEntries = mapEntries;
        parseInitializerEntries(parser, mapEntries);
    }

    /* Pop the '}' token from the follow set. */
    popFollowToken(parser);

    /* Match and discard the '}' token. */
    match(parser, KUSH_TOKEN_RIGHT_BRACE);


}

/*
 * mapEntries
 * :	mapEntry (',' mapEntry)*
 * ;
 */
void parseInitializerEntries(k_Parser_t* parser) {


    k_MapEntries_t* context = k_MapEntries_new();

    k_ASTNode_t* mapEntry = k_ASTNode_new();
    jtk_ArrayList_add(context->m_mapEntries, mapEntry);
    parseInitializerEntry(parser, mapEntry);

    while (la(parser, 1) == KUSH_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        mapEntry = k_ASTNode_new();
        jtk_ArrayList_add(context->m_mapEntries, mapEntry);
        parseInitializerEntry(parser, mapEntry);
    }


}

/*
 * mapEntry
 * :	IDENTIFIER ':' expression
 * ;
 */
void parseInitializerEntry(k_Parser_t* parser) {


    k_MapEntry_t* context = k_MapEntry_new();

    // TODO: The key should be an identifier.
    k_ASTNode_t* keyExpression = k_ASTNode_new();
    context->m_keyExpression = keyExpression;
    parseExpression(parser, keyExpression);

    /* Match and discard the ':' token. */
    match(parser, KUSH_TOKEN_COLON);

    k_ASTNode_t* valueExpression = k_ASTNode_new();
    context->m_valueExpression = valueExpression;
    parseExpression(parser, valueExpression);


}

/*
 * arrayExpression
 * :    '[' expressions ']'
 * ;
 */
void parseArrayExpression(k_Parser_t* parser) {


    k_ListExpression_t* context = k_ListExpression_new();

    /* Match and discard the '[' token. */
    match(parser, KUSH_TOKEN_LEFT_SQUARE_BRACKET);

    if (isExpressionFollow(la(parser, 1))) {
        /* If expression fails, discard tokens until the ')' token is
        * encountered.
        */
        pushFollowToken(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);

        k_ASTNode_t* expressions = k_ASTNode_new();
        context->m_expressions = expressions;
        parseExpressions(parser, expressions);

        /* Pop the ']' token from the follow set. */
        popFollowToken(parser);
    }

    /* Match and discard the ']' token. */
    match(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);


}