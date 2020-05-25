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

const char k_Parser_ruleNames[][50] = {
    "<unknown>",
    "<terminal>",

    "compilationUnit",
    "importDeclaration",
    "annotatedComponentDeclaration",
    "annotations",
    "annotation",
    "annotationType",
    "annotationAttribute",
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
    "constantDeclaration",
    "constantDeclarator",
    "assertStatement",
    "breakStatement",
    "continueStatement",
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
    "typeName",
    "finallyClause",
    "synchronizeStatement",
    "withStatement",
    "withParameters",
    "withParameter",
    "classDeclaration",
    "classClassExtendsClause",
    "classSuite",
    "classMember",
    // "constructorDeclaration",
    // "enumerationDeclaration",
    // "enumerationBaseClause",
    // "enumerationSuite",
    // "enumerate",
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
    "mapExpression",
    "mapEntries",
    "mapEntry",
    "listExpression",
    "newExpression"
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
    return k_Parser_ruleNames[(int32_t)type];
}

/* Syntax Error */

// void k_Parser_reportSyntaxError(k_Parser_t* parser, k_Token_t* token,
//     const char* message) {
//     fprintf(stderr, "[error] %d-%d:%d-%d: %s\n", token->m_startLine + 1,
//         token->m_stopLine + 1, token->m_startColumn + 1, token->m_stopColumn + 1,
//         message);
// }

/* Terminal Node */

k_ASTNode_t* k_Parser_newTerminalNode(k_ASTNode_t* node, k_Token_t* token) {
    k_ASTNode_t* terminalNode = k_ASTNode_new(node);
    terminalNode->m_type = ZEN_AST_NODE_TYPE_TERMINAL;
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
void k_Parser_recover(k_Parser_t* parser) {
    /* The parser is now in recovery mode; flag other parts of the parser. */
    parser->m_recovery = true;

    if (parser->m_followSetSize > 0) {
        k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
        /* The parser tries to recover until a token from the follow set or
         * the end-of-stream token is encountered.
         */
        while (lt1->m_type != ZEN_TOKEN_END_OF_STREAM) {
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

bool k_Parser_ensureFollowSetSpace(k_Parser_t* parser, int32_t capacity) {
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

void k_Parser_pushFollowToken(k_Parser_t* parser, k_TokenType_t type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    /* Make sure that the set is large enough to hold another token type. */
    k_Parser_ensureFollowSetSpace(parser, parser->m_followSetSize + 1);
    /* Insert the follow token to the set. */
    parser->m_followSet[parser->m_followSetSize] = type;
    /* Increment the size of the follow set. */
    parser->m_followSetSize++;
}

void k_Parser_popFollowToken(k_Parser_t* parser) {
    jtk_Assert_assertTrue(parser->m_followSetSize > 0, "The follow set is empty.");

    parser->m_followSetSize--;
}

/* Match */

void k_Parser_match(k_Parser_t* parser, k_TokenType_t type) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    k_Parser_matchAndYield(parser, type);
}

void k_Parser_reportAndRecover(k_Parser_t* parser, k_TokenType_t expected) {
    /* Do not report the error if the parser is in recovery mode. Otherwise,
    * duplicate syntax errors will be reported to the end user.
    */
    if (!parser->m_recovery) {
        k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
        k_Compiler_t* compiler = parser->m_compiler;
        k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
        k_ErrorHandler_handleSyntacticalError(errorHandler, parser,
            ZEN_ERROR_CODE_UNEXPECTED_TOKEN, lt1, expected);
    }

    /* Try to resychronize the parser with the input. */
    k_Parser_recover(parser);
}

k_Token_t* k_Parser_matchAndYield(k_Parser_t* parser, k_TokenType_t type) {
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
        if (lt1->m_type != ZEN_TOKEN_END_OF_STREAM) {
            k_TokenStream_consume(parser->m_tokens);
        }
    }
    else {
        k_Parser_reportAndRecover(parser, type);
    }
    return lt1;
}

// Reset

void k_Parser_reset(k_Parser_t* parser, k_TokenStream_t* tokens) {
    jtk_Assert_assertObject(parser, "The specified parser is null.");

    parser->m_tokens = tokens;
    parser->m_followSetSize = 0;
    parser->m_recovery = false;
    parser->m_mainComponent = ZEN_AST_NODE_TYPE_UNKNOWN;
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
void k_Parser_compilationUnit(k_Parser_t* parser, k_ASTNode_t* node) {
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
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_NEWLINE) {
        /* Consume the newline token. */
        k_TokenStream_consume(parser->m_tokens);
    }

	/* Zero or more import declarations may occur in the source code.
	 * Therefore, repeatedly parse import declarations as long as
	 * LA(1) is the 'import' keyword.
	 *
	 * The following code represents: importDeclaration*
	 */
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_IMPORT) {
		k_ASTNode_t* importDeclaration = k_ASTNode_new(node);
        k_Parser_importDeclaration(parser, importDeclaration);

		jtk_ArrayList_add(context->m_importDeclarations, importDeclaration);
    }

	/* The following text describes the algorithm employed previously.
     * The implementation of the previous algorithm was commented out.
     * The current implementation extended the grammar to accomodate
     * arbitrary newlines at the beginning of the input, between components,
     * and end of the input. Albeit deprecated, this text has some valuable
     * information.
     *
     * Zero or more function declarations, class declarations, and enum
	 * declarations may occur in the source code. Interestingly, the parser
	 * imposes no restrictions in the order in which they appear.
	 * Things get complicated when each declaration may be annotated
	 * individually zero or more times. Therefore, the following code
	 * is not a direct translation of the grammar.
	 *
	 * The following actions are performed while parsing syntax
	 * of the form: (annotation (functionDeclaration | classDeclaration | enumerationDeclaration))*
	 * Firstly, we parse one or more annotations if LA(1) is the '@' token.
	 * Otherwise, we parse function declaration, class declaration, enum
	 * declaration if LA(1) is 'function', 'class', 'enum', respectively.
     * Annotations and declarations are paired within the AST.
	 *
	 * When an unknown token appears in the loop, we stop the loop
	 * immediately. After which the 'end-of-stream' token is expected.
	 * Moreover, a syntax error is reported if annotation(s) are not
	 * followed by a declaration.
	 */
    /*
    while (true) {
        jtk_Pair_t* declarationPair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_declarations, declarationPair);

        if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_AT) {
			k_ASTNode_t* annotations = k_ASTNode_new(node);
            declarationPair->m_leftElement = annotations;
            k_Parser_annotations(parser, annotations);
        }

        switch (k_TokenStream_la(parser->m_tokens, 1)) {
            case ZEN_TOKEN_KEYWORD_FUNCTION: {
				k_ASTNode_t* functionDeclaration = k_ASTNode_new(node);
                declarationPair->m_rightElement = functionDeclaration;
				k_Parser_functionDeclaration(parser, functionDeclaration, 0);
                break;
            }

            case ZEN_TOKEN_KEYWORD_CLASS: {
				k_ASTNode_t* classDeclaration = k_ASTNode_new(node);
                declarationPair->m_rightElement = classDeclaration;
				k_Parser_classDeclaration(parser, classDeclaration);
                break;
            }

            case ZEN_TOKEN_KEYWORD_ENUM: {
				k_ASTNode_t* enumerationDeclaration = k_ASTNode_new(node);
                declarationPair->m_rightElement = enumerationDeclaration;
				k_Parser_enumerationDeclaration(parser, enumerationDeclaration);
                break;
            }

            default: {
				// Check if annotation(s) is not followed by a declaration.
				if ((declarationPair->m_leftElement != NULL) &&
                    (declarationPair->m_rightElement == NULL)) {
                    // Syntax Error: Expected function, class, or enum declaration after annotation
                }
                goto breakPoint;
            }
        }
    }
    breakPoint:
    */

    while (k_Parser_isAnnotatedComponentDeclarationFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_ASTNode_t* annotatedComponentDeclaration = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_annotatedComponentDeclarations, annotatedComponentDeclaration);
        k_Parser_annotatedComponentDeclaration(parser, annotatedComponentDeclaration);
    }

	/* We are expecting an 'end of stream' here. */
    k_Parser_match(parser, ZEN_TOKEN_END_OF_STREAM);

    k_StackTrace_exit();
}

/*
 * importDeclaration
 * :    'import' importTarget NEWLINE
 * ;
 *
 * TODO:
 *
 * importTarget
 * :    IDENTIFIER ('.' IDENTIFIER)* ('.' '*' | ('as' IDENTIFIER))?
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void k_Parser_importDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_ImportDeclarationContext_t* context = k_ImportDeclarationContext_new(node);

	/* An import statement begins with the 'import' keyword. Therefore,
	 * we are expecting it.
	 */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_IMPORT);

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
    k_Parser_pushFollowToken(parser, ZEN_TOKEN_NEWLINE);

	/* The user is expected to specify at least, one identifier.
	 * Consume it. The consumed identifier saved for later inspection.
	 */
    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifierNode = k_Parser_newTerminalNode(node, identifier);

	jtk_ArrayList_add(context->m_identifiers, identifierNode);

	/* Optionally, the user may specify more identifiers (with each identifier
	 * separated by the '.' token. Therefore, we repeatedly consume the '.' and
	 * identifier tokens as long as LA(1) is the '.' token and LA(2) is the
	 * identifier token.
	 */
    while ((k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_DOT) &&
           (k_TokenStream_la(parser->m_tokens, 2) == ZEN_TOKEN_IDENTIFIER)) {
		/* Consume and discard the '.' token. */
        k_TokenStream_consume(parser->m_tokens);

		/* The consumed identifier is saved for later inspection.
		 */
        identifier = k_TokenStream_lt(parser->m_tokens, 1);
        identifierNode = k_Parser_newTerminalNode(node, identifier);
        k_TokenStream_consume(parser->m_tokens);
		jtk_ArrayList_add(context->m_identifiers, identifierNode);
    }

	/* Optionally, the user may specify a wildcard; recognized when
	 * LA(1) is the '.' token and LA(2) is the '*' token.
	 */
    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_DOT) {
		k_TokenStream_consume(parser->m_tokens);
		k_Parser_match(parser, ZEN_TOKEN_ASTERISK);
        context->m_wildcard = true;
    }

    /* Pop the newline token from the follow set. */
    k_Parser_popFollowToken(parser);

	/* The import declaration is terminated with a newline.
	 * Therefore, we are expecting it here.
	 */
    k_Parser_match(parser, ZEN_TOKEN_NEWLINE);

    k_StackTrace_exit();
}

bool k_Parser_isAnnotatedComponentDeclarationFollow(k_TokenType_t token) {
    return (token == ZEN_TOKEN_AT) ||
           (token == ZEN_TOKEN_KEYWORD_FUNCTION) ||
           (token == ZEN_TOKEN_KEYWORD_CLASS) ||
           (token == ZEN_TOKEN_KEYWORD_ENUM);
}

/*
 * annotatedComponentDeclaration
 * :    annotations? componentDeclaration
 * ;
 */
void k_Parser_annotatedComponentDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AnnotatedComponentDeclarationContext_t* context = k_AnnotatedComponentDeclarationContext_new(node);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_AT) {
        k_ASTNode_t* annotations = k_ASTNode_new(node);
        context->m_annotations = annotations;
        k_Parser_annotations(parser, annotations);
    }

    k_ASTNode_t* componentDeclaration = k_ASTNode_new(node);
    context->m_componentDeclaration = componentDeclaration;
    k_Parser_componentDeclaration(parser, componentDeclaration);

    k_StackTrace_exit();
}

