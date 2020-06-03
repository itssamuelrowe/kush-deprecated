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

#include <kush/symbol-table/Analyzer.h>

/*
 * The following text describes a naive algorithm that I developed to analyze
 * if the left-value evaluates to a placeholder. I am not sure if the algorithm
 * described here already exists. If it does not, I would like to call it
 * the "Placeholder-Value AST Annotation Method". Further, please let me know
 * if there is a better way to do this.
 *
 * A placeholder is a location where a reference can be stored, it may be a local
 * variable or a local class member, or an object's attribute. A placeholder
 * is commonly referred to as lvalue, a value that can be the target of an
 * assignment. The "l" stands for "left", as in the left hand side of the
 * assignment operator.
 *
 * A consequent is a consequence of any expression, such as, function invocation,
 * variable/constant reference, addition, subtraction, multiplication, and so on.
 * It may produce either a placeholder or a value. A consequent is generally
 * known as rvalue, a result that is produced by an expression that appears on
 * the right hand side of the assignment operator. The "r" stands for "right",
 * as in the right hand side of the assignment operator. Henceforth in this text,
 * I use consequent and rvalue interchangeably, unless specified
 * otherwise.
 *
 * The expression `x = (y = 5)` can be represented with the following abstract
 * syntax tree (AST).
 *
 *   =V
 *  / \
 * xP  =V
 *    / \
 *   yP  5V
 *
 * Here, V and P are annotations that imply consequent and placeholder, respectively.
 *
 * 1. Consider a variable `label`, which is initialized to 'unknown'.
 * 2. Walk down the abstract syntax tree with an inorder depth first
 *    fashion.
 * 3. At any given node, if the term produces a consequent, the label is marked
 *    as 'consequent'.
 * 4. At any given node, if the term produces a placholder, the label is marked
 *    as 'placholder'. Since the definition of consequent cleary indicates that
 *    a consequent is a superset of the placeholder, all the terms can be
 *    labelled as 'consequent'. However, this behavior is not desirable. Therefore,
 *    the 'placholder' label has more priority. For example, `x`, a variable
 *    reference is always labelled as placeholder, even if it appears on the
 *    right hand side of an equation.
 * 5. After every node in the expression's AST is visited, the label contained
 *    by the `label` variable determines the nature of the expression.
 *
 * It should be noted that, certain a priori information is needed to handle
 * special situations. For example, the algorithm cannot differentiate between
 * a member attribute access expression and a member function invocation, if the
 * design of the AST is inherently unfavorable to the algorithm. In such cases,
 * special supervision is required.
 *
 * The efficiency of the algorithm can be drastically improved without visiting
 * every node in the AST, provided that the annotation is required only to
 * determine if an expression evaluates to placeholder or a consequent.
 * Only the root nodes of the expressions need to be inspected!
 *
 * Consider the following AST, with all its nodes annotated.
 *   =V
 *  / \
 * xP  =V
 *    / \
 *   yP  5V
 *
 * To determine if the left hand expression is a placeholder and the right hand
 * side expression is a consequent, the following annotation is satisfactory.
 *   =
 *  / \
 * xP  =V
 *    / \
 *   y   5
 */

k_Primitives_t primitives = {
    .i8 = {
        .tag = K_TYPE_INTEGER,
        .integer = {
            .size = 1
        }
    },

    .i16 = {
        .tag = K_TYPE_INTEGER,
        .integer = {
            .size = 2
        }
    },

    .i32 = {
        .tag = K_TYPE_INTEGER,
        .integer = {
            .size = 4
        }
    },

    .i64 = {
        .tag = K_TYPE_INTEGER,
        .integer = {
            .size = 8
        }
    },

    .f32 = {
        .tag = K_TYPE_DECIMAL,
        .decimal = {
            .size = 4
        }
    },

    .f64 = {
        .tag = K_TYPE_DECIMAL,
        .decimal = {
            .size = 8
        }
    },

    .void_ = {
        .tag = K_TYPE_VOID
    }
};

// Import

bool import(k_Analyzer_t* analyzer, const char* name, int32_t size,
    bool wildcard) {
    k_Module_t* module = NULL; // TODO

    return true;
}

