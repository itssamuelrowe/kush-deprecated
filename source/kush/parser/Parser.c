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
static void parseVariableDeclarator(k_Parser_t* parser, k_ASTNode_t* node);
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
    "statementSuite",
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
    k_ASTNode_t* terminalNode = k_ASTNode_new(node);
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
 *      annotatedComponentDeclaration*
 *      EOF
 * ;
 *
 */
void parseCompilationUnit(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	/* Create and attach the context of this rule to the given node. */
	k_CompilationUnitContext_t* context = k_CompilationUnitContext_new(node);

    /* A newline token may optionally appear at the beginning of the source
     * code. Such arbitrary newline tokens are recognized here in order
     * to parse inputs with such construct.
     *
     * // This is a comment.
     * function main
     *     ...
     */
    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_NEWLINE) {
        /* Consume the newline token. */
        k_TokenStream_consume(parser->m_tokens);
    }

	/* Zero or more import declarations may occur in the source code.
	 * Therefore, repeatedly parse import declarations as long as
	 * LA(1) is the 'import' keyword.
	 *
	 * The following code represents: importDeclaration*
	 */
    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_KEYWORD_IMPORT) {
		k_ASTNode_t* importDeclaration = k_ASTNode_new(node);
        parseImportDeclaration( parser, importDeclaration);

		jtk_ArrayList_add(context->m_importDeclarations, importDeclaration);
    }

    while (isComponentFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_ASTNode_t* declaration = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_annotatedComponentDeclarations, declaration);
        parseComponentDeclaration( parser, declaration);
    }

	/* We are expecting an 'end of stream' here. */
    match(parser, KUSH_TOKEN_END_OF_STREAM);

    k_StackTrace_exit();
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
void parseImportDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_ImportDeclarationContext_t* context = k_ImportDeclarationContext_new(node);

	/* An import statement begins with the 'import' keyword. Therefore,
	 * we are expecting it.
	 */
    match(parser, KUSH_TOKEN_KEYWORD_IMPORT);

	/* The user is required to specify the symbol he/she wants to
	 * import. The following code parses the import target.
	 */

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
    pushFollowToken(parser, KUSH_TOKEN_NEWLINE);

	/* The user is expected to specify at least, one identifier.
	 * Consume it. The consumed identifier saved for later inspection.
	 */
    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifierNode = newTerminalNode(node, identifier);

	jtk_ArrayList_add(context->m_identifiers, identifierNode);

	/* Optionally, the user may specify more identifiers (with each identifier
	 * separated by the '.' token. Therefore, we repeatedly consume the '.' and
	 * identifier tokens as long as LA(1) is the '.' token and LA(2) is the
	 * identifier token.
	 */
    while ((k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_DOT) &&
           (k_TokenStream_la(parser->m_tokens, 2) == KUSH_TOKEN_IDENTIFIER)) {
		/* Consume and discard the '.' token. */
        k_TokenStream_consume(parser->m_tokens);

		/* The consumed identifier is saved for later inspection.
		 */
        identifier = k_TokenStream_lt(parser->m_tokens, 1);
        identifierNode = newTerminalNode(node, identifier);
        k_TokenStream_consume(parser->m_tokens);
		jtk_ArrayList_add(context->m_identifiers, identifierNode);
    }

	/* Optionally, the user may specify a wildcard; recognized when
	 * LA(1) is the '.' token and LA(2) is the '*' token.
	 */
    if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_DOT) {
		k_TokenStream_consume(parser->m_tokens);
		match(parser, KUSH_TOKEN_ASTERISK);
        context->m_wildcard = true;
    }

    /* Pop the newline token from the follow set. */
    popFollowToken(parser);

	/* The import declaration is terminated with a newline.
	 * Therefore, we are expecting it here.
	 */
    match(parser, KUSH_TOKEN_NEWLINE);

    k_StackTrace_exit();
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

/*
 * componentDeclaration
 * :    functionDeclaration
 * |    structureDeclaration
 * ;
 */
void parseComponentDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_Compiler_t* compiler = parser->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ComponentDeclarationContext_t* context = k_ComponentDeclarationContext_new(node);

    k_ASTNodeType_t component = KUSH_AST_NODE_TYPE_UNKNOWN;
    k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
    if (lt1->m_type == KUSH_TOKEN_KEYWORD_STRUCT) {
        k_ASTNode_t* declaration = k_ASTNode_new(node);
        context->m_component = declaration;
        parseStructureDeclaration(parser, declaration);
    }
    else {
        k_ASTNode_t* declaration = k_ASTNode_new(node);
        context->m_component = declaration;
        parseFunctionDeclaration( parser, declaration, 0);
    }

    k_StackTrace_exit();
}

/*
 * functionDeclaration
 * :    returnType IDENTIFIER functionParameters (functionBody | SEMICOLON)
 * ;
 */
void parseFunctionDeclaration(k_Parser_t* parser, k_ASTNode_t* node,
    uint32_t modifiers) {
    k_StackTrace_enter();

	k_FunctionDeclarationContext_t* context = k_FunctionDeclarationContext_new(node);

    // Consume and discard the return type
    // TODO: Save the return type!
    k_TokenStream_consume(parser->m_tokens);

    switch (k_TokenStream_la(parser->m_tokens, 1)) {
        case KUSH_TOKEN_IDENTIFIER: {
            k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
            context->m_identifier = newTerminalNode(node, identifier);
            k_TokenStream_consume(parser->m_tokens);
            break;
        }

        default: {
            reportAndRecover(parser, KUSH_TOKEN_IDENTIFIER);
        }
    }

    /* If functionParameters fails, discard tokens until the newline token is
     * encountered.
     *
     * TODO
     */
    // pushFollowToken(parser, KUSH_TOKEN_NEWLINE);

    k_ASTNode_t* parameters = k_ASTNode_new(node);
    context->m_functionParameters = parameters;
    parseFunctionParameters( parser, parameters);

    /* Pop the newline token from the follow set. */
    popFollowToken(parser);

    if (!k_Modifier_hasNative(modifiers)) {
        k_ASTNode_t* body = k_ASTNode_new(node);
        context->m_functionBody = body;
        parseFunctionBody( parser, body);
    }
    else {
        match(parser, KUSH_TOKEN_SEMICOLON);
    }

    k_StackTrace_exit();
}