/*
 * annotations
 * :    annotation+
 * ;
 */
void k_Parser_annotations(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_AnnotationsContext_t* context = k_AnnotationsContext_new(node);

    /* This rule expects at least, one annotation. Further annotations
	 * are parsed if LA(1) is the '@' token.
	 */
	do {
		k_ASTNode_t* annotation = k_ASTNode_new(node);
		jtk_ArrayList_add(context->m_annotations, annotation);

		k_Parser_annotation(parser, annotation);
    }
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_AT);

    k_StackTrace_exit();
}

/*
 * annotation
 * :    '@' annotationType annotationAttribute* NEWLINE
 * ;
 */
void k_Parser_annotation(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_AnnotationContext_t* context = k_AnnotationContext_new(node);

    /* We are expecting the '@' token here. */
    k_Parser_match(parser, ZEN_TOKEN_AT);

    /* If annotationType fails, discard tokens until the newline token is
     * encountered.
     */
    k_Parser_pushFollowToken(parser, ZEN_TOKEN_NEWLINE);

    /* We are expecting an annotation type here. */
    k_ASTNode_t* annotationType = k_ASTNode_new(node);
	context->m_annotationType = annotationType;
	k_Parser_annotationType(parser, annotationType);

    /* An optional attribute list may be specified, recognized
	 * if LA(1) is an identifier.
	 */
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_IDENTIFIER) {
		k_ASTNode_t* annotationAttribute = k_ASTNode_new(node);
		jtk_ArrayList_add(context->m_annotationAttributes, annotationAttribute);
        k_Parser_annotationAttribute(parser, annotationAttribute);
    }

    /* Pop the newline token from the follow set. */
    k_Parser_popFollowToken(parser);

    /* We are expecting a newline token here. The newline token marks the
	 * end of an annotation.
	 */
    k_Parser_match(parser, ZEN_TOKEN_NEWLINE);

    k_StackTrace_exit();
}

/*
 * annotationType
 * :    IDENTIFIER ('.' IDENTIFIER)*
 * ;
 */
void k_Parser_annotationType(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AnnotationTypeContext_t* context = k_AnnotationTypeContext_new(node);

    /* We are expecting an IDENTIFIER token here. */
    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifierNode = k_Parser_newTerminalNode(node, identifier);
    jtk_ArrayList_add(context->m_identifiers, identifierNode);

    /* Optionally, the user may specify more identifiers separated
     * by the '.' token.
     */
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_DOT) {
        /* Consume and discard the '.' token. */
        k_TokenStream_consume(parser->m_tokens);

        /* We are expecting an identifier here. The consumed token is
		 * saved for later inspection.
		 */
        identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
        identifierNode = k_Parser_newTerminalNode(node, identifier);
        jtk_ArrayList_add(context->m_identifiers, identifierNode);
    }

    k_StackTrace_exit();
}

/*
 * annotationAttribute
 * :    IDENTIFIER '=' literal
 * ;
 */
void k_Parser_annotationAttribute(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_AnnotationAttributeContext_t* context = k_AnnotationAttributeContext_new(node);

	/* We are expecting an identifier here. The consumed token is
	 * saved for later inspection.
	 */
    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
	context->m_identifier = k_Parser_newTerminalNode(node, identifier);

	/* The attribute key and value pair is separated by the '=' token.
	 * Match and discard the '=' token.
	 */
    k_Parser_match(parser, ZEN_TOKEN_EQUAL);

	/* The value should always be a literal. As of now, expressions cannot
	 * be specified to attributes. Parse a literal.
	 */
    if (k_Parser_isLiteralFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_Token_t* literal = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_literal = k_Parser_newTerminalNode(node, literal);
        /* Consume the literal token. */
        k_TokenStream_consume(parser->m_tokens);
    }
    else {
        // TODO: string, integer, decimal, boolean, or null literal
        k_Parser_reportAndRecover(parser, ZEN_TOKEN_STRING_LITERAL);
    }

    k_StackTrace_exit();
}

/*
 * componentDeclaration
 * :    functionDeclaration
 * |    classDeclaration
 * // |    enumerationDeclaration
 * ;
 */
void k_Parser_componentDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_Compiler_t* compiler = parser->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ComponentDeclarationContext_t* context = k_ComponentDeclarationContext_new(node);

    k_ASTNodeType_t component = ZEN_AST_NODE_TYPE_UNKNOWN;
    k_Token_t* lt1 = k_TokenStream_lt(parser->m_tokens, 1);
    switch (lt1->m_type) {
        case ZEN_TOKEN_KEYWORD_FUNCTION: {
            component = ZEN_AST_NODE_TYPE_FUNCTION_DECLARATION;

            k_ASTNode_t* functionDeclaration = k_ASTNode_new(node);
            context->m_component = functionDeclaration;
            k_Parser_functionDeclaration(parser, functionDeclaration, 0);
            break;
        }

        case ZEN_TOKEN_KEYWORD_CLASS: {
            component = ZEN_AST_NODE_TYPE_CLASS_DECLARATION;

            k_ASTNode_t* classDeclaration = k_ASTNode_new(node);
            context->m_component = classDeclaration;
            k_Parser_classDeclaration(parser, classDeclaration);
            break;
        }

        // case ZEN_TOKEN_KEYWORD_ENUM: {
        //     k_ASTNode_t* enumerationDeclaration = k_ASTNode_new(node);
        //     context->m_component = enumerationDeclaration;
        //     k_Parser_enumerationDeclaration(parser, enumerationDeclaration);
        //     break;
        // }

        default: {
            // TODO: Expected function or class
            k_Parser_reportAndRecover(parser, ZEN_TOKEN_KEYWORD_CLASS);
            break;
        }
    }

    if (parser->m_mainComponent == ZEN_AST_NODE_TYPE_UNKNOWN) {
        parser->m_mainComponent = component;
    }
    else {
        if (parser->m_mainComponent != component) {
            k_ErrorHandler_handleSemanticalError(errorHandler, parser,
                ZEN_ERROR_CODE_CANNOT_DECLARE_MULTIPLE_TYPES_OF_COMPONENT, lt1);
        }
    }


    k_StackTrace_exit();
}

/*
 * functionDeclaration
 * :    'function' functionIdentifier functionParameters (functionBody | NEWLINE)
 * ;
 *
 * functionIdentifier
 * :    IDENTIFIER
 * // |    'static'
 * |    'new'
 * ;
 */
void k_Parser_functionDeclaration(k_Parser_t* parser, k_ASTNode_t* node,
    uint32_t modifiers) {
    k_StackTrace_enter();

	k_FunctionDeclarationContext_t* context = k_FunctionDeclarationContext_new(node);

    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_FUNCTION);

    switch (k_TokenStream_la(parser->m_tokens, 1)) {
        case ZEN_TOKEN_IDENTIFIER:
        case ZEN_TOKEN_KEYWORD_NEW:
        case ZEN_TOKEN_KEYWORD_STATIC: {
            k_Token_t* identifierOrKeyword = k_TokenStream_lt(parser->m_tokens, 1);
            context->m_identifier = k_Parser_newTerminalNode(node, identifierOrKeyword);
            k_TokenStream_consume(parser->m_tokens);
            break;
        }

        default: {
            // TODO: Expected identifier, new, or static
            k_Parser_reportAndRecover(parser, ZEN_TOKEN_IDENTIFIER);
        }
    }

    /* Previously, functions and constructors were two different entities.
     * The former construct has been removed.
     */
    // k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    // context->m_identifier = k_Parser_newTerminalNode(node, identifier);

    /* If functionParameters fails, discard tokens until the newline token is
     * encountered.
     */
    k_Parser_pushFollowToken(parser, ZEN_TOKEN_NEWLINE);

    /* TODO:
     * The parenthesis are considered as enclosures by the lexer. Any
     * newline token in between parenthesis are generated on the hidden
     * channel. Therefore, the error recovery strategy does not work
     * well unless the hidden channel is also considered.
     */
    k_ASTNode_t* functionParameters = k_ASTNode_new(node);
    context->m_functionParameters = functionParameters;
    k_Parser_functionParameters(parser, functionParameters);

    /* Pop the newline token from the follow set. */
    k_Parser_popFollowToken(parser);

    if (!k_Modifier_hasNative(modifiers) && !k_Modifier_hasAbstract(modifiers)) {
        k_ASTNode_t* functionBody = k_ASTNode_new(node);
        context->m_functionBody = functionBody;
        k_Parser_functionBody(parser, functionBody);
    }
    else {
        k_Parser_match(parser, ZEN_TOKEN_NEWLINE);
    }

    k_StackTrace_exit();
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
void k_Parser_functionParameters(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_FunctionParametersContext_t* context = k_FunctionParametersContext_new(node);

    /* Match and discard the '(' token. */
    k_Parser_match(parser, ZEN_TOKEN_LEFT_PARENTHESIS);

    /* Optionally, the programmer may specify one or more parameters.
     * The following statements parse both fixed and variable parameters.
     */
    switch (k_TokenStream_la(parser->m_tokens, 1)) {
        case ZEN_TOKEN_IDENTIFIER: {
            k_Token_t* fixedParameter = k_TokenStream_lt(parser->m_tokens, 1);
            k_ASTNode_t* fixedParameterNode = k_Parser_newTerminalNode(node, fixedParameter);
			jtk_ArrayList_add(context->m_fixedParameters, fixedParameterNode);
			k_TokenStream_consume(parser->m_tokens);

            while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COMMA) {
                /* Consume and discard the ',' token. */
                k_TokenStream_consume(parser->m_tokens);
                switch (k_TokenStream_la(parser->m_tokens, 1)) {
                    case ZEN_TOKEN_IDENTIFIER: {
                        fixedParameter = k_TokenStream_lt(parser->m_tokens, 1);
                        fixedParameterNode = k_Parser_newTerminalNode(node, fixedParameter);
						jtk_ArrayList_add(context->m_fixedParameters, fixedParameterNode);
                        k_TokenStream_consume(parser->m_tokens);
                        break;
                    }

                    case ZEN_TOKEN_ELLIPSIS: {
						/* Consume the '...' token. */
                        k_TokenStream_consume(parser->m_tokens);
                        k_Token_t* variableParameter = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
						context->m_variableParameter = k_Parser_newTerminalNode(node, variableParameter);
                        goto loopExit;
                    }

					default: {
                        k_Parser_reportAndRecover(parser, ZEN_TOKEN_IDENTIFIER);
                        break;
					}
                }
            }
            loopExit:
            break;
        }

        case ZEN_TOKEN_ELLIPSIS: {
			/* Consume and discard the '...' token. */
            k_TokenStream_consume(parser->m_tokens);
            k_Token_t* variableParameter = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
			context->m_variableParameter = k_Parser_newTerminalNode(node, variableParameter);
            break;
        }
    }

    /* Match and discard the ')' token. */
    k_Parser_match(parser, ZEN_TOKEN_RIGHT_PARENTHESIS);

    k_StackTrace_exit();
}

/*
 * functionBody
 * :    statementSuite
 * ;
 */