void importDefaults(k_Analyzer_t* analyzer) {
    import(analyzer, "kush.core", 9, true);
}

// Define

void defineSymbols(k_Analyzer_t* analyzer, k_CompilationUnit_t* unit) {
    unit->scope = k_Scope_forCompilationUnit();
    analyzer->scope = unit->scope;

    int32_t structureCount = jtk_ArrayList_getSize(unit->structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        k_Structure_t* structure = (k_Structure_t*)jtk_ArrayList_getValue(
            unit->structures, j);
        defineStructure(analyzer, structure);
    }

    int32_t functionCount = jtk_ArrayList_getSize(unit->functions);
    int32_t i;
    for (i = 0; i < functionCount; i++) {
        k_Function_t* function = (k_Function_t*)jtk_ArrayList_getValue(
            unit->functions, i);
        defineFunction(analyzer, function);
    }
}

void defineStructure(k_Analyzer_t* analyzer, k_Structure_t* structure) {
    structure->parent = analyzer->scope;
    structure->scope = k_Scope_forStructure(analyzer->scope);
    k_Scope_addStructure(analyzer->scope, structure);

    int32_t limit = jtk_ArrayList_getSize(declaration->variables);
    int32_t i;
    for (i = 0; i < limit; i++) {
        k_Storage_t* storage =
            (k_Storage_t*)jtk_ArrayList_getValue(declaration->variables, i);
        declarator->parent = structure->scope;
        k_Scope_addStorage(structure->scope, storage);
    }
}

void defineFunction(k_Analyzer_t* analyzer, k_Function_t* function) {
    function->parent = analyzer->scope;
    function->scope = analyzer->scope =
        k_Scope_forFunction(analyzer->scope);
    k_Scope_addFunction(analyzer->scope, function);

    k_Scope_t* scope = defineLocals(analyzer, function->body);
    int32_t parameterCount = jtk_ArrayList_getSize(function->parameters);
    int32_t i;
    for (i = 0; i < parameterCount; i++) {
        k_Variable_t* parameter = (k_Storage_t*)jtk_ArrayList_getValue(function->parameters, i);
        k_Scope_addVariable(scope, parameter);
    }
}

k_Scope_t* defineLocals(k_Analyzer_t* analyzer, k_BlockStatement_t* block) {
    block->scope = analyzer->scope = k_Scope_forLocal(analyzer->scope);

    int32_t limit = jtk_ArrayList_getSize(block->statements);
    int32_t i;
    for (i = 0; i < limit; i++) {
        k_Context_t* context = (k_Context_t*)jtk_ArrayList_getValue(
            block->statements, i);
        switch (context->type) {
            case K_CONTEXT_ITERATIVE_STATEMENT: {
                k_IterativeStatement_t* statement = (k_IterativeStatement_t*)context;
                if (statement->label != NULL) {
                    jtk_Scope_addLabel(analyzer->scope, &statement->label);
                }
                if (statement->parameter != NULL) {
                    jtk_Scope_addVariable(scope, statement->parameter);
                }
                defineLocals(analyzer, statement->body);
                break;
            }

            case K_CONTEXT_IF_STATEMENT: {
                k_IfStatement_t* statement = (k_IfStatement_t*)context;
                defineLocals(analyzer, statement->ifClause->body);
                int32_t count = jtk_ArrayList_getSize(statement->elseIfClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_IfClause_t* clause = (k_IfClause_t*)jtk_ArrayList_getValue(
                        statement->elseIfClauses, j);
                    defineLocals(analyzer, clause->body);
                }
                if (statement->elseClause != NULL) {
                    defineLocals(analyzer, statement->elseClause);
                }
                break;
            }

            case K_CONTEXT_TRY_STATEMENT: {
                k_TryStatement_t* statement = (k_TryStatement_t*)context;
                defineLocals(analyzer, statement->tryClause);

                int32_t count = jtk_ArrayList_getSize(statement->catchClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_CatchClause_t* clause = (k_CatchClause_t*)jtk_ArrayList_getValue(
                        statement->catchClauses, j);
                    k_Scope_t* scope = defineLocals(clause->body);
                    defineVariable(analyzer, scope, clause->parameter);
                }

                if (statement->finallyClause != NULL) {
                    defineLocals(analyzer, statement->finallyClause);
                }

                break;
            }

            case K_CONTEXT_VARIABLE_DECLARATION: {
                k_VariableDeclaration_t* statement = (k_VariableDeclaration_t*)context;
                int32_t count = jtk_ArrayList_getSize(statement->variables);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_Variable_t* variable = (k_Variable_t*)jtk_ArrayList_getValue(
                        statement->variables, j);
                    defineVariable(analyzer->scope, variable);
                }
                break;
            }
        }
    }

    analyzer->scope = analyzer->scope->parent;
}