void type(k_Parser_t* parser) {
}

bool isType(k_TokenType_t token) {
    return true;
}

/*
 *  functionParameters
 *  :   '(' actualFunctionParameters? ')'
 *  ;
 *
 * actualFunctionParameters
 * :    IDENTIFIER (',' IDENTIFIER)* (',' '...' IDENTIFIER)?
 * |    '...' IDENTIFIER
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void parseFunctionParameters(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_FunctionParametersContext_t* context = k_FunctionParametersContext_new(node);

    /* Match and discard the '(' token. */
    match(parser, KUSH_TOKEN_LEFT_PARENTHESIS);

    /* Optionally, the programmer may specify one or more parameters.
     * The following statements parse both fixed and variable parameters.
     */

    type(parser);

    switch (k_TokenStream_la(parser->m_tokens, 1)) {
        case KUSH_TOKEN_IDENTIFIER: {
            k_Token_t* fixedParameter = k_TokenStream_lt(parser->m_tokens, 1);
            k_ASTNode_t* fixedParameterNode = newTerminalNode(node, fixedParameter);
			jtk_ArrayList_add(context->m_fixedParameters, fixedParameterNode);
			k_TokenStream_consume(parser->m_tokens);

            while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_COMMA) {
                /* Consume and discard the ',' token. */
                k_TokenStream_consume(parser->m_tokens);
                switch (k_TokenStream_la(parser->m_tokens, 1)) {
                    case KUSH_TOKEN_IDENTIFIER: {
                        fixedParameter = k_TokenStream_lt(parser->m_tokens, 1);
                        fixedParameterNode = newTerminalNode(node, fixedParameter);
						jtk_ArrayList_add(context->m_fixedParameters, fixedParameterNode);
                        k_TokenStream_consume(parser->m_tokens);
                        break;
                    }

                    case KUSH_TOKEN_ELLIPSIS: {
						/* Consume the '...' token. */
                        k_TokenStream_consume(parser->m_tokens);
                        k_Token_t* variableParameter = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
						context->m_variableParameter = newTerminalNode(node, variableParameter);
                        goto loopExit;
                    }

					default: {
                        reportAndRecover(parser, KUSH_TOKEN_IDENTIFIER);
                        break;
					}
                }
            }
            loopExit:
            break;
        }

        case KUSH_TOKEN_ELLIPSIS: {
			/* Consume and discard the '...' token. */
            k_TokenStream_consume(parser->m_tokens);
            k_Token_t* variableParameter = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
			context->m_variableParameter = newTerminalNode(node, variableParameter);
            break;
        }
    }

    /* Match and discard the ')' token. */
    match(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

    k_StackTrace_exit();
}

/* TODO: Merge this with parseFunctionDeclaration( )
 *
 * functionBody
 * :    statementSuite
 * ;
 */
void parseFunctionBody(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_FunctionBodyContext_t* context = k_FunctionBodyContext_new(node);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
	context->m_statementSuite = statementSuite;
    blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/* In order to help the users read code easily, the simple statements were
 * removed from statement suites. Previously, a statement suite was either
 * a simple statement or a block consisting of a newline at the begining,
 * indentation and dedentation.
 */
/*
 * blockStatement
 * |    '{' statement+ '}'
 * ;
 */
void blockStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_StatementSuiteContext_t* context = k_StatementSuiteContext_new(node);

    /* Consume and discard the dedent token. */
    match(parser, KUSH_TOKEN_LEFT_BRACE);

    /* If statement fails, discard tokens until the dedentation token is
        * encountered.
        */
    pushFollowToken(parser, KUSH_TOKEN_DEDENTATION);

    do {
        k_ASTNode_t* node = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_statements, node);
        parseStatement( parser, node);
    }
    while (isStatementFollow(k_TokenStream_la(parser->m_tokens, 1)));

    /* Pop the dedentation token from the follow set. */
    popFollowToken(parser);

    /* Consume and discard the dedent token. */
    match(parser, KUSH_TOKEN_RIGHT_BRACE);

    k_StackTrace_exit();
}

bool isSimpleStatementFollow(k_TokenType_t type) {
    bool result = false;
    switch (type) {
        case KUSH_TOKEN_SEMICOLON: /* simpleStatement -> emptyStatement */
        case KUSH_TOKEN_KEYWORD_VAR: /* simpleStatement -> variableDeclaration */
        case KUSH_TOKEN_KEYWORD_LET: /* simpleStatement -> variableDeclaration */
        case KUSH_TOKEN_KEYWORD_BREAK: /* simpleStatement -> breakStatement */
        case KUSH_TOKEN_KEYWORD_RETURN: /* simpleStatement -> returnStatement */
        case KUSH_TOKEN_KEYWORD_THROW: /* simpleStatement -> throwStatement */
        {
            result = true;
            break;
        }

        default: {
            result = k_Parser_isExpressionFollow(type); /* simpleStatement -> expressionStatement */
            break;
        }
    }
    return result;
}