void k_Parser_functionBody(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_FunctionBodyContext_t* context = k_FunctionBodyContext_new(node);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
	context->m_statementSuite = statementSuite;
    k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/* In order to help the users read code easily, the simple statements were
 * removed from statement suites. Previously, a statement suite was either
 * a simple statement or a block consisting of a newline at the begining,
 * indentation and dedentation.
 */
/*
 * statementSuite
 * |    NEWLINE INDENT statement+ DEDENT
 * ;
 */
void k_Parser_statementSuite(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_StatementSuiteContext_t* context = k_StatementSuiteContext_new(node);

    // k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);

    /* if (k_Parser_isSimpleStatementFollow(la1)) {
        k_ASTNode_t* simpleStatement = k_ASTNode_new(node);
        context->m_simpleStatement = simpleStatement;
        k_Parser_simpleStatement(parser, simpleStatement);
    }
    */

    // if (la1 == ZEN_TOKEN_NEWLINE) {
        /* Consume and discard the newline token. */
        k_Parser_match(parser, ZEN_TOKEN_NEWLINE);
        /* Consume and discard the dedent token. */
        k_Parser_match(parser, ZEN_TOKEN_INDENTATION);

        /* If statement fails, discard tokens until the dedentation token is
         * encountered.
         */
        k_Parser_pushFollowToken(parser, ZEN_TOKEN_DEDENTATION);

        do {
            k_ASTNode_t* statement = k_ASTNode_new(node);
            jtk_ArrayList_add(context->m_statements, statement);
            k_Parser_statement(parser, statement);
        }
        while (k_Parser_isStatementFollow(k_TokenStream_la(parser->m_tokens, 1)));

        /* Pop the dedentation token from the follow set. */
        k_Parser_popFollowToken(parser);

        /* Consume and discard the dedent token. */
        k_Parser_match(parser, ZEN_TOKEN_DEDENTATION);
    // }

    k_StackTrace_exit();
}

bool k_Parser_isSimpleStatementFollow(k_TokenType_t type) {
    bool result = false;
    switch (type){
        case ZEN_TOKEN_SEMICOLON: /* simpleStatement -> emptyStatement */
        case ZEN_TOKEN_KEYWORD_VAR: /* simpleStatement -> variableDeclaration */
        case ZEN_TOKEN_KEYWORD_FINAL: /* simpleStatement -> constantDeclaration */
        case ZEN_TOKEN_KEYWORD_ASSERT: /* simpleStatement -> assertStatement */
        case ZEN_TOKEN_KEYWORD_BREAK: /* simpleStatement -> breakStatement */
        case ZEN_TOKEN_KEYWORD_CONTINUE: /* simpleStatement -> continueStatement */
        case ZEN_TOKEN_KEYWORD_RETURN: /* simpleStatement -> returnStatement */
        case ZEN_TOKEN_KEYWORD_THROW: /* simpleStatement -> throwStatement */
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
void k_Parser_simpleStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_SimpleStatementContext_t* context = k_SimpleStatementContext_new(node);

    /* If expressionStatement, emptyStatement, variableDeclaration,
     * constantDeclaration, assertStatement, breakStatement, continueStatement,
     * returnStatement, or throwStatement fails, discard tokens until the newline
     * token is encountered.
     */
    k_Parser_pushFollowToken(parser, ZEN_TOKEN_NEWLINE);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (k_Parser_isExpressionFollow(la1)) {
        k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_statement = expression;
        k_Parser_expression(parser, expression);
    }
    else {
        switch (la1) {
            case ZEN_TOKEN_SEMICOLON: {
                k_ASTNode_t* emptyStatement = k_ASTNode_new(node);
                context->m_statement = emptyStatement;
                k_Parser_emptyStatement(parser, emptyStatement);
                break;
            }

            case ZEN_TOKEN_KEYWORD_VAR: {
                k_ASTNode_t* variableDeclaration = k_ASTNode_new(node);
                context->m_statement = variableDeclaration;
                k_Parser_variableDeclaration(parser, variableDeclaration);
                break;
            }

            case ZEN_TOKEN_KEYWORD_FINAL: {
                k_ASTNode_t* constantDeclaration = k_ASTNode_new(node);
                context->m_statement = constantDeclaration;
                k_Parser_constantDeclaration(parser, constantDeclaration);
                break;
            }

            case ZEN_TOKEN_KEYWORD_ASSERT: {
                k_ASTNode_t* assertStatement = k_ASTNode_new(node);
                context->m_statement = assertStatement;
                k_Parser_assertStatement(parser, assertStatement);
                break;
            }

            case ZEN_TOKEN_KEYWORD_BREAK: {
                k_ASTNode_t* breakStatement = k_ASTNode_new(node);
                context->m_statement = breakStatement;
                k_Parser_breakStatement(parser, breakStatement);
                break;
            }

            case ZEN_TOKEN_KEYWORD_CONTINUE: {
                k_ASTNode_t* continueStatement = k_ASTNode_new(node);
                context->m_statement = continueStatement;
                k_Parser_continueStatement(parser, continueStatement);
                break;
            }

            case ZEN_TOKEN_KEYWORD_RETURN: {
                k_ASTNode_t* returnStatement = k_ASTNode_new(node);
                context->m_statement = returnStatement;
                k_Parser_returnStatement(parser, returnStatement);
                break;
            }

            case ZEN_TOKEN_KEYWORD_THROW: {
                k_ASTNode_t* throwStatement = k_ASTNode_new(node);
                context->m_statement = throwStatement;
                k_Parser_throwStatement(parser, throwStatement);
                break;
            }
        }
    }

    /* Pop the newline token from the follow set. */
    k_Parser_popFollowToken(parser);

    /* Match and discard the newline token. */
	k_Parser_match(parser, ZEN_TOKEN_NEWLINE);

    k_StackTrace_exit();
}

/*
 * statement
 * :    simpleStatement
 * |    compoundStatement
 * ;
 */
void k_Parser_statement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_StatementContext_t* context = k_StatementContext_new(node);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (k_Parser_isSimpleStatementFollow(la1)) {
        k_ASTNode_t* simpleStatement = k_ASTNode_new(node);
        context->m_simpleStatement = simpleStatement;
        k_Parser_simpleStatement(parser, simpleStatement);
    }
    else if (k_Parser_isCompoundStatementFollow(la1)) {
        k_ASTNode_t* compoundStatement = k_ASTNode_new(node);
        context->m_compoundStatement = compoundStatement;
        k_Parser_compoundStatement(parser, compoundStatement);
    }
    else {
        // TODO: Expected simple or compound statement
        k_Parser_reportAndRecover(parser, ZEN_TOKEN_KEYWORD_VAR);
    }

    k_StackTrace_exit();
}

bool k_Parser_isStatementFollow(k_TokenType_t type) {
    return k_Parser_isSimpleStatementFollow(type) || k_Parser_isCompoundStatementFollow(type);
}

/*
 * emptyStatement
 * :    ';'
 * ;
 */
void k_Parser_emptyStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    /* k_EmptyStatementContext_t* context = */ k_EmptyStatementContext_new(node);

    /* The empty statement rule has no context. */
    /* Match and discard the ';' token. */
	k_Parser_match(parser, ZEN_TOKEN_SEMICOLON);

    k_StackTrace_exit();
}

/*
 * variableDeclaration
 * :    'var' variableDeclarator (',' variableDeclarator)*
 * ;
 */
void k_Parser_variableDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_VariableDeclarationContext_t* context = k_VariableDeclarationContext_new(node);

    /* Match and discard the 'var' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_VAR);

	k_ASTNode_t* variableDeclarator = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_variableDeclarators, variableDeclarator);
	k_Parser_variableDeclarator(parser, variableDeclarator);

	while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

		variableDeclarator = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_variableDeclarators, variableDeclarator);
		k_Parser_variableDeclarator(parser, variableDeclarator);
	}

    k_StackTrace_exit();
}

/*
 * variableDeclarator
 * :    IDENTIFIER ('=' expression)?
 * ;
 */
void k_Parser_variableDeclarator(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_VariableDeclaratorContext_t* context = k_VariableDeclaratorContext_new(node);

    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    context->m_identifier = k_Parser_newTerminalNode(node, identifier);

	if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_EQUAL) {
        /* Consume and discard the '=' token. */
		k_TokenStream_consume(parser->m_tokens);

		k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_expression = expression;
		k_Parser_expression(parser, expression);
	}

    k_StackTrace_exit();
}

/* TODO: I have accidentally forwarded the node that we receive in the rule
 *       to child rules in many places. Please check and correct such bugs.
 */
/*
 * constantDeclaration
 * :    'final' constantDeclarator (',' constantDeclarator)*
 * ;
 */
void k_Parser_constantDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ConstantDeclarationContext_t* context = k_ConstantDeclarationContext_new(node);

    /* Match and discard the 'final' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_FINAL);

	k_ASTNode_t* constantDeclarator = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_constantDeclarators, constantDeclarator);
	k_Parser_constantDeclarator(parser, constantDeclarator);

	while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COMMA) {
        /* Match and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

		constantDeclarator = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_constantDeclarators, constantDeclarator);
		k_Parser_constantDeclarator(parser, constantDeclarator);
	}

    k_StackTrace_exit();
}

/* TODO: Please check if all the rules have contexts.
 */
/*
 * constantDeclarator
 * :    IDENTIFIER '=' expression
 * ;
 */
void k_Parser_constantDeclarator(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ConstantDeclaratorContext_t* context = k_ConstantDeclaratorContext_new(node);

	k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    context->m_identifier = k_Parser_newTerminalNode(node, identifier);

    /* Match and discard the '=' token. */
	k_Parser_match(parser, ZEN_TOKEN_EQUAL);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	k_Parser_expression(parser, expression);

    k_StackTrace_exit();
}

/*
 * assertStatement
 * :    'assert' expression (':' expression)?
 * ;
 */
void k_Parser_assertStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AssertStatementContext_t* context = k_AssertStatementContext_new(node);

    /* Match and discard the 'assert' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_ASSERT);

    k_ASTNode_t* conditionExpression = k_ASTNode_new(node);
    context->m_conditionExpression = conditionExpression;
    k_Parser_expression(parser, conditionExpression);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_IDENTIFIER) {
        /* Consume and discard the ':' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* messageExpression = k_ASTNode_new(node);
        context->m_messageExpression = messageExpression;
        k_Parser_expression(parser, messageExpression);
    }

    k_StackTrace_exit();
}

/*
 * breakStatement
 * :    'break' IDENTIFIER?
 * ;
 */
void k_Parser_breakStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_BreakStatementContext_t* context = k_BreakStatementContext_new(node);

    /* Match and discard the 'break' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_BREAK);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_IDENTIFIER) {
        k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_identifier = k_Parser_newTerminalNode(node, identifier);
        k_TokenStream_consume(parser->m_tokens);
    }

    k_StackTrace_exit();
}

/*
 * continueStatement
 * :    'continue' IDENTIFIER?
 * ;
 */
void k_Parser_continueStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ContinueStatementContext_t* context = k_ContinueStatementContext_new(node);

    /* Match and discard the 'continue' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_CONTINUE);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_IDENTIFIER) {
        k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_identifier = k_Parser_newTerminalNode(node, identifier);
        k_TokenStream_consume(parser->m_tokens);
    }

    k_StackTrace_exit();
}

/*
 * returnStatement
 * :    'return' expression
 * ;
 */
void k_Parser_returnStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ReturnStatementContext_t* context = k_ReturnStatementContext_new(node);

    /* Match and discard the 'return' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_RETURN);

    /* An expression is mandatory after the 'return' keyword. */
    k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
    k_Parser_expression(parser, expression);

    k_StackTrace_exit();
}