// Resovle



// Constructor

k_Analyzer_t* k_Analyzer_new(k_Compiler_t* compiler) {
    k_Analyzer_t* analyzer = k_Memory_allocate(k_Analyzer_t, 1);
    analyzer->compiler = compiler;
    analyzer->symbolTable = NULL;
    analyzer->package = NULL;
    analyzer->packageSize = -1;

    return analyzer;
}

// Destructor

void k_Analyzer_delete(k_Analyzer_t* analyzer) {
    jtk_Assert_assertObject(analyzer, "The specified analyzer is null.");

    jtk_Memory_deallocate(analyzer);
}

// Analyze

void k_Analyzer_analyze(k_Analyzer_t* analyzer, k_Module_t* module) {
    defineSymbols(analyzer, module);
    resolveSymbols(analyzer, module);
}

// Reset

void k_Analyzer_reset(k_Analyzer_t* analyzer,
    k_SymbolTable_t* symbolTable, const uint8_t* package, int32_t packageSize) {
    analyzer->symbolTable = symbolTable;
    analyzer->package = package;
    analyzer->packageSize = packageSize;
}


uint8_t* getModuleName(jtk_ArrayList_t* identifiers, int32_t* size) {
    int32_t identifierCount = jtk_ArrayList_getSize(identifiers);
    jtk_StringBuilder_t* builder = jtk_StringBuilder_new();
    int32_t i;
    for (i = 0; i < identifierCount; i++) {
        k_Token_t* identifier = (k_Token_t*)jtk_ArrayList_getValue(identifiers, i);
        jtk_StringBuilder_appendEx_z(builder, identifier->text, identifier->length);
        if (i + 1 < identifierCount) {
            jtk_StringBuilder_append_c(builder, '.');
        }
    }

    uint8_t* result = jtk_StringBuilder_toCString(builder, size);
    jtk_StringBuilder_delete(builder);

    return result;
}

void resolve(k_Analyzer_t* analyzer, k_Module_t* module) {
    k_Compiler_t* compiler = analyzer->compiler;
    k_ErrorHandler_t* handler = compiler->errorHandler;

    analyzer->scope = module->scope;
    if (!analyzer->compiler->coreApi) {
        importDefaults(analyzer);
    }

    int32_t importCount = jtk_ArrayList_getSize(module->imports);
    int32_t i;
    for (i = 0; i < importCount; i++) {
        k_ImportDeclaration_t* declaration = (k_ImportDeclaration_t*)jtk_ArrayList_getValue(
            module->imports, i);
        int32_t size;
        uint8_t* name = getModuleName(declaration->identifiers, &size);

        // If a module was previously imported, we should'nt complain.
        k_Module_t* newModule = import(analyzer, name, size, declaration->wildcard);
        if (newModule == NULL) {
            k_Token_t* lastToken = (k_Token_t*)jtk_ArrayList_getValue(declaration->identifiers, -1);
            reportError(analyzer, KUSH_ERROR_UNKNOWN_MODULE, lastToken);
        }
        else {
            // TODO
        }

        jtk_CString_delete(name);
    }

    int32_t functionCount = jtk_ArrayList_getSize(unit->functions);
    int32_t i;
    for (i = 0; i < functionCount; i++) {
        k_Function_t* function = (k_Function_t*)jtk_ArrayList_getValue(
            unit->functions, i);
        resolveLocals(analyzer, function->body);
    }
}

