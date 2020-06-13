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
#include <jtk/core/StringBuilder.h>

#include <kush/analyzer.h>
#include <kush/error-handler.h>

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

static Context* find(Scope* scope, const uint8_t* identifier, int32_t size);
static bool import(Analyzer* analyzer, const char* name, int32_t size,
    bool wildcard);
static void importDefaults(Analyzer* analyzer);
static void defineStructure(Analyzer* analyzer, Structure* structure);
static void defineFunction(Analyzer* analyzer, Function* function);
static Scope* defineLocals(Analyzer* analyzer, Block* block);
static uint8_t* getModuleName(jtk_ArrayList_t* identifiers, int32_t* size);
static void resolve(Analyzer* analyzer, Module* module);
static Scope* resolveLocals(Analyzer* analyzer, Block* block);
static Type* resolveExpression(Analyzer* analyzer, Context* context);
static Type* resolveToken(Analyzer* analyzer, Token* token);

// String:equals(x, y)
// x.equals(y)
// bool k_String_equals(k_Runtime_t* runtime, k_String_t* self, k_String_t* other) {
//     k_assertObject(self);
// }

Context* find(Scope* scope, const uint8_t* identifier, int32_t size) {
    Context* symbol = NULL;
    while (scope != NULL) {
        symbol = NULL; // scopeResolve(scope, identifier, size);
        if (symbol != NULL) {
            break;
        }
        else {
            scope = scope->parent;
        }
    }

    return symbol;
}

// Import

bool import(Analyzer* analyzer, const char* name, int32_t size,
    bool wildcard) {
    Module* module = NULL; // TODO

    return true;
}

void importDefaults(Analyzer* analyzer) {
    import(analyzer, "kush.core", 9, true);
}

// Define

void defineSymbols(Analyzer* analyzer, Module* module) {
    module->scope = scopeForModule();
    analyzer->scope = module->scope;

    int32_t structureCount = jtk_ArrayList_getSize(module->structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        Structure* structure = (Structure*)jtk_ArrayList_getValue(
            module->structures, j);
        defineStructure(analyzer, structure);
    }

    int32_t functionCount = jtk_ArrayList_getSize(module->functions);
    int32_t i;
    for (i = 0; i < functionCount; i++) {
        Function* function = (Function*)jtk_ArrayList_getValue(
            module->functions, i);
        defineFunction(analyzer, function);
    }
}

void defineStructure(Analyzer* analyzer, Structure* structure) {
    structure->scope = scopeForStructure(analyzer->scope);
    scopeAddStructure(analyzer->scope, structure);

    int32_t limit = jtk_ArrayList_getSize(structure->variables);
    int32_t i;
    for (i = 0; i < limit; i++) {
        Variable* variable =
            (Variable*)jtk_ArrayList_getValue(structure->variables, i);
        variable->parent = structure->scope;
        scopeAddVariable(structure->scope, variable);
    }
}

void defineFunction(Analyzer* analyzer, Function* function) {
    function->scope = analyzer->scope =
        Scope_forFunction(analyzer->scope);
    Scope_addFunction(analyzer->scope, function);

    Scope* scope = defineLocals(analyzer, function->body);
    int32_t parameterCount = jtk_ArrayList_getSize(function->parameters);
    int32_t i;
    for (i = 0; i < parameterCount; i++) {
        Variable* parameter = (Variable*)jtk_ArrayList_getValue(function->parameters, i);
        Scope_addVariable(scope, parameter);
    }
}