/*
 * throwStatement
 * :    'throw' expression?
 * ;
 */
void k_Parser_throwStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_ThrowStatementContext_t* context = k_ThrowStatementContext_new(node);

    /* Match and discard the 'throw' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_THROW);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_expression = expression;
        k_Parser_expression(parser, expression);
    }

    k_StackTrace_exit();
}

/*
 * compoundStatement
 * :    ifStatement
 * |    iterativeStatement
 * |    tryStatement
 * |    synchronizeStatement
 * |    withStatement
 * // |    functionDeclaration
 * // |    classDeclaration
 * ;
 */
void k_Parser_compoundStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_CompoundStatementContext_t* context = k_CompoundStatementContext_new(node);

	switch (k_TokenStream_la(parser->m_tokens, 1)) {
		case ZEN_TOKEN_KEYWORD_IF: {
			k_ASTNode_t* ifStatement = k_ASTNode_new(node);
            context->m_statement = ifStatement;
			k_Parser_ifStatement(parser, ifStatement);
			break;
		}

		case ZEN_TOKEN_HASH:
		case ZEN_TOKEN_KEYWORD_WHILE:
		case ZEN_TOKEN_KEYWORD_FOR: {
			k_ASTNode_t* iterativeStatement = k_ASTNode_new(node);
            context->m_statement = iterativeStatement;
			k_Parser_iterativeStatement(parser, iterativeStatement);
			break;
		}

		case ZEN_TOKEN_KEYWORD_TRY: {
			k_ASTNode_t* tryStatement = k_ASTNode_new(node);
            context->m_statement = tryStatement;
			k_Parser_tryStatement(parser, tryStatement);
			break;
		}

		case ZEN_TOKEN_KEYWORD_SYNCHRONIZE: {
			k_ASTNode_t* synchronizeStatement = k_ASTNode_new(node);
            context->m_statement = synchronizeStatement;
			k_Parser_synchronizeStatement(parser, synchronizeStatement);
			break;
		}

		case ZEN_TOKEN_KEYWORD_WITH: {
			k_ASTNode_t* withStatement = k_ASTNode_new(node);
            context->m_statement = withStatement;
			k_Parser_withStatement(parser, withStatement);
			break;
		}

		/*case ZEN_TOKEN_KEYWORD_FUNCTION: {
			k_ASTNode_t* functionDeclaration = k_ASTNode_new(node);
            context->m_statement = functionDeclaration;
			k_Parser_functionDeclaration(parser, functionDeclaration);
			break;
		}

		case ZEN_TOKEN_KEYWORD_CLASS: {
			k_ASTNode_t* classDeclaration = k_ASTNode_new(node);
            context->m_statement = classDeclaration;
			k_Parser_classDeclaration(parser, classDeclaration);
			break;
		}
        */

        /* NOTE: Enumerations are invalid within the scope of a function.
         * 1. Increases the complexity of the program. After all, we are
         *    looking to create the simplest general purpose scripting language!
         * 2. Once we exit the scope the enumeration becomes invalid. We are
         *    not interested giving birth to "anonymous enumerations" anyway.
         *
         * TODO: Add a grammar to report local enumeration declarations.
         */

		default: {
			printf("Control should not reach here.");

			break;
		}
	}

    k_StackTrace_exit();
}

bool k_Parser_isCompoundStatementFollow(k_TokenType_t type) {
    bool result = false;
    switch (type) {
		case ZEN_TOKEN_KEYWORD_IF: /* compoundStatement -> ifStatement */
		case ZEN_TOKEN_HASH: /* compoundStatement -> iterativeStatement */
		case ZEN_TOKEN_KEYWORD_WHILE: /* compoundStatement -> ... -> whileStatement */
		case ZEN_TOKEN_KEYWORD_FOR: /* compoundStatement -> ... -> forStatement */
		case ZEN_TOKEN_KEYWORD_TRY: /* compoundStatement -> tryStatement */
		case ZEN_TOKEN_KEYWORD_SYNCHRONIZE: /* compoundStatement -> synchronizeStatement */
		case ZEN_TOKEN_KEYWORD_WITH: /* compoundStatement -> withStatement */
		// case ZEN_TOKEN_KEYWORD_FUNCTION: /* compoundStatement -> functionDeclaration */
		// case ZEN_TOKEN_KEYWORD_CLASS: /* compoundStatement -> classDeclaration */
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
void k_Parser_ifStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_IfStatementContext_t* context = k_IfStatementContext_new(node);

	k_ASTNode_t* ifClause = k_ASTNode_new(node);
    context->m_ifClause = ifClause;
	k_Parser_ifClause(parser, ifClause);

	while ((k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_ELSE) &&
	       (k_TokenStream_la(parser->m_tokens, 2) == ZEN_TOKEN_KEYWORD_IF)) {
		k_ASTNode_t* elseIfClause = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_elseIfClauses, elseIfClause);
		k_Parser_elseIfClause(parser, elseIfClause);
	}

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_ELSE) {
        k_ASTNode_t* elseClause = k_ASTNode_new(node);
        context->m_elseClause = elseClause;
        k_Parser_elseClause(parser, elseClause);
    }

    k_StackTrace_exit();
}

/*
 * ifClause
 * :    'if' expression statementSuite
 * ;
 */
void k_Parser_ifClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_IfClauseContext_t* context = k_IfClauseContext_new(node);

    /* Match and discard the 'if' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_IF);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	k_Parser_expression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * elseIfClause
 * :    'else' 'if' expression statementSuite
 * ;
 */
void k_Parser_elseIfClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ElseIfClauseContext_t* context = k_ElseIfClauseContext_new(node);

    /* Match and discard the 'else' 'if' tokens. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_ELSE);
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_IF);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	k_Parser_expression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * elseClause
 * :    'else' statementSuite
 * ;
 */
void k_Parser_elseClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ElseClauseContext_t* context = k_ElseClauseContext_new(node);

    /* Match and discard the 'else' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_ELSE);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * iterativeStatement
 * :    labelClause? (whileStatement | forStatement)
 * ;
 */
void k_Parser_iterativeStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_IterativeStatementContext_t* context = k_IterativeStatementContext_new(node);

	if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_HASH) {
		k_ASTNode_t* labelClause = k_ASTNode_new(node);
        context->m_labelClause = labelClause;
		k_Parser_labelClause(parser, labelClause);
	}

	switch (k_TokenStream_la(parser->m_tokens, 1)) {
		case ZEN_TOKEN_KEYWORD_WHILE: {
			k_ASTNode_t* whileStatement = k_ASTNode_new(node);
            context->m_statement = whileStatement;
			k_Parser_whileStatement(parser, whileStatement);
			break;
		}

		case ZEN_TOKEN_KEYWORD_FOR: {
			k_ASTNode_t* forStatement = k_ASTNode_new(node);
            context->m_statement = forStatement;
			k_Parser_forStatement(parser, forStatement);
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
void k_Parser_labelClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_LabelClauseContext_t* context = k_LabelClauseContext_new(node);

    /* Match and discard the '#' token. */
	k_Parser_match(parser, ZEN_TOKEN_HASH);

	k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    context->m_identifier = k_Parser_newTerminalNode(node, identifier);

    k_StackTrace_exit();
}

/*
 * whileStatement
 * :    'while' expression statementSuite
 * ;
 *
 * TODO: Remove elseClause!
 */
void k_Parser_whileStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_WhileStatementContext_t* context = k_WhileStatementContext_new(node);

    /* Match and discard the 'while' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_WHILE);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	k_Parser_expression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    /*
	if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_ELSE) {
		k_ASTNode_t* elseClause = k_ASTNode_new(node);
        context->m_elseClause = elseClause;
		k_Parser_elseClause(parser, elseClause);
	}
    */

    k_StackTrace_exit();
}


/*
 * forParameter
 * :    ('var' | 'final')? IDENTIFIER
 * ;
 */
void k_Parser_forParameter(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ForParameterContext_t* context = k_ForParameterContext_new(node);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if ((la1 == ZEN_TOKEN_KEYWORD_VAR) || (la1 == ZEN_TOKEN_KEYWORD_FINAL)) {
        context->m_declaration = true;
        context->m_variable = (la1 == ZEN_TOKEN_KEYWORD_VAR);

        /* Consume 'var' or 'final' token. */
        k_TokenStream_consume(parser->m_tokens);
    }

    k_Token_t* identifierToken = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifier = k_Parser_newTerminalNode(parser, identifierToken);
    context->m_identifier = identifier;

    k_StackTrace_exit();
}

/*
 * forStatement
 * :    'for' forParameter 'in' expression statementSuite
 * ;
 *
 * TODO: Remove elseClause!
 */
void k_Parser_forStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_ForStatementContext_t* context = k_ForStatementContext_new(node);

    /* Match and discard the 'for' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_FOR);

    k_ASTNode_t* forParameter = k_ASTNode_new(node);
    context->m_forParameter = forParameter;
    k_Parser_forParameter(parser, forParameter);

    /* Match and discard the 'in' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_IN);

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	k_Parser_expression(parser, expression);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    /*
	if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_ELSE) {
		k_ASTNode_t* elseClause = k_ASTNode_new(node);
        context->m_elseClause = elseClause;
		k_Parser_elseClause(parser, elseClause);
	}
    */

    k_StackTrace_exit();
}

/*
 * tryStatement
 * :    tryClause catchClause* finallyClause?
 * ;
 */
void k_Parser_tryStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_TryStatementContext_t* context = k_TryStatementContext_new(node);

	bool hasCatch = false;
	bool hasFinally = false;

    k_Token_t* tryKeyword = k_TokenStream_lt(parser->m_tokens, 1);
    k_ASTNode_t* tryClause = k_ASTNode_new(node);
    context->m_tryClause = tryClause;
    k_Parser_tryClause(parser, tryClause);

	while ((k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_CATCH)) {
		hasCatch = true;

		k_ASTNode_t* catchClause = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_catchClauses, catchClause);
		k_Parser_catchClause(parser, catchClause);
	}

	if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_FINALLY) {
		hasFinally = true;
		k_ASTNode_t* finallyClause = k_ASTNode_new(node);
        context->m_finallyClause = finallyClause;
		k_Parser_finallyClause(parser, finallyClause);
	}

	if (!hasCatch && !hasFinally) {
		/* Try clause without catch or finally. According to the grammar of Zen,
         * this is not an error. However, the Zen specification requires a try
         * clause to be followed by at least a catch or finally clause.
         */
        k_ErrorHandler_handleSyntacticalError(parser->m_compiler->m_errorHandler,
            parser, ZEN_ERROR_CODE_TRY_STATEMENT_EXPECTS_CATCH_OR_FINALLY,
            tryKeyword, ZEN_TOKEN_UNKNOWN);
	}

    k_StackTrace_exit();
}

/*
 * tryClause
 * :    'try' statementSuite
 * ;
 */
void k_Parser_tryClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_TryClauseContext_t* context = k_TryClauseContext_new(node);

    /* Match and discard the 'try' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_TRY);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/* I am in a dilemma. Which one of the following constructs is the best?!
 *
 * catch RuntimeException | NullPointerException exception      (This is the construct which we currently recognize.)
 * catch RuntimeException | NullPointerException in exception
 *
 * catch RuntimeException or NullPointerException exception
 * catch RuntimeException or NullPointerException in exception
 */
/*
 * catchClause
 * :	'catch' catchFilter IDENTIFIER statementSuite'
 * ;
 */