void resolveLocals(k_Analyzer_t* analyzer, k_BlockStatement_t* block) {
    analyzer->scope = block->scope;

    int32_t limit = jtk_ArrayList_getSize(block->statements);
    int32_t i;
    for (i = 0; i < limit; i++) {
        k_Context_t* context = (k_Context_t*)jtk_ArrayList_getValue(
            block->statements, i);
        switch (context->type) {
            case K_CONTEXT_ITERATIVE_STATEMENT: {
                k_IterativeStatement_t* statement = (k_IterativeStatement_t*)context;
                if (statement->label != NULL) {
                    jtk_Scope_addLabel(analyzer->scope, &statement->label);
                }
                if (statement->parameter != NULL) {
                    jtk_Scope_addVariable(scope, statement->parameter);
                }
                resolveLocals(analyzer, statement->body);
                break;
            }

            case K_CONTEXT_IF_STATEMENT: {
                k_IfStatement_t* statement = (k_IfStatement_t*)context;
                resolveLocals(analyzer, statement->ifClause->body);
                int32_t count = jtk_ArrayList_getSize(statement->elseIfClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_IfClause_t* clause = (k_IfClause_t*)jtk_ArrayList_getValue(
                        statement->elseIfClauses, j);
                    resolveLocals(analyzer, clause->body);
                }
                if (statement->elseClause != NULL) {
                    resolveLocals(analyzer, statement->elseClause);
                }
                break;
            }

            case K_CONTEXT_TRY_STATEMENT: {
                k_TryStatement_t* statement = (k_TryStatement_t*)context;
                resolveLocals(analyzer, statement->tryClause);

                int32_t count = jtk_ArrayList_getSize(statement->catchClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_CatchClause_t* clause = (k_CatchClause_t*)jtk_ArrayList_getValue(
                        statement->catchClauses, j);
                    k_Scope_t* scope = resolveLocals(clause->body);
                    defineVariable(analyzer, scope, clause->parameter);
                }

                if (statement->finallyClause != NULL) {
                    resolveLocals(analyzer, statement->finallyClause);
                }

                break;
            }

            case K_CONTEXT_VARIABLE_DECLARATION: {
                k_VariableDeclaration_t* statement = (k_VariableDeclaration_t*)context;
                int32_t count = jtk_ArrayList_getSize(statement->variables);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_Variable_t* variable = (k_Variable_t*)jtk_ArrayList_getValue(
                        statement->variables, j);
                    defineVariable(analyzer->scope, variable);
                }
                break;
            }
        }
    }

    analyzer->scope = analyzer->scope->parent;
}