Scope* defineLocals(Analyzer* analyzer, Block* block) {
    block->scope = analyzer->scope = Scope_forLocal(analyzer->scope);

    int32_t limit = jtk_ArrayList_getSize(block->statements);
    int32_t i;
    for (i = 0; i < limit; i++) {
        Context* context = (Context*)jtk_ArrayList_getValue(
            block->statements, i);
        switch (context->tag) {
            case CONTEXT_ITERATIVE_STATEMENT: {
                IterativeStatement* statement = (IterativeStatement*)context;
                if (statement->label != NULL) {
                    scopeAddLabel(analyzer->scope, &statement->label);
                }
                Scope* localScope = defineLocals(analyzer, statement->body);
                if (statement->parameter != NULL) {
                    scopeAddVariable(localScope, statement->parameter);
                }

                break;
            }

            case CONTEXT_IF_STATEMENT: {
                IfStatement* statement = (IfStatement*)context;
                defineLocals(analyzer, statement->ifClause->body);
                int32_t count = jtk_ArrayList_getSize(statement->elseIfClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    IfClause* clause = (IfClause*)jtk_ArrayList_getValue(
                        statement->elseIfClauses, j);
                    defineLocals(analyzer, clause->body);
                }
                if (statement->elseClause != NULL) {
                    defineLocals(analyzer, statement->elseClause);
                }
                break;
            }

            case CONTEXT_TRY_STATEMENT: {
                TryStatement* statement = (TryStatement*)context;
                defineLocals(analyzer, statement->tryClause);

                int32_t count = jtk_ArrayList_getSize(statement->catchClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    CatchClause* clause = (CatchClause*)jtk_ArrayList_getValue(
                        statement->catchClauses, j);
                    Scope* scope = defineLocals(analyzer, clause->body);
                    defineVariable(analyzer, scope, clause->parameter);
                }

                if (statement->finallyClause != NULL) {
                    defineLocals(analyzer, statement->finallyClause);
                }

                break;
            }

            case CONTEXT_VARIABLE_DECLARATION: {
                VariableDeclaration* statement = (VariableDeclaration*)context;
                int32_t count = jtk_ArrayList_getSize(statement->variables);
                int32_t j;
                for (j = 0; j < count; j++) {
                    Variable* variable = (Variable*)jtk_ArrayList_getValue(
                        statement->variables, j);
                    defineVariable(analyzer->scope, variable);
                }
                break;
            }
        }
    }

    analyzer->scope = analyzer->scope->parent;
}

uint8_t* getModuleName(jtk_ArrayList_t* identifiers, int32_t* size) {
    int32_t identifierCount = jtk_ArrayList_getSize(identifiers);
    jtk_StringBuilder_t* builder = jtk_StringBuilder_new();
    int32_t i;
    for (i = 0; i < identifierCount; i++) {
        Token* identifier = (Token*)jtk_ArrayList_getValue(identifiers, i);
        jtk_StringBuilder_appendEx_z(builder, identifier->text, identifier->length);
        if (i + 1 < identifierCount) {
            jtk_StringBuilder_append_c(builder, '.');
        }
    }

    uint8_t* result = jtk_StringBuilderoCString(builder, size);
    jtk_StringBuilder_delete(builder);

    return result;
}

void resolve(Analyzer* analyzer, Module* module) {
    Compiler* compiler = analyzer->compiler;
    ErrorHandler* handler = compiler->errorHandler;

    analyzer->scope = module->scope;
    if (!analyzer->compiler->coreApi) {
        importDefaults(analyzer);
    }

    int32_t importCount = jtk_ArrayList_getSize(module->imports);
    int32_t i;
    for (i = 0; i < importCount; i++) {
        ImportDeclaration* declaration = (ImportDeclaration*)jtk_ArrayList_getValue(
            module->imports, i);
        int32_t size;
        uint8_t* name = getModuleName(declaration->identifiers, &size);

        // If a module was previously imported, we should'nt complain.
        Module* newModule = NULL; // import(analyzer, name, size, declaration->wildcard);
        if (newModule == NULL) {
            Token* lastToken = (Token*)jtk_ArrayList_getValue(declaration->identifiers, -1);
            reportError(analyzer, ERROR_UNKNOWN_MODULE, lastToken);
        }
        else {
            // TODO
        }

        jtk_CString_delete(name);
    }

    int32_t functionCount = jtk_ArrayList_getSize(module->functions);
    for (i = 0; i < functionCount; i++) {
        Function* function = (Function*)jtk_ArrayList_getValue(
            module->functions, i);
        resolveLocals(analyzer, function->body);
    }
}