void k_Parser_catchClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_CatchClauseContext_t* context = k_CatchClauseContext_new(node);

    /* Match and discard the 'catch' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_CATCH);

    k_ASTNode_t* catchFilter = k_ASTNode_new(node);
    context->m_catchFilter = catchFilter;
    k_Parser_catchFilter(parser, catchFilter);

    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    context->m_identifier = k_Parser_newTerminalNode(node, identifier);

    k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
    k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * catchFilter
 * :	typeName ('|' typeName)*
 * ;
 */
void k_Parser_catchFilter(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_CatchFilterContext_t* context = k_CatchFilterContext_new(node);

	k_ASTNode_t* typeName = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_typeNames, typeName);
	k_Parser_typeName(parser, typeName);

	while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_VERTICAL_BAR) {
        /* Consume and discard the '|' token. */
		k_TokenStream_consume(parser->m_tokens);

		typeName = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_typeNames, typeName);
		k_Parser_typeName(parser, typeName);
	}

    k_StackTrace_exit();
}

/*
 * typeName
 * :    IDENTIFIER ('.' IDENTIFIER)*
 * ;
 */
void k_Parser_typeName(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_TypeNameContext_t* context = k_TypeNameContext_new(node);

    k_Token_t* identifierToken = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    k_ASTNode_t* identifier = k_Parser_newTerminalNode(node, identifierToken);
    jtk_ArrayList_add(context->m_identifiers, identifier);

    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_DOT) {
        /* Consume and discard the '.' token. */
        k_TokenStream_consume(parser->m_tokens);

        identifierToken = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
        identifier = k_Parser_newTerminalNode(node, identifierToken);
        jtk_ArrayList_add(context->m_identifiers, identifier);
    }

    k_StackTrace_exit();
}

/* TODO: We are invoking consume() instead of match() at certain locations.
 *       Especially check at the starting of a rule.
 */
/*
 * finallyClause
 * :	'finally' statementSuite
 * ;
 */
void k_Parser_finallyClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_FinallyClauseContext_t* context = k_FinallyClauseContext_new(node);

    /* Match and discard the 'finally' token. */
	k_Parser_match(parser, ZEN_TOKEN_KEYWORD_FINALLY);

	k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
	k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * synchronizeStatement
 * :	'synchronize' expression statementSuite
 * ;
 */
void k_Parser_synchronizeStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_SynchronizeStatementContext_t* context = k_SynchronizeStatementContext_new(node);

    /* Match and discard the 'synchronize' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_SYNCHRONIZE);

    k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
    k_Parser_expression(parser, expression);

    k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
    k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * withStatement
 * :	'with' withParameters statementSuite
 * ;
 */
void k_Parser_withStatement(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_WithStatementContext_t* context = k_WithStatementContext_new(node);

    /* Match and discard the 'with' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_WITH);

    k_ASTNode_t* withParameters = k_ASTNode_new(node);
    context->m_withParameters = withParameters;
    k_Parser_withParameters(parser, withParameters);

    k_ASTNode_t* statementSuite = k_ASTNode_new(node);
    context->m_statementSuite = statementSuite;
    k_Parser_statementSuite(parser, statementSuite);

    k_StackTrace_exit();
}

/*
 * withParameters
 * :    withParameter (',' withParameter)*
 * ;
 */
void k_Parser_withParameters(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_WithParametersContext_t* context = k_WithParametersContext_new(node);

    k_ASTNode_t* withParameter = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_withParameters, withParameter);
    k_Parser_withParameter(parser, withParameter);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    while (la1 == ZEN_TOKEN_COMMA) {
        /* Match and discard the ',' token. */
        k_Parser_match(parser, ZEN_TOKEN_COMMA);

        k_ASTNode_t* withParameter0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_withParameters, withParameter0);
        k_Parser_withParameter(parser, withParameter0);

        la1 = k_TokenStream_la(parser->m_tokens, 1);
    }

    k_StackTrace_exit();
}

/*
 * withParameter
 * :    (('var' | 'final') IDENTIFIER '=' )? expression
 * ;
 */
void k_Parser_withParameter(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_WithParameterContext_t* context = k_WithParameterContext_new(node);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if ((la1 == ZEN_TOKEN_KEYWORD_VAR) || (la1 == ZEN_TOKEN_KEYWORD_FINAL)) {
        /* Consume 'var' or 'final' token. */
        k_TokenStream_consume(parser->m_tokens);

        context->m_variable = (la1 == ZEN_TOKEN_KEYWORD_VAR);

        k_Token_t* identifierToken = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
        k_ASTNode_t* identifier = k_Parser_newTerminalNode(parser, identifierToken);
        context->m_identifier = identifier;

        /* Match and discard the '=' token. */
        k_Parser_match(parser, ZEN_TOKEN_EQUAL);
    }

	k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
	k_Parser_expression(parser, expression);

    k_StackTrace_exit();
}

/* TODO: At many locations I have forgotten to consume and discard a token.
 *       Please search for such bugs. Look near la() and lt() calls.
 */
/*
 * classDeclaration
 * :	'class' IDENTIFIER classClassExtendsClause? classSuite
 * ;
 */
void k_Parser_classDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassDeclarationContext_t* context = k_ClassDeclarationContext_new(node);

    /* Match and discard the 'class' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_CLASS);

    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    context->m_identifier = k_Parser_newTerminalNode(node, identifier);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_LEFT_ANGLE_BRACKET_EQUAL) {
        k_ASTNode_t* classClassExtendsClause = k_ASTNode_new(node);
        context->m_classExtendsClause = classClassExtendsClause;
        k_Parser_classExtendsClause(parser, classClassExtendsClause);
    }

    k_ASTNode_t* classSuite = k_ASTNode_new(node);
    context->m_classSuite = classSuite;
    k_Parser_classSuite(parser, classSuite);

    k_StackTrace_exit();
}

/*
 * classClassExtendsClause
 * :	'<=' typeName (',' typeName)*
 * ;
 */
void k_Parser_classExtendsClause(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassExtendsClauseContext_t* context = k_ClassExtendsClauseContext_new(node);

    /* Consume and discard the '<=' token. */
    k_Parser_match(parser, ZEN_TOKEN_LEFT_ANGLE_BRACKET_EQUAL);

    k_ASTNode_t* typeName = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_typeNames, typeName);
    k_Parser_typeName(parser, typeName);

    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        typeName = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_typeNames, typeName);
        k_Parser_typeName(parser, typeName);
    }

    k_StackTrace_exit();
}

/*
 * classSuite
 * :	NEWLINE INDENT classMember+ DEDENT
 * ;
 */
void k_Parser_classSuite(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassSuiteContext_t* context = k_ClassSuiteContext_new(node);

    /* Match and discard the newline and indent tokens. */
    k_Parser_match(parser, ZEN_TOKEN_NEWLINE);
    k_Parser_match(parser, ZEN_TOKEN_INDENTATION);

    do {
        k_ASTNode_t* classMember = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_classMembers, classMember);
        k_Parser_classMember(parser, classMember);
    }
    while (k_Parser_isClassMemberFollow(k_TokenStream_la(parser->m_tokens, 1)));

    /* Match and discard the dedent token. */
    k_Parser_match(parser, ZEN_TOKEN_DEDENTATION);

    k_StackTrace_exit();
}

bool k_Parser_isClassMemberFollow(k_TokenType_t type) {
    return k_Parser_isClassMemberModifier(type) || /* classMember -> modifier */
           (type == ZEN_TOKEN_KEYWORD_VAR)        || /* classMember -> ... -> variableDeclaration */
           (type == ZEN_TOKEN_KEYWORD_FINAL)      || /* classMember -> ... -> constantDeclaration */
           (type == ZEN_TOKEN_KEYWORD_FUNCTION)   || /* classMember -> ... -> functionDeclaration */
           // (type == ZEN_TOKEN_IDENTIFIER)      || /* classMember -> ... -> constructorDeclaration */
           // (type == ZEN_TOKEN_KEYWORD_CLASS)      || /* classMember -> ... -> classDeclaration */
           // (type == ZEN_TOKEN_KEYWORD_ENUM)       || /* classMember -> ... -> enumerationDeclaration */
           (type == ZEN_TOKEN_AT);                   /* classMember -> ... -> annotatedClassMember */
}

/*
 * classMember
 * :	annotations? classMemberModifier* unmodifiedClassMember
 * ;
 *
 * unmodifiedClassMember
 * :	variableDeclaration
 * |	constantDeclaration
 * |	functionDeclaration
 * // |	constructorDeclaration
 * // |	classDeclaration
 * // |	enumerationDeclaration
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void k_Parser_classMember(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ClassMemberContext_t* context = k_ClassMemberContext_new(node);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_AT) {
        k_ASTNode_t* annotations = k_ASTNode_new(node);
        context->m_annotations = annotations;
        k_Parser_annotations(parser, annotations);
    }

    uint32_t modifiers = 0;
    while (k_Parser_isClassMemberModifier(k_TokenStream_la(parser->m_tokens, 1))) {
        k_Token_t* modifierToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* modifier = k_Parser_newTerminalNode(node, modifierToken);
        jtk_ArrayList_add(context->m_modifiers, modifier);
        k_TokenStream_consume(parser->m_tokens);

        modifiers |= k_TokenType_toModifiers(modifierToken->m_type);
    }

    switch (k_TokenStream_la(parser->m_tokens, 1)) {
        case ZEN_TOKEN_KEYWORD_VAR: {
            /* If variableDeclaration fails, discard tokens until the newline
             * token is encountered.
             */
            k_Parser_pushFollowToken(parser, ZEN_TOKEN_NEWLINE);

            k_ASTNode_t* variableDeclaration = k_ASTNode_new(node);
            context->m_declaration = variableDeclaration;
            k_Parser_variableDeclaration(parser, variableDeclaration);

            /* Pop the newline token from the follow set. */
            k_Parser_popFollowToken(parser);

            /* Match and discard the newline token. */
            k_Parser_match(parser, ZEN_TOKEN_NEWLINE);

            break;
        }

        case ZEN_TOKEN_KEYWORD_FINAL: {
            /* If constantDeclaration fails, discard tokens until the newline
             * token is encountered.
             */
            k_Parser_pushFollowToken(parser, ZEN_TOKEN_NEWLINE);

            k_ASTNode_t* constantDeclaration = k_ASTNode_new(node);
            context->m_declaration = constantDeclaration;
            k_Parser_constantDeclaration(parser, constantDeclaration);

            /* Pop the newline token from the follow set. */
            k_Parser_popFollowToken(parser);

            /* Match and discard the newline token. */
            k_Parser_match(parser, ZEN_TOKEN_NEWLINE);

            break;
        }

        case ZEN_TOKEN_KEYWORD_FUNCTION: {
            k_ASTNode_t* functionDeclaration = k_ASTNode_new(node);
            context->m_declaration = functionDeclaration;
            k_Parser_functionDeclaration(parser, functionDeclaration, modifiers);

            break;
        }

//         case ZEN_TOKEN_IDENTIFIER: {
//             k_ASTNode_t* constructorDeclaration = k_ASTNode_new(node);
//             context->m_declaration = constructorDeclaration;
//             k_Parser_constructorDeclaration(parser, constructorDeclaration);