k_Type_t* resolveExpression(k_Analyzer_t* analyzer, k_Context_t* context) {
    switch (context->type) {
        case K_CONTEXT_RELATIONAL_EXPRESSION:
        case K_CONTEXT_EQUALITY_EXPRESSION: {
            k_BinaryExpression_t* expression = (k_BinaryExpression_t*)context;
            k_Type_t* leftType = resolveExpression(analyzer, expression->left);

            int32_t count = jtk_ArrayList_getSize(expression->others);
            if (count == 1) {
                jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
                    expression->others, 0);
                k_Type_t* rightType = resolveExpression(analyzer, pair->right);

                if (leftType != rightType) {
                    reportError(analyzer, K_ERROR_INCOMPATIBLE_TYPES, pair->left);
                }
            }
            else if (count > 1) {
                reportError(analyzer, K_ERROR_COMBINING_EQUALITY_OPERATORS);
            }

            break;
        }

        case K_CONTEXT_ASSIGNMENT_EXPRESSION:
        case K_CONTEXT_LOGICAL_OR_EXPRESSION:
        case K_CONTEXT_LOGICAL_AND_EXPRESSION:
        case K_CONTEXT_INCLUSIVE_OR_EXPRESSION:
        case K_CONTEXT_EXCLUSIVE_OR_EXPRESSION:
        case K_CONTEXT_AND_EXPRESSION:
        case K_CONTEXT_SHIFT_EXPRESSION:
        case K_CONTEXT_ADDITIVE_EXPRESSION:
        case K_CONTEXT_MULTIPLICATIVE_EXPRESSION: {
            k_BinaryExpression_t* expression = (k_BinaryExpression_t*)context;
            k_Type_t* leftType = resolveExpression(analyzer, expression->left);

            int32_t count = jtk_ArrayList_getSize(expression->others);
            int32_t i;
            for (i = 0; i < count; i++) {
                jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
                    expression->others, i);
                k_Type_t* rightType = resolveExpression(analyzer, pair->right);

                if (leftType != rightType) {
                    reportError(analyzer, K_ERROR_INCOMPATIBLE_TYPES, pair->left);
                }
            }

            break;
        }

        case K_CONTEXT_UNARY_EXPRESSION: {
            k_UnaryExpression_t* expression = (k_UnaryExpression_t*)context;
            k_Type_t* type = resolveExpression(expression->m_expression);
            k_Token_t* operator = expression->operator;
            if (operator != NULL) {
                k_TokenType_t token = operator->type;
                if ((token == K_TOKEN_PLUS) || (token == K_TOKEN_MINUS)) {
                    if ((type.tag != K_TYPE_INTEGER) && (type.tag != K_TYPE_DECIMAL)) {
                        reportError(analyzer, K_ERROR_INCOMPATIBLE_OPERAND, operator);
                    }
                }
                else if (token == K_TOKEN_TILDE) {
                    if (type.tag != K_TYPE_INTEGER) {
                        reportError(analyzer, K_ERROR_INCOMPATIBLE_OPERAND, operator);
                    }
                }
                else if (token == KUSH_TOKEN_EXCLAMATION_MARK) {
                    if (type.tag != K_TYPE_BOOLEAN) {
                        reportError(analyzer, K_ERROR_INCOMPATIBLE_OPERAND, operator);
                    }
                }
                else {
                    printf("[internal error] Control should not reach here.\n");
                }
            }

            break;
        }
    }
}

void k_Analyzer_onEnterAssignmentExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_AssignmentExpressionContext_t* context = (k_AssignmentExpressionContext_t*)node->context;
    k_Compiler_t* compiler = analyzer->compiler;
    k_ErrorHandler_t* errorHandler = compiler->errorHandler;

    k_ASTNode_t* assignmentOperator = context->assignmentOperator;
    if (assignmentOperator != NULL) {
        k_ASTWalker_walk(astListener, context->conditionalExpression);
        if (analyzer->label == KUSH_EXPRESSION_ANNOTATION_VALUE) {
            k_Token_t* assignmentOperatorToken = (k_Token_t*)assignmentOperator->context;
            k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
            KUSH_ERROR_CODE_INVALID_LVALUE, assignmentOperatorToken);
        }
        else {
            /* Do not walk through the assignment expression when the left value
             * is invalid.
             */
            k_ASTWalker_walk(astListener, context->assignmentExpression);
        }
        k_ASTListener_skipChildren(astListener);
    }
}

// conditionalExpression

void k_Analyzer_onExitConditionalExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_ConditionalExpressionContext_t* context = (k_ConditionalExpressionContext_t*)node->context;

    if (context->thenExpression != NULL) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

// logicalOrExpression