/*
 * simpleStatement
 * :    unterminatedSimpleStatement NEWLINE
 * ;
 *
 * unterminatedSimpleStatement
 * :    expressionStatement
 * |    emptyStatement
 * |    variableDeclaration
 * |    constantDeclaration
 * |    assertStatement
 * |    breakStatement
 * |    continueStatement
 * |    returnStatement
 * |    throwStatement
 * ;
 *
 * expressionStatement
 * :    expression
 * ;
 *
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void parseSimpleStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_SimpleStatementContext_t* context = k_SimpleStatementContext_new(node);

    /* If expressionStatement, emptyStatement, variableDeclaration,
     * constantDeclaration, assertStatement, breakStatement, continueStatement,
     * returnStatement, or throwStatement fails, discard tokens until the newline
     * token is encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_NEWLINE);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (k_Parser_isExpressionFollow(la1)) {
        k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_statement = expression;
        parseExpression(parser, expression);
    }
    else {
        switch (la1) {
            case KUSH_TOKEN_SEMICOLON: {
                k_ASTNode_t* emptyStatement = k_ASTNode_new(node);
                context->m_statement = emptyStatement;
                parseEmptyStatement( parser, emptyStatement);
                break;
            }

            // TODO: Look ahead and determine if it's a declaration or expression
            case KUSH_TOKEN_IDENTIFIER:
            case KUSH_TOKEN_KEYWORD_VAR:
            case KUSH_TOKEN_KEYWORD_LET: {
                k_ASTNode_t* variableDeclaration = k_ASTNode_new(node);
                context->m_statement = variableDeclaration;
                parseVariableDeclaration( parser, variableDeclaration);
                break;
            }

            case KUSH_TOKEN_KEYWORD_BREAK: {
                k_ASTNode_t* breakStatement = k_ASTNode_new(node);
                context->m_statement = breakStatement;
                parseBreakStatement( parser, breakStatement);
                break;
            }

            case KUSH_TOKEN_KEYWORD_RETURN: {
                k_ASTNode_t* returnStatement = k_ASTNode_new(node);
                context->m_statement = returnStatement;
                parseReturnStatement( parser, returnStatement);
                break;
            }

            case KUSH_TOKEN_KEYWORD_THROW: {
                k_ASTNode_t* throwStatement = k_ASTNode_new(node);
                context->m_statement = throwStatement;
                parseThrowStatement( parser, throwStatement);
                break;
            }
        }
    }

    /* Pop the newline token from the follow set. */
    popFollowToken(parser);

    /* Match and discard the newline token. */
	match(parser, KUSH_TOKEN_NEWLINE);

    k_StackTrace_exit();
}

/*
 * statement
 * :    simpleStatement
 * |    compoundStatement
 * ;
 */
void parseStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_StatementContext_t* context = k_StatementContext_new(node);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (isSimpleStatementFollow(la1)) {
        k_ASTNode_t* simpleStatement = k_ASTNode_new(node);
        context->m_simpleStatement = simpleStatement;
        parseSimpleStatement( parser, simpleStatement);
    }
    else if (isCompoundStatementFollow(la1)) {
        k_ASTNode_t* compoundStatement = k_ASTNode_new(node);
        context->m_compoundStatement = compoundStatement;
        parseCompoundStatement( parser, compoundStatement);
    }
    else {
        // TODO: Expected simple or compound statement
        reportAndRecover(parser, KUSH_TOKEN_KEYWORD_VAR);
    }

    k_StackTrace_exit();
}

bool isStatementFollow(k_TokenType_t type) {
    return isSimpleStatementFollow(type) || isCompoundStatementFollow(type);
}

/*
 * emptyStatement
 * :    ';'
 * ;
 */
void parseEmptyStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    /* k_EmptyStatementContext_t* context = */ k_EmptyStatementContext_new(node);

    /* The empty statement rule has no context. */
    /* Match and discard the ';' token. */
	match(parser, KUSH_TOKEN_SEMICOLON);

    k_StackTrace_exit();
}

int32_t matchEx(k_Parser_t* parser, k_TokenType_t* types, int32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        // TODO
    }
    return i;
}

/*
 * variableDeclaration
 * :    'var' variableDeclarator (',' variableDeclarator)*
 * ;
 */
void parseVariableDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_VariableDeclarationContext_t* context = k_VariableDeclarationContext_new(node);

    /* Match and discard the 'var' token. */
    k_TokenType_t start = {
        KUSH_TOKEN_KEYWORD_VAR,
        KUSH_TOKEN_KEYWORD_LET,
        KUSH_TOKEN_IDENTIFIER
    };
    int32_t keywordUsed = matchEx(parser, start, 3);

	k_ASTNode_t* variableDeclarator = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_variableDeclarators, variableDeclarator);
	parseVariableDeclarator( parser, variableDeclarator);

	while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

		variableDeclarator = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_variableDeclarators, variableDeclarator);
		parseVariableDeclarator( parser, variableDeclarator);
	}

    k_StackTrace_exit();
}

/*
 * variableDeclarator
 * :    IDENTIFIER ('=' expression)?
 * ;
 */
void parseVariableDeclarator(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_VariableDeclaratorContext_t* context = k_VariableDeclaratorContext_new(node);

    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);

	if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_EQUAL) {
        /* Consume and discard the '=' token. */
		k_TokenStream_consume(parser->m_tokens);

		k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_expression = expression;
		parseExpression(parser, expression);
	}

    k_StackTrace_exit();
}

/*
 * breakStatement
 * :    'break' IDENTIFIER?
 * ;
 */
void parseBreakStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_BreakStatementContext_t* context = k_BreakStatementContext_new(node);

    /* Match and discard the 'break' token. */
    match(parser, KUSH_TOKEN_KEYWORD_BREAK);

    if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_IDENTIFIER) {
        k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_identifier = newTerminalNode(node, identifier);
        k_TokenStream_consume(parser->m_tokens);
    }

    k_StackTrace_exit();
}

/*
 * returnStatement
 * :    'return' expression
 * ;
 */
void parseReturnStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ReturnStatementContext_t* context = k_ReturnStatementContext_new(node);

    /* Match and discard the 'return' token. */
    match(parser, KUSH_TOKEN_KEYWORD_RETURN);

    /* An expression is mandatory after the 'return' keyword. */
    k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
    parseExpression(parser, expression);

    k_StackTrace_exit();
}

/*
 * throwStatement
 * :    'throw' expression?
 * ;
 */
void parseThrowStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_ThrowStatementContext_t* context = k_ThrowStatementContext_new(node);

    /* Match and discard the 'throw' token. */
    match(parser, KUSH_TOKEN_KEYWORD_THROW);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_expression = expression;
        parseExpression(parser, expression);
    }

    k_StackTrace_exit();
}