//             break;
//         }

        // case ZEN_TOKEN_KEYWORD_CLASS: {
        //     k_ASTNode_t* classDeclaration = k_ASTNode_new(node);
        //     context->m_declaration = classDeclaration;
        //     k_Parser_classDeclaration(parser, classDeclaration);

        //     break;
        // }

        // case ZEN_TOKEN_KEYWORD_ENUM: {
        //     k_ASTNode_t* enumerationDeclaration = k_ASTNode_new(node);
        //     context->m_declaration = enumerationDeclaration;
        //     k_Parser_enumerationDeclaration(parser, enumerationDeclaration);

        //     break;
        // }

        default: {
            /* TODO: Expected var, final, or function. */
            k_Parser_reportAndRecover(parser, ZEN_TOKEN_KEYWORD_FUNCTION);

            break;
        }
    }

    k_StackTrace_exit();
}

/*
 * classMemberModifier
 * :    'static'
 * |    'native'
 * |    'abstract'
 * |    'public'
 * |    'private'
 * |    'secret'
 * ;
 */
bool k_Parser_isClassMemberModifier(k_TokenType_t type) {
    return (type == ZEN_TOKEN_KEYWORD_STATIC) ||
           (type == ZEN_TOKEN_KEYWORD_NATIVE) ||
           (type == ZEN_TOKEN_KEYWORD_ABSTRACT) ||
           (type == ZEN_TOKEN_KEYWORD_PUBLIC) ||
           (type == ZEN_TOKEN_KEYWORD_PRIVATE) ||
           (type == ZEN_TOKEN_KEYWORD_SECRET);
}

/*
 * constructorDeclaration
 * :	IDENTIFIER functionParameters? statementSuite
 * ;
 *
 */
// void k_Parser_constructorDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
//     k_StackTrace_enter();
//
//     k_ConstructorDeclarationContext_t* context = k_ConstructorDeclarationContext_new(node);
//
//     k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
//     context->m_identifier = k_Parser_newTerminalNode(node, identifier);
//
//     if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_LEFT_PARENTHESIS) {
//         k_ASTNode_t* functionParameters = k_ASTNode_new(node);
//         context->m_functionParameters = functionParameters;
//         k_Parser_functionParameters(parser, functionParameters);
//     }
//
//     k_ASTNode_t* statementSuite = k_ASTNode_new(node);
//     context->m_statementSuite = statementSuite;
//     k_Parser_statementSuite(parser, statementSuite);
//
//     k_StackTrace_exit();
// }

/*
 * enumerationDeclaration
 * :   'enum' IDENTIFIER enumerationBaseClause? enumerationSuite
 * ;
 */
// void k_Parser_enumerationDeclaration(k_Parser_t* parser, k_ASTNode_t* node) {
//     k_StackTrace_enter();

//     k_EnumerationDeclarationContext_t* context = k_EnumerationDeclarationContext_new(node);

//     /* Match and discard the 'enum' token. */
//     k_Parser_match(parser, ZEN_TOKEN_KEYWORD_ENUM);

//     k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
//     context->m_identifier = k_Parser_newTerminalNode(node, identifier);

//     if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COLON) {
//         k_ASTNode_t* enumerationBaseClause = k_ASTNode_new(node);
//         context->m_enumerationBaseClause = enumerationBaseClause;
//         k_Parser_enumerationBaseClause(parser, enumerationBaseClause);
//     }

//     k_ASTNode_t* enumerationSuite = k_ASTNode_new(node);
//     context->m_enumerationSuite = enumerationSuite;
//     k_Parser_enumerationSuite(parser, enumerationSuite);

//     k_StackTrace_exit();
// }

/*
 * enumerationBaseClause
 * :   ':' typeName
 * ;
 */
// void k_Parser_enumerationBaseClause(k_Parser_t* parser, k_ASTNode_t* node) {
//     k_StackTrace_enter();

//     k_EnumerationBaseClauseContext_t* context = k_EnumerationBaseClauseContext_new(node);

//     /* Match and discard the ':' token. */
//     k_Parser_match(parser, ZEN_TOKEN_COLON);

//     k_ASTNode_t* typeName = k_ASTNode_new(node);
//     context->m_typeName = typeName;
//     k_Parser_typeName(parser, typeName);

//     k_StackTrace_exit();
// }

/*
 * enumerationSuite
 * :   NEWLINE INDENT enumerate+ DEDENT
 * ;
 */
// void k_Parser_enumerationSuite(k_Parser_t* parser, k_ASTNode_t* node) {
//     k_StackTrace_enter();

//     k_EnumerationSuiteContext_t* context = k_EnumerationSuiteContext_new(node);

//     /* Match and discard the newline token. */
//     k_Parser_match(parser, ZEN_TOKEN_NEWLINE);
//     /* Match and discard the indent token. */
//     k_Parser_match(parser, ZEN_TOKEN_INDENTATION);

//     do {
//         k_ASTNode_t* enumerate = k_ASTNode_new(node);
//         jtk_ArrayList_add(context->m_enumerates, enumerate);
//         k_Parser_enumerate(parser, enumerate);
//     }
//     while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_IDENTIFIER);

//     /* Match and discard the dedent token. */
//     k_Parser_match(parser, ZEN_TOKEN_DEDENTATION);

//     k_StackTrace_exit();
// }

/*
 * enumerate
 * :   IDENTIFIER functionArguments? NEWLINE
 * ;
 */
// void k_Parser_enumerate(k_Parser_t* parser, k_ASTNode_t* node) {
//     k_StackTrace_enter();

//     k_EnumerateContext_t* context = k_EnumerateContext_new(node);

//     k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
//     context->m_identifier = k_Parser_newTerminalNode(node, identifier);

//     if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_LEFT_PARENTHESIS) {
//         k_ASTNode_t* functionArguments = k_ASTNode_new(node);
//         context->m_functionArguments = functionArguments;
//         k_Parser_functionArguments(parser, functionArguments);
//     }

//     /* Match and discard the newline token. */
//     k_Parser_match(parser, ZEN_TOKEN_NEWLINE);

//     k_StackTrace_exit();
// }

/*
 * expression
 * :    expression (',' expression)*
 * ;
 */
void k_Parser_expressions(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ExpressionsContext_t* context = k_ExpressionsContext_new(node);

    k_ASTNode_t* expression = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_expressions, expression);
    k_Parser_expression(parser, expression);

    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        expression = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_expressions, expression);
        k_Parser_expression(parser, expression);
    }

    k_StackTrace_exit();
}

/*
 * expression
 * :	assignmentExpression
 * ;
 *
 */
void k_Parser_expression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ExpressionContext_t* context = k_ExpressionContext_new(node);

    k_ASTNode_t* assignmentExpression = k_ASTNode_new(node);
    context->m_assignmentExpression = assignmentExpression;
    k_Parser_assignmentExpression(parser, assignmentExpression);
}

bool k_Parser_isExpressionFollow(k_TokenType_t type) {
    return k_Parser_isUnaryExpressionFollow(type);
}

/*
 * assignmentExpression
 * :	conditionalExpression (assignmentOperator assignmentExpression)?
 * ;
 */
void k_Parser_assignmentExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AssignmentExpressionContext_t* context = k_AssignmentExpressionContext_new(node);

    k_ASTNode_t* conditionalExpression = k_ASTNode_new(node);
    context->m_conditionalExpression = conditionalExpression;
    k_Parser_conditionalExpression(parser, conditionalExpression);

    if (k_Parser_isAssignmentOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        k_Token_t* assignmentOperator = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_assignmentOperator = k_Parser_newTerminalNode(node, assignmentOperator);
        /* Consume the assignment operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* assignmentExpression = k_ASTNode_new(node);
        context->m_assignmentExpression = assignmentExpression;
        k_Parser_assignmentExpression(parser, assignmentExpression);
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
bool k_Parser_isAssignmentOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_EQUAL) ||
           (type == ZEN_TOKEN_ASTERISK_EQUAL) ||
           (type == ZEN_TOKEN_FORWARD_SLASH_EQUAL) ||
           (type == ZEN_TOKEN_MODULUS_EQUAL) ||
           (type == ZEN_TOKEN_PLUS_EQUAL) ||
           (type == ZEN_TOKEN_DASH_EQUAL) ||
           (type == ZEN_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == ZEN_TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL) ||
           (type == ZEN_TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL) ||
           (type == ZEN_TOKEN_AMPERSAND_EQUAL) ||
           (type == ZEN_TOKEN_CARET_EQUAL) ||
           (type == ZEN_TOKEN_VERTICAL_BAR_EQUAL);
}

/*
 * conditionalExpression
 * :	logicalOrExpression ('then' expression 'else' conditionalExpression)?
 * ;
 */
void k_Parser_conditionalExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ConditionalExpressionContext_t* context = k_ConditionalExpressionContext_new(node);

    k_ASTNode_t* logicalOrExpression = k_ASTNode_new(node);
    context->m_logicalOrExpression = logicalOrExpression;
    k_Parser_logicalOrExpression(parser, logicalOrExpression);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_HOOK) {
        /* Consume and discard the '?' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* expression = k_ASTNode_new(node);
        context->m_thenExpression = expression;
        k_Parser_expression(parser, expression);

		/* Consume and discard the ':' token. */
        k_Parser_match(parser, ZEN_TOKEN_COLON);

        k_ASTNode_t* conditionalExpression = k_ASTNode_new(node);
        context->m_elseExpression = conditionalExpression;
        k_Parser_conditionalExpression(parser, conditionalExpression);
    }

    k_StackTrace_exit();
}

/*
 * logicalOrExpression
 * :	logicalAndExpression ('or' logicalAndExpression)*
 * ;
 */