void k_Analyzer_onExitLogicalOrExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_LogicalOrExpressionContext_t* context = (k_LogicalOrExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->logicalAndExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

// logicalAndExpression

void k_Analyzer_onExitLogicalAndExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_LogicalAndExpressionContext_t* context = (k_LogicalAndExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->inclusiveOrExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterLogicalAndExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// inclusiveOrExpression

void k_Analyzer_onExitInclusiveOrExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_InclusiveOrExpressionContext_t* context = (k_InclusiveOrExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->exclusiveOrExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterInclusiveOrExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// exclusiveOrExpression

void k_Analyzer_onExitExclusiveOrExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_ExclusiveOrExpressionContext_t* context = (k_ExclusiveOrExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->andExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterExclusiveOrExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// andExpression

void k_Analyzer_onExitAndExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_AndExpressionContext_t* context = (k_AndExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->equalityExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterAndExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// equalityExpression

void k_Analyzer_onExitEqualityExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_EqualityExpressionContext_t* context = (k_EqualityExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->relationalExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterEqualityExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// relationalExpression

void k_Analyzer_onExitRelationalExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_RelationalExpressionContext_t* context = (k_RelationalExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->shiftExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterRelationalExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// shiftExpression

void k_Analyzer_onExitShiftExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_ShiftExpressionContext_t* context = (k_ShiftExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->additiveExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterShiftExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// additiveExpression

void k_Analyzer_onExitAdditiveExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_AdditiveExpressionContext_t* context = (k_AdditiveExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->multiplicativeExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterAdditiveExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
}

// multiplicativeExpression

void k_Analyzer_onExitMultiplicativeExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_MultiplicativeExpressionContext_t* context = (k_MultiplicativeExpressionContext_t*)node->context;

    if (!jtk_ArrayList_isEmpty(context->unaryExpressions)) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

// unaryExpression

void k_Analyzer_onExitUnaryExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_UnaryExpressionContext_t* context = (k_UnaryExpressionContext_t*)node->context;

    if (context->unaryOperator != NULL) {
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

// postfixExpression

void k_Analyzer_onExitPostfixExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {

    k_PostfixExpressionContext_t* context = (k_PostfixExpressionContext_t*)node->context;
    k_Compiler_t* compiler = analyzer->compiler;
    k_ErrorHandler_t* errorHandler = compiler->errorHandler;
    k_ASTNode_t* primaryExpression = context->primaryExpression;
    k_PrimaryExpressionContext_t* primaryExpressionContext = (k_PrimaryExpressionContext_t*)primaryExpression->context;

    k_Symbol_t* primarySymbol = NULL;
    int32_t postfixPartCount = jtk_ArrayList_getSize(context->postfixParts);

    k_ASTNode_t* expression = primaryExpressionContext->expression;
    k_Token_t* token = NULL;
    /* Check if the primary expression is a literal or an identifier. */
    if (k_ASTNode_isTerminal(expression)) {
        /* Retrieve the token that the primary expression represents. */
        token = (k_Token_t*)expression->context;

        switch (k_Token_getType(token)) {
            case KUSH_TOKEN_IDENTIFIER: {
                /* Retrieve the string equivalent to the identifier node. */
                const uint8_t* identifierText = k_Token_getText(token);
                /* Resolve the symbol in the symbol table. */
                k_Symbol_t* symbol = k_SymbolTable_resolve(analyzer->symbolTable, identifierText);

                if (symbol != NULL) {
                    k_Scope_t* enclosingScope = k_Symbol_getEnclosingScope(symbol);
                    if (k_Symbol_isVariable(symbol) || k_Symbol_isConstant(symbol)) {
                        /* Annotate the AST node as placeholder. */
                        analyzer->label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;
                    }
                    else {
                        /* Pass the reference to the symbol to the next phase. */
                        primarySymbol = symbol;
                    }

                    if ((enclosingScope != NULL) && k_Scope_isLocalScope(enclosingScope)) {
                        k_Token_t* symbolToken = (k_Token_t*)symbol->identifier->context;
                        if (token->startIndex <= symbolToken->startIndex) {
                            k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
                                    KUSH_ERROR_CODE_UNDECLARED_IDENTIFIER, token);
                        }
                    }
                }
                else {
                    k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
                            KUSH_ERROR_CODE_UNDECLARED_IDENTIFIER, token);
                }

                break;
            }

            case KUSH_TOKEN_INTEGER_LITERAL:
            case KUSH_TOKEN_STRING_LITERAL:
            case KUSH_TOKEN_KEYWORD_NULL:
            case KUSH_TOKEN_KEYWORD_FALSE:
            case KUSH_TOKEN_KEYWORD_THIS:
            case KUSH_TOKEN_KEYWORD_TRUE: {
                /* Annotate the AST node as value. */
                analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;

                break;
            }
        }
    }
    else if ((expression->type == KUSH_AST_NODE_TYPE_MAP_EXPRESSION) ||
        (expression->type == KUSH_AST_NODE_TYPE_LIST_EXPRESSION) ||
        (expression->type == KUSH_AST_NODE_TYPE_EXPRESSION) ||
        (expression->type == KUSH_AST_NODE_TYPE_NEW_EXPRESSION)) {
        k_ASTWalker_walk(astListener, expression);

        /* Annotate the AST node as value. */
        analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }

    int32_t i;
    for (i = 0; i < postfixPartCount; i++) {
        k_ASTNode_t* postfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
            context->postfixParts, i);
        k_ASTNodeType_t type = k_ASTNode_getType(postfixPart);

        switch (type) {
            case KUSH_AST_NODE_TYPE_SUBSCRIPT: {
                /* Annotate the AST node as placeholder. */
                analyzer->label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;

                k_SubscriptContext_t* subscriptContext = (k_SubscriptContext_t*)postfixPart->context;

                /* Visit the index expression node and analyze. */
                k_ASTWalker_walk(astListener, subscriptContext->expression);

                /* The normal behaviour of the AST walker causes the analyzer to
                 * visit the index in an undesirable fashion. Therefore, we partially
                 * switch from the analyzer to visitor design pattern. The AST walker
                 * can be guided to switch to this mode via k_ASTListener_skipChildren()
                 * function which causes the AST walker to skip iterating over the children
                 * nodes.
                 */
                k_ASTListener_skipChildren(astListener);

                break;
            }

            case KUSH_AST_NODE_TYPE_FUNCTION_ARGUMENTS: {
                /* Annotate the AST node as value. */
                analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;

                k_FunctionArgumentsContext_t* functionArgumentsContext =
                    (k_FunctionArgumentsContext_t*)postfixPart->context;

                if (i == 0) {
                    if (primarySymbol == NULL) {
                        k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
                            KUSH_ERROR_CODE_VARIABLE_TREATED_AS_FUNCTION, token);
                    }
                    else if (k_Symbol_isFunction(primarySymbol)) {
                        k_FunctionSymbol_t* functionSymbol = &primarySymbol->context.asFunction;
                        k_ASTNode_t* expressions = functionArgumentsContext->expressions;
                        if (expressions != NULL) {
                            k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->context;
                            int32_t argumentCount = jtk_ArrayList_getSize(expressionsContext->expressions);
                            if (argumentCount > 0) {
                                int32_t argumentIndex;
                                for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                                    /* Retrieve the expression for the current argument. */
                                    k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                                        expressionsContext->expressions, argumentIndex);

                                    /* Visit the expression node and analyze. */
                                    k_ASTWalker_walk(astListener, argument);
                                }
                            }
                        }
                    }
                    else {
                        printf("[error] Trying to invoke a non-function entity.\n");
                    }
                }
                else {
                    printf("[internal error] Control should not reach here.\n");
                }

                break;
            }

            case KUSH_AST_NODE_TYPE_MEMBER_ACCESS: {
                k_MemberAccessContext_t* memberAccessContext =
                    (k_MemberAccessContext_t*)postfixPart->context;
                k_ASTNode_t* identifier = memberAccessContext->identifier;
                k_Token_t* identifierToken = (k_Token_t*)identifier->context;

                k_ASTNode_t* functionArguments = NULL;

                /* Annotate the AST node as placeholder. */
                analyzer->label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;

                if ((i + 1) < postfixPartCount) {
                    k_ASTNode_t* nextPostfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
                        context->postfixParts, i + 1);
                    k_ASTNodeType_t nextPostfixPartType = k_ASTNode_getType(nextPostfixPart);
                    if (nextPostfixPartType == KUSH_AST_NODE_TYPE_FUNCTION_ARGUMENTS) {
                        functionArguments = nextPostfixPart;
                        i++;
                    }
                }

                if (functionArguments != NULL) {
                    /* Annotate the AST node as placeholder. */
                    analyzer->label = KUSH_EXPRESSION_ANNOTATION_VALUE;

                    k_FunctionArgumentsContext_t* functionArgumentsContext =
                        (k_FunctionArgumentsContext_t*)functionArguments->context;

                    k_ASTNode_t* expressions = functionArgumentsContext->expressions;
                    if (expressions != NULL) {
                        k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->context;
                        int32_t argumentCount = jtk_ArrayList_getSize(expressionsContext->expressions);
                        if (argumentCount > 0) {
                            int32_t argumentIndex;
                            for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                                /* Retrieve the expression for the current argument. */
                                k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                                    expressionsContext->expressions, argumentIndex);

                                /* Visit the expression node and generate the relevant instructions. */
                                k_ASTWalker_walk(astListener, argument);
                            }
                        }
                    }
                }
                break;
            }

            /*
            case KUSH_AST_NODE_TYPE_POSTFIX_OPERATOR: {

                break;
            }
            */

            default: {
                printf("[error] Invalid AST node type %d encountered.\n", type);
            }
        }
    }
}

