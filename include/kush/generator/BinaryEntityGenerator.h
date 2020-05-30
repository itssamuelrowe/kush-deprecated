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

// Friday, March 20, 2018

#ifndef KUSH_COMPILER_GENERATOR_BINARY_ENTITY_GENERATOR_H
#define KUSH_COMPILER_GENERATOR_BINARY_ENTITY_GENERATOR_H

// #include <jtk/collection/map/DualHashMap.h>
#include <jtk/collection/list/ArrayList.h>
#include <jtk/io/OutputStream.h>

#include <kush/Configuration.h>

#include <kush/Compiler.h>
#include <kush/ast/ASTListener.h>
#include <kush/ast/ASTNode.h>
#include <kush/ast/ASTAnnotations.h>
#include <kush/ast/ExpressionAnnotation.h>
#include <kush/generator/ConstantPoolBuilder.h>
#include <kush/symbol-table/SymbolTable.h>

#include <kush/virtual-machine/feb/ByteCode.h>
#include <kush/virtual-machine/feb/EntityFile.h>

/*******************************************************************************
 * BinaryEntityGenerator                                                       *
 *******************************************************************************/

struct k_FunctionRecord_t {
};

/**
 * @author Samuel Rowe
 * @since zen 1.0
 * @ingroup k_compiler_generator
 */
struct k_BinaryEntityGenerator_t {
    k_Compiler_t* m_compiler;
    k_ASTListener_t* m_astListener;
    k_BinaryEntityBuilder_t* m_builder;
    k_SymbolTable_t* m_symbolTable;
    k_ASTAnnotations_t* m_scopes;
    k_ASTNode_t* m_compilationUnit;

    /**
     * The output stream where the generated binary entity will be written to.
     */
    jtk_OutputStream_t* m_outputStream;

    k_EntityFile_t* m_entityFile;
    k_ConstantPoolBuilder_t* m_constantPoolBuilder;
    uint8_t* m_package;
    int32_t m_packageSize;
    jtk_ArrayList_t* m_fields;
    jtk_ArrayList_t* m_functions;

    /* The following attributes are used to generate the instruction attribute.
     * Unfortunately, these fields are overwritten when multiple functions are being
     * declared simultaneously. For example, this situation may arise in nested
     * functions such as lambdas. A simple solution would be to refactor these
     * fields to another structure, say FunctionContext. Then the structure can
     * be stored in a hash map or stack contingent on the design of the code
     * generator.
     */

    // TODO: Rename this class!
    /**
     * The builder holds the channels where the instructions of the function
     * are written as they are generated.
     */
    k_BinaryEntityBuilder_t* m_instructions;

    /**
     * The maximum number of operands the operand stack can store.
     * A simple way to count this is to increment this field whenever
     * a push_* instruction is generated.
     *
     * TODO: In the future, please make sure that this field does not exceed
     * the bounds of `uint16_t`.
     *
     */
    uint16_t m_maxStackSize;

    /**
     * The total number of local variables declared within the function being
     * declared.
     *
     * A simple way to count this is to increment this field whenever
     * a variable is declared within a function scope.
     */
    int32_t m_localVariableCount;

    /**
     * The list of exception handler sites within the function being declared.
     */
    jtk_ArrayList_t* m_exceptionHandlerSites;

    /* The descriptor of the function for which instructions are being
     * generated.
     */
    uint8_t* m_descriptor;
    int32_t m_descriptorSize;

    int32_t* m_breakRecords;
    int32_t m_breakRecordsCount;
    int32_t m_breakRecordsCapacity;
    int32_t m_nextLoopLabel;
    int32_t m_currentLoopLabel;

    k_ASTNodeType_t m_mainComponent;
    bool m_classPrepared;
    uint8_t* m_className;
    int32_t m_classNameSize;

    uint16_t* m_cpfIndexes;
};

/**
 * @memberof BinaryEntityGenerator
 */
typedef struct k_BinaryEntityGenerator_t k_BinaryEntityGenerator_t;

// Constructor

/**
 * @memberof BinaryEntityGenerator
 */
k_BinaryEntityGenerator_t* k_BinaryEntityGenerator_new(k_Compiler_t* compiler);

// Destructor

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_delete(k_BinaryEntityGenerator_t* generator);

// Initialize

void k_BinaryEntityGenerator_initializeCPFCache(k_BinaryEntityGenerator_t* generator);

// Generate

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_generate(k_BinaryEntityGenerator_t* generator);

// Reset

void k_BinaryEntityGenerator_reset(k_BinaryEntityGenerator_t* generator,
    k_SymbolTable_t* symbolTable, k_ASTAnnotations_t* scopes,
    k_ASTNode_t* compilationUnit, const uint8_t* package, int32_t packageSize,
    jtk_OutputStream_t* outputStream);

// Descriptor

uint8_t* k_BinaryEntityGenerator_getDescriptorEx(k_ASTNode_t* functionParameters,
    bool constructor, int32_t* descriptorSize);

// Event Handlers

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onVisitErrorNode(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onVisitTerminal(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterElseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitElseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterLabel(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitLabel(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterForStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitForStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterForParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitForParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterTryClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitTryClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterClassMember(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitClassMember(k_ASTListener_t* listener, k_ASTNode_t* node);
// void k_BinaryEntityGenerator_onEnterConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);
// void k_BinaryEntityGenerator_onExitConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEnumrationBaseClass(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEnumerationBaseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterExpressions(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitExpressions(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterSubscript(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitSubscript(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onExitMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_BinaryEntityGenerator_onEnterMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_onExitMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_onEnterListExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_onExitListExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_onEnterNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof BinaryEntityGenerator
 */
void k_BinaryEntityGenerator_onExitNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

#endif /* KUSH_COMPILER_GENERATOR_BINARY_ENTITY_GENERATOR_H */