void k_Parser_logicalOrExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_LogicalOrExpressionContext_t* context = k_LogicalOrExpressionContext_new(node);

    k_ASTNode_t* logicalAndExpression = k_ASTNode_new(node);
    context->m_logicalAndExpression = logicalAndExpression;
    k_Parser_logicalAndExpression(parser, logicalAndExpression);

    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_OR) {
        /* Consume and discard the 'or' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* logicalAndExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_logicalAndExpressions, logicalAndExpression0);
        k_Parser_logicalAndExpression(parser, logicalAndExpression0);
    }

    k_StackTrace_exit();
}

/*
 * logicalAndExpression
 * :	inclusiveOrExpression ('and' logicalAndExpression)?
 * ;
 */
void k_Parser_logicalAndExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_LogicalAndExpressionContext_t* context = k_LogicalAndExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* inclusiveOrExpression = k_ASTNode_new(node);
    context->m_inclusiveOrExpression = inclusiveOrExpression;
    k_Parser_inclusiveOrExpression(parser, inclusiveOrExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_KEYWORD_AND) {
        /* Consume and discard the 'and' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* inclusiveOrExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_inclusiveOrExpressions, inclusiveOrExpression0);
        k_Parser_inclusiveOrExpression(parser, inclusiveOrExpression0);
    }

    k_StackTrace_exit();
}

/*
 * inclusiveOrExpression
 * :	exclusiveOrExpression ('|' exclusiveOrExpression)*
 * ;
 */
void k_Parser_inclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_InclusiveOrExpressionContext_t* context = k_InclusiveOrExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* exclusiveOrExpression = k_ASTNode_new(node);
    context->m_exclusiveOrExpression = exclusiveOrExpression;
    k_Parser_exclusiveOrExpression(parser, exclusiveOrExpression);

    /* Parse the expression to the right of the operator, if any. */
    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_VERTICAL_BAR) {
        /* Consume and discard the '|' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* exclusiveOrExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_exclusiveOrExpressions, exclusiveOrExpression0);
        k_Parser_exclusiveOrExpression(parser, exclusiveOrExpression0);
    }

    k_StackTrace_exit();
}

/*
 * exclusiveOrExpression
 * :	andExpression ('^' andExpression)*
 * ;
 */
void k_Parser_exclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ExclusiveOrExpressionContext_t* context = k_ExclusiveOrExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* andExpression = k_ASTNode_new(node);
    context->m_andExpression = andExpression;
    k_Parser_andExpression(parser, andExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_CARET) {
        /* Consume and discard the '^' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* andExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_andExpressions, andExpression0);
        k_Parser_andExpression(parser, andExpression0);
    }

    k_StackTrace_exit();
}

/*
 * andExpression
 * :	equalityExpression ('&' equalityExpression)*
 * ;
 */
void k_Parser_andExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AndExpressionContext_t* context = k_AndExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* equalityExpression = k_ASTNode_new(node);
    context->m_equalityExpression = equalityExpression;
    k_Parser_equalityExpression(parser, equalityExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_AMPERSAND) {

        /* Consume and discard the '&' token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* equalityExpression0 = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_equalityExpressions, equalityExpression0);
        k_Parser_equalityExpression(parser, equalityExpression0);
    }

    k_StackTrace_exit();
}

/*
 * equalityExpression
 * :	relationalExpression (equalityOperator relationalExpression)*
 * ;
 */
void k_Parser_equalityExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_EqualityExpressionContext_t* context = k_EqualityExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* relationalExpression = k_ASTNode_new(node);
    context->m_relationalExpression = relationalExpression;
    k_Parser_relationalExpression(parser, relationalExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_Parser_isEqualityOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_relationalExpressions, pair);

        k_Token_t* equalityOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* equalityOperator = k_Parser_newTerminalNode(node, equalityOperatorToken);
        pair->m_left = equalityOperator;
        /* Consume the equality operator. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* relationalExpression0 = k_ASTNode_new(node);
        pair->m_right = relationalExpression0;
        k_Parser_relationalExpression(parser, relationalExpression0);
    }

    k_StackTrace_exit();
}

/*
 * equalityOperator
 * :	'=='
 * |	'!='
 * ;
 */
bool k_Parser_isEqualityOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_EQUAL_2) ||
           (type == ZEN_TOKEN_EXCLAMATION_MARK_EQUAL);
}

/*
 * relationalExpression
 * :	shiftExpression (relationalOperator shiftExpression)*
 * ;
 */
void k_Parser_relationalExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_RelationalExpressionContext_t* context = k_RelationalExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* shiftExpression = k_ASTNode_new(node);
    context->m_shiftExpression = shiftExpression;
    k_Parser_shiftExpression(parser, shiftExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_Parser_isRelationalOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_shiftExpressions, pair);

        k_Token_t* relationalOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* relationalOperator = k_Parser_newTerminalNode(node, relationalOperatorToken);
        pair->m_left = relationalOperator;
        /* Consume the relational operator. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* shiftExpression0 = k_ASTNode_new(node);
        pair->m_right = shiftExpression0;
        k_Parser_shiftExpression(parser, shiftExpression0);
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
bool k_Parser_isRelationalOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_LEFT_ANGLE_BRACKET) ||
           (type == ZEN_TOKEN_RIGHT_ANGLE_BRACKET) ||
           (type == ZEN_TOKEN_LEFT_ANGLE_BRACKET_EQUAL) ||
           (type == ZEN_TOKEN_RIGHT_ANGLE_BRACKET_EQUAL) ||
           (type == ZEN_TOKEN_KEYWORD_IS);
}

/*
 * shiftExpression
 * :	additiveExpression (shiftOperator additiveExpression)*
 * ;
 */
void k_Parser_shiftExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ShiftExpressionContext_t* context = k_ShiftExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* additiveExpression = k_ASTNode_new(node);
    context->m_additiveExpression = additiveExpression;
    k_Parser_additiveExpression(parser, additiveExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_Parser_isShiftOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_additiveExpressions, pair);

        k_Token_t* shiftOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* shiftOperator = k_Parser_newTerminalNode(node, shiftOperatorToken);
        pair->m_left = shiftOperator;
        /* Consume the shift operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* additiveExpression0 = k_ASTNode_new(node);
        pair->m_right = additiveExpression0;
        k_Parser_additiveExpression(parser, additiveExpression0);
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
bool k_Parser_isShiftOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_LEFT_ANGLE_BRACKET_2) ||
           (type == ZEN_TOKEN_RIGHT_ANGLE_BRACKET_2) ||
           (type == ZEN_TOKEN_RIGHT_ANGLE_BRACKET_3);
}

/*
 * additiveExpression
 * :	multiplicativeExpression (multiplicativeOperator multiplicativeExpression)*
 * ;
 */
void k_Parser_additiveExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_AdditiveExpressionContext_t* context = k_AdditiveExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* multiplicativeExpression = k_ASTNode_new(node);
    context->m_multiplicativeExpression = multiplicativeExpression;
    k_Parser_multiplicativeExpression(parser, multiplicativeExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_Parser_isAdditiveOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_multiplicativeExpressions, pair);

        k_Token_t* additiveOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* additiveOperator = k_Parser_newTerminalNode(node, additiveOperatorToken);
        pair->m_left = additiveOperator;
        /* Consume the additive operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* multiplicativeExpression0 = k_ASTNode_new(node);
        pair->m_right = multiplicativeExpression0;
        k_Parser_multiplicativeExpression(parser, multiplicativeExpression0);
    }

    k_StackTrace_exit();
}

/*
 * additiveOperator
 * :	'+'
 * |	'-'
 * ;
 */
bool k_Parser_isAdditiveOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_PLUS) ||
           (type == ZEN_TOKEN_DASH);
}

/*
 * multiplicativeExpression
 * :	unaryExpression (multiplicativeOperator unaryExpression)*
 * ;
 */
void k_Parser_multiplicativeExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MultiplicativeExpressionContext_t* context = k_MultiplicativeExpressionContext_new(node);

    /* Parse the operand left of the operator. */
    k_ASTNode_t* unaryExpression = k_ASTNode_new(node);
    context->m_unaryExpression = unaryExpression;
    k_Parser_unaryExpression(parser, unaryExpression);

    /* Parse the expression to the right of the operator, if any. */
    while (k_Parser_isMultiplicativeOperator(k_TokenStream_la(parser->m_tokens, 1))) {
        jtk_Pair_t* pair = jtk_Pair_new();
        jtk_ArrayList_add(context->m_unaryExpressions, pair);

        k_Token_t* multiplicativeOperatorToken = k_TokenStream_lt(parser->m_tokens, 1);
        k_ASTNode_t* multiplicativeOperator = k_Parser_newTerminalNode(node, multiplicativeOperatorToken);
        pair->m_left = multiplicativeOperator;
        /* Consume the multiplicative operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* unaryExpression0 = k_ASTNode_new(node);
        pair->m_right = unaryExpression0;
        k_Parser_unaryExpression(parser, unaryExpression0);
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
bool k_Parser_isMultiplicativeOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_ASTERISK) ||
           (type == ZEN_TOKEN_FORWARD_SLASH) ||
           (type == ZEN_TOKEN_MODULUS);
}

/*
 * unaryExpression
 * :	unaryOperator unaryExpression
 * |	postfixExpression
 * ;
 */
void k_Parser_unaryExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_UnaryExpressionContext_t* context = k_UnaryExpressionContext_new(node);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (k_Parser_isUnaryOperator(la1)) {
        k_Token_t* unaryOperator = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_unaryOperator = k_Parser_newTerminalNode(node, unaryOperator);
        /* Consume the unary operator token. */
        k_TokenStream_consume(parser->m_tokens);

        k_ASTNode_t* unaryExpression = k_ASTNode_new(node);
        context->m_unaryExpression = unaryExpression;
        k_Parser_unaryExpression(parser, unaryExpression);
    }
    else if (k_Parser_isPostfixExpressionFollow(la1)) {
        k_ASTNode_t* postfixExpression = k_ASTNode_new(node);
        context->m_postfixExpression = postfixExpression;
        k_Parser_postfixExpression(parser, postfixExpression);
    }
    else {
        // Error: Expected unary operator or postfix expression follow
        k_Parser_reportAndRecover(parser, ZEN_TOKEN_INTEGER_LITERAL);
    }

    k_StackTrace_exit();
}

bool k_Parser_isUnaryExpressionFollow(k_TokenType_t type) {
    return k_Parser_isUnaryOperator(type) || k_Parser_isPostfixExpressionFollow(type);
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
bool k_Parser_isUnaryOperator(k_TokenType_t type) {
    return (type == ZEN_TOKEN_PLUS) ||
           (type == ZEN_TOKEN_DASH) ||
           (type == ZEN_TOKEN_TILDE) ||
           (type == ZEN_TOKEN_EXCLAMATION_MARK);
           /*
           ||
           (type == ZEN_TOKEN_PLUS_2) ||
           (type == ZEN_TOKEN_DASH_2);
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
 * |    postfixOperator
 * ;
 *
 * The following function combines both the rules. This measure was
 * taken to avoid redundant nodes in the AST.
 */
void k_Parser_postfixExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_PostfixExpressionContext_t* context = k_PostfixExpressionContext_new(node);

    k_ASTNode_t* primaryExpression = k_ASTNode_new(node);
    context->m_primaryExpression = primaryExpression;
    k_Parser_primaryExpression(parser, primaryExpression);

    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    while (k_Parser_isPostfixPartFollow(la1)) {
        switch (la1) {
            case ZEN_TOKEN_LEFT_SQUARE_BRACKET: {
                k_ASTNode_t* subscript = k_ASTNode_new(node);
                jtk_ArrayList_add(context->m_postfixParts, subscript);
                k_Parser_subscript(parser, subscript);
                break;
            }

            case ZEN_TOKEN_LEFT_PARENTHESIS: {
                k_ASTNode_t* functionArguments = k_ASTNode_new(node);
                jtk_ArrayList_add(context->m_postfixParts, functionArguments);

                k_Parser_functionArguments(parser, functionArguments);

                break;
            }

            case ZEN_TOKEN_DOT: {
                k_ASTNode_t* memberAccess = k_ASTNode_new(node);
                jtk_ArrayList_add(context->m_postfixParts, memberAccess);
                k_Parser_memberAccess(parser, memberAccess);
                break;
            }

            // case ZEN_TOKEN_PLUS_2:
            // case ZEN_TOKEN_DASH_2: {
            //     k_ASTNode_t* postfixOperator = k_ASTNode_new(node);
            //     jtk_ArrayList_add(context->m_postfixParts, postfixOperator);
            //     k_Parser_postfixOperator(parser, postfixOperator);
            //     break;
            // }
        }
        la1 = k_TokenStream_la(parser->m_tokens, 1);
    }

    k_StackTrace_exit();
}

bool k_Parser_isPostfixExpressionFollow(k_TokenType_t type) {
    return k_Parser_isPrimaryExpressionFollow(type);
}

bool k_Parser_isPostfixPartFollow(k_TokenType_t type) {
    return (type == ZEN_TOKEN_LEFT_SQUARE_BRACKET) ||
           (type == ZEN_TOKEN_LEFT_PARENTHESIS) ||
           (type == ZEN_TOKEN_DOT) ||
           (type == ZEN_TOKEN_PLUS_2) ||
           (type == ZEN_TOKEN_DASH_2);
}

/*
 * subscript
 * :	'[' expression ']'
 * ;
 */
void k_Parser_subscript(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_SubscriptContext_t* context = k_SubscriptContext_new(node);

    /* Match and discard the '[' token. */
    k_Parser_match(parser, ZEN_TOKEN_LEFT_SQUARE_BRACKET);

    /* If expression fails, discard tokens until the ']' token is
     * encountered.
     */
    k_Parser_pushFollowToken(parser, ZEN_TOKEN_RIGHT_SQUARE_BRACKET);

    k_ASTNode_t* expression = k_ASTNode_new(node);
    context->m_expression = expression;
    k_Parser_expression(parser, expression);

    /* Pop the ']' token from the follow set. */
    k_Parser_popFollowToken(parser);

    /* Match and discard the ']' token. */
    k_Parser_match(parser, ZEN_TOKEN_RIGHT_SQUARE_BRACKET);

    k_StackTrace_exit();
}

/*
 * functionArguments
 * :	'(' expressions? ')'
 * ;
 */
void k_Parser_functionArguments(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_FunctionArgumentsContext_t* context = k_FunctionArgumentsContext_new(node);

    /* Match and discard the '(' token. */
    k_Parser_match(parser, ZEN_TOKEN_LEFT_PARENTHESIS);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        /* If expression fails, discard tokens until the ')' token is
        * encountered.
        */
        k_Parser_pushFollowToken(parser, ZEN_TOKEN_RIGHT_PARENTHESIS);

        k_ASTNode_t* expressions = k_ASTNode_new(node);
        context->m_expressions = expressions;
        k_Parser_expressions(parser, expressions);

        /* Pop the ')' token from the follow set. */
        k_Parser_popFollowToken(parser);
    }

    /* Match and discard the ')' token. */
    k_Parser_match(parser, ZEN_TOKEN_RIGHT_PARENTHESIS);

    k_StackTrace_exit();
}