// New Expression

void k_Analyzer_onEnterNewExpression(k_Analyzer_t* analyzer,
    k_ASTNode_t* node) {
    // TODO: Debug this function when superclasses are implemented!


    k_Compiler_t* compiler = analyzer->compiler;
    k_ErrorHandler_t* errorHandler = compiler->errorHandler;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = compiler->logger;

    k_NewExpressionContext_t* context = (k_NewExpressionContext_t*)node->context;

    /* Retrieve the scope within which the new expression appears. */
    // k_Scope_t* scope = k_SymbolTable_getCurrentScope(generator->symbolTable);

    /* Retrieve the string equivalent of the type name node. */
    int32_t typeNameSize;
    uint8_t* typeNameText = k_ASTNode_toCString(context->typeName, &typeNameSize);
    k_TypeNameContext_t* typeName = (k_TypeNameContext_t*)context->typeName->context;
    int32_t identifierCount = jtk_ArrayList_getSize(typeName->identifiers);
    k_ASTNode_t* lastIdentifier = jtk_ArrayList_getValue(typeName->identifiers, identifierCount - 1);
    k_Token_t* lastIdentifierToken = (k_Token_t*)lastIdentifier->context;

    /* Resolve the class symbol for the type name. */
    k_Symbol_t* symbol = k_SymbolTable_resolve(analyzer->symbolTable, typeNameText);

    if (symbol == NULL) {
        k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
            KUSH_ERROR_CODE_UNDECLARED_CLASS, lastIdentifierToken);
    }
    else {
        if (k_Symbol_isExternal(symbol)) {
            symbol = symbol->context.asExternal;
        }

        if (k_Symbol_isClass(symbol)) {
            k_ClassSymbol_t* classSymbol = &symbol->context.asClass;
            /* Retrieve the scope corresponding to the class symbol. */
            k_Scope_t* scope = classSymbol->classScope;
            if (k_Scope_isClassScope(scope)) {
                /* Retrieve the constructor declared in this class. */
                k_Symbol_t* constructorSymbol = k_Scope_resolve(scope, "new");

                if ((constructorSymbol == NULL) ||
                    (k_Symbol_getEnclosingScope(constructorSymbol) != scope)) {
                    k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
                        KUSH_ERROR_CODE_NO_SUITABLE_CONSTRUCTOR, lastIdentifierToken);
                }
                else {
                    if (!k_Symbol_isFunction(constructorSymbol)) {
                        printf("[internal error] 'new' declared as non-constructor symbol in class %s.\n", typeNameText);
                        printf("[warning] Looks like the syntactical phase failed.\n");
                    }
                }
            }
            else {
                printf("[internal error] %s is a non-class scope\n", typeNameText);
                printf("[warning] Looks like the syntactical phase failed.\n");
            }
        }
        else {
            k_ErrorHandler_handleSemanticalError(errorHandler, analyzer,
                KUSH_ERROR_CODE_INSTANTIATION_OF_NON_CLASS_SYMBOL, lastIdentifierToken);
        }
    }

    /* Delete the type name text. The typeNameText is used to generate the constructor
     * entry in the constant pool. Therefore, we delay its destruction.
     */
    jtk_CString_delete(typeNameText);
}