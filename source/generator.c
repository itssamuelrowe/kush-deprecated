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

// Sunday, June 18 2020

#include <jtk/collection/Pair.h>
#include <kush/generator.h>

#define invalidate(generator) generator->scope = generator->scope->parent

/******************************************************************************
 * Generator                                                                  *
 ******************************************************************************/

static void generateType(Generator* generator, Type* type);
static void generateForwardReferences(Generator* generator, Module* module);
static void generateStructures(Generator* generator, Module* module);
static void generateBinary(Generator* generator, BinaryExpression* expression);
static void generateConditional(Generator* generator, ConditionalExpression* expression);
static void generateUnary(Generator* generator, UnaryExpression* expression);
static void generateSubscript(Generator* generator, Subscript* subscript);
static void generateFunctionArguments(Generator* generator, FunctionArguments* arguments);
static void generateMemberAccess(Generator* generator, MemberAccess* access);
static void generatePostfix(Generator* generator, PostfixExpression* expression);
static void generateToken(Generator* generator, Token* token);
static void generateInitializer(Generator* generator, InitializerExpression* expression);
static void generateArray(Generator* generator, ArrayExpression* expression);
static void generateExpression(Generator* generator, Context* context);
static void generateIndentation(Generator* generator, int32_t depth);
static void generateBlock(Generator* generator, Block* block, int32_t depth);
static void generateFunction(Generator* generator, Function* function);
static void generateFunctions(Generator* generator, Module* module);
static void generateHead(Generator* generator);

void generateType(Generator* generator, Type* type) {
    const char* output = NULL;
    if (type == &primitives.boolean) {
        output = "bool";
    }
    else if (type == &primitives.i8) {
        output = "int8_t";
    }
    else if (type == &primitives.i16) {
        output = "int16_t";
    }
    else if (type == &primitives.i32) {
        output = "int32_t";
    }
    else if (type == &primitives.i64) {
        output = "int64_t";
    }
    else if (type == &primitives.ui8) {
        output = "uint8_t";
    }
    else if (type == &primitives.ui16) {
        output = "uint16_t";
    }
    else if (type == &primitives.ui32) {
        output = "uint32_t";
    }
    else if (type == &primitives.ui64) {
        output = "uint64_t";
    }
    else if (type == &primitives.f32) {
        output = "float";
    }
    else if (type == &primitives.f64) {
        output = "double";
    }
    else if (type == &primitives.void_) {
        output = "void";
    }
    else if (type == &primitives.string) {
        output = "String*";
    }
    else {
        printf("TODO: generateType()\n");
    }
    printf("%s", output);
}

void generateForwardReferences(Generator* generator, Module* module) {
    int32_t structureCount = jtk_ArrayList_getSize(module->structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        Structure* structure = (Structure*)jtk_ArrayList_getValue(
            module->structures, j);
        printf("typedef struct %s %s;\n", structure->name, structure->name);
    }
    printf("\n");

    int32_t functionCount = jtk_ArrayList_getSize(module->functions);
    int32_t i;
    for (i = 0; i < functionCount; i++) {
        Function* function = (Function*)jtk_ArrayList_getValue(
            module->functions, i);
        generateType(generator, function->returnType);
        printf(" %s();\n", function->name);
    }
    printf("\n");
}

void generateStructures(Generator* generator, Module* module) {
    int32_t structureCount = jtk_ArrayList_getSize(module->structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        Structure* structure = (Structure*)jtk_ArrayList_getValue(
            module->structures, j);
        printf("struct %s {\n", structure->name);

        int32_t declarationCount = jtk_ArrayList_getSize(structure->declarations);
        int32_t i;
        for (i = 0; i < declarationCount; i++) {
            VariableDeclaration* declaration =
                (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);

            int32_t limit = jtk_ArrayList_getSize(declaration->variables);
            int32_t j;
            for (j = 0; j < limit; j++) {
                Variable* variable = (Variable*)jtk_ArrayList_getValue(declaration->variables, j);
                printf("    ");
                generateType(generator, variable->type);
                printf(" %s;\n", variable->name);
            }
        }

        printf("};\n");
    }
    printf("\n");
}

/* Return the type of the first expression, even if there are errors in the
 * right hand side.
 */
void generateBinary(Generator* generator, BinaryExpression* expression) {
    generateExpression(generator, (Context*)expression->left);

    int32_t count = jtk_ArrayList_getSize(expression->others);
    if (count > 0) {
        int32_t i;
        for (i = 0; i < count; i++) {
            jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
            printf(" %s ", ((Token*)pair->m_left)->text);
            generateExpression(generator, (Context*)pair->m_right);
        }
    }
}

void generateConditional(Generator* generator, ConditionalExpression* expression) {
    generateExpression(generator, (Context*)expression->condition);

    if (expression->hook != NULL) {
        printf("? ");
        generateExpression(generator, (Context*)expression->then);
        printf(" : ");
        generateExpression(generator, (Context*)expression->otherwise);
    }
}

void generateUnary(Generator* generator, UnaryExpression* expression) {
    generateExpression(generator, (Context*)expression->expression);

    Token* operator = expression->operator;
    if (operator != NULL) {
        printf("%s", operator->text);
    }
}

void generateSubscript(Generator* generator, Subscript* subscript) {
    printf("[");
    generateExpression(generator, (Context*)subscript->expression);
    printf("]");
}

void generateFunctionArguments(Generator* generator, FunctionArguments* arguments) {
    printf("(");
    int32_t count = jtk_ArrayList_getSize(arguments->expressions);
    int32_t j;
    for (j = 0; j < count; j++) {
        Context* context = (Context*)jtk_ArrayList_getValue(arguments->expressions, j);
        generateExpression(generator, context);

        if (j + 1 < count) {
            printf(", ");
        }
    }
    printf(")");
}

void generateMemberAccess(Generator* generator, MemberAccess* access) {
    printf("->%s", access->identifier->text);
}

void generatePostfix(Generator* generator, PostfixExpression* expression) {
    if (expression->token) {
        generateToken(generator, (Token*)expression->primary);
    }
    else {
        printf("(");
        generateExpression(generator, (Context*)expression->primary);
        printf(")");
    }

    int32_t count = jtk_ArrayList_getSize(expression->postfixParts);
    int32_t i;
    for (i = 0; i < count; i++) {
        Context* postfix = (Context*)jtk_ArrayList_getValue(
            expression->postfixParts, i);

        if (postfix->tag == CONTEXT_SUBSCRIPT) {
            generateSubscript(generator, (Subscript*)postfix);
        }
        else if (postfix->tag == CONTEXT_FUNCTION_ARGUMENTS) {
            generateFunctionArguments(generator, (FunctionArguments*)postfix);
        }
        else if (postfix->tag == CONTEXT_MEMBER_ACCESS) {
            generateMemberAccess(generator, (MemberAccess*)postfix);
        }
        else {
            controlError();
            break;
        }
    }
}

void generateToken(Generator* generator, Token* token) {
    switch (token->type) {
        case TOKEN_KEYWORD_TRUE:
        case TOKEN_KEYWORD_FALSE:
        case TOKEN_IDENTIFIER: {
            printf("%s", token->text);
            break;
        }

        case TOKEN_INTEGER_LITERAL: {
            // TODO
            printf("%s", token->text);
            break;
        }

        case TOKEN_FLOATING_POINT_LITERAL: {
            printf("%s", token->text);
            break;
        }

        case TOKEN_STRING_LITERAL: {
            printf("%.*s", token->length - 2, token->text + 1);
            break;
        }

        case TOKEN_KEYWORD_NULL: {
            printf("NULL");
            break;
        }

        default: {
            printf("[internal error] Control should not reach here.\n");
        }
    }
}

void generateInitializer(Generator* generator, InitializerExpression* expression) {
}

void generateArray(Generator* generator, ArrayExpression* expression) {
}