Scope* resolveLocals(Analyzer* analyzer, Block* block) {
    analyzer->scope = block->scope;

    int32_t limit = jtk_ArrayList_getSize(block->statements);
    int32_t i;
    for (i = 0; i < limit; i++) {
        Context* context = (Context*)jtk_ArrayList_getValue(
            block->statements, i);
        switch (context->tag) {
            case CONTEXT_ITERATIVE_STATEMENT: {
                IterativeStatement* statement = (IterativeStatement*)context;
                if (statement->label != NULL) {
                    scopeAddLabel(analyzer->scope, &statement->label);
                }
                Scope* localScope = resolveLocals(analyzer, statement->body);
                if (statement->parameter != NULL) {
                    scopeAddVariable(localScope, statement->parameter);
                }

                break;
            }

            case CONTEXT_IF_STATEMENT: {
                IfStatement* statement = (IfStatement*)context;
                resolveLocals(analyzer, statement->ifClause->body);
                int32_t count = jtk_ArrayList_getSize(statement->elseIfClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    IfClause* clause = (IfClause*)jtk_ArrayList_getValue(
                        statement->elseIfClauses, j);
                    resolveLocals(analyzer, clause->body);
                }
                if (statement->elseClause != NULL) {
                    resolveLocals(analyzer, statement->elseClause);
                }
                break;
            }

            case CONTEXT_TRY_STATEMENT: {
                TryStatement* statement = (TryStatement*)context;
                resolveLocals(analyzer, statement->tryClause);

                int32_t count = jtk_ArrayList_getSize(statement->catchClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    CatchClause* clause = (CatchClause*)jtk_ArrayList_getValue(
                        statement->catchClauses, j);
                    Scope* localScope = resolveLocals(analyzer, clause->body);
                    defineVariable(analyzer, localScope, clause->parameter);
                }

                if (statement->finallyClause != NULL) {
                    resolveLocals(analyzer, statement->finallyClause);
                }

                break;
            }

            case CONTEXT_VARIABLE_DECLARATION: {
                VariableDeclaration* statement = (VariableDeclaration*)context;
                int32_t count = jtk_ArrayList_getSize(statement->variables);
                int32_t j;
                for (j = 0; j < count; j++) {
                    Variable* variable = (Variable*)jtk_ArrayList_getValue(
                        statement->variables, j);
                    defineVariable(analyzer->scope, variable);
                }
                break;
            }
        }
    }

    analyzer->scope = analyzer->scope->parent;
}

Type* resolveExpression(Analyzer* analyzer, Context* context) {
    switch (context->tag) {
        case CONTEXT_RELATIONAL_EXPRESSION:
        case CONTEXT_EQUALITY_EXPRESSION: {
            BinaryExpression* expression = (BinaryExpression*)context;
            Type* leftType = resolveExpression(analyzer, expression->left);

            int32_t count = jtk_ArrayList_getSize(expression->others);
            if (count == 1) {
                jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
                    expression->others, 0);
                Type* rightType = resolveExpression(analyzer, pair->m_right);

                if (leftType != rightType) {
                    reportError(analyzer, ERROR_INCOMPATIBLE_OPERAND_TYPES, pair->m_left);
                }
            }
            else if (count > 1) {
                reportError(analyzer, ERROR_COMBINING_EQUALITY_OPERATORS);
            }

            break;
        }

        case CONTEXT_ASSIGNMENT_EXPRESSION:
        case CONTEXT_LOGICAL_OR_EXPRESSION:
        case CONTEXT_LOGICAL_AND_EXPRESSION:
        case CONTEXT_INCLUSIVE_OR_EXPRESSION:
        case CONTEXT_EXCLUSIVE_OR_EXPRESSION:
        case CONTEXT_AND_EXPRESSION:
        case CONTEXT_SHIFT_EXPRESSION:
        case CONTEXT_ADDITIVE_EXPRESSION:
        case CONTEXT_MULTIPLICATIVE_EXPRESSION: {
            BinaryExpression* expression = (BinaryExpression*)context;
            Type* leftType = resolveExpression(analyzer, expression->left);

            int32_t count = jtk_ArrayList_getSize(expression->others);
            int32_t i;
            for (i = 0; i < count; i++) {
                jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
                    expression->others, i);
                Type* rightType = resolveExpression(analyzer, pair->m_right);

                if (leftType != rightType) {
                    reportError(analyzer, ERROR_INCOMPATIBLE_OPERAND_TYPES, pair->m_left);
                }
            }

            break;
        }

        case CONTEXT_UNARY_EXPRESSION: {
            UnaryExpression* expression = (UnaryExpression*)context;
            Type* type = resolveExpression(analyzer, expression->expression);
            Token* operator = expression->operator;
            if (operator != NULL) {
                TokenType token = operator->type;
                if ((token == TOKEN_PLUS) || (token == TOKEN_DASH)) {
                    if ((type->tag != TYPE_INTEGER) && (type->tag != TYPE_DECIMAL)) {
                        reportError(analyzer, ERROR_INCOMPATIBLE_OPERAND, operator);
                    }
                }
                else if (token == TOKEN_TILDE) {
                    if (type->tag != TYPE_INTEGER) {
                        reportError(analyzer, ERROR_INCOMPATIBLE_OPERAND, operator);
                    }
                }
                else if (token == TOKEN_EXCLAMATION_MARK) {
                    if (type->tag != TYPE_BOOLEAN) {
                        reportError(analyzer, ERROR_INCOMPATIBLE_OPERAND, operator);
                    }
                }
                else {
                    printf("[internal error] Control should not reach here.\n");
                }
            }

            break;
        }

        case CONTEXT_POSTFIX_EXPRESSION: {
            PostfixExpression* expression = (PostfixExpression*)context;
            Type* type = expression->token?
                resolveTokenType(analyzer, (Token*)expression->primary) :
                resolveExpression(analyzer, expression->primary);

            Type* previous = type;
            int32_t count = jtk_ArrayList_getSize(expression->postfixParts);
            int32_t i;
            for (i = 0; i < count; i++) {
                Context* postfix = (Context*)jtk_ArrayList_getValue(
                    expression->postfixParts, i);

                Type* postfixType = NULL;
                if (postfix->tag == CONTEXT_SUBSCRIPT) {
                    Subscript* subscript = (Subscript*)postfix;
                    resolveExpression(analyzer, subscript->expression);
                    if (!previous->indexable) {
                        reportError(analyzer, ERROR_INVALID_LEFT_OPERAND,
                            subscript->bracket);
                    }
                }
                else if (postfix->tag == CONTEXT_FUNCTION_ARGUMENTS) {
                    FunctionArguments* arguments = (FunctionArguments*)postfix;
                    if (!previous->callable) {
                        reportError(analyzer, ERROR_INVALID_FUNCTION_INVOCATION,
                            arguments->parenthesis);
                    }
                    int32_t j;
                    for (j = 0; j < arguments->expressions->m_size; j++) {
                        BinaryExpression* argument = (BinaryExpression*)
                            arguments->expressions->m_values[j];
                        resolveExpression(analyzer, argument);
                    }
                }
                else if (postfix->tag == CONTEXT_MEMBER_ACCESS) {
                    MemberAccess* access = (MemberAccess*)postfix;
                    if (!previous->accessible) {
                        reportError(analyzer, ERROR_INVALID_MEMBER_ACCESS,
                            access->identifier);
                    }
                    // resolve the type of the member
                }
            }

            break;
        }

        case CONTEXT_INITIALIZER_EXPRESSION: {
            InitializerExpression* expression = (InitializerExpression*)context;
            // TODO
            break;
        }

        case CONTEXT_ARRAY_EXPRESSION: {
            ArrayExpression* expression = (ArrayExpression*)context;
            break;
        }
    }
}

Type* resolveToken(Analyzer* analyzer, Token* token) {
    Type* result = NULL;
    switch (token->type) {
        case TOKEN_IDENTIFIER: {
            // TODO
            break;
        }

        case TOKEN_INTEGER_LITERAL: {
            result = &(primitives.i32);
            break;
        }

        case TOKEN_FLOATING_POINT_LITERAL: {
            result = &(primitives.f64);
            break;
        }

        case TOKEN_KEYWORD_TRUE:
        case TOKEN_KEYWORD_FALSE: {
            result = &(primitives.boolean);
            break;
        }

        case TOKEN_STRING_LITERAL: {
            result = &(primitives.string);
            break;
        }

        case TOKEN_KEYWORD_NULL: {
            result = &(primitives.null);
            break;
        }

        default: {
            printf("[internal error] Control should not reach here.\n");
        }
    }
    return result;
}

// Constructor

Analyzer* Analyzer_new(Compiler* compiler) {
    Analyzer* analyzer = allocate(Analyzer, 1);
    analyzer->compiler = compiler;
    analyzer->package = NULL;
    analyzer->packageSize = -1;

    return analyzer;
}

// Destructor

void Analyzer_delete(Analyzer* analyzer) {
    jtAssert_assertObject(analyzer, "The specified analyzer is null.");

    jtMemory_deallocate(analyzer);
}

// Analyze

void Analyzer_analyze(Analyzer* analyzer, Module* module) {
    defineSymbols(analyzer, module);
    resolveSymbols(analyzer, module);
}

// Reset

void Analyzer_reset(Analyzer* analyzer,
    const uint8_t* package, int32_t packageSize) {
    analyzer->package = package;
    analyzer->packageSize = packageSize;
}