/*
 * compoundStatement
 * :    ifStatement
 * |    iterativeStatement
 * |    tryStatement
 * ;
 */
void parseCompoundStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_CompoundStatementContext_t* context = k_CompoundStatementContext_new(node);

	switch (k_TokenStream_la(parser->m_tokens, 1)) {
		case KUSH_TOKEN_KEYWORD_IF: {
			k_ASTNode_t* ifStatement = k_ASTNode_new(node);
            context->m_statement = ifStatement;
			parseIfStatement( parser, ifStatement);
			break;
		}

		case KUSH_TOKEN_HASH:
		case KUSH_TOKEN_KEYWORD_WHILE:
		case KUSH_TOKEN_KEYWORD_FOR: {
			k_ASTNode_t* iterativeStatement = k_ASTNode_new(node);
            context->m_statement = iterativeStatement;
			parseIterativeStatement( parser, iterativeStatement);
			break;
		}

		case KUSH_TOKEN_KEYWORD_TRY: {
			k_ASTNode_t* tryStatement = k_ASTNode_new(node);
            context->m_statement = tryStatement;
			parseTryStatement( parser, tryStatement);
			break;
		}

		default: {
			printf("[internal error] Control should not reach here.\n");
			break;
		}
	}

    k_StackTrace_exit();
}

bool isCompoundStatementFollow(k_TokenType_t type) {
    bool result = false;
    switch (type) {
		case KUSH_TOKEN_KEYWORD_IF: /* compoundStatement -> ifStatement */
		case KUSH_TOKEN_HASH: /* compoundStatement -> iterativeStatement */
		case KUSH_TOKEN_KEYWORD_WHILE: /* compoundStatement -> ... -> whileStatement */
		case KUSH_TOKEN_KEYWORD_FOR: /* compoundStatement -> ... -> forStatement */
		case KUSH_TOKEN_KEYWORD_TRY: /* compoundStatement -> tryStatement */
		{
            result = true;
			break;
		}
	}
    return result;
}

/*
 * ifStatement
 * :    ifClause elseIfClause* elseClause?
 * ;
 */
void parseIfStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_IfStatementContext_t* context = k_IfStatementContext_new(node);

	k_ASTNode_t* ifClause = k_ASTNode_new(node);
    context->m_ifClause = ifClause;
	parseIfClause( parser, ifClause);

	while ((k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_KEYWORD_ELSE) &&
	       (k_TokenStream_la(parser->m_tokens, 2) == KUSH_TOKEN_KEYWORD_IF)) {
		k_ASTNode_t* elseIfClause = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_elseIfClauses, elseIfClause);
		parseElseIfClause( parser, elseIfClause);
	}

    if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_KEYWORD_ELSE) {
        k_ASTNode_t* elseClause = k_ASTNode_new(node);
        context->m_elseClause = elseClause;
        parseElseClause( parser, elseClause);
    }

    k_StackTrace_exit();
}

/*
 * ifClause
 * :    'if' expression statementSuite
 * ;
 */
void parseIfClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_IfClauseContext_t* context = k_IfClauseContext_new(node);

    /* Match and discard the 'if' token. */
	match(parser, KUSH_TOKEN_KEYWORD_IF);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	parseExpression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/* TODO: reuse IfClauseContext*
 *
 * elseIfClause
 * :    'else' 'if' expression statementSuite
 * ;
 */
void parseElseIfClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ElseIfClauseContext_t* context = k_ElseIfClauseContext_new(node);

    /* Match and discard the 'else' 'if' tokens. */
	match(parser, KUSH_TOKEN_KEYWORD_ELSE);
	match(parser, KUSH_TOKEN_KEYWORD_IF);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	parseExpression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * elseClause
 * :    'else' statementSuite
 * ;
 */
void parseElseClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ElseClauseContext_t* context = k_ElseClauseContext_new(node);

    /* Match and discard the 'else' token. */
	match(parser, KUSH_TOKEN_KEYWORD_ELSE);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * iterativeStatement
 * :    labelClause? (whileStatement | forStatement)
 * ;
 */
void parseIterativeStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_IterativeStatementContext_t* context = k_IterativeStatementContext_new(node);

	if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_HASH) {
		k_ASTNode_t* labelClause = k_ASTNode_new(node);
        context->m_labelClause = labelClause;
		k_Parser_parseLabelClause( parser, labelClause);
	}

	switch (k_TokenStream_la(parser->m_tokens, 1)) {
		case KUSH_TOKEN_KEYWORD_WHILE: {
			k_ASTNode_t* whileStatement = k_ASTNode_new(node);
            context->m_statement = whileStatement;
			parseWhileStatement( parser, whileStatement);
			break;
		}

		case KUSH_TOKEN_KEYWORD_FOR: {
			k_ASTNode_t* forStatement = k_ASTNode_new(node);
            context->m_statement = forStatement;
			parseForStatement( parser, forStatement);
			break;
		}
	}

    k_StackTrace_exit();
}

/*
 * labelClause
 * :    '#' IDENTIFIER
 * ;
 */
void k_Parser_parseLabelClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_LabelClauseContext_t* context = k_LabelClauseContext_new(node);

    /* Match and discard the '#' token. */
	match(parser, KUSH_TOKEN_HASH);

	k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);

    k_StackTrace_exit();
}

/*
 * whileStatement
 * :    'while' expression statementSuite
 * ;
 */
void parseWhileStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_WhileStatementContext_t* context = k_WhileStatementContext_new(node);

    /* Match and discard the 'while' token. */
	match(parser, KUSH_TOKEN_KEYWORD_WHILE);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	parseExpression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

// TODO: Move this rule to forStatement!
/*
 * forParameter
 * :    'let' IDENTIFIER
 * ;
 */
void parseForParameter(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ForParameterContext_t* context = k_ForParameterContext_new(node);

    match(parser, KUSH_TOKEN_KEYWORD_LET);
    k_Token_t* identifierToken = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifier = newTerminalNode(parser, identifierToken);
    context->m_identifier = identifier;

    k_StackTrace_exit();
}