void generateExpression(Generator* generator, Context* context) {
    switch (context->tag) {
        case CONTEXT_ASSIGNMENT_EXPRESSION:
        case CONTEXT_LOGICAL_OR_EXPRESSION:
        case CONTEXT_LOGICAL_AND_EXPRESSION:
        case CONTEXT_INCLUSIVE_OR_EXPRESSION:
        case CONTEXT_EXCLUSIVE_OR_EXPRESSION:
        case CONTEXT_AND_EXPRESSION:
        case CONTEXT_EQUALITY_EXPRESSION:
        case CONTEXT_RELATIONAL_EXPRESSION:
        case CONTEXT_SHIFT_EXPRESSION:
        case CONTEXT_ADDITIVE_EXPRESSION:
        case CONTEXT_MULTIPLICATIVE_EXPRESSION: {
            generateBinary(generator, (BinaryExpression*)context);
            break;
        }

        case CONTEXT_CONDITIONAL_EXPRESSION: {
            generateConditional(generator, (ConditionalExpression*)context);
            break;
        }

        case CONTEXT_UNARY_EXPRESSION: {
            generateUnary(generator, (UnaryExpression*)context);
            break;
        }

        case CONTEXT_POSTFIX_EXPRESSION: {
            generatePostfix(generator, (PostfixExpression*)context);
            break;
        }

        case CONTEXT_INITIALIZER_EXPRESSION: {
            generateInitializer(generator, (InitializerExpression*)context);
           break;
        }

        case CONTEXT_ARRAY_EXPRESSION: {
            generateArray(generator, (ArrayExpression*)context);
           break;
        }

        default: {
            controlError();
            break;
        }
    }
}

void generateIndentation(Generator* generator, int32_t depth) {
    int32_t i;
    for (i = 0; i < depth; i++) {
        printf("    ");
    }
}

void generateBlock(Generator* generator, Block* block, int32_t depth) {
    printf("{\n");

    generator->scope = block->scope;
    int32_t limit = jtk_ArrayList_getSize(block->statements);
    if (limit > 0) {
        depth++;
        generateIndentation(generator, depth);

        int32_t i;
        for (i = 0; i < limit; i++) {
            Context* context = (Context*)jtk_ArrayList_getValue(
                block->statements, i);
            switch (context->tag) {
                case CONTEXT_ITERATIVE_STATEMENT: {
                    IterativeStatement* statement = (IterativeStatement*)context;

                    if (statement->name != NULL) {
                        printf("%s: ", statement->name);
                    }

                    if (statement->keyword->type == TOKEN_KEYWORD_WHILE) {
                        printf("while (");
                        generateExpression(generator, (Context*)statement->expression);
                        printf(") ");
                    }

                    generateBlock(generator, statement->body, depth);

                    if (statement->name != NULL) {
                        generateIndentation(generator, depth);
                        printf("__%sExit:\n", statement->name);
                    }

                    break;
                }

                case CONTEXT_IF_STATEMENT: {
                    IfStatement* statement = (IfStatement*)context;
                    printf("if (");
                    generateExpression(generator, (Context*)statement->ifClause->expression);
                    printf(") ");
                    generateBlock(generator, statement->ifClause->body, depth);

                    int32_t count = jtk_ArrayList_getSize(statement->elseIfClauses);
                    int32_t j;
                    for (j = 0; j < count; j++) {
                        generateIndentation(generator, depth);
                        IfClause* clause = (IfClause*)jtk_ArrayList_getValue(
                            statement->elseIfClauses, j);
                        printf("else if (");
                        generateExpression(generator, (Context*)clause->expression);
                        printf(") ");
                        generateBlock(generator, clause->body, depth);
                    }

                    if (statement->elseClause != NULL) {
                        generateIndentation(generator, depth);
                        printf("else ");
                        generateBlock(generator, statement->elseClause, depth);
                    }

                    break;
                }

                /*
                case CONTEXT_TRY_STATEMENT: {
                    TryStatement* statement = (TryStatement*)context;
                    defineLocals(generator, statement->tryClause);

                    int32_t count = jtk_ArrayList_getSize(statement->catchClauses);
                    int32_t j;
                    for (j = 0; j < count; j++) {
                        CatchClause* clause = (CatchClause*)jtk_ArrayList_getValue(
                            statement->catchClauses, j);
                        Variable* parameter = clause->parameter;
                        Scope* localScope = defineLocals(generator, clause->body);

                        if (isUndefined(localScope, parameter->identifier->text)) {
                            defineSymbol(localScope, (Symbol*)parameter);
                        }
                        else {
                            handleSemanticError(handler, generator, ERROR_REDECLARATION_AS_CATCH_PARAMETER,
                                parameter->identifier);
                        }
                    }

                    if (statement->finallyClause != NULL) {
                        defineLocals(generator, statement->finallyClause);
                    }

                break;
                }*/

                case CONTEXT_VARIABLE_DECLARATION: {
                    VariableDeclaration* statement = (VariableDeclaration*)context;
                    int32_t count = jtk_ArrayList_getSize(statement->variables);
                    int32_t j;
                    for (j = 0; j < count; j++) {
                        Variable* variable = (Variable*)jtk_ArrayList_getValue(
                            statement->variables, j);
                        generateType(generator, variable->type);
                        printf(" %s", variable->name);
                        if (variable->expression != NULL) {
                            printf(" = ");
                            generateExpression(generator, (Context*)variable->expression);
                        }
                        printf(";\n");
                    }
                    break;
                }

                case CONTEXT_ASSIGNMENT_EXPRESSION: {
                    generateExpression(generator, context);
                    printf(";\n");
                    break;
                }

                case CONTEXT_BREAK_STATEMENT: {
                    BreakStatement* statement = (BreakStatement*)context;
                    if (statement->identifier != NULL) {
                        printf("goto __%sExit;\n", statement->identifier->text);
                    }
                    else {
                        printf("break;\n");
                    }
                    break;
                }


                case CONTEXT_RETURN_STATEMENT: {
                    ReturnStatement* statement = (ReturnStatement*)context;
                    printf("return ");
                    generateExpression(generator, (Context*)statement->expression);
                    printf(";\n");

                    break;
                }

                /*
                case CONTEXT_THROW_STATEMENT: {
                    generateThrowStatement(generator, (ThrowStatement*)context);
                    break;
                }
                */

                default: {
                    controlError();
                    break;
                }
            }
            if (i + 1 < limit) {
                generateIndentation(generator, depth);
            }
        }
        generateIndentation(generator, depth - 1);
    }
    else {
        generateIndentation(generator, depth);
    }

    printf("}\n");
    invalidate(generator);
}