/*
 * memberAccess
 * :	'.' IDENTIFIER
 * ;
 */
void k_Parser_memberAccess(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MemberAccessContext_t* context = k_MemberAccessContext_new(node);

    /* Match and discard the '.' token. */
    k_Parser_match(parser, ZEN_TOKEN_DOT);

    k_Token_t* identifier = k_Parser_matchAndYield(parser, ZEN_TOKEN_IDENTIFIER);
    context->m_identifier = k_Parser_newTerminalNode(node, identifier);

    k_StackTrace_exit();
}

/*
 * postfixOperator
 * :	'++'
 * |	'--'
 * ;
 */
void k_Parser_postfixOperator(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_PostfixOperatorContext_t* context = k_PostfixOperatorContext_new(node);

    switch (k_TokenStream_la(parser->m_tokens, 1)) {
        case ZEN_TOKEN_PLUS_2:
        case ZEN_TOKEN_DASH_2: {
            k_Token_t* postfixOperator = k_TokenStream_lt(parser->m_tokens, 1);
            context->m_postfixOperator = k_Parser_newTerminalNode(node, postfixOperator);
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
 * |	listExpression
 * |    newExpression
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
void k_Parser_primaryExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_PrimaryExpressionContext_t* context = k_PrimaryExpressionContext_new(node);
    k_TokenType_t la1 = k_TokenStream_la(parser->m_tokens, 1);
    if (k_Parser_isLiteralFollow(la1)) {
        k_Token_t* literal = k_TokenStream_lt(parser->m_tokens, 1);
        context->m_expression = k_Parser_newTerminalNode(node, literal);
        /* Consume the literal token. */
        k_TokenStream_consume(parser->m_tokens);
    }
    else {
        switch (la1) {
            case ZEN_TOKEN_IDENTIFIER: {
                k_Token_t* identifier = k_TokenStream_lt(parser->m_tokens, 1);
                context->m_expression = k_Parser_newTerminalNode(node, identifier);
                /* Consume the identifier token. */
                k_TokenStream_consume(parser->m_tokens);

                break;
            }

            case ZEN_TOKEN_LEFT_PARENTHESIS: {
                /* Consume and discard the '(' token. */
                k_TokenStream_consume(parser->m_tokens);

                /* If expression fails, discard tokens until the ')' token is
                 * encountered.
                 */
                k_Parser_pushFollowToken(parser, ZEN_TOKEN_RIGHT_PARENTHESIS);

                k_ASTNode_t* expression = k_ASTNode_new(node);
                context->m_expression = expression;
                k_Parser_expression(parser, expression);

                /* Pop the ')' token from the follow set. */
                k_Parser_popFollowToken(parser);

                /* Match and discard the ')' token. */
                k_Parser_match(parser, ZEN_TOKEN_RIGHT_PARENTHESIS);

                break;
            }

            case ZEN_TOKEN_LEFT_BRACE: {
                k_ASTNode_t* mapExpression = k_ASTNode_new(node);
                context->m_expression = mapExpression;
                k_Parser_mapExpression(parser, mapExpression);

                break;
            }

            case ZEN_TOKEN_LEFT_SQUARE_BRACKET: {
                k_ASTNode_t* listExpression = k_ASTNode_new(node);
                context->m_expression = listExpression;
                k_Parser_listExpression(parser, listExpression);

                break;
            }

            case ZEN_TOKEN_KEYWORD_NEW: {
                k_ASTNode_t* newExpression = k_ASTNode_new(node);
                context->m_expression = newExpression;
                k_Parser_newExpression(parser, newExpression);

                break;
            }

            case ZEN_TOKEN_KEYWORD_THIS: {
                k_Token_t* keyword = k_TokenStream_lt(parser->m_tokens, 1);
                context->m_expression = k_Parser_newTerminalNode(node, keyword);
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

bool k_Parser_isPrimaryExpressionFollow(k_TokenType_t type) {
    bool result = false;
    if (k_Parser_isLiteralFollow(type)) {
        result = true;
    }
    else {
        switch (type) {
            case ZEN_TOKEN_KEYWORD_THIS:
            case ZEN_TOKEN_IDENTIFIER:
            case ZEN_TOKEN_LEFT_PARENTHESIS:
            case ZEN_TOKEN_LEFT_BRACE:
            case ZEN_TOKEN_LEFT_SQUARE_BRACKET:
            case ZEN_TOKEN_LEFT_ANGLE_BRACKET:
            case ZEN_TOKEN_KEYWORD_NEW: {
                result = true;
                break;
            }
        }
    }
    return result;
}

bool k_Parser_isLiteral(k_TokenType_t type) {
    bool result = false;
    switch (type) {
        case ZEN_TOKEN_INTEGER_LITERAL:
        case ZEN_TOKEN_FLOATING_POINT_LITERAL:
        case ZEN_TOKEN_KEYWORD_TRUE:
        case ZEN_TOKEN_KEYWORD_FALSE:
        case ZEN_TOKEN_STRING_LITERAL:
        case ZEN_TOKEN_KEYWORD_NULL: {
            result = true;
            break;
        }
    }
    return result;
}

bool k_Parser_isLiteralFollow(k_TokenType_t type) {
    return k_Parser_isLiteral(type);
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
void k_Parser_mapExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

	k_MapExpressionContext_t* context = k_MapExpressionContext_new(node);

    /* Match and discard the '{' token. */
    k_Parser_match(parser, ZEN_TOKEN_LEFT_BRACE);

    /* If expression fails, discard tokens until the '}' token is
     * encountered.
     */
    k_Parser_pushFollowToken(parser, ZEN_TOKEN_RIGHT_BRACE);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        k_ASTNode_t* mapEntries = k_ASTNode_new(node);
        context->m_mapEntries = mapEntries;
        k_Parser_mapEntries(parser, mapEntries);
    }

    /* Pop the '}' token from the follow set. */
    k_Parser_popFollowToken(parser);

    /* Match and discard the '}' token. */
    k_Parser_match(parser, ZEN_TOKEN_RIGHT_BRACE);

    k_StackTrace_exit();
}

/*
 * mapEntries
 * :	mapEntry (',' mapEntry)*
 * ;
 */
void k_Parser_mapEntries(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MapEntriesContext_t* context = k_MapEntriesContext_new(node);

    k_ASTNode_t* mapEntry = k_ASTNode_new(node);
    jtk_ArrayList_add(context->m_mapEntries, mapEntry);
    k_Parser_mapEntry(parser, mapEntry);

    while (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_COMMA) {
        /* Consume and discard the ',' token. */
        k_TokenStream_consume(parser->m_tokens);

        mapEntry = k_ASTNode_new(node);
        jtk_ArrayList_add(context->m_mapEntries, mapEntry);
        k_Parser_mapEntry(parser, mapEntry);
    }

    k_StackTrace_exit();
}

/*
 * mapEntry
 * :	expression ':' expression
 * ;
 */
void k_Parser_mapEntry(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_MapEntryContext_t* context = k_MapEntryContext_new(node);

    k_ASTNode_t* keyExpression = k_ASTNode_new(node);
    context->m_keyExpression = keyExpression;
    k_Parser_expression(parser, keyExpression);

    /* Match and discard the ':' token. */
    k_Parser_match(parser, ZEN_TOKEN_COLON);

    k_ASTNode_t* valueExpression = k_ASTNode_new(node);
    context->m_valueExpression = valueExpression;
    k_Parser_expression(parser, valueExpression);

    k_StackTrace_exit();
}

/*
 * listExpression
 * :    '[' expressions ']'
 * ;
 */
void k_Parser_listExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_ListExpressionContext_t* context = k_ListExpressionContext_new(node);

    /* Match and discard the '[' token. */
    k_Parser_match(parser, ZEN_TOKEN_LEFT_SQUARE_BRACKET);

    if (k_Parser_isExpressionFollow(k_TokenStream_la(parser->m_tokens, 1))) {
        /* If expression fails, discard tokens until the ')' token is
        * encountered.
        */
        k_Parser_pushFollowToken(parser, ZEN_TOKEN_RIGHT_SQUARE_BRACKET);

        k_ASTNode_t* expressions = k_ASTNode_new(node);
        context->m_expressions = expressions;
        k_Parser_expressions(parser, expressions);

        /* Pop the ']' token from the follow set. */
        k_Parser_popFollowToken(parser);
    }

    /* Match and discard the ']' token. */
    k_Parser_match(parser, ZEN_TOKEN_RIGHT_SQUARE_BRACKET);

    k_StackTrace_exit();
}

/*
 * newExpression
 * :   'new' typeName functionArguments?
 * ;
 */
void k_Parser_newExpression(k_Parser_t* parser, k_ASTNode_t* node) {
    k_StackTrace_enter();

    k_NewExpressionContext_t* context = k_NewExpressionContext_new(node);

    /* Match and discard the 'new' token. */
    k_Parser_match(parser, ZEN_TOKEN_KEYWORD_NEW);

    k_ASTNode_t* typeName = k_ASTNode_new(node);
    context->m_typeName = typeName;
    k_Parser_typeName(parser, typeName);

    if (k_TokenStream_la(parser->m_tokens, 1) == ZEN_TOKEN_LEFT_PARENTHESIS) {
        k_ASTNode_t* functionArguments = k_ASTNode_new(node);
        context->m_functionArguments = functionArguments;
        k_Parser_functionArguments(parser, functionArguments);
    }

    k_StackTrace_exit();
}

// TODO: recover(IMPORT_DECLARATION_RECOVERY_SET);