/*
 * forStatement
 * :    'for' forParameter ':' expression statementSuite
 * ;
 *
 * TODO: Remove elseClause!
 */
void parseForStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_ForStatementContext_t* context = k_ForStatementContext_new(node);

    /* Match and discard the 'for' token. */
    match(parser, KUSH_TOKEN_KEYWORD_FOR);

    k_ASTNode_t* forParameter = k_ASTNode_new(node);
    context->m_forParameter = forParameter;
    parseForParameter(parser, forParameter);

    /* Match and discard the ':' token. */
	match(parser, KUSH_TOKEN_COLON);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	parseExpression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * tryStatement
 * :    tryClause catchClause* finallyClause?
 * ;
 */
void parseTryStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_TryStatementContext_t* context = k_TryStatementContext_new(node);

	bool hasCatch = false;
	bool hasFinally = false;

    k_Token_t* tryKeyword = k_TokenStream_lt(parser->m_tokens, 1);
    k_ASTNode_t* tryClause = k_ASTNode_new(node);
    context->m_tryClause = tryClause;
    parseTryClause( parser, tryClause);

	while ((k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_KEYWORD_CATCH)) {
		hasCatch = true;

		k_ASTNode_t* catchClause = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_catchClauses, catchClause);
		parseCatchClause( parser, catchClause);
	}

	if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_KEYWORD_FINALLY) {
		hasFinally = true;
		k_ASTNode_t* finallyClause = k_ASTNode_new(node);
        context->m_finallyClause = finallyClause;
		parseFinallyClause( parser, finallyClause);
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

    k_StackTrace_exit();
}

/*
 * tryClause
 * :    'try' statementSuite
 * ;
 */
void parseTryClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_TryClauseContext_t* context = k_TryClauseContext_new(node);

    /* Match and discard the 'try' token. */
	match(parser, KUSH_TOKEN_KEYWORD_TRY);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * catchClause
 * :	'catch' catchFilter IDENTIFIER statementSuite
 * ;
 */
void parseCatchClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_CatchClauseContext_t* context = k_CatchClauseContext_new(node);

    /* Match and discard the 'catch' token. */
    match(parser, KUSH_TOKEN_KEYWORD_CATCH);

    k_ASTNode_t* catchFilter = k_ASTNode_new(node);
    context->m_catchFilter = catchFilter;
    parseCatchFilter( parser, catchFilter);

    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);

    k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
    blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/* TODO: Merge catchFilter context with catch clause context.
 *
 * catchFilter
 * :	(STRING_LITERAL | IDENTIFIER) ('|' (STRING_LITERAL | IDENTIFIER))*
 * ;
 */
void parseCatchFilter(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_CatchFilterContext_t* context = k_CatchFilterContext_new(node);

    // TODO

	k_ASTNode_t* typeName = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_typeNames, typeName);
	k_Parser_typeName(parser, typeName);

	while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_VERTICAL_BAR) {
        /* Consume and discard the '|' token. */
		k_TokenStream_consume(parser->m_tokens);

		typeName = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_typeNames, typeName);
		k_Parser_typeName(parser, typeName);
	}

    k_StackTrace_exit();
}

// TODO: Remove this
/*
 * typeName
 * :    IDENTIFIER ('.' IDENTIFIER)*
 * ;
 */
void k_Parser_typeName(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_TypeNameContext_t* context = k_TypeNameContext_new(node);

    k_Token_t* identifierToken = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifier = newTerminalNode(node, identifierToken);
    jtk_ArrayList_add(context->m_identifiers, identifier);

    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_DOT) {
        /* Consume and discard the '.' token. */
        k_TokenStream_consume(parser->m_tokens);

        identifierToken = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
        identifier = newTerminalNode(node, identifierToken);
        jtk_ArrayList_add(context->m_identifiers, identifier);
    }

    k_StackTrace_exit();
}

/*
 * finallyClause
 * :	'finally' statementSuite
 * ;
 */
void parseFinallyClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_FinallyClauseContext_t* context = k_FinallyClauseContext_new(node);

    /* Match and discard the 'finally' token. */
	match(parser, KUSH_TOKEN_KEYWORD_FINALLY);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	blockStatement(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * structureDeclaration
 * :	'class' IDENTIFIER extendsClause? structureSuite
 * ;
 */
void parseStructureDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassDeclarationContext_t* context = k_ClassDeclarationContext_new(node);

    /* Match and discard the 'struct' token. */
    match(parser, KUSH_TOKEN_KEYWORD_STRUCT);

    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);

    k_ASTNode_t* structureSuite = k_ASTNode_new(node);
    context->m_structureSuite = structureSuite;
    parseStructureSuite(parser, structureSuite);

    k_StackTrace_exit();
}

/*
 * structureSuite
 * :	'{' structureMember+ '}'
 * ;
 */
void parseStructureSuite(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassSuiteContext_t* context = k_ClassSuiteContext_new(node);

    /* Match and discard the left brace token. */
    match(parser, KUSH_TOKEN_LEFT_BRACE);

    do {
        k_ASTNode_t* structureMember = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_classMembers, structureMember);
        parseStructureMember(parser, structureMember);
    }
    while (isType(k_TokenStream_la(parser->m_tokens, 1)));

    /* Match and discard the right brace token. */
    match(parser, KUSH_TOKEN_RIGHT_BRACE);

    k_StackTrace_exit();
}

/*
 * structureMember
 * :	variableDeclaration ';'
 * ;
 */
void parseStructureMember(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassMemberContext_t* context = k_ClassMemberContext_new(node);

    /* If parseVariableDeclaration( ) fails, discard tokens until the semicolon token is encountered. */
    pushFollowToken(parser, KUSH_TOKEN_SEMICOLON);

    // TODO: variableDeclaration should not accept initializers!
    k_ASTNode_t* variableDeclaration = k_ASTNode_new(node);
    context->m_declaration = variableDeclaration;
    parseVariableDeclaration( parser, variableDeclaration);

    /* Match and discard the newline token. */
    match(parser, KUSH_TOKEN_SEMICOLON);

    /* Pop the newline token from the follow set. */
    popFollowToken(parser);

    k_StackTrace_exit();
}