void generateFunction(Generator* generator, Function* function) {
    generator->scope = function->scope;

    generateType(generator, function->returnType);
    printf(" %s(", function->name);

    int32_t parameterCount = jtk_ArrayList_getSize(function->parameters);
    int32_t i;
    for (i = 0; i < parameterCount; i++) {
        Variable* parameter = (Variable*)jtk_ArrayList_getValue(function->parameters, i);
        generateType(generator, parameter->type);
        printf(" %s", parameter->name);
        if (i + 1 < parameterCount) {
            printf(", ");
        }
    }

    // TODO: Variable parameter

    printf(") ");

    generateBlock(generator, function->body, 0);
    printf("\n");

    invalidate(generator);
}

void generateFunctions(Generator* generator, Module* module) {
    int32_t functionCount = jtk_ArrayList_getSize(module->functions);
    int32_t i;
    for (i = 0; i < functionCount; i++) {
        Function* function = (Function*)jtk_ArrayList_getValue(
            module->functions, i);
        generateFunction(generator, function);
    }
}

void generateHead(Generator* generator) {
    printf("// Do not edit this file. It was automatically generated by the Kush compiler.\n\n");
    printf("#include <stdio.h>\n");
    printf("#include <stdint.h>\n");
    printf("\n");
}

void generateC(Generator* generator, Module* module) {
    generateHead(generator);
    generateForwardReferences(generator, module);
    generateStructures(generator, module);
    generateFunctions(generator, module);
}

Generator* newGenerator(Compiler* compiler) {
    Generator* generator = allocate(Generator, 1);
    generator->compiler = compiler;
    generator->scope = NULL;
    return generator;
}

void deleteGenerator(Generator* generator) {
    deallocate(generator);
}