/*
 * expressions
 * :    expression (',' expression)*
 * ;
 */
void parseExpressions(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ExpressionsContext_t* context = k_ExpressionsContext_new(node);

    k_ASTNode_t* expression = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_expressions, expression);
    parseExpression(parser, expression);

    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        expression = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_expressions, expression);
        parseExpression(parser, expression);
    }

    k_StackTrace_exit();
}

/*
 * expression
 * :	parseAssignmentExpression(
 * ;
 *
 */
void parseExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ExpressionContext_t* context = k_ExpressionContext_new(node);

    k_ASTNode_t* assignmentExpression = k_ASTNode_new(node);
    context->m_assignmentExpression = assignmentExpression;
    parseAssignmentExpression(parser, parseAssignmentExpression);
}

bool k_Parser_isExpressionFollow(k_TokenType_t type) {
    return isUnaryExpressionFollow(type);
}

/*
 * parseAssignmentExpression(
 * :	conditionalExpression (assignmentOperator parseAssignmentExpression()?
 * ;
 */
void parseAssignmentExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AssignmentExpressionContext_t* context = k_AssignmentExpressionContext_new(node);

    k_ASTNode_t* conditionalExpression = k_ASTNode_new(node);
    context->m_conditionalExpression = conditionalExpression;
    parseConditionalExpression(parser, conditionalExpression);

    if (isAssignmentOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        k_Token_t* assignmentOperator = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_assignmentOperator = newTerminalNode(node, assignmentOperator);
        /* Consume the assignment operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* assignmentExpression = k_ASTNode_new(node);
        context->m_assignmentExpression = parseAssignmentExpression;
        parseAssignmentExpression(parser, parseAssignmentExpression);
    }

    k_StackTrace_exit();
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
void parseConditionalExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ConditionalExpressionContext_t* context = k_ConditionalExpressionContext_new(node);

    k_ASTNode_t* logicalOrExpression = k_ASTNode_new(node);
    context->m_logicalOrExpression = logicalOrExpression;
    parseLogicalOrExpression(parser, logicalOrExpression);

    if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_HOOK) {
        /* Consume and discard the '?' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_thenExpression = expression;
        parseExpression(parser, expression);

		/* Consume and discard the ':' token. */
        match(parser, KUSH_TOKEN_COLON);

        k_ASTNode_t* conditionalExpression = k_ASTNode_new(node);
        context->m_elseExpression = conditionalExpression;
        parseConditionalExpression(parser, conditionalExpression);
    }

    k_StackTrace_exit();
}

/*
 * logicalOrExpression
 * :	logicalAndExpression ('or' logicalAndExpression)*
 * ;
 */
void parseLogicalOrExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_LogicalOrExpressionContext_t* context = k_LogicalOrExpressionContext_new(node);

    k_ASTNode_t* logicalAndExpression = k_ASTNode_new(node);
    context->m_logicalAndExpression = logicalAndExpression;
    parseLogicalAndExpression(parser, logicalAndExpression);

    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_VERTICAL_BAR_2) {
        /* Consume and discard the 'or' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* logicalAndExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_logicalAndExpressions, logicalAndExpression0);
        parseLogicalAndExpression(parser, logicalAndExpression0);
    }

    k_StackTrace_exit();
}

/*
 * logicalAndExpression
 * :	inclusiveOrExpression ('and' logicalAndExpression)?
 * ;
 */
void parseLogicalAndExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_LogicalAndExpressionContext_t* context = k_LogicalAndExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* inclusiveOrExpression = k_ASTNode_new(node);
    context->m_inclusiveOrExpression = inclusiveOrExpression;
    parseInclusiveOrExpression(parser, inclusiveOrExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_AMPERSAND_2) {
        /* Consume and discard the 'and' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* inclusiveOrExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_inclusiveOrExpressions, inclusiveOrExpression0);
        parseInclusiveOrExpression(parser, inclusiveOrExpression0);
    }

    k_StackTrace_exit();
}

/*
 * inclusiveOrExpression
 * :	exclusiveOrExpression ('|' exclusiveOrExpression)*
 * ;
 */
void parseInclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_InclusiveOrExpressionContext_t* context = k_InclusiveOrExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* exclusiveOrExpression = k_ASTNode_new(node);
    context->m_exclusiveOrExpression = exclusiveOrExpression;
    parseExclusiveOrExpression(parser, exclusiveOrExpression);

    /* Parse the expression to the right of the operator, if any. */
    if (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_VERTICAL_BAR) {
        /* Consume and discard the '|' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* exclusiveOrExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_exclusiveOrExpressions, exclusiveOrExpression0);
        parseExclusiveOrExpression(parser, exclusiveOrExpression0);
    }

    k_StackTrace_exit();
}

/*
 * exclusiveOrExpression
 * :	andExpression ('^' andExpression)*
 * ;
 */
void parseExclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ExclusiveOrExpressionContext_t* context = k_ExclusiveOrExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* andExpression = k_ASTNode_new(node);
    context->m_andExpression = andExpression;
    parseAndExpression(parser, andExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_CARET) {
        /* Consume and discard the '^' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* andExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_andExpressions, andExpression0);
        parseAndExpression(parser, andExpression0);
    }

    k_StackTrace_exit();
}

/*
 * andExpression
 * :	equalityExpression ('&' equalityExpression)*
 * ;
 */
void parseAndExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AndExpressionContext_t* context = k_AndExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* equalityExpression = k_ASTNode_new(node);
    context->m_equalityExpression = equalityExpression;
    parseEqualityExpression(parser, equalityExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_AMPERSAND) {

        /* Consume and discard the '&' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* equalityExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_equalityExpressions, equalityExpression0);
        parseEqualityExpression(parser, equalityExpression0);
    }

    k_StackTrace_exit();
}

/*
 * equalityExpression
 * :	relationalExpression (equalityOperator relationalExpression)*
 * ;
 */
void parseEqualityExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_EqualityExpressionContext_t* context = k_EqualityExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* relationalExpression = k_ASTNode_new(node);
    context->m_relationalExpression = relationalExpression;
    parseRelationalExpression(parser, relationalExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isEqualityOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_relationalExpressions, pair);

        k_Token_t* equalityOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* equalityOperator = newTerminalNode(node, equalityOperatorToken);
        pair->m_left = equalityOperator;
        /* Consume the equality operator. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* relationalExpression0 = k_ASTNode_new(node);
        pair->m_right = relationalExpression0;
        parseRelationalExpression(parser, relationalExpression0);
    }

    k_StackTrace_exit();
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
void parseRelationalExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_RelationalExpressionContext_t* context = k_RelationalExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* shiftExpression = k_ASTNode_new(node);
    context->m_shiftExpression = shiftExpression;
    parseShiftExpression(parser, shiftExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isRelationalOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_shiftExpressions, pair);

        k_Token_t* relationalOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* relationalOperator = newTerminalNode(node, relationalOperatorToken);
        pair->m_left = relationalOperator;
        /* Consume the relational operator. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* shiftExpression0 = k_ASTNode_new(node);
        pair->m_right = shiftExpression0;
        parseShiftExpression(parser, shiftExpression0);
    }

    k_StackTrace_exit();
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
void parseShiftExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ShiftExpressionContext_t* context = k_ShiftExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* additiveExpression = k_ASTNode_new(node);
    context->m_additiveExpression = additiveExpression;
    parseAdditiveExpression(parser, additiveExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isShiftOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_additiveExpressions, pair);

        k_Token_t* shiftOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* shiftOperator = newTerminalNode(node, shiftOperatorToken);
        pair->m_left = shiftOperator;
        /* Consume the shift operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* additiveExpression0 = k_ASTNode_new(node);
        pair->m_right = additiveExpression0;
        parseAdditiveExpression(parser, additiveExpression0);
    }

    k_StackTrace_exit();
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
void parseAdditiveExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AdditiveExpressionContext_t* context = k_AdditiveExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* multiplicativeExpression = k_ASTNode_new(node);
    context->m_multiplicativeExpression = multiplicativeExpression;
    parseMultiplicativeExpression(parser, multiplicativeExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isAdditiveOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_multiplicativeExpressions, pair);

        k_Token_t* additiveOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* additiveOperator = newTerminalNode(node, additiveOperatorToken);
        pair->m_left = additiveOperator;
        /* Consume the additive operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* multiplicativeExpression0 = k_ASTNode_new(node);
        pair->m_right = multiplicativeExpression0;
        parseMultiplicativeExpression(parser, multiplicativeExpression0);
    }

    k_StackTrace_exit();
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
void parseMultiplicativeExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MultiplicativeExpressionContext_t* context = k_MultiplicativeExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* unaryExpression = k_ASTNode_new(node);
    context->m_unaryExpression = unaryExpression;
    parseUnaryExpression(parser, unaryExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (isMultiplicativeOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_unaryExpressions, pair);

        k_Token_t* multiplicativeOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* multiplicativeOperator = newTerminalNode(node, multiplicativeOperatorToken);
        pair->m_left = multiplicativeOperator;
        /* Consume the multiplicative operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* unaryExpression0 = k_ASTNode_new(node);
        pair->m_right = unaryExpression0;
        parseUnaryExpression(parser, unaryExpression0);
    }

    k_StackTrace_exit();
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
void parseUnaryExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_UnaryExpressionContext_t* context = k_UnaryExpressionContext_new(node);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (isUnaryOperator(la1)) {
        k_Token_t* unaryOperator = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_unaryOperator = newTerminalNode(node, unaryOperator);
        /* Consume the unary operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* unaryExpression = k_ASTNode_new(node);
        context->m_unaryExpression = unaryExpression;
        parseUnaryExpression(parser, unaryExpression);
    }
    else if (isPostfixExpressionFollow(la1)) {
        k_ASTNode_t* postfixExpression = k_ASTNode_new(node);
        context->m_postfixExpression = postfixExpression;
        parsePostfixExpression(parser, postfixExpression);
    }
    else {
        // Error: Expected unary operator or postfix expression follow
        reportAndRecover(parser, KUSH_TOKEN_INTEGER_LITERAL);
    }

    k_StackTrace_exit();
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
void parsePostfixExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_PostfixExpressionContext_t* context = k_PostfixExpressionContext_new(node);

    k_ASTNode_t* primaryExpression = k_ASTNode_new(node);
    context->m_primaryExpression = primaryExpression;
    parsePrimaryExpression(parser, primaryExpression);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    while (isPostfixPartFollow(la1)) {
        switch (la1) {
            case KUSH_TOKEN_LEFT_SQUARE_BRACKET: {
                k_ASTNode_t* subscript = k_ASTNode_new(node);
                jtk_ArrayList_add(context->m_postfixParts, subscript);
                parseSubscript(parser, subscript);
                break;
            }

            case KUSH_TOKEN_LEFT_PARENTHESIS: {
                k_ASTNode_t* functionArguments = k_ASTNode_new(node);
                jtk_ArrayList_add(context->m_postfixParts, functionArguments);

                parseFunctionArguments(parser, functionArguments);

                break;
            }

            case KUSH_TOKEN_DOT: {
                k_ASTNode_t* memberAccess = k_ASTNode_new(node);
                jtk_ArrayList_add(context->m_postfixParts, memberAccess);
                parseMemberAccess(parser, memberAccess);
                break;
            }

            // case KUSH_TOKEN_PLUS_2:
            // case KUSH_TOKEN_DASH_2: {
            //     k_ASTNode_t* parsePostfixOperator( = k_ASTNode_new(node);
            //     jtk_ArrayList_add(context->m_postfixParts, parsePostfixOperator();
            //     parsePostfixOperator((parser, parsePostfixOperator();
            //     break;
            // }
        }
        la1 = k_TokenStream_la(parser->m_tokens, 1);
    }

    k_StackTrace_exit();
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
void parseSubscript(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_SubscriptContext_t* context = k_SubscriptContext_new(node);

    /* Match and discard the '[' token. */
    match(parser, KUSH_TOKEN_LEFT_SQUARE_BRACKET);

    /* If expression fails, discard tokens until the ']' token is
     * encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);

    k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
    parseExpression(parser, expression);

    /* Pop the ']' token from the follow set. */
    popFollowToken(parser);

    /* Match and discard the ']' token. */
    match(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);

    k_StackTrace_exit();
}

/*
 * functionArguments
 * :	'(' expressions? ')'
 * ;
 */
void parseFunctionArguments(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_FunctionArgumentsContext_t* context = k_FunctionArgumentsContext_new(node);

    /* Match and discard the '(' token. */
    match(parser, KUSH_TOKEN_LEFT_PARENTHESIS);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        /* If expression fails, discard tokens until the ')' token is
        * encountered.
        */
        pushFollowToken(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

        k_ASTNode_t* expressions = k_ASTNode_new(node);
        context->m_expressions = expressions;
        parseExpressions(parser, expressions);

        /* Pop the ')' token from the follow set. */
        popFollowToken(parser);
    }

    /* Match and discard the ')' token. */
    match(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

    k_StackTrace_exit();
}

/*
 * memberAccess
 * :	'.' IDENTIFIER
 * ;
 */
void parseMemberAccess(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MemberAccessContext_t* context = k_MemberAccessContext_new(node);

    /* Match and discard the '.' token. */
    match(parser, KUSH_TOKEN_DOT);

    k_Token_t* identifier = matchAndYield(parser, KUSH_TOKEN_IDENTIFIER);
    context->m_identifier = newTerminalNode(node, identifier);

    k_StackTrace_exit();
}

/*
 * parsePostfixOperator(
 * :	'++'
 * |	'--'
 * ;
 */
void parsePostfixOperator(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_PostfixOperatorContext_t* context = k_PostfixOperatorContext_new(node);

    switch (k_TokenStream_la(parser->m_tokens, 1)) {
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

    k_StackTrace_exit();
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
void parsePrimaryExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_PrimaryExpressionContext_t* context = k_PrimaryExpressionContext_new(node);
    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
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

                k_ASTNode_t* expression = k_ASTNode_new(node);
                context->m_expression = expression;
                parseExpression(parser, expression);

                /* Pop the ')' token from the follow set. */
                popFollowToken(parser);

                /* Match and discard the ')' token. */
                match(parser, KUSH_TOKEN_RIGHT_PARENTHESIS);

                break;
            }

            case KUSH_TOKEN_LEFT_BRACE: {
                k_ASTNode_t* mapExpression = k_ASTNode_new(node);
                context->m_expression = mapExpression;
                parseInitializerExpression(parser, mapExpression);

                break;
            }

            case KUSH_TOKEN_LEFT_SQUARE_BRACKET: {
                k_ASTNode_t* arrayExpression = k_ASTNode_new(node);
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

    k_StackTrace_exit();
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
void parseInitializerExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_MapExpressionContext_t* context = k_MapExpressionContext_new(node);

    /* Match and discard the '{' token. */
    match(parser, KUSH_TOKEN_LEFT_BRACE);

    /* If expression fails, discard tokens until the '}' token is
     * encountered.
     */
    pushFollowToken(parser, KUSH_TOKEN_RIGHT_BRACE);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_ASTNode_t* mapEntries = k_ASTNode_new(node);
        context->m_mapEntries = mapEntries;
        parseInitializerEntries(parser, mapEntries);
    }

    /* Pop the '}' token from the follow set. */
    popFollowToken(parser);

    /* Match and discard the '}' token. */
    match(parser, KUSH_TOKEN_RIGHT_BRACE);

    k_StackTrace_exit();
}

/*
 * mapEntries
 * :	mapEntry (',' mapEntry)*
 * ;
 */
void parseInitializerEntries(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MapEntriesContext_t* context = k_MapEntriesContext_new(node);

    k_ASTNode_t* mapEntry = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_mapEntries, mapEntry);
    parseInitializerEntry(parser, mapEntry);

    while (k_TokenStream_la(parser->m_tokens, 1) == KUSH_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        mapEntry = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_mapEntries, mapEntry);
        parseInitializerEntry(parser, mapEntry);
    }

    k_StackTrace_exit();
}

/*
 * mapEntry
 * :	IDENTIFIER ':' expression
 * ;
 */
void parseInitializerEntry(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MapEntryContext_t* context = k_MapEntryContext_new(node);

    // TODO: The key should be an identifier.
    k_ASTNode_t* keyExpression = k_ASTNode_new(node);
    context->m_keyExpression = keyExpression;
    parseExpression(parser, keyExpression);

    /* Match and discard the ':' token. */
    match(parser, KUSH_TOKEN_COLON);

    k_ASTNode_t* valueExpression = k_ASTNode_new(node);
    context->m_valueExpression = valueExpression;
    parseExpression(parser, valueExpression);

    k_StackTrace_exit();
}

/*
 * arrayExpression
 * :    '[' expressions ']'
 * ;
 */
void parseArrayExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ListExpressionContext_t* context = k_ListExpressionContext_new(node);

    /* Match and discard the '[' token. */
    match(parser, KUSH_TOKEN_LEFT_SQUARE_BRACKET);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        /* If expression fails, discard tokens until the ')' token is
        * encountered.
        */
        pushFollowToken(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);

        k_ASTNode_t* expressions = k_ASTNode_new(node);
        context->m_expressions = expressions;
        parseExpressions(parser, expressions);

        /* Pop the ']' token from the follow set. */
        popFollowToken(parser);
    }

    /* Match and discard the ']' token. */
    match(parser, KUSH_TOKEN_RIGHT_SQUARE_BRACKET);

    k_StackTrace_exit();
}