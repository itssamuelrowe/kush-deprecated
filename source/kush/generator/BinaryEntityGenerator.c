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

// Friday, March 30, 2018

#include <jtk/core/CString.h>
#include <jtk/core/Integer.h>
#include <jtk/core/Long.h>
#include <jtk/core/VariableArguments.h>
#include <jtk/core/StringBuilder.h>
#include <jtk/collection/Pair.h>
#include <jtk/collection/array/Arrays.h>

#include <kush/ast/ASTWalker.h>
#include <kush/ast/context/Context.h>
#include <kush/ast/ASTAnnotation.h>
#include <kush/ast/ASTNode.h>
#include <kush/ast/ASTHelper.h>
#include <kush/lexer/Token.h>
#include <kush/lexer/TokenType.h>
#include <kush/generator/BinaryEntityBuilder.h>
#include <kush/generator/DataChannel.h>
#include <kush/symbol-table/Symbol.h>
#include <kush/symbol-table/Scope.h>
#include <kush/generator/BinaryEntityGenerator.h>
#include <com/onecube/zen/virtual-machine/feb/EntityType.h>
#include <com/onecube/zen/virtual-machine/feb/attribute/InstructionAttribute.h>
#include <com/onecube/zen/virtual-machine/feb/attribute/PredefinedAttribute.h>

/* TODO: Move the variable to k_BinaryEntityGenerator_t. */
bool lhs = false;

// Initialize

#define ZEN_BINARY_ENTITY_GENERATOR_CPF_COUNT 6
#define ZEN_BINARY_ENTITY_GENERATOR_BOOLEAN_GET_VALUE 0
#define ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_EVALUATE 1
#define ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_STORE_FIELD 2
#define ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_INVOKE 3
#define ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_INVOKE_EX 4
#define ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_LOAD_FIELD 5

uint16_t k_Symbol_findFunctionIndex(k_ConstantPoolBuilder_t* builder,
    k_Symbol_t* symbol,
    const uint8_t* functionName, int32_t functionNameSize,
    const uint8_t* descriptor, uint8_t* descriptorSize) {
    k_Symbol_t* function = k_Scope_resolve(symbol->m_context.m_asClass.m_classScope,
        functionName);
    k_FunctionSignature_t* signature = k_Symbol_getFunctionSignatureEx(
        function, descriptor, descriptorSize);

    const uint8_t* className = symbol->m_context.m_asClass.m_qualifiedName;
    int32_t classNameSize = symbol->m_context.m_asClass.m_qualifiedNameSize;

    uint8_t* classDescriptor = jtk_CString_newEx(className, classNameSize);
    jtk_Arrays_replace_b(classDescriptor, classNameSize, '.', '/');

    uint16_t index = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        builder, classDescriptor, classNameSize, descriptor,
        descriptorSize, functionName, functionNameSize,
        signature->m_tableIndex
    );

    jtk_CString_delete(classDescriptor);

    return index;
}

void k_BinaryEntityGenerator_initializeCPFCache(k_BinaryEntityGenerator_t* generator) {
    generator->m_cpfIndexes = jtk_Memory_allocate(uint16_t, ZEN_BINARY_ENTITY_GENERATOR_CPF_COUNT);

    // Boolean
    k_Symbol_t* booleanClass = k_Compiler_resolveSymbol(generator->m_compiler,
        "zen.core.Boolean", 16);
     // TODO: "z:v", 3
    generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_BOOLEAN_GET_VALUE] =
        k_Symbol_findFunctionIndex(generator->m_constantPoolBuilder,
            booleanClass, "getValue", 8, "(zen/core/Object):v", 19);

    // ZenKernel
    k_Symbol_t* zenKernelClass = k_Compiler_resolveSymbol(generator->m_compiler,
        "zen.core.ZenKernel", 18);
    generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_EVALUATE] =
        k_Symbol_findFunctionIndex(generator->m_constantPoolBuilder,
            zenKernelClass, "evaluate", 8, "(zen/core/Object):(zen/core/Object)(zen/core/Object)(zen/core/Object)", 69);
    generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_STORE_FIELD] =
        k_Symbol_findFunctionIndex(generator->m_constantPoolBuilder,
            zenKernelClass, "storeField", 10, "(zen/core/Object):(zen/core/Object)(zen/core/Object)(zen/core/Object)", 69);
    generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_INVOKE] =
        k_Symbol_findFunctionIndex(generator->m_constantPoolBuilder,
            zenKernelClass, "invoke", 6, "(zen/core/Object):(zen/core/Object)(zen/core/Object)", 52);
    generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_INVOKE_EX] =
        k_Symbol_findFunctionIndex(generator->m_constantPoolBuilder,
            zenKernelClass, "invokeEx", 8, "(zen/core/Object):(zen/core/Object)(zen/core/Object)@(zen/core/Object)", 70);
    generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_LOAD_FIELD] =
        k_Symbol_findFunctionIndex(generator->m_constantPoolBuilder,
            zenKernelClass, "loadField", 9, "(zen/core/Object):(zen/core/Object)(zen/core/Object)", 52);
}

// Constructor

k_BinaryEntityGenerator_t* k_BinaryEntityGenerator_new(
    k_Compiler_t* compiler) {
    k_BinaryEntityGenerator_t* generator = k_Memory_allocate(k_BinaryEntityGenerator_t, 1);
    generator->m_compiler = compiler;
    generator->m_astListener = k_ASTListener_newWithContext(generator);
    generator->m_builder = k_BinaryEntityBuilder_new();
    generator->m_symbolTable = NULL;
    generator->m_scopes = NULL;
    generator->m_compilationUnit = NULL;
    generator->m_outputStream = NULL;
    generator->m_entityFile = k_Memory_allocate(k_EntityFile_t, 1);
    generator->m_constantPoolBuilder = k_ConstantPoolBuilder_new();
    generator->m_package = NULL;
    generator->m_fields = jtk_ArrayList_new();
    generator->m_functions = jtk_ArrayList_new();

    generator->m_maxStackSize = 0;
    generator->m_localVariableCount = 0;
    generator->m_exceptionHandlerSites = jtk_ArrayList_new();

    generator->m_breakRecords = NULL;
    generator->m_breakRecordsCount = 0;
    generator->m_breakRecordsCapacity = 0;
    generator->m_nextLoopLabel = 0;
    generator->m_currentLoopLabel = -1;

    generator->m_mainComponent = ZEN_AST_NODE_TYPE_UNKNOWN;
    generator->m_classPrepared = false;
    generator->m_className = NULL;
    generator->m_classNameSize = -1;
    generator->m_cpfIndexes = NULL;

    k_ASTListener_t* astListener = generator->m_astListener;

    astListener->m_onVisitErrorNode = k_BinaryEntityGenerator_onVisitErrorNode;
    astListener->m_onVisitTerminal = k_BinaryEntityGenerator_onVisitTerminal;

    astListener->m_onEnterEveryRule = k_BinaryEntityGenerator_onEnterEveryRule;
    astListener->m_onExitEveryRule = k_BinaryEntityGenerator_onExitEveryRule;

    astListener->m_onEnterCompilationUnit = k_BinaryEntityGenerator_onEnterCompilationUnit;
    astListener->m_onExitCompilationUnit = k_BinaryEntityGenerator_onExitCompilationUnit;

    astListener->m_onEnterImportDeclaration = k_BinaryEntityGenerator_onEnterImportDeclaration;
    astListener->m_onExitImportDeclaration = k_BinaryEntityGenerator_onExitImportDeclaration;

    astListener->m_onEnterAnnotatedComponentDeclaration = k_BinaryEntityGenerator_onEnterAnnotatedComponentDeclaration;
    astListener->m_onExitAnnotatedComponentDeclaration = k_BinaryEntityGenerator_onExitAnnotatedComponentDeclaration;

    astListener->m_onEnterAnnotations = k_BinaryEntityGenerator_onEnterAnnotations;
    astListener->m_onExitAnnotations = k_BinaryEntityGenerator_onExitAnnotations;

    astListener->m_onEnterAnnotation = k_BinaryEntityGenerator_onEnterAnnotation;
    astListener->m_onExitAnnotation = k_BinaryEntityGenerator_onExitAnnotation;

    astListener->m_onEnterAnnotationType = k_BinaryEntityGenerator_onEnterAnnotationType;
    astListener->m_onExitAnnotationType = k_BinaryEntityGenerator_onExitAnnotationType;

    astListener->m_onEnterAnnotationAttribute = k_BinaryEntityGenerator_onEnterAnnotationAttribute;
    astListener->m_onExitAnnotationAttribute = k_BinaryEntityGenerator_onExitAnnotationAttribute;

    astListener->m_onEnterComponentDeclaration = k_BinaryEntityGenerator_onEnterComponentDeclaration;
    astListener->m_onExitComponentDeclaration = k_BinaryEntityGenerator_onExitComponentDeclaration;

    astListener->m_onEnterFunctionDeclaration = k_BinaryEntityGenerator_onEnterFunctionDeclaration;
    astListener->m_onExitFunctionDeclaration = k_BinaryEntityGenerator_onExitFunctionDeclaration;

    astListener->m_onEnterFunctionParameters = k_BinaryEntityGenerator_onEnterFunctionParameters;
    astListener->m_onExitFunctionParameters = k_BinaryEntityGenerator_onExitFunctionParameters;

    astListener->m_onEnterFunctionBody = k_BinaryEntityGenerator_onEnterFunctionBody;
    astListener->m_onExitFunctionBody = k_BinaryEntityGenerator_onExitFunctionBody;

    astListener->m_onEnterStatementSuite = k_BinaryEntityGenerator_onEnterStatementSuite;
    astListener->m_onExitStatementSuite = k_BinaryEntityGenerator_onExitStatementSuite;

    astListener->m_onEnterSimpleStatement = k_BinaryEntityGenerator_onEnterSimpleStatement;
    astListener->m_onExitSimpleStatement = k_BinaryEntityGenerator_onExitSimpleStatement;

    astListener->m_onEnterStatement = k_BinaryEntityGenerator_onEnterStatement;
    astListener->m_onExitStatement = k_BinaryEntityGenerator_onExitStatement;

    astListener->m_onEnterEmptyStatement = k_BinaryEntityGenerator_onEnterEmptyStatement;
    astListener->m_onExitEmptyStatement = k_BinaryEntityGenerator_onExitEmptyStatement;

    astListener->m_onEnterVariableDeclaration = k_BinaryEntityGenerator_onEnterVariableDeclaration;
    astListener->m_onExitVariableDeclaration = k_BinaryEntityGenerator_onExitVariableDeclaration;

    astListener->m_onEnterVariableDeclarator = k_BinaryEntityGenerator_onEnterVariableDeclarator;
    astListener->m_onExitVariableDeclarator = k_BinaryEntityGenerator_onExitVariableDeclarator;

    astListener->m_onEnterConstantDeclaration = k_BinaryEntityGenerator_onEnterConstantDeclaration;
    astListener->m_onExitConstantDeclaration = k_BinaryEntityGenerator_onExitConstantDeclaration;

    astListener->m_onEnterConstantDeclarator = k_BinaryEntityGenerator_onEnterConstantDeclarator;
    astListener->m_onExitConstantDeclarator = k_BinaryEntityGenerator_onExitConstantDeclarator;

    astListener->m_onEnterAssertStatement = k_BinaryEntityGenerator_onEnterAssertStatement;
    astListener->m_onExitAssertStatement = k_BinaryEntityGenerator_onExitAssertStatement;

    astListener->m_onEnterBreakStatement = k_BinaryEntityGenerator_onEnterBreakStatement;
    astListener->m_onExitBreakStatement = k_BinaryEntityGenerator_onExitBreakStatement;

    astListener->m_onEnterContinueStatement = k_BinaryEntityGenerator_onEnterContinueStatement;
    astListener->m_onExitContinueStatement = k_BinaryEntityGenerator_onExitContinueStatement;

    astListener->m_onEnterReturnStatement = k_BinaryEntityGenerator_onEnterReturnStatement;
    astListener->m_onExitReturnStatement = k_BinaryEntityGenerator_onExitReturnStatement;

    astListener->m_onEnterThrowStatement = k_BinaryEntityGenerator_onEnterThrowStatement;
    astListener->m_onExitThrowStatement = k_BinaryEntityGenerator_onExitThrowStatement;

    astListener->m_onEnterCompoundStatement = k_BinaryEntityGenerator_onEnterCompoundStatement;
    astListener->m_onExitCompoundStatement = k_BinaryEntityGenerator_onExitCompoundStatement;

    astListener->m_onEnterIfStatement = k_BinaryEntityGenerator_onEnterIfStatement;
    astListener->m_onExitIfStatement = k_BinaryEntityGenerator_onExitIfStatement;

    astListener->m_onEnterIfClause = k_BinaryEntityGenerator_onEnterIfClause;
    astListener->m_onExitIfClause = k_BinaryEntityGenerator_onExitIfClause;

    astListener->m_onEnterElseIfClause = k_BinaryEntityGenerator_onEnterElseIfClause;
    astListener->m_onExitElseIfClause = k_BinaryEntityGenerator_onExitElseIfClause;

    astListener->m_onEnterElseClause = k_BinaryEntityGenerator_onEnterElseClause;
    astListener->m_onExitElseClause = k_BinaryEntityGenerator_onExitElseClause;

    astListener->m_onEnterIterativeStatement = k_BinaryEntityGenerator_onEnterIterativeStatement;
    astListener->m_onExitIterativeStatement = k_BinaryEntityGenerator_onExitIterativeStatement;

    astListener->m_onEnterLabelClause = k_BinaryEntityGenerator_onEnterLabel;
    astListener->m_onExitLabelClause = k_BinaryEntityGenerator_onExitLabel;

    astListener->m_onEnterWhileStatement = k_BinaryEntityGenerator_onEnterWhileStatement;
    astListener->m_onExitWhileStatement = k_BinaryEntityGenerator_onExitWhileStatement;

    astListener->m_onEnterForStatement = k_BinaryEntityGenerator_onEnterForStatement;
    astListener->m_onExitForStatement = k_BinaryEntityGenerator_onExitForStatement;

    astListener->m_onEnterForParameter = k_BinaryEntityGenerator_onEnterForParameters;
    astListener->m_onExitForParameter = k_BinaryEntityGenerator_onExitForParameters;

    astListener->m_onEnterTryStatement = k_BinaryEntityGenerator_onEnterTryStatement;
    astListener->m_onExitTryStatement = k_BinaryEntityGenerator_onExitTryStatement;

    astListener->m_onEnterTryClause = k_BinaryEntityGenerator_onEnterTryClause;
    astListener->m_onExitTryClause = k_BinaryEntityGenerator_onExitTryClause;

    astListener->m_onEnterCatchClause = k_BinaryEntityGenerator_onEnterCatchClause;
    astListener->m_onExitCatchClause = k_BinaryEntityGenerator_onExitCatchClause;

    astListener->m_onEnterCatchFilter = k_BinaryEntityGenerator_onEnterCatchFilter;
    astListener->m_onExitCatchFilter = k_BinaryEntityGenerator_onExitCatchFilter;

    astListener->m_onEnterFinallyClause = k_BinaryEntityGenerator_onEnterFinallyClause;
    astListener->m_onExitFinallyClause = k_BinaryEntityGenerator_onExitFinallyClause;

    astListener->m_onEnterSynchronizeStatement = k_BinaryEntityGenerator_onEnterSynchronizeStatement;
    astListener->m_onExitSynchronizeStatement = k_BinaryEntityGenerator_onExitSynchronizeStatement;

    astListener->m_onEnterWithStatement = k_BinaryEntityGenerator_onEnterWithStatement;
    astListener->m_onExitWithStatement = k_BinaryEntityGenerator_onExitWithStatement;

    astListener->m_onEnterClassDeclaration = k_BinaryEntityGenerator_onEnterClassDeclaration;
    astListener->m_onExitClassDeclaration = k_BinaryEntityGenerator_onExitClassDeclaration;

    astListener->m_onEnterClassExtendsClause = k_BinaryEntityGenerator_onEnterClassExtendsClause;
    astListener->m_onExitClassExtendsClause = k_BinaryEntityGenerator_onExitClassExtendsClause;

    astListener->m_onEnterClassSuite = k_BinaryEntityGenerator_onEnterClassSuite;
    astListener->m_onExitClassSuite = k_BinaryEntityGenerator_onExitClassSuite;

    astListener->m_onEnterClassMember = k_BinaryEntityGenerator_onEnterClassMember;
    astListener->m_onExitClassMember = k_BinaryEntityGenerator_onExitClassMember;

    // astListener->m_onEnterConstructorDeclaration = k_BinaryEntityGenerator_onEnterConstructorDeclaration;
    // astListener->m_onExitConstructorDeclaration = k_BinaryEntityGenerator_onExitConstructorDeclaration;

    astListener->m_onEnterEnumerationDeclaration = k_BinaryEntityGenerator_onEnterEnumerationDeclaration;
    astListener->m_onExitEnumerationDeclaration = k_BinaryEntityGenerator_onExitEnumerationDeclaration;

    astListener->m_onEnterEnumerationBaseClause = k_BinaryEntityGenerator_onEnterEnumrationBaseClass;
    astListener->m_onExitEnumerationBaseClause = k_BinaryEntityGenerator_onExitEnumerationBaseClause;

    astListener->m_onEnterEnumerationSuite = k_BinaryEntityGenerator_onEnterEnumerationSuite;
    astListener->m_onExitEnumerationSuite = k_BinaryEntityGenerator_onExitEnumerationSuite;

    astListener->m_onEnterEnumerate = k_BinaryEntityGenerator_onEnterEnumerate;
    astListener->m_onExitEnumerate = k_BinaryEntityGenerator_onExitEnumerate;

    astListener->m_onEnterExpressions = k_BinaryEntityGenerator_onEnterExpressions;
    astListener->m_onExitExpressions = k_BinaryEntityGenerator_onExitExpressions;

    astListener->m_onEnterExpression = k_BinaryEntityGenerator_onEnterExpression;
    astListener->m_onExitExpression = k_BinaryEntityGenerator_onExitExpression;

    astListener->m_onEnterAssignmentExpression = k_BinaryEntityGenerator_onEnterAssignmentExpression;
    astListener->m_onExitAssignmentExpression = k_BinaryEntityGenerator_onExitAssignmentExpression;

    astListener->m_onEnterConditionalExpression = k_BinaryEntityGenerator_onEnterConditionalExpression;
    astListener->m_onExitConditionalExpression = k_BinaryEntityGenerator_onExitConditionalExpression;

    astListener->m_onEnterLogicalOrExpression = k_BinaryEntityGenerator_onEnterLogicalOrExpression;
    astListener->m_onExitLogicalOrExpression = k_BinaryEntityGenerator_onExitLogicalOrExpression;

    astListener->m_onEnterLogicalAndExpression = k_BinaryEntityGenerator_onEnterLogicalAndExpression;
    astListener->m_onExitLogicalAndExpression = k_BinaryEntityGenerator_onExitLogicalAndExpression;

    astListener->m_onEnterInclusiveOrExpression = k_BinaryEntityGenerator_onEnterInclusiveOrExpression;
    astListener->m_onExitInclusiveOrExpression = k_BinaryEntityGenerator_onExitInclusiveOrExpression;

    astListener->m_onEnterExclusiveOrExpression = k_BinaryEntityGenerator_onEnterExclusiveOrExpression;
    astListener->m_onExitExclusiveOrExpression = k_BinaryEntityGenerator_onExitExclusiveOrExpression;

    astListener->m_onEnterAndExpression = k_BinaryEntityGenerator_onEnterAndExpression;
    astListener->m_onExitAndExpression = k_BinaryEntityGenerator_onExitAndExpression;

    astListener->m_onEnterEqualityExpression = k_BinaryEntityGenerator_onEnterEqualityExpression;
    astListener->m_onExitEqualityExpression = k_BinaryEntityGenerator_onExitEqualityExpression;

    astListener->m_onEnterRelationalExpression = k_BinaryEntityGenerator_onEnterRelationalExpression;
    astListener->m_onExitRelationalExpression = k_BinaryEntityGenerator_onExitRelationalExpression;

    astListener->m_onEnterShiftExpression = k_BinaryEntityGenerator_onEnterShiftExpression;
    astListener->m_onExitShiftExpression = k_BinaryEntityGenerator_onExitShiftExpression;

    astListener->m_onEnterAdditiveExpression = k_BinaryEntityGenerator_onEnterAdditiveExpression;
    astListener->m_onExitAdditiveExpression = k_BinaryEntityGenerator_onExitAdditiveExpression;

    astListener->m_onEnterMultiplicativeExpression = k_BinaryEntityGenerator_onEnterMultiplicativeExpression;
    astListener->m_onExitMultiplicativeExpression = k_BinaryEntityGenerator_onExitMultiplicativeExpression;

    astListener->m_onEnterUnaryExpression = k_BinaryEntityGenerator_onEnterUnaryExpression;
    astListener->m_onExitUnaryExpression = k_BinaryEntityGenerator_onExitUnaryExpression;

    astListener->m_onEnterPostfixExpression = k_BinaryEntityGenerator_onEnterPostfixExpression;
    astListener->m_onExitPostfixExpression = k_BinaryEntityGenerator_onExitPostfixExpression;

    astListener->m_onEnterSubscript = k_BinaryEntityGenerator_onEnterSubscript;
    astListener->m_onExitSubscript = k_BinaryEntityGenerator_onExitSubscript;

    astListener->m_onEnterFunctionArguments = k_BinaryEntityGenerator_onEnterFunctionArguments;
    astListener->m_onExitFunctionArguments = k_BinaryEntityGenerator_onExitFunctionArguments;

    astListener->m_onEnterMemberAccess = k_BinaryEntityGenerator_onEnterMemberAccess;
    astListener->m_onExitMemberAccess = k_BinaryEntityGenerator_onExitMemberAccess;

    astListener->m_onEnterPostfixOperator = k_BinaryEntityGenerator_onEnterPostfixOperator;
    astListener->m_onExitPostfixOperator = k_BinaryEntityGenerator_onExitPostfixOperator;

    astListener->m_onEnterPrimaryExpression = k_BinaryEntityGenerator_onEnterPrimaryExpression;
    astListener->m_onExitPrimaryExpression = k_BinaryEntityGenerator_onExitPrimaryExpression;

    astListener->m_onEnterMapExpression = k_BinaryEntityGenerator_onEnterMapExpression;
    astListener->m_onExitMapExpression = k_BinaryEntityGenerator_onExitMapExpression;

    astListener->m_onEnterMapEntries = k_BinaryEntityGenerator_onEnterMapEntries;
    astListener->m_onExitMapEntries = k_BinaryEntityGenerator_onExitMapEntries;

    astListener->m_onEnterMapEntry = k_BinaryEntityGenerator_onEnterMapEntry;
    astListener->m_onExitMapEntry = k_BinaryEntityGenerator_onExitMapEntry;

    astListener->m_onEnterListExpression = k_BinaryEntityGenerator_onEnterListExpression;
    astListener->m_onExitListExpression = k_BinaryEntityGenerator_onExitListExpression;

    astListener->m_onEnterNewExpression = k_BinaryEntityGenerator_onEnterNewExpression;
    astListener->m_onExitNewExpression = k_BinaryEntityGenerator_onExitNewExpression;

    if (!generator->m_compiler->m_coreApi) {
        k_BinaryEntityGenerator_initializeCPFCache(generator);
    }

    return generator;
}

// Constructor

void k_BinaryEntityGenerator_clearExceptionHandlerSites(k_BinaryEntityGenerator_t* generator) {
    int32_t exceptionHandlerSiteCount = jtk_ArrayList_getSize(generator->m_exceptionHandlerSites);
    int32_t exceptionHandlerSiteIndex;
    for (exceptionHandlerSiteIndex = 0;
        exceptionHandlerSiteIndex < exceptionHandlerSiteCount;
        exceptionHandlerSiteIndex++) {
        k_ExceptionHandlerSite_t* exceptionHandlerSite =
            (k_ExceptionHandlerSite_t*)jtk_ArrayList_getValue(
                generator->m_exceptionHandlerSites,
                exceptionHandlerSiteIndex);
        jtk_Memory_deallocate(exceptionHandlerSite);
    }
    jtk_ArrayList_clear(generator->m_exceptionHandlerSites);
}

void k_BinaryEntityGenerator_clearFields(k_BinaryEntityGenerator_t* generator) {
    int32_t fieldCount = jtk_ArrayList_getSize(generator->m_fields);
    int32_t fieldIndex;
    for (fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        k_FieldEntity_t* fieldEntity = (k_FieldEntity_t*)jtk_ArrayList_getValue(
            generator->m_fields, fieldIndex);
        k_FieldEntity_delete(fieldEntity);
    }
    jtk_ArrayList_clear(generator->m_fields);
}

void k_BinaryEntityGenerator_clearFunctions(k_BinaryEntityGenerator_t* generator) {
    int32_t functionCount = jtk_ArrayList_getSize(generator->m_functions);
    int32_t functionIndex;
    for (functionIndex = 0; functionIndex < functionCount; functionIndex++) {
        k_FunctionEntity_t* functionEntity = (k_FunctionEntity_t*)jtk_ArrayList_getValue(
            generator->m_functions, functionIndex);
        k_FunctionEntity_delete(functionEntity);
    }
    jtk_ArrayList_clear(generator->m_functions);
}

void k_BinaryEntityGenerator_delete(k_BinaryEntityGenerator_t* generator) {
    jtk_Assert_assertObject(generator, "The specified generator is null.");

#warning "TODO: The exception handler sites must be deleted when the associated function entity and instruction attribute are destroyed."
    k_BinaryEntityGenerator_clearExceptionHandlerSites(generator);
    jtk_ArrayList_delete(generator->m_exceptionHandlerSites);

    k_BinaryEntityBuilder_delete(generator->m_builder);

    k_BinaryEntityGenerator_clearFields(generator);
    jtk_ArrayList_delete(generator->m_fields);

    k_BinaryEntityGenerator_clearFunctions(generator);
    jtk_ArrayList_delete(generator->m_functions);

    /* Destroy the constant pool builder. It takes care of destroying the
     * constant pool entries.
     */
    k_ConstantPoolBuilder_delete(generator->m_constantPoolBuilder);
    k_Memory_deallocate(generator->m_entityFile);

    if (generator->m_breakRecords != NULL) {
        jtk_Memory_deallocate(generator->m_breakRecords);
    }

    if (generator->m_className != NULL) {
        jtk_CString_delete(generator->m_className);
    }

    if (generator->m_cpfIndexes != NULL) {
        jtk_Memory_deallocate(generator->m_cpfIndexes);
    }

    k_ASTListener_delete(generator->m_astListener);
    jtk_Memory_deallocate(generator);
}

// Generate

void k_BinaryEntityGenerator_generate(k_BinaryEntityGenerator_t* generator) {
    jtk_Assert_assertObject(generator, "The specified generator is null.");

    k_ASTWalker_walk(generator->m_astListener, generator->m_compilationUnit);
}

// Reset

void k_BinaryEntityGenerator_reset(k_BinaryEntityGenerator_t* generator,
    k_SymbolTable_t* symbolTable, k_ASTAnnotations_t* scopes,
    k_ASTNode_t* compilationUnit, const uint8_t* package, int32_t packageSize,
    jtk_OutputStream_t* outputStream) {
    jtk_Assert_assertObject(generator, "The specified generator is null.");

    // k_BinaryEntityBuilder_clear(generator->m_builder);

    if (generator->m_className != NULL) {
        jtk_CString_delete(generator->m_className);
    }

    generator->m_symbolTable = symbolTable;
    generator->m_scopes = scopes;
    generator->m_compilationUnit = compilationUnit;
    generator->m_package = package;
    generator->m_packageSize = packageSize;
    generator->m_outputStream = outputStream;
    generator->m_mainComponent = ZEN_AST_NODE_TYPE_UNKNOWN;
    generator->m_classPrepared = false;
    generator->m_className = NULL;
    generator->m_classNameSize = -1;
}

// Event Handlers

void k_BinaryEntityGenerator_onVisitErrorNode(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    fprintf(stderr, "[warning] Cannot resolve symbols on erroneous AST.\n");
}

void k_BinaryEntityGenerator_onVisitTerminal(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onEnterEveryRule(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitEveryRule(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

/* compilationUnit */

void k_BinaryEntityGenerator_onEnterCompilationUnit(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the scope associated with the current AST node. */
    k_Scope_t* scope = k_ASTAnnotations_get(generator->m_scopes, node);
    /* Activate the scope associated with the current AST node. */
    k_SymbolTable_setCurrentScope(generator->m_symbolTable, scope);

    /* Push a data channel, where the bytes generated will be written. */
    int32_t primaryChannelIndex = k_BinaryEntityBuilder_addChannel(generator->m_builder);
    k_BinaryEntityBuilder_setActiveChannelIndex(generator->m_builder, primaryChannelIndex);

    /* Set the major version of the target binary entity format. */
    generator->m_entityFile->m_version.m_majorVersion = 0x0000;
    /* Set the minor version of the target binary entity format. */
    generator->m_entityFile->m_version.m_minorVersion = 0x0001;

    generator->m_mainComponent = ZEN_AST_NODE_TYPE_UNKNOWN;
}

void k_BinaryEntityGenerator_onExitCompilationUnit(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;

    /* Invalidate the current scope in the symbol table. */
    k_SymbolTable_invalidateCurrentScope(generator->m_symbolTable);

    if (generator->m_mainComponent != ZEN_AST_NODE_TYPE_CLASS_DECLARATION) {
        k_BinaryEntityGenerator_writeEntity(generator);
    }
}

#include <stdio.h>

void k_BinaryEntityGenerator_writeOutput(k_BinaryEntityGenerator_t* generator,
    k_Entity_t* entity) {
    k_ConstantPoolUtf8_t* name = k_ConstantPoolBuilder_getUtf8Entry(
        generator->m_constantPoolBuilder, entity->m_reference);
    jtk_StringBuilder_t* builder = jtk_StringBuilder_new();
    jtk_StringBuilder_appendEx_z(builder, name->m_bytes, name->m_length);
    jtk_StringBuilder_appendEx_z(builder, ".feb", 4);
    int32_t pathSize;
    uint8_t* path = jtk_StringBuilder_toCString(builder, &pathSize);
    jtk_StringBuilder_delete(builder);

    k_DataChannel_t* channel = jtk_ArrayList_getValue(generator->m_builder->m_channels, 0);
    if (generator->m_compiler->m_dumpInstructions) {
        k_BinaryEntityDisassembler_disassemble(generator->m_compiler->m_disassembler,
            channel->m_bytes, channel->m_index);
    }

    FILE* fp = fopen(path, "w+");
    if (fp != NULL) {
        fwrite(channel->m_bytes, channel->m_index, 1, fp);
        fclose(fp);
    }
    else {
        fprintf(stderr, "[error] Failed to create output file '%s'.\n", path);
    }
    jtk_CString_delete(path);
}

void k_BinaryEntityGenerator_writeEntity(k_BinaryEntityGenerator_t* generator) {
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Write magic number, major version, and minor version on the main channel. */
    k_BinaryEntityBuilder_writeMagicNumber(generator->m_builder);
    /* Write the major version of the binary entity file format the stream is encoded in. */
    k_BinaryEntityBuilder_writeMajorVersion(generator->m_builder,
        generator->m_entityFile->m_version.m_majorVersion);
    /* Write the minor version of the binary entity file format the stream is encoded in. */
    k_BinaryEntityBuilder_writeMinorVersion(generator->m_builder,
        generator->m_entityFile->m_version.m_minorVersion);
    /* Write additional flags on how the binary entity file should be loaded. */
    k_BinaryEntityBuilder_writeStreamFlags(generator->m_builder,
        generator->m_entityFile->m_flags);

    /* At this point, all the constant pool entries required by the binary entity
     * file should be available to the constant pool builder. The constant pool
     * can now be built.
     */
    int32_t entryCount = k_ConstantPoolBuilder_countEntries(generator->m_constantPoolBuilder);
    /* The constant pool builder counts the null entry at the beginning, too.
     * However, the virtual machine's binary entity parser implicitly recognizes
     * first entry. Therefore, subtract the constant pool entry count by one.
     */
    k_BinaryEntityBuilder_writeConstantPoolHeader(generator->m_builder, entryCount - 1);
    int32_t i;
    for (i = 1; i < entryCount; i++) {
        k_ConstantPoolEntry_t* entry = k_ConstantPoolBuilder_getEntry(generator->m_constantPoolBuilder, i);
        k_BinaryEntityBuilder_writeConstantPoolEntry(generator->m_builder, entry);
    }

    /* Retrieve the entity to write. */
    k_Entity_t* entity = &generator->m_entityFile->m_entity;
    /* Write the entity header. */
    k_BinaryEntityBuilder_writeEntityHeader(generator->m_builder, entity->m_type,
        entity->m_flags, entity->m_reference);
    /* Write the superclasses. */
    k_BinaryEntityBuilder_writeSuperclasses(generator->m_builder, entity->m_superclassCount,
        entity->m_superclasses);
    /* Write the attribute count. */
    k_BinaryEntityBuilder_writeAttributeCount(generator->m_builder, entity->m_attributeTable.m_size);
    // TODO: Write the attribute

    /* Retrieve the field count. */
    int32_t fieldCount = jtk_ArrayList_getSize(generator->m_fields);
    /* Write the field count. */
    // TODO: The field table size should be computed when inheritance is implemented.
    k_BinaryEntityBuilder_writeFieldsHeader(generator->m_builder, fieldCount,
        fieldCount);

    int32_t fieldIndex;
    for (fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        /* Retrieve the next field to write. */
        k_FieldEntity_t* fieldEntity = (k_FieldEntity_t*)jtk_ArrayList_getValue(
            generator->m_fields, fieldIndex);

        /* Write the field to the data channel. */
        k_BinaryEntityBuilder_writeField(generator->m_builder, fieldEntity->m_flags,
            fieldEntity->m_nameIndex, fieldEntity->m_descriptorIndex,
            fieldEntity->m_tableIndex);

        /* Write the attribute count. */
        k_BinaryEntityBuilder_writeAttributeCount(generator->m_builder, fieldEntity->m_attributeTable.m_size);

        // TODO: Write the attribute!
    }

    /* Retrieve the function count. */
    int32_t functionCount = jtk_ArrayList_getSize(generator->m_functions);
    /* Write the function count. */
    // TODO: Function table size should be computed when inheritance is supported.
    k_BinaryEntityBuilder_writeFunctionsHeader(generator->m_builder, functionCount,
        functionCount);
    /* Log the function count. */
    jtk_Logger_debug(logger, "Entity has %d functions.", functionCount);

    int32_t functionIndex;
    for (functionIndex = 0; functionIndex < functionCount; functionIndex++) {
        /* Retrieve the next function to write. */
        k_FunctionEntity_t* functionEntity = (k_FunctionEntity_t*)jtk_ArrayList_getValue(
            generator->m_functions, functionIndex);

        /* Write the function to the data channel. */
        k_BinaryEntityBuilder_writeFunction(generator->m_builder,
            functionEntity->m_flags, functionEntity->m_nameIndex,
            functionEntity->m_descriptorIndex, functionEntity->m_tableIndex);

        /* Retrieve the attribute table for the current function entity. */
        k_AttributeTable_t* attributeTable = &functionEntity->m_attributeTable;

        /* Write the total number of attributes. */
        k_BinaryEntityBuilder_writeAttributeCount(generator->m_builder, attributeTable->m_size);

        int32_t attributeIndex;
        for (attributeIndex = 0; attributeIndex < attributeTable->m_size; attributeIndex++) {
            k_Attribute_t* attribute = attributeTable->m_attributes[attributeIndex];

            /* Retrieve the name of the current attribute. */
            k_ConstantPoolUtf8_t* name = k_ConstantPoolBuilder_getUtf8Entry(
                generator->m_constantPoolBuilder, attribute->m_nameIndex);

            /* If the current attribute is an instruction attribute, extract
             * the instruction attribute and write to the data channel.
             */
            if (jtk_CString_equals(name->m_bytes, name->m_length,
                ZEN_PREDEFINED_ATTRIBUTE_INSTRUCTION, ZEN_PREDEFINED_ATTRIBUTE_INSTRUCTION_SIZE)) {
                /* Cast to k_InstructionAttribute_t to extract further information. */
                k_InstructionAttribute_t* instructionAttribute =
                    (k_InstructionAttribute_t*)attribute;

                // TODO: CHANGE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!
                instructionAttribute->m_maxStackSize = 100;

                /* Write the instruction attribute for the current function. */
                k_BinaryEntityBuilder_writeInstructionAttribute(
                    generator->m_builder,
                    instructionAttribute->m_nameIndex,
                    instructionAttribute->m_length,
                    instructionAttribute->m_maxStackSize,
                    instructionAttribute->m_localVariableCount,
                    instructionAttribute->m_instructionLength,
                    instructionAttribute->m_instructions,
                    &instructionAttribute->m_exceptionTable);
            }
        }
    }

    k_BinaryEntityGenerator_writeOutput(generator, entity);
}

// importDeclaration

void k_BinaryEntityGenerator_onEnterImportDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitImportDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// annotatedComponentDeclaration

void k_BinaryEntityGenerator_onEnterAnnotatedComponentDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitAnnotatedComponentDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// annotations

void k_BinaryEntityGenerator_onEnterAnnotations(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitAnnotations(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// annotation

void k_BinaryEntityGenerator_onEnterAnnotation(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitAnnotation(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// annotationType

void k_BinaryEntityGenerator_onEnterAnnotationType(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitAnnotationType(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// annotationAttribute

void k_BinaryEntityGenerator_onEnterAnnotationAttribute(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitAnnotationAttribute(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// componentDeclaration

void k_BinaryEntityGenerator_onEnterComponentDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitComponentDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// functionDeclaration

void k_BinaryEntityGenerator_assignParameterIndexes(k_BinaryEntityGenerator_t* generator,
    k_ASTNode_t* functionParameters) {
    k_FunctionParametersContext_t* functionParametersContext =
        (k_FunctionParametersContext_t*)functionParameters->m_context;

    /* Retrieve the current scope from the symbol table. */
    k_Scope_t* currentScope = k_SymbolTable_getCurrentScope(generator->m_symbolTable);

    int32_t fixedParameterCount = jtk_ArrayList_getSize(functionParametersContext->m_fixedParameters);
    int32_t i;
    for (i = 0; i < fixedParameterCount; i++) {
        k_ASTNode_t* identifier = (k_ASTNode_t*)jtk_ArrayList_getValue(
            functionParametersContext->m_fixedParameters, i);
        int32_t identifierSize;
        uint8_t* identifierText = k_ASTNode_toCString(identifier, &identifierSize);

        k_Symbol_t* symbol = k_Scope_resolve(currentScope, identifierText);

        /* Generate an index for the parameter. */
        symbol->m_index = generator->m_localVariableCount;
        /* Update the local variable count, each parameter is a reference. Therefore,
         * increment the count by 2.
         */
        generator->m_localVariableCount += 2;

        jtk_CString_delete(identifierText);
    }

    if (functionParametersContext->m_variableParameter != NULL) {
        int32_t identifierSize;
        uint8_t* identifierText = k_ASTNode_toCString(
            functionParametersContext->m_variableParameter, &identifierSize);

        k_Symbol_t* symbol = k_Scope_resolve(currentScope, identifierText);
        /* Generate an index for the parameter. */
        symbol->m_index = generator->m_localVariableCount;
        /* Update the local variable count, each parameter is a reference. Therefore,
         * increment the count by 2.
         */
        generator->m_localVariableCount += 2;

        jtk_CString_delete(identifierText);
    }
}

void k_BinaryEntityGenerator_prepareClass(k_BinaryEntityGenerator_t* generator,
    const uint8_t* name, int32_t nameSize, const int16_t* superclassIndexes,
    int32_t superclassCount) {
    uint8_t* reference = NULL;
    uint8_t* reference0 = NULL;
    int32_t referenceSize;
    if (generator->m_package != NULL) {
        uint8_t* strings[] = {
            generator->m_package,
            "/",
            name
        };
        int32_t sizes[] = {
            generator->m_packageSize,
            1,
            nameSize
        };
        reference = jtk_CString_joinAll(strings, sizes, 3, &referenceSize);
        jtk_Arrays_replace_b(reference, referenceSize, '.', '/');
        reference0 = reference;
    }
    else {
        reference = name;
        referenceSize = nameSize;
    }

    uint16_t flags = 0;
    uint16_t referenceIndex =
        k_ConstantPoolBuilder_getUtf8EntryIndexEx(generator->m_constantPoolBuilder,
        reference, referenceSize);

    /* At this point, the reference is not required anymore. We have the index into
     * the constant pool which represents it. Therefore, destroy it.
     */
    if (reference0 != NULL) {
        jtk_CString_delete(reference0);
    }

    k_Entity_t* entity = &generator->m_entityFile->m_entity;
    entity->m_type = ZEN_ENTITY_TYPE_CLASS;
    entity->m_flags = 0;
    entity->m_reference = referenceIndex;
    entity->m_superclassCount = superclassCount;
    entity->m_superclasses = superclassIndexes;
    entity->m_attributeTable.m_size = 0;
    entity->m_fieldCount = 0;
    entity->m_fields = NULL;
    entity->m_functionCount = 0;
    entity->m_functions = NULL;

    generator->m_classPrepared = true;

    k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable,
        generator->m_className);
    k_SymbolTable_setCurrentScope(generator->m_symbolTable, symbol->m_context.m_asClass.m_classScope);
}

void k_BinaryEntityGenerator_initializeClassName(k_BinaryEntityGenerator_t* generator) {
    k_Compiler_t* compiler = generator->m_compiler;
    const uint8_t* fileName = (const uint8_t*)jtk_ArrayList_getValue(
        compiler->m_inputFiles, compiler->m_currentFileIndex);
    int32_t size = jtk_CString_getSize(fileName);
    int32_t slashIndex = jtk_CString_findLast_c(fileName, size, '/');
    int32_t dotIndex = jtk_CString_findLast_c(fileName, size, '.');
    generator->m_className = jtk_CString_substringEx(fileName, size, slashIndex + 1,
        dotIndex);
    generator->m_classNameSize = dotIndex - (slashIndex + 1);
}

void k_BinaryEntityGenerator_onEnterFunctionDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_FunctionDeclarationContext_t* context = (k_FunctionDeclarationContext_t*)node->m_context;
    k_FunctionParametersContext_t* functionParameters = (k_FunctionParametersContext_t*)context->m_functionParameters->m_context;

    if ((generator->m_mainComponent != ZEN_AST_NODE_TYPE_CLASS_DECLARATION)
        && !generator->m_classPrepared) {
        /* The extends clause has not been explicitly written. Therefore,
         * the compiler generates the default extends clause which inherits
         * the zen.core.Object class.
         */

        int32_t superclassCount = 1;
        uint16_t* superclassIndexes = jtk_Memory_allocate(uint16_t, 1);
        superclassIndexes[0] = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
            generator->m_constantPoolBuilder, "zen/core/Object", 15);

        k_BinaryEntityGenerator_initializeClassName(generator);
        k_BinaryEntityGenerator_prepareClass(generator, generator->m_className,
            generator->m_classNameSize, superclassIndexes, superclassCount);
    }

    int32_t instructionChannelIndex = k_BinaryEntityBuilder_addChannel(generator->m_builder);
    k_BinaryEntityBuilder_setActiveChannelIndex(generator->m_builder, instructionChannelIndex);

    // TODO: Remove the following statement. Make sure that the instruction
    // length is never zero.
    k_BinaryEntityBuilder_emitNop(generator->m_builder);

    k_ASTNode_t* identifier = context->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
    k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable,
        identifierToken->m_text);
    bool constructor = k_Token_getType(identifierToken) == ZEN_TOKEN_KEYWORD_NEW;
    generator->m_descriptor = k_BinaryEntityGenerator_getDescriptorEx(
        context->m_functionParameters, constructor,
        &generator->m_descriptorSize);

    k_Scope_t* scope = k_ASTAnnotations_get(generator->m_scopes, node);
    k_SymbolTable_setCurrentScope(generator->m_symbolTable, scope);

    int32_t parameterCount = jtk_ArrayList_getSize(functionParameters->m_fixedParameters);
    k_FunctionSignature_t* signature = k_Symbol_getFunctionSignature(symbol,
        parameterCount);

    /* The first local variable in the local variable array is reserved for the "this" pointer.
     * Therefore, do not perform the following increment for static functions.
     */
    if (!k_Modifier_hasStatic(signature->m_modifiers)) {
        generator->m_localVariableCount += 2;
    }

    k_BinaryEntityGenerator_assignParameterIndexes(generator, context->m_functionParameters);

    lhs = false;
}

/* The format of a function descriptor is shown below:
 * descriptor
 * :    returnType ':' parameters
 * ;
 *
 * returnType
 * :    type
 * ;
 *
 * parameters
 * :    type+
 * ;
 *
 * type
 * :    'v'
 * |    valueType
 * ;
 *
 * valueType
 * :    'z'
 * |    'b'
 * |    'c'
 * |    's'
 * |    'i'
 * |    'l'
 * |    'f'
 * |    'd'
 * |    className
 * |    arrayType
 * ;
 *
 * arrayType
 * :    '@'+ valueType
 * ;
 *
 * className
 * :    '(' IDENTIFIER ('/' IDENTIFIER)* ')'
 * ;
 *
 * Here is an example function.
 * function printf(format, ...arguments)
 *
 * The function descriptor for this function is shown below.
 * (zen/core/Object):(zen/core/Object)@(zen/core/Object)
 */
uint8_t* k_BinaryEntityGenerator_getDescriptorEx(k_ASTNode_t* functionParameters,
    bool constructor, int32_t* descriptorSize) {
    k_FunctionParametersContext_t* functionParametersContext =
        (k_FunctionParametersContext_t*)functionParameters->m_context;

    jtk_StringBuilder_t* builder = jtk_StringBuilder_new();
    jtk_StringBuilder_append_z(builder, constructor? "v:" : "(zen/core/Object):");

    int32_t fixedParameterCount = jtk_ArrayList_getSize(functionParametersContext->m_fixedParameters);
    int32_t i;
    for (i = 0; i < fixedParameterCount; i++) {
        jtk_StringBuilder_append_z(builder, "(zen/core/Object)");
    }

    if (functionParametersContext->m_variableParameter != NULL) {
        jtk_StringBuilder_append_z(builder, "@(zen/core/Object)");
    }

    if ((fixedParameterCount == 0) &&
        (functionParametersContext->m_variableParameter == NULL)) {
        jtk_StringBuilder_append_c(builder, 'v');
    }

    uint8_t* result = jtk_StringBuilder_toCString(builder, descriptorSize);

    /* Destroy the string builder. */
    jtk_StringBuilder_delete(builder);

    return result;
}

uint8_t* k_BinaryEntityGenerator_getDescriptor(k_ASTNode_t* functionParameters,
    int32_t* descriptorSize) {
    return k_BinaryEntityGenerator_getDescriptorEx(functionParameters, false, descriptorSize);
}

k_InstructionAttribute_t* k_BinaryEntityGenerator_makeInstructionAttribute(
    k_BinaryEntityGenerator_t* generator) {

    /* Retrieve the data channel on which the instructions of the
     * function/initializer were written.
     */
    k_DataChannel_t* channel = k_BinaryEntityBuilder_getActiveChannel(generator->m_builder);
    /* Retrieve the bytes that were written on the data channel. */
    uint8_t* instructionBytes = k_DataChannel_getBytes(channel);

    /* Retrieve a valid index into the constant pool where an UTF-8 entry
     * represents "vm/primary/Instruction".
     */
    uint16_t attributeNameIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
        generator->m_constantPoolBuilder, ZEN_PREDEFINED_ATTRIBUTE_INSTRUCTION,
        ZEN_PREDEFINED_ATTRIBUTE_INSTRUCTION_SIZE);
    /* Load the maximum stack size. */
    uint16_t maxStackSize = generator->m_maxStackSize;
    /* Load the number of local variables. */
    uint16_t localVariableCount = generator->m_localVariableCount;
    /* Retrieve the length of the instructions. */
    uint32_t instructionLength = k_DataChannel_getSize(channel);
    /* The instructions of the function. */
    uint8_t* instructions = jtk_Arrays_clone_b(instructionBytes, instructionLength);
    /* The total number of exception handler sistes within the function. */
    uint16_t exceptionHandlerSiteCount = 0;
    /* Calculate the length of the attribute immediately after the m_length
     * field.
     */
    uint32_t attributeLength =
        2 + // maxStackSize occupies two bytes.
        2 + // localVariableCount occupies two bytes.
        4 + // instructionLength occupies four bytes.
        instructionLength + // The total number of bytes the instructions occupy.
        2 + // m_exceptionTable.m_size occupies two bytes.
        exceptionHandlerSiteCount * ( // Each exception handler site entry occupies the following width.
            2 + // startIndex occupies two bytes.
            2 + // stopIndex occupies two bytes.
            2 + // handlerIndex occupies two bytes.
            2); // exceptionClassIndex occupies two bytes.

    k_InstructionAttribute_t* instructionAttribute = k_InstructionAttribute_new(
        attributeNameIndex,
        attributeLength,
        maxStackSize,
        localVariableCount,
        instructionLength,
        instructions);

    k_ExceptionTable_t* exceptionTable = &instructionAttribute->m_exceptionTable;
    exceptionTable->m_size = jtk_ArrayList_getSize(generator->m_exceptionHandlerSites);
    exceptionTable->m_exceptionHandlerSites = jtk_Memory_allocate(k_ExceptionHandlerSite_t*,
        exceptionTable->m_size);
#warning "TODO: Destroy the memory allocated here."

    int32_t exceptionHandlerSiteIndex;
    for (exceptionHandlerSiteIndex = 0;
        exceptionHandlerSiteIndex < exceptionTable->m_size;
        exceptionHandlerSiteIndex++) {
        k_ExceptionHandlerSite_t* exceptionHandlerSite =
            (k_ExceptionHandlerSite_t*)jtk_ArrayList_getValue(
                generator->m_exceptionHandlerSites, exceptionHandlerSiteIndex);
        exceptionTable->m_exceptionHandlerSites[exceptionHandlerSiteIndex] =
            exceptionHandlerSite;
    }

    return instructionAttribute;
}

// TODO: Somebody has to destroy the instruction attribute that was allocated here.
void k_BinaryEntityGenerator_onExitFunctionDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Retrieve the context associated with the AST node. */
    k_FunctionDeclarationContext_t* context = (k_FunctionDeclarationContext_t*)node->m_context;
    k_FunctionParametersContext_t* parameters = (k_FunctionParametersContext_t*)context->m_functionParameters->m_context;
    int32_t parameterCount = jtk_ArrayList_getSize(parameters->m_fixedParameters);

    k_ASTNode_t* identifier = context->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

    bool constructor = k_Token_getType(identifierToken) == ZEN_TOKEN_KEYWORD_NEW;

    uint8_t* temporary = jtk_CString_newEx(identifierToken->m_text, identifierToken->m_length);
    k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable, temporary);
    jtk_Memory_deallocate(temporary);

    // TODO: What happens when variable parameters are present?!
    k_FunctionSignature_t* signature = k_Symbol_getFunctionSignature(symbol,
        parameterCount);

    uint16_t flags = signature->m_modifiers;
    uint16_t nameIndex = constructor?
        k_ConstantPoolBuilder_getUtf8EntryIndexEx(
            generator->m_constantPoolBuilder, "<initialize>", 12) :
        k_ConstantPoolBuilder_getUtf8EntryIndexEx(
            generator->m_constantPoolBuilder, identifierToken->m_text,
            identifierToken->m_length);
    uint16_t descriptorIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
        generator->m_constantPoolBuilder, generator->m_descriptor,
        generator->m_descriptorSize);
    uint16_t tableIndex = signature->m_tableIndex;

    k_FunctionEntity_t* functionEntity = k_FunctionEntity_new(flags,
        nameIndex, descriptorIndex, tableIndex);
    k_AttributeTable_t* attributeTable = &functionEntity->m_attributeTable;

    k_InstructionAttribute_t* instructionAttribute =
        k_BinaryEntityGenerator_makeInstructionAttribute(generator);

    attributeTable->m_size = 1;
    attributeTable->m_attributes = k_Memory_allocate(k_Attribute_t*, 1);
    attributeTable->m_attributes[0] = (k_Attribute_t*)instructionAttribute;

    /* Add the function entity to the list of functions. */
    jtk_ArrayList_add(generator->m_functions, functionEntity);

    k_SymbolTable_invalidateCurrentScope(generator->m_symbolTable);

    int32_t exceptionHandlerSiteCount = jtk_ArrayList_getSize(
        generator->m_exceptionHandlerSites);
    int32_t exceptionHandlerSiteIndex;
    jtk_Logger_debug(logger, "Exception Handler Sites");
    for (exceptionHandlerSiteIndex = 0;
        exceptionHandlerSiteIndex < exceptionHandlerSiteCount;
        exceptionHandlerSiteIndex++) {
        k_ExceptionHandlerSite_t* exceptionHandlerSite =
            (k_ExceptionHandlerSite_t*)jtk_ArrayList_getValue(
                generator->m_exceptionHandlerSites, exceptionHandlerSiteIndex);
        jtk_Logger_debug(logger, "(startIndex = %d, stopIndex = %d, handlerIndex = %d, exceptionClassIndex = %d)",
            exceptionHandlerSite->m_startIndex, exceptionHandlerSite->m_stopIndex,
            exceptionHandlerSite->m_handlerIndex, exceptionHandlerSite->m_exceptionClassIndex);
    }

    /* Here, we assume that the channels are added and removed as if they were
     * stored in a stack. The primary channel is assumed to be at zeroth index.
     */
    int32_t instructionChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(generator->m_builder);
    k_BinaryEntityBuilder_removeChannel(generator->m_builder, instructionChannelIndex);

#warning "TODO: Implement a stack like behaviour to alter active channel."
    k_BinaryEntityBuilder_setActiveChannelIndex(generator->m_builder, 0);

    /* Destroy the descriptor of the function. */
    jtk_CString_delete(generator->m_descriptor);
    /* The exception handler sites are destroyed when the entity file is destroyed. */
    jtk_ArrayList_clear(generator->m_exceptionHandlerSites);

    /* Reset the counters used for tracking certain properties of the function
     * being declared.
     */
    generator->m_maxStackSize = 0;
    generator->m_localVariableCount = 0;
    generator->m_descriptor = NULL;
    generator->m_breakRecordsCount = 0;
    generator->m_nextLoopLabel = 0;
    generator->m_currentLoopLabel = -1;
}

// functionParameters

void k_BinaryEntityGenerator_onEnterFunctionParameters(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitFunctionParameters(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// functionBody

void k_BinaryEntityGenerator_onEnterFunctionBody(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitFunctionBody(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// statementSuite

void k_BinaryEntityGenerator_onEnterStatementSuite(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;

    k_Scope_t* scope = k_ASTAnnotations_get(generator->m_scopes, node);
    k_SymbolTable_setCurrentScope(generator->m_symbolTable, scope);
}

void k_BinaryEntityGenerator_onExitStatementSuite(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;

    k_SymbolTable_invalidateCurrentScope(generator->m_symbolTable);
}

// simpleStatement

void k_BinaryEntityGenerator_onEnterSimpleStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitSimpleStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_SimpleStatementContext_t* context = (k_SimpleStatementContext_t*)node->m_context;
    k_Compiler_t* compiler = generator->m_compiler;
    jtk_Logger_t* logger = compiler->m_logger;

    if (context->m_statement->m_type == ZEN_AST_NODE_TYPE_EXPRESSION) {
        /* Emit the pop2 instruction to clear the operand stack. Without the generation
        * of this instruction, the operand stack will overflow.
        */
        k_BinaryEntityBuilder_emitPop2(generator->m_builder);

        /* Log the emission of the `pop2` instruction. */
        jtk_Logger_debug(logger, "Emitted `pop2`");
    }
}

// statement

void k_BinaryEntityGenerator_onEnterStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// emptyStatement

void k_BinaryEntityGenerator_onEnterEmptyStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator =
        (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Emit the nop instruction. */
    k_BinaryEntityBuilder_emitNop(generator->m_builder);

    /* Log the emission of the nop instruction. */
    jtk_Logger_debug(logger, "Emitted nop");
}

void k_BinaryEntityGenerator_onExitEmptyStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// variableDeclaration

void k_BinaryEntityGenerator_onEnterVariableDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Retrieve the context associated with the AST node. */
    k_VariableDeclarationContext_t* context = (k_VariableDeclarationContext_t*)node->m_context;

    /* Retrieve the current scope from the symbol table. */
    k_Scope_t* currentScope = k_SymbolTable_getCurrentScope(generator->m_symbolTable);

    int32_t variableCount = jtk_ArrayList_getSize(context->m_variableDeclarators);
    int32_t i;
    for (i = 0; i < variableCount; i++) {
        k_ASTNode_t* variableDeclarator = (k_ASTNode_t*)jtk_ArrayList_getValue(
            context->m_variableDeclarators, i);
        k_VariableDeclaratorContext_t* variableDeclaratorContext =
            (k_VariableDeclaratorContext_t*)variableDeclarator->m_context;

        /* Retrieve the identifier AST node. */
        k_ASTNode_t* identifier = variableDeclaratorContext->m_identifier;
        /* Retrieve the identifier token. */
        k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

        /* If a class scope encloses the variable being declared, we are processing
         * a class member declaration.
         */
        if (k_Scope_isClassScope(currentScope)) {
            uint16_t flags = 0;

            /* Retrieve the constant pool index for the variable name. */
            uint16_t nameIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
                generator->m_constantPoolBuilder,
                identifierToken->m_text, identifierToken->m_length);

            uint16_t descriptorIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
                generator->m_constantPoolBuilder,
                "zen/core/Object", 15);

            /* Create an instance of the field entity that represents the variable
             * declared.
             */
            k_FieldEntity_t* fieldEntity = k_FieldEntity_new(flags, nameIndex, descriptorIndex);

            /* Add the field entity to the list of fields. */
            jtk_ArrayList_add(generator->m_fields, fieldEntity);
        }
        else if (k_Scope_isLocalScope(currentScope)) {
            /* Retrieve the string equivalent to the identifier node. */
            int32_t identifierSize;
            uint8_t* identifierText = k_ASTNode_toCString(identifier, &identifierSize);

            /* TODO: If the local scope belongs to an instance function, then a local
             * variable for the "this" reference should be created.
             */

            k_Symbol_t* symbol = k_Scope_resolve(currentScope, identifierText);
            if (k_Symbol_isVariable(symbol)) {
                /* Generate and assign the index of the local variable only if it
                 * was not previously assigned an index.
                 *
                 * I don't remember why this condition should be checked.
                 */
                if (symbol->m_index < 0) {
                    symbol->m_index = generator->m_localVariableCount;
                    // TODO: Temporary fix. References are considered as 8 bytes.
                    // TODO: Design the local variable array correctly.
                    generator->m_localVariableCount += 2;

                    if (variableDeclaratorContext->m_expression != NULL) {
                        k_ASTWalker_walk(astListener, variableDeclaratorContext->m_expression);

                        /* Store the obtained result in the local variable.
                        * The actual emission of the instruction is delegated to the
                        * k_BinaryEntityGenerator_storeLocalReference() function which takes
                        * care of optimizing the emission.
                        *
                        * TODO: Implement the k_BinaryEntityGenerator_storeLocalReference() function.
                        */
                        k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, symbol->m_index);

                        /* Log the emission of the store_a instruction. */
                        jtk_Logger_debug(logger, "Emitted store_a %d", symbol->m_index);
                    }
                }
            }
            else {
                printf("[internal error] Identifier recognized by variable declarator registered as a non-variable entity in the symbol table.\n");
            }
        }
    }

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitVariableDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// variableDeclarator

void k_BinaryEntityGenerator_onEnterVariableDeclarator(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitVariableDeclarator(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// constantDeclaration

void k_BinaryEntityGenerator_onEnterConstantDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Retrieve the context associated with the AST node. */
    k_ConstantDeclarationContext_t* context = (k_ConstantDeclarationContext_t*)node->m_context;

    /* Retrieve the current scope from the symbol table. */
    k_Scope_t* currentScope = k_SymbolTable_getCurrentScope(generator->m_symbolTable);

    int32_t variableCount = jtk_ArrayList_getSize(context->m_constantDeclarators);
    int32_t i;
    for (i = 0; i < variableCount; i++) {
        k_ASTNode_t* constantDeclarator = (k_ASTNode_t*)jtk_ArrayList_getValue(
            context->m_constantDeclarators, i);
        k_ConstantDeclaratorContext_t* constantDeclaratorContext =
            (k_ConstantDeclaratorContext_t*)constantDeclarator->m_context;

        /* Retrieve the identifier AST node. */
        k_ASTNode_t* identifier = constantDeclaratorContext->m_identifier;
        /* Retrieve the identifier token. */
        k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

        /* If a class scope encloses the variable being declared, we are processing
         * a class member declaration.
         */
        if (k_Scope_isClassScope(currentScope)) {
            uint16_t flags = 0;

            /* Retrieve the constant pool index for the variable name. */
            uint16_t nameIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
                generator->m_constantPoolBuilder,
                identifierToken->m_text, identifierToken->m_length);

            uint16_t descriptorIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
                generator->m_constantPoolBuilder, "zen/core/Object", 15);

            /* Create an instance of the field entity that represents the variable
             * declared.
             */
            k_FieldEntity_t* fieldEntity = k_FieldEntity_new(flags, nameIndex, descriptorIndex);

            /* Add the field entity to the list of fields. */
            jtk_ArrayList_add(generator->m_fields, fieldEntity);
        }
        else if (k_Scope_isLocalScope(currentScope)) {
            /* Retrieve the string equivalent to the identifier node. */
            int32_t identifierSize;
            uint8_t* identifierText = k_ASTNode_toCString(identifier, &identifierSize);

            /* TODO: If the local scope belongs to an instance function, then a local
             * variable for the "this" reference should be created.
             */

            k_Symbol_t* symbol = k_Scope_resolve(currentScope, identifierText);
            if (k_Symbol_isConstant(symbol)) {
                /* Generate and assign the index of the local variable only if it
                 * was not previously assigned an index.
                 */
                symbol->m_index = generator->m_localVariableCount;
                // TODO: Temporary fix. References are considered as 8 bytes.
                // TODO: Design the local variable array correctly.
                generator->m_localVariableCount += 2;

                if (constantDeclaratorContext->m_expression != NULL) {
                    k_ASTWalker_walk(astListener, constantDeclaratorContext->m_expression);

                    /* Store the obtained result in the local constant.
                     * The actual emission of the instruction is delegated to the
                     * k_BinaryEntityGenerator_storeLocalReference() function which takes
                     * care of optimizing the emission.
                     *
                     * TODO: Implement the k_BinaryEntityGenerator_storeLocalReference() function.
                     */
                    k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, symbol->m_index);

                    /* Log the emission of the store_a instruction. */
                    jtk_Logger_debug(logger, "Emitted store_a %d", symbol->m_index);
                }
            }
            else {
                printf("[internal error] Identifier recognized by constant declarator registered as a non-constant entity in the symbol table.\n");
            }
        }
    }

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitConstantDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// constantDeclarator

void k_BinaryEntityGenerator_onEnterConstantDeclarator(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitConstantDeclarator(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// assertStatement

/*
 * ALGORITHM TO GENERATE INSTRUCTIONS CORRESPONDING TO ASSERT STATEMENT
 *
 * When an assert statement is used within the bounds of a class, a static
 * field is created by the compiler. This field has the identifier '$assertionEnabled'.
 * It is initialized in the static initializer block, which is generated
 * by the compiler.
 *
 * 1. Load the static field '$assertionEnabled'.
 * 2. If assertions are disabled, skip the instruction assertion statement.
 * 3. Otherwise, execute the instructions corresponding to the condition expression
 *    specified to the assert statement.
 * 4. Convert the resulting object into a Boolean primitive.
 * 5. If the result is true, then skip the assert statement.
 * 6. Otherwise, execute the instructions corresponding to the message expression
 *    specified to the assert statement.
 * 7. Generate the instructions required to create and throw an exception.
 *    The exception is an instance of the zen.core.AssertionError class.
 */
void k_BinaryEntityGenerator_onEnterAssertStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the context of the AST node. */
    k_AssertStatementContext_t* context = (k_AssertStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitAssertStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    /* Retrieve the context of the AST node. */
    k_AssertStatementContext_t* context = (k_AssertStatementContext_t*)node->m_context;

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
         generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
         generator->m_builder, parentChannelIndex);

    uint8_t* className = "Unknown"; // TODO: Get the name of the current class!
    int32_t classNameSize = 7;
    uint8_t* descriptor = "z";
    int32_t descriptorSize = 1;
    uint8_t* name = "$assertionEnabled";
    int32_t nameSize = 17;
    int32_t assertionEnabledIndex = k_ConstantPoolBuilder_getFieldEntryIndexEx(
        generator->m_constantPoolBuilder, className, classNameSize,
        descriptor, descriptorSize, name, nameSize);

    const uint8_t* booleanClassName = "zen/core/Boolean";
    int32_t booleanClassNameSize = 16;
    // const uint8_t* getValueDescriptor = "z:v";
    // int32_t getValueDescriptorSize = 3;
    const uint8_t* getValueDescriptor = "(zen/core/Object):v";
    int32_t getValueDescriptorSize = 19;
    const uint8_t* getValueName = "getValue";
    int32_t getValueNameSize = 8;
    uint16_t getValueIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, booleanClassName, booleanClassNameSize,
        getValueDescriptor, getValueDescriptorSize, getValueName,
        getValueNameSize, 0);

    const uint8_t* assertionErrorClassName = "zen/core/AssertionError";
    int32_t assertionErrorClassNameSize = 26;
    uint16_t assertionErrorClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, assertionErrorClassName,
        assertionErrorClassNameSize);

    const uint8_t* constructor1Descriptor = "v:@(zen/core/Object)";
    int32_t constructor1DescriptorSize = 20;
    const uint8_t* constructorName = "<initialize>";
    int32_t constructorNameSize = 10;
    uint16_t constructor1Index = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, assertionErrorClassName,
        assertionErrorClassNameSize, constructor1Descriptor, constructor1DescriptorSize,
        constructorName, constructorNameSize, 0);

    const uint8_t* constructor2Descriptor = "v:v";
    int32_t constructor2DescriptorSize = 3;
    uint16_t constructor2Index = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, assertionErrorClassName,
        assertionErrorClassNameSize, constructor2Descriptor, constructor2DescriptorSize,
        constructorName, constructorNameSize, 0);

    /* Emit the load_static_field instruction. */
    k_BinaryEntityBuilder_emitLoadStaticField(generator->m_builder,
        assertionEnabledIndex);
    /* Log the emission of the load_static_field instruction. */
    jtk_Logger_debug(logger, "Emitted load_static_field %d", assertionEnabledIndex);

    /* Emit the jump_eq0_i instruction. */
    k_BinaryEntityBuilder_emitJumpEqual0Integer(generator->m_builder, 0);
    /* Log the emission of the jump_eq0_i instruction. */
    jtk_Logger_debug(logger, "Emitted jump_eq0_i 0 (dummy index)");

    /* Save the index of the byte where the dummy data was written. */
    int32_t updateIndex1 = k_DataChannel_getSize(parentChannel) - 2;

    /* Generate the instructions corresponding to the condition expression
     * specified to the assert statement.
     */
    k_ASTWalker_walk(astListener, context->m_conditionExpression);

    /* Invoke the Boolean#getValue() function to retrieve the primitive equivalent
     * of the resulting object.
     */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder,
        getValueIndex);
    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", getValueIndex);

    /* Emit the jump_ne0_i instruction. */
    k_BinaryEntityBuilder_emitJumpNotEqual0Integer(generator->m_builder, 0);
    /* Log the emission of the jump_ne0_i instruction. */
    jtk_Logger_debug(logger, "Emitted jump_ne0_i 0 (dummy index)");

    /* Save the index of the byte where the dummy data was written. */
    int32_t updateIndex2 = k_DataChannel_getSize(parentChannel) - 2;

    int32_t constructorIndex = constructor2Index;
    if (context->m_messageExpression != NULL) {
        /* Generate the instructions corresponding to the message expression
         * specified to the assert statement.
         */
        k_ASTWalker_walk(astListener, context->m_messageExpression);

        /* Use the constructor that accepts a detail message parameter. */
        constructorIndex = constructor1Index;
    }

    /* Create an instance of the AssertionError class. */
    k_BinaryEntityBuilder_emitNew(generator->m_builder, assertionErrorClassIndex);
    /* Log the emission of the new instruction. */
    jtk_Logger_debug(logger, "Emitted new %d", assertionErrorClassIndex);

    /* Duplicate the reference of the newly created exception. */
    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);
    /* Log the emission of the duplicate instruction. */
    jtk_Logger_debug(logger, "Emitted duplicate");

    /* Invoke the constructor to initialize the new exception instance. */
    k_BinaryEntityBuilder_emitInvokeSpecial(generator->m_builder,
        constructorIndex);
    /* Log the emission of the invoke_special instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_special %d", constructorIndex);

    /* Throw the newly created exception. */
    k_BinaryEntityBuilder_emitThrow(generator->m_builder);
    /* Log the emission of the throw instruction. */
    jtk_Logger_debug(logger, "Emitted throw");

    int32_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
    /* Update the jump offset of the first jump instruction. */
    parentChannel->m_bytes[updateIndex1] = (newParentChannelSize & 0x0000FF00) >> 8;
    parentChannel->m_bytes[updateIndex1 + 1] = newParentChannelSize & 0x000000FF;
    /* Update the jump offset of the second jump instruction. */
    parentChannel->m_bytes[updateIndex2] = (newParentChannelSize & 0x0000FF00) >> 8;
    parentChannel->m_bytes[updateIndex2 + 1] = newParentChannelSize & 0x000000FF;
}

// breakStatement

#define ZEN_BREAK_RECORDS_BUFFER_INCREMENT 8

void k_BinaryEntityGenerator_recordBreak(k_BinaryEntityGenerator_t* generator,
    int32_t loopIdentifier, int32_t updateIndex) {
    if (generator->m_breakRecordsCount + 1 >= generator->m_breakRecordsCapacity) {
        int32_t newCapacity = generator->m_breakRecordsCapacity + ZEN_BREAK_RECORDS_BUFFER_INCREMENT;
        int32_t* newBuffer = jtk_Memory_allocate(int32_t, newCapacity * 2);

        /* Copy the values in the old buffer and destroy it. */
        if (generator->m_breakRecords != NULL) {
            jtk_Arrays_copyEx_i(generator->m_breakRecords, generator->m_breakRecordsCount * 2,
                0, newBuffer, generator->m_breakRecordsCount * 2, 0,
                generator->m_breakRecordsCount * 2);
            jtk_Memory_deallocate(generator->m_breakRecords);
        }

        generator->m_breakRecords = newBuffer;
        generator->m_breakRecordsCapacity = newCapacity;
    }

    generator->m_breakRecordsCount++;
    int32_t breakRecordIndex = (generator->m_breakRecordsCount - 1) * 2;
    generator->m_breakRecords[breakRecordIndex] = loopIdentifier;
    generator->m_breakRecords[breakRecordIndex + 1] = updateIndex;
}

/* When the "enter" listener of a loop statement is invoked, a unique identifier
 * is generated for the loop statement. If the loop statement has a label, the
 * label symbol is associated with this identifier. The code generator emits a
 * jump instruction with a dummy offset for the break statement. The position
 * of the dummy offset and the identifier of the current loop statement are
 * recorded. When the "exit" listener of a loop statement is invoked, all the
 * offsets with the identifier of the loop statement are updated.
 *
 * | [loop_identifier][jump_offset_index] |
 * ----------------------------------------
 *          break statement record
 */
void k_BinaryEntityGenerator_onEnterBreakStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    /* Retrieve the context of the AST node. */
    k_BreakStatementContext_t* context = (k_BreakStatementContext_t*)node->m_context;

    if (generator->m_currentLoopLabel >= 0) {
        int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
            generator->m_builder);
        k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
            generator->m_builder, parentChannelIndex);

        /* Emit the jump instruction. */
        k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);
        /* Log the emission of the jump instruction. */
        jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

        /* Calculate the inndex of the byte where the dummy data was written. */
        int32_t updateIndex = k_DataChannel_getSize(parentChannel) - 2;
        /* Determine the identifier of the loop to break. */
        int32_t loopIdentifier;
        if (context->m_identifier != NULL) {
            int32_t identifierSize;
            uint8_t* identifierText = k_ASTNode_toCString(context->m_identifier, &identifierSize);

            /* Resolve the parameter symbol in the symbol table. */
            k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable, identifierText);
            /* Retrieve the label symbol. */
            loopIdentifier = symbol->m_index;
        }
        else {
            loopIdentifier = generator->m_currentLoopLabel;
        }

        k_BinaryEntityGenerator_recordBreak(generator, loopIdentifier, updateIndex);
    }
    else {
        printf("[error] Break statement outside an iterative statement.\n");
        printf("[warning] Please move this error detection to the syntax analysis phase.\n");
    }
}

void k_BinaryEntityGenerator_onExitBreakStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// continueStatement

void k_BinaryEntityGenerator_onEnterContinueStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitContinueStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// returnStatement

void k_BinaryEntityGenerator_onEnterReturnStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitReturnStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Retrieve the context of the AST node. */
    k_ReturnStatementContext_t* context =
        (k_ReturnStatementContext_t*)node->m_context;

    /* Emit the return_a instruction. */
    k_BinaryEntityBuilder_emitReturnReference(generator->m_builder);

    /* Log the emission of the instruction. */
    jtk_Logger_debug(logger, "Emitted return_a");
}

// throwStatement

void k_BinaryEntityGenerator_onEnterThrowStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitThrowStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Retrieve the context of the AST node. */
    k_ThrowStatementContext_t* context =
        (k_ThrowStatementContext_t*)node->m_context;

    /* Emit the throw instruction. */
    k_BinaryEntityBuilder_emitThrow(generator->m_builder);

    /* Log the emission of the instruction. */
    jtk_Logger_debug(logger, "Emitted throw");
}

// compoundStatement

void k_BinaryEntityGenerator_onEnterCompoundStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitCompoundStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// ifStatement

void k_BinaryEntityGenerator_onEnterIfStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_IfStatementContext_t* context = (k_IfStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitIfStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_IfStatementContext_t* context = (k_IfStatementContext_t*)node->m_context;

    uint16_t getValueIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_BOOLEAN_GET_VALUE];

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
         generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
         generator->m_builder, parentChannelIndex);

    k_ASTNode_t* ifClause = context->m_ifClause;
    k_IfClauseContext_t* ifClauseContext = (k_IfClauseContext_t*)ifClause->m_context;
    k_ASTNode_t* expression = ifClauseContext->m_expression;
    k_ASTNode_t* statementSuite = ifClauseContext->m_statementSuite;

    int32_t size = jtk_ArrayList_getSize(context->m_elseIfClauses);
    int32_t* skipIndexes = jtk_Memory_allocate(int32_t, size + 1);

    int32_t updateIndex = -1;
    int32_t index = -1;
    do {
        /* Generate the instructions corresponding to the conditional expression
         * specified to the if clause.
         */
        k_ASTWalker_walk(astListener, expression);

        /* Invoke the Boolean#getValue() function to retrieve the primitive equivalent
         * of the resulting object.
         */
        k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder,
            getValueIndex);

        /* Log the emission of the invoke_special instruction. */
        jtk_Logger_debug(logger, "Emitted invoke_virtual %d", getValueIndex);

        /* Emit the jump_eq0_i instruction. */
        k_BinaryEntityBuilder_emitJumpEqual0Integer(generator->m_builder, 0);

        /* Log the emission of the jump_eq0_i instruction. */
        jtk_Logger_debug(logger, "Emitted jump_eq0_i 0 (dummy index)");

        /* Save the index of the byte where the dummy data was written. */
        updateIndex = k_DataChannel_getSize(parentChannel) - 2;

        /* Generate the instructions corresponding to the statement suite specified
         * to the if clause.
         */
        k_ASTWalker_walk(astListener, statementSuite);

        /* Update the loop counter and prepare for the next iteration, if any. */
        index++;

        /* A jump instrution should not be generated only if the current
         * clause is the last if/else if clause.
         */
        if (index != size) {
            /* If the if clause was selected and executed all the other clauses should
             * be skipped. Generate a jump instruction to skip the ladder. Given the
             * whole ladder has not been generated yet, the jump offset cannot be
             * evaluated right now. Therefore, emit the jump instruction with a dummy
             * offset.
             */
            k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);

            /* Log the emission of the jump instruction. */
            jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

            /* Save the index of the bytes where the dummy data was written. */
            skipIndexes[index] = k_DataChannel_getSize(parentChannel) - 2;

            k_ASTNode_t* elseIfClause = jtk_ArrayList_getValue(context->m_elseIfClauses, index);
            k_ElseIfClauseContext_t* elseIfClauseContext =
                (k_ElseIfClauseContext_t*)elseIfClause->m_context;
            expression = elseIfClauseContext->m_expression;
            statementSuite = elseIfClauseContext->m_statementSuite;
        }

        /* The new parent channel size is equal to the offset where the jump
         * instruction should branch when the condition is false.
         *
         * Note: the instructions are indexed beginning from zero. Otherwise,
         * an extra 1 should be added to the offset.
         */
        int32_t newParentChannelSize = k_DataChannel_getSize(parentChannel);

        parentChannel->m_bytes[updateIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
        parentChannel->m_bytes[updateIndex + 1] = newParentChannelSize & 0x000000FF;
    }
    while (index < size);

    /* When there is no else clause, the number of skip indexes to fill is given
     * by the number of else if clauses. However, when an else clause is present,
     * the number of skip indexes to fill is given by the number of else if clauses
     * plus 1, which is also equal to the maximum number of skip indexes.
     */
    int32_t numberOfSkips = size;
    /* A jump instruction is generated immediately after the instructions
     * corresponding to the body of an if/else if clause when an else clause
     * is present.
     */
    if (context->m_elseClause != NULL) {
        /* If the previous if/else if clause was selected and executed then
         * the else clause should be skipped. Generate a jump instruction to skip
         * the else clause. Given the else clause has not been generated yet,
         * the jump offset cannot be evaluated right now. Therefore, emit the
         * jump instruction with a dummy offset.
         */
        k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);

        /* Log the emission of the jump instruction. */
        jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

        /* The last if clause is unaware of the else clause. In other words, when
         * none of the if clauses are selected, the jump_ne0_i instruction causes
         * the program to reach the jump instruction generated here to skip the else
         * clause, meaning the else clause will never be executed! Therefore, update
         * the index given to the jump_ne0_i instruction corresponding to the last
         * if clause.
         */
        int32_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
        parentChannel->m_bytes[updateIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
        parentChannel->m_bytes[updateIndex + 1] = newParentChannelSize & 0x000000FF;

        /* Save the index of the bytes where the dummy data was written. */
        skipIndexes[index] = k_DataChannel_getSize(parentChannel) - 2;

        /* Increase the number of skips to indicate the presence of an else clause. */
        numberOfSkips++;

        /* Retrieve the AST node for the else clause. */
        k_ASTNode_t* elseClause = context->m_elseClause;
        /* Retrieve the context associated with the AST node of the else clause. */
        k_ElseClauseContext_t* elseClauseContext =
            (k_ElseClauseContext_t*)elseClause->m_context;
        /* Generate the instructions for the statements within the else clause. */
        k_ASTWalker_walk(astListener, elseClauseContext->m_statementSuite);
    }

    int32_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
    int32_t i;
    for (i = 0; i < numberOfSkips; i++) {
        uint16_t skipIndex = skipIndexes[i];
        parentChannel->m_bytes[skipIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
        parentChannel->m_bytes[skipIndex + 1] = newParentChannelSize & 0x000000FF;
    }

    jtk_Memory_deallocate(skipIndexes);
}

// ifClause

void k_BinaryEntityGenerator_onEnterIfClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitIfClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// elseIfClause

void k_BinaryEntityGenerator_onEnterElseIfClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitElseIfClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// elseClause

void k_BinaryEntityGenerator_onEnterElseClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitElseClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// iterativeStatement

void k_BinaryEntityGenerator_onEnterIterativeStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    /* Retrieve the context of the AST node. */
    k_IterativeStatementContext_t* context = (k_IterativeStatementContext_t*)node->m_context;

    int32_t parentLoop = generator->m_currentLoopLabel;
    int32_t loopIdentifier = generator->m_nextLoopLabel++;

    if (context->m_labelClause != NULL) {
        k_LabelClauseContext_t* labelClauseContext =
            (k_LabelClauseContext_t*)context->m_labelClause->m_context;
        int32_t identifierTextSize;
        uint8_t* identifierText = k_ASTNode_toCString(labelClauseContext->m_identifier,
            &identifierTextSize);

        /* Resolve the parameter symbol in the symbol table. */
        k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable, identifierText);
        /* Associate the label symbol with the identifier generated
         * for the current loop.
         */
        symbol->m_index = loopIdentifier;
    }

    generator->m_currentLoopLabel = loopIdentifier;
    k_ASTWalker_walk(astListener, context->m_statement);
    generator->m_currentLoopLabel = parentLoop;

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
         generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
         generator->m_builder, parentChannelIndex);

    int32_t channelSize = k_DataChannel_getSize(parentChannel);
    int32_t i;
    for (i = 0; i < generator->m_breakRecordsCount; i++) {
        int32_t index = i * 2;
        /* Update the jump offset only for the current loop. */
        if (generator->m_breakRecords[i] == loopIdentifier) {
            int32_t updateIndex = generator->m_breakRecords[i + 1];
            /* Update the offset of the jump instruction. */
            parentChannel->m_bytes[updateIndex] = (channelSize & 0x0000FF00) >> 8;
            parentChannel->m_bytes[updateIndex + 1] = channelSize & 0x000000FF;
        }
    }

    /* Cause the AST walker to skip iterating over the children nodes. */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitIterativeStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// label

void k_BinaryEntityGenerator_onEnterLabel(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitLabel(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// whileStatement

void k_BinaryEntityGenerator_onEnterWhileStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_WhileStatementContext_t* context = (k_WhileStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitWhileStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_WhileStatementContext_t* context = (k_WhileStatementContext_t*)node->m_context;

    uint16_t getValueIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_BOOLEAN_GET_VALUE];

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
        generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
        generator->m_builder, parentChannelIndex);
    uint16_t loopIndex = k_DataChannel_getSize(parentChannel);

    /* Generate the instructions corresponding to the conditional expression
     * specified to the while statement.
     */
    k_ASTWalker_walk(astListener, context->m_expression);

    /* Invoke the Boolean#getValue() function to retrieve the primitive equivalent
     * of the resulting object.
     */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder,
        getValueIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", getValueIndex);

    /* Emit the jump_eq0_i instruction. */
    k_BinaryEntityBuilder_emitJumpEqual0Integer(generator->m_builder, 0);

    /* Log the emission of the jump_eq0_i instruction. */
    jtk_Logger_debug(logger, "Emitted jump_eq0_i 0 (dummy index)");

    /* Save the index of the byte where the dummy data was written. */
    int32_t updateIndex = k_DataChannel_getSize(parentChannel) - 2;

    /* Generate the instructions corresponding to the statement suite specified
     * to the while statement.
     */
    k_ASTWalker_walk(astListener, context->m_statementSuite);

    /* Generate a jump instruction to loop back to the conditional expression. */
    k_BinaryEntityBuilder_emitJump(generator->m_builder, loopIndex);

    /* Log the emission of the jump instruction. */
    jtk_Logger_debug(logger, "Emitted jump %d", loopIndex);

    uint16_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
    parentChannel->m_bytes[updateIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
    parentChannel->m_bytes[updateIndex + 1] = newParentChannelSize & 0x000000FF;
}

// forStatement

void k_BinaryEntityGenerator_onEnterForStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ForStatementContext_t* context = (k_ForStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitForStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_ForStatementContext_t* context = (k_ForStatementContext_t*)node->m_context;

    const uint8_t* iterableClassName = "zen/core/Iterable";
    int32_t iterableClassNameSize = 17;
    const uint8_t* getIteratorDescriptor = "(zen/core/Object):v";
    int32_t getIteratorDescriptorSize = 19;
    const uint8_t* getIteratorName = "getIterator";
    int32_t getIteratorNameSize = 11;
    uint16_t getIteratorIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, iterableClassName, iterableClassNameSize,
        getIteratorDescriptor, getIteratorDescriptorSize, getIteratorName,
        getIteratorNameSize, 0);

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
        generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
        generator->m_builder, parentChannelIndex);

    const uint8_t* iteratorClassName = "zen/core/Iterator";
    int32_t iteratorClassNameSize = 17;
    const uint8_t* hasNextDescriptor = "(zen/core/Object):v";
    int32_t hasNextDescriptorSize = 19;
    const uint8_t* hasNextName = "hasNext";
    int32_t hasNextNameSize = 7;
    uint16_t hasNextIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, iteratorClassName, iteratorClassNameSize,
        hasNextDescriptor, hasNextDescriptorSize, hasNextName,
        hasNextNameSize, 0);

    const uint8_t* getNextDescriptor = "(zen/core/Object):v";
    int32_t getNextDescriptorSize = 19;
    const uint8_t* getNextName = "getNext";
    int32_t getNextNameSize = 7;
    uint16_t getNextIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, iteratorClassName, iteratorClassNameSize,
        getNextDescriptor, getNextDescriptorSize, getNextName,
        getNextNameSize, 0);

    uint16_t getValueIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_BOOLEAN_GET_VALUE];

    k_ASTNode_t* forParameter = context->m_forParameter;
    k_ForParameterContext_t* forParameterContext =
        (k_ForParameterContext_t*)forParameter->m_context;

    /* Generate the instructions corresponding to the expression specified to
     * the for statement.
     */
    k_ASTWalker_walk(astListener, context->m_expression);

    /* Invoke the Iterable#getIterator() function to retrieve the iterator.
     */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder,
        getIteratorIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", getIteratorIndex);

    int32_t iteratorIndex = generator->m_localVariableCount;
    generator->m_localVariableCount += 2;

    /* Store the obtained iterator in a local variable for future reference.
     * The actual emission of the instruction is delegated to the
     * k_BinaryEntityGenerator_storeLocalReference() function which takes
     * care of optimizing the emission.
     *
     * TODO: Implement the k_BinaryEntityGenerator_storeLocalReference() function.
     */
    k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, iteratorIndex);

    /* Log the emission of the store_a instruction. */
    jtk_Logger_debug(logger, "Emitted store_a %d", iteratorIndex);

    uint16_t loopIndex = k_DataChannel_getSize(parentChannel);

    /* Load the iterator from the local variable.
     * The actual emission of the instruction is delegated to the
     * k_BinaryEntityGenerator_loadLocalReference() function which takes
     * care of optimizing the emission.
     *
     * TODO: Implement the k_BinaryEntityGenerator_loadLocalReference() function.
     */
    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, iteratorIndex);

    /* Log the emission of the load_a instruction. */
    jtk_Logger_debug(logger, "Emitted load_a %d", iteratorIndex);

    /* Invoke the Iterator#hasNext() function to determine whether the iterator
     * has more values to return.
     */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, hasNextIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", hasNextIndex);

    /* Invoke the Boolean#getValue() function to determine whether the iterator
     * has more values to return.
     */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, getValueIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", getValueIndex);

    /* Emit the jump_eq0_i instruction. */
    k_BinaryEntityBuilder_emitJumpEqual0Integer(generator->m_builder, 0);

    /* Log the emission of the jump_eq0_i instruction. */
    jtk_Logger_debug(logger, "Emitted jump_eq0_i 0 (dummy index)");

    /* Save the index of the byte where the dummy data was written. */
    int32_t updateIndex = k_DataChannel_getSize(parentChannel) - 2;

    /* Load the iterator from the local variable so we can invoke
     * Iterator#getNext(). The actual emission of the instruction is delegated to the
     * k_BinaryEntityGenerator_loadLocalReference() function which takes
     * care of optimizing the emission.
     *
     * TODO: Implement the k_BinaryEntityGenerator_loadLocalReference() function.
     */
    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, iteratorIndex);

    /* Log the emission of the load_a instruction. */
    jtk_Logger_debug(logger, "Emitted load_a %d", iteratorIndex);

    /* Invoke the Iterator#getNext() function to retrieve the next value. */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, getNextIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", getNextIndex);

    int32_t parameterIndex = -1;
    /* Retrieve the string equivalent to the identifier node. */
    int32_t identifierSize;
    uint8_t* identifierText = k_ASTNode_toCString(forParameterContext->m_identifier, &identifierSize);
    /* Resolve the parameter symbol in the symbol table. */
    k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable, identifierText);
    if (forParameterContext->m_declaration) {
        /* Generate an index for the parameter. */
        parameterIndex = generator->m_localVariableCount;
        /* Update the local variable count, each parameter is a reference. Therefore,
         * increment the count by 2.
         */
        generator->m_localVariableCount += 2;

        symbol->m_index = parameterIndex;
    }
    else {
        parameterIndex = symbol->m_index;
    }

    /* Store the retrieved value in a local variable. */
    k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, parameterIndex);

    /* Log the emission of the store_a instruction. */
    jtk_Logger_debug(logger, "Emitted store_a %d", parameterIndex);

    /* Generate the instructions corresponding to the statement suite specified
     * to the while statement.
     */
    k_ASTWalker_walk(astListener, context->m_statementSuite);

    /* Generate a jump instruction to loop back to the conditional expression. */
    k_BinaryEntityBuilder_emitJump(generator->m_builder, loopIndex);

    /* Log the emission of the jump instruction. */
    jtk_Logger_debug(logger, "Emitted jump %d", loopIndex);

    uint16_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
    parentChannel->m_bytes[updateIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
    parentChannel->m_bytes[updateIndex + 1] = newParentChannelSize & 0x000000FF;
}

// forParameters

void k_BinaryEntityGenerator_onEnterForParameters(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitForParameters(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// tryStatement

void k_BinaryEntityGenerator_onEnterTryStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the context of the AST node. */
    k_TryStatementContext_t* context = (k_TryStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

/*
 * The algorithm for generating instructions corresponding to a try statement is
 * given below.
 *
 * ### Algorithm 1
 *
 * - Try Clause
 *   1. Generate instructions corresponding to the statement suite specified
 *      to the try clause.
 *   2. Record the start and stop indexes of these instructions on the data
 *      channel. They will be later used to generate the exception table.
 *   3. Generate instructions corresponding to the statement suite specified to
 *      the finally clause.
 *   4. Jump to the instruction immediately following the try statement.
 *
 * - Catch Clause
 *   1. The virtual machine pushes the exception that was caught to the operand stack.
 *      Store this reference in a local variable.
 *   2. Generate instructions corresponding to the statement suite specified
 *      to the catch clause.
 *   3. Record the start and stop indexes of these instructions on the data
 *      channel. They will be later used to generate the exception table.
 *   4. Generate instructions corresponding to the statement suite specified to
 *      the finally clause.
 *   5. Jump to the instruction immediately following the try statement.
 *
 * - Finally Clause
 *   This section of instructions handle exceptions that were triggered either
 *   by the statement suite specified to the try clause or the catch clause.
 *   In both the cases, the exceptions are thrown again.
 *
 *   This section is basically an implicit catch clause that handles instances
 *   of the Throwable class. A record of this implicit clause is added to the
 *   exception table.
 *
 *   1. The virtual machine pushes the exception that was caught to the operand
 *      stack. Store this reference in a local variable.
 *   2. Generate instructions corresponding to the statement suite specified
 *      to the finally clause.
 *   3. Load the caught exception from the local variable.
 *   4. Throw the caught exception again.
 *
 * The primary disadvantage with the algorithm described above is the duplicate
 * copies of the finally clause. Consider a try statement with `n + 1` clauses,
 * where n is the number catch clauses and 1 represents the try clause. For such
 * a statement this algorithm generates `n + 2` copies of the finally clause.
 * The additional 1 represents the finally clause generated to handle unhandled
 * exceptions thrown either by the try clause or the catch clause.
 *
 * The following algorithm describes another approach where there are only
 * 2 copies of the finally clause.
 *
 * ### Algorithm 2
 *
 * - Try Clause
 *   1. Generate instructions corresponding to the statement suite specified
 *      to the try clause.
 *   2. Record the start and stop indexes of these instructions on the data
 *      channel. They will be later used to generate the exception table.
 *   3. Jump to the section (FC1) where the instructions of the finally clause
 *      was generated.
 *
 * - Catch Clause
 *   1. The virtual machine pushes the exception that was caught to the operand
 *      stack. Store this reference in a local variable.
 *   2. Generate instructions corresponding to the statement suite specified
 *      to the catch clause.
 *   3. Record the start and stop indexes of these instructions on the data
 *      channel. They will be later used to generate the exception table.
 *   4. Jump to the section (FC1) where the instructions of the finally clause
 *      was generated.
 *
 * - Finally Clause 1 (FC1)
 *   1. Generate instructions corresponding to the statement suite specified
 *      to the finally clause.
 *   2. Jump to the instruction immediately following the try statement.
 *
 * - Finally Clause 2 (FC2)
 *   This section of instructions handle exceptions that were triggered either
 *   by the statement suite specified to the try clause or the catch clause.
 *   In both the cases, the exceptions are thrown again.
 *
 *   This section is basically an implicit catch clause that handles instances
 *   of the Throwable class. A record of this implicit clause is added to the
 *   exception table.
 *
 *   1. The virtual machine pushes the exception that was caught to the operand
 *      stack. Store this reference in a local variable.
 *   2. Generate instructions corresponding to the statement suite specified
 *      to the finally clause.
 *   3. Load the caught exception from the local variable.
 *   4. Throw the caught exception again.
 *
 * For both the algorithms, the following records are stored in the exception
 * table.
 *  - The exceptions triggered within the try clause are handled by the catch
 *    clauses. These exceptions are not implicitly thrown again
 *  - The exceptions triggered within the try clause but are not handled by the
 *    catch clauses. These exceptions are thrown again implicity by the finally
 *    clause.
 *  - The exceptions triggered within the catch clause. These exceptions are thrown
 *    again implicity by the finally clause.
 *
 * The last two types of exception records are added to the exception table
 * only if a finally clause is present in the try statement.
 *
 * The latter algorithm is used in this implementation.
 */
void k_BinaryEntityGenerator_onExitTryStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    /* Retrieve the context of the AST node. */
    k_TryStatementContext_t* context = (k_TryStatementContext_t*)node->m_context;

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
        generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
        generator->m_builder, parentChannelIndex);

    k_ASTNode_t* tryClause = context->m_tryClause;
    k_TryClauseContext_t* tryClauseContext = (k_TryClauseContext_t*)tryClause->m_context;

    k_ASTNode_t* statementSuite = tryClauseContext->m_statementSuite;

    int32_t numberOfCatchClauses = jtk_ArrayList_getSize(context->m_catchClauses);
    /* This includes indexes for the try clause and n - 1 catch clauses,
     * where n represents the number of catch clauses. A jump instruction
     * is not generated for the last catch clause. Because it can fall through
     * to the FC1 section without an explicit jump.
     */
    int32_t* updateIndexes = jtk_Memory_allocate(int32_t, numberOfCatchClauses);
    int32_t* catchClauseIndexes = jtk_Memory_allocate(int32_t, numberOfCatchClauses * 2);

    int32_t tryClauseStartIndex = -1;
    int32_t tryClauseStopIndex = -1;

    k_ASTNode_t* catchClause = NULL;
    k_CatchClauseContext_t* catchClauseContext = NULL;

    /* Generate an index into the local variable array. This index is
     * used for all the catch clause parameters in the current try
     * statement.
     */
    int32_t parameterIndex = -1;
    if (numberOfCatchClauses > 0) {
        parameterIndex = generator->m_localVariableCount;
        generator->m_localVariableCount += 2;
    }

    int32_t index = -1;
    do {
        /* Save the index where the instruction section for the current clause
         * begins (inclusive).
         */
        int32_t startIndex = k_DataChannel_getSize(parentChannel);

        /* Generate the instructions corresponding to the statement suite specified
         * to the current clause.
         */
        k_ASTWalker_walk(astListener, statementSuite);

        /* Save the index where the instruction section for the current clause
         * ends (exclusive).
         */
        int32_t stopIndex = k_DataChannel_getSize(parentChannel);

        if (index == -1) {
            tryClauseStartIndex = startIndex;
            tryClauseStopIndex = stopIndex;
        }
        else {
            /* At this point, the load_a instruction that loads the caught
             * exception onto the operand stack has been written to the parent
             * data channel. Therefore, adjust the start index by subtracting
             * 2, the length of the load_a instruction.
             */
            catchClauseIndexes[(index * 2) + 0] = startIndex - 2;
            catchClauseIndexes[(index * 2) + 1] = stopIndex;

            k_ASTNode_t* catchFilter = catchClauseContext->m_catchFilter;
            k_CatchFilterContext_t* catchFilterContext =
                (k_CatchFilterContext_t*)catchFilter->m_context;
            int32_t filterCount = jtk_ArrayList_getSize(catchFilterContext->m_typeNames);
            int32_t filterIndex;
            for (filterIndex = 0; filterIndex < filterCount; filterIndex++) {
                k_ASTNode_t* typeName = (k_ASTNode_t*)jtk_ArrayList_getValue(
                    catchFilterContext->m_typeNames, filterIndex);

                int32_t exceptionClassNameSize;
                uint8_t* exceptionClassName = k_ASTNode_toCString(catchClauseContext->m_catchFilter,
                    &exceptionClassNameSize);
                k_Symbol_t* exceptionClass = k_SymbolTable_resolve(generator->m_symbolTable,
                    exceptionClassName);
                exceptionClassNameSize = exceptionClass->m_context.m_asClass.m_qualifiedNameSize;
                uint8_t* descriptor = jtk_CString_newEx(exceptionClass->m_context.m_asClass.m_qualifiedName,
                    exceptionClass->m_context.m_asClass.m_qualifiedNameSize);
                jtk_Arrays_replace_b(descriptor, exceptionClassNameSize, '.', '/');
                // TODO: Resolve the symbol of the class with the given name.
                uint16_t exceptionClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
                    generator->m_constantPoolBuilder, descriptor, exceptionClassNameSize);
                jtk_CString_delete(descriptor);

                /* In this exception handler site, the exceptions triggered within the
                 * try clause are handled by the catch clauses. These exceptions are
                 * not implicitly thrown again.
                 */
                k_ExceptionHandlerSite_t* type1Handler =
                    jtk_Memory_allocate(k_ExceptionHandlerSite_t, 1);
                type1Handler->m_startIndex = tryClauseStartIndex;
                type1Handler->m_stopIndex = tryClauseStopIndex;
                type1Handler->m_handlerIndex = startIndex - 2;
                type1Handler->m_exceptionClassIndex = exceptionClassIndex;

                jtk_ArrayList_add(generator->m_exceptionHandlerSites, type1Handler);
            }
        }

        index++;

        /* A jump instrution should not be generated only if there are no
         * catch clauses or if the current catch clause is the last catch
         * clause.
         */
        if (index != numberOfCatchClauses) {
            /* If the current clause is a try clause and it executes completely,
             * without triggering any exception, the catch clauses and the finally
             * clause should be skipped.
             *
             * Similarly, if the current clause is catch clause and it executes
             * completely, without triggering any exception, subsequence catch
             * clauses and the finally clause should be skipped.
             *
             * Given the statement has not been generated yet, the jump offset
             * cannot be evaluated right now. Therefore, emit the jump instruction
             * with a dummy offset.
             *
             * Emit the jump instruction to skip the other clauses.
             */
            k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);

            /* Log the emission of the jump instruction. */
            jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

            /* Save the index of the bytes where the dummy data was written. */
            updateIndexes[index] = k_DataChannel_getSize(parentChannel) - 2;

            catchClause = (k_ASTNode_t*)jtk_ArrayList_getValue(context->m_catchClauses,
                index);
            catchClauseContext =(k_CatchClauseContext_t*)catchClause->m_context;
            statementSuite = catchClauseContext->m_statementSuite;

            /* Invalidate the the previous local scope. */
            if (index >= 1) {
                k_SymbolTable_invalidateCurrentScope(generator->m_symbolTable);
            }
            /* Activate the scope of the catch clause. */
            k_Scope_t* scope = k_ASTAnnotations_get(generator->m_scopes, catchClause);
            k_SymbolTable_setCurrentScope(generator->m_symbolTable, scope);
            // Update the exception class name here.

            k_ASTNode_t* catchParameter = catchClauseContext->m_identifier;
            int32_t catchParameterSize;
            uint8_t* catchParameterText = k_ASTNode_toCString(catchParameter, &catchParameterSize);
            k_Symbol_t* catchParameterSymbol = k_SymbolTable_resolve(generator->m_symbolTable, catchParameterText);
            catchParameterSymbol->m_index = parameterIndex;

            /* The virtual machine pushes the exception that was caught to the
             * operand stack. Store this reference in a local variable.
             */
            k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, parameterIndex);

            /* Log the emission of the load_a instruction. */
            jtk_Logger_debug(logger, "Emitted store_a %d", parameterIndex);
        }
    }
    while (index < numberOfCatchClauses);

    /* Invalidate the local scope of the last catch clause. */
    if (numberOfCatchClauses > 0) {
        k_SymbolTable_invalidateCurrentScope(generator->m_symbolTable);
    }

    int32_t fc1StartIndex = k_DataChannel_getSize(parentChannel);
    int32_t i;
    for (i = 0; i < numberOfCatchClauses; i++) {
        uint16_t updateIndex = updateIndexes[i];
        parentChannel->m_bytes[updateIndex] = (0x0000FF00 & fc1StartIndex) >> 8;
        parentChannel->m_bytes[updateIndex + 1] = (0x000000FF & fc1StartIndex);
    }

    if (context->m_finallyClause != NULL) {
        /* Retrieve the AST node for the finally clause. */
        k_ASTNode_t* finallyClause = context->m_finallyClause;
        /* Retrieve the context associated with the AST node of the finally clause. */
        k_FinallyClauseContext_t* finallyClauseContext =
            (k_FinallyClauseContext_t*)finallyClause->m_context;

        /* Generate the instructions for the statement suite specified to the
         * finally clause. This constitutes the bulk of the FC1 section.
         */
        k_ASTWalker_walk(astListener, finallyClauseContext->m_statementSuite);

        /* Once the FC1 section is completed, the program should skip the FC2
         * section.
         *
         * Given the FC2 section has not been generated yet, the jump offset
         * cannot be evaluated right now. Therefore, emit the jump instruction
         * with a dummy offset.
         *
         * Emit the jump instruction to skip the FC2 section.
         */
        k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);

        /* Log the emission of the jump instruction. */
        jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

        /* Save the index of the bytes where the dummy data was written. */
        int32_t skipIndex = k_DataChannel_getSize(parentChannel) - 2;

        /* Generate the FC2 section. */

        int32_t fc2StartIndex = k_DataChannel_getSize(parentChannel);

        /* In this exception handler site, the exceptions triggered within the try
         * clause but are not handled by the catch clauses are taken care of. These
         * exceptions are thrown again implicity by the finally clause.
         */
        k_ExceptionHandlerSite_t* type2Handler =
            jtk_Memory_allocate(k_ExceptionHandlerSite_t, 1);
        type2Handler->m_startIndex = tryClauseStartIndex;
        type2Handler->m_stopIndex = tryClauseStopIndex;
        type2Handler->m_handlerIndex = fc2StartIndex;
        type2Handler->m_exceptionClassIndex = 0;

        jtk_ArrayList_add(generator->m_exceptionHandlerSites, type2Handler);

        /* The virtual machine pushes the exception that was caught to the operand
         * stack. Store this reference in a local variable.
         */
        k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, 0);

        /* Log the emission of the store_a instruction. */
        jtk_Logger_debug(logger, "Emitted store_a 0 (dummy index)");

        /* Generate the instructions for the statement suite specified to the
         * finally clause. This constitutes the bulk of the FC2 section.
         */
        k_ASTWalker_walk(astListener, finallyClauseContext->m_statementSuite);

        /* Load the caught exception from the local variable. */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);

        /* Log the emission of the store_a instruction. */
        jtk_Logger_debug(logger, "Emitted load_a 0 (dummy index)");

        /* Throw the caught exception again. */
        k_BinaryEntityBuilder_emitThrow(generator->m_builder);

        /* Log the emission of the throw instruction. */
        jtk_Logger_debug(logger, "Emitted throw");

        int32_t fc2StopIndex = k_DataChannel_getSize(parentChannel);

        /* Update the jump instruction in the FC1 section that skips the FC2
         * section.
         */
        parentChannel->m_bytes[skipIndex] = (0x0000FF00 & fc2StopIndex) >> 8;
        parentChannel->m_bytes[skipIndex + 1] = (0x000000FF & fc2StopIndex);

        /* In these exception handler sites, the exceptions triggered within
         * the catch clauses are taken care of. These exceptions are thrown
         * again implicity by the finally clause.
         */
        int32_t j;
        for (j = 0; j < numberOfCatchClauses; j++) {
            k_ExceptionHandlerSite_t* type3Handler =
                jtk_Memory_allocate(k_ExceptionHandlerSite_t, 1);
            type3Handler->m_startIndex = catchClauseIndexes[(j * 2) + 0];
            type3Handler->m_stopIndex = catchClauseIndexes[(j * 2) + 1];
            type3Handler->m_handlerIndex = fc2StartIndex;
            type3Handler->m_exceptionClassIndex = 0;

            jtk_ArrayList_add(generator->m_exceptionHandlerSites, type3Handler);
        }
    }

    jtk_Memory_deallocate(catchClauseIndexes);
    jtk_Memory_deallocate(updateIndexes);
}

// tryClause

void k_BinaryEntityGenerator_onEnterTryClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitTryClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// catchClause

void k_BinaryEntityGenerator_onEnterCatchClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitCatchClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// catchFilter

void k_BinaryEntityGenerator_onEnterCatchFilter(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitCatchFilter(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// finallyClause

void k_BinaryEntityGenerator_onEnterFinallyClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitFinallyClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// synchronizeStatement

void k_BinaryEntityGenerator_onEnterSynchronizeStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator =
        (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the context of the AST node. */
    k_SynchronizeStatementContext_t* context =
        (k_SynchronizeStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

/*
 * ALGORITHM FOR GENERATING INSTRUCTIONS CORRESPONDING TO SYNCHRONIZE
 * STATEMENTS
 *
 * 1. Generate the instructions corresponding to the expression specified
 *    to the synchronize statement.
 * 2. Make a duplicate of the resulting lock object.
 * 3. Store the duplicate reference in a local variable that only the
 *    compiler has access to.
 * 4. Invoke the Lock#acquire() function.
 * 5. Generate the instructions corresponding to the statement suite specified
 *    to the synchronize statement.
 * 6. Load the reference to the lock object from the local variable.
 * 7. Invoke the Lock#release() function.
 * 8. The synchronize statement generates an implicit finally clause that releases
 *    the lock when an exception is triggered within the statement suite specified
 *    to the synchronize statement. Therefore, generate a jump instruction to
 *    skip the finally clause section.
 * 9. An exception is pushed onto the operand stack for the finally clause to
 *    throw again. Therefore, store the reference to the exception in a local
 *    variable.
 * 10. Load the reference to the lock object from the local variable.
 * 11. Invoke the Lock#release() function.
 * 12. Load the reference to the exception that was thrown.
 * 13. Rethrow the exception that caused the finally clause to execute.
 *
 * The following records will be stored in the exception table.
 * 1. Any exception that is triggered within the statement suite is handled
 *    by the implicit finally clause.
 * 2. Any exception that is thrown by the Lock#release() function is handled
 *    by the finally clause.
 *
 * The Lock#release() may throw an exception within the bounds of the finally
 * clause. From the above records we know that the control is passed to the
 * implicit finally clause when an exception is thrown by the Lock#release()
 * function. This results in the implicit finally clause forming a loop.
 */
void k_BinaryEntityGenerator_onExitSynchronizeStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator =
        (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    /* Retrieve the context of the AST node. */
    k_SynchronizeStatementContext_t* context =
        (k_SynchronizeStatementContext_t*)node->m_context;

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
        generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
        generator->m_builder, parentChannelIndex);

    const uint8_t* lockClassName = "zen/concurrency/lock/Lock";
    int32_t lockClassNameSize = 25;

    const uint8_t* acquireDescriptor = "v:v";
    int32_t acquireDescriptorSize = 3;
    const uint8_t* acquireName = "acquire";
    int32_t acquireNameSize = 7;
    uint16_t acquireIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, lockClassName, lockClassNameSize,
        acquireDescriptor, acquireDescriptorSize, acquireName, acquireNameSize, 0);

    const uint8_t* releaseDescriptor = "v:v";
    int32_t releaseDescriptorSize = 3;
    const uint8_t* releaseName = "release";
    int32_t releaseNameSize = 7;
    uint16_t releaseIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, lockClassName, lockClassNameSize,
        releaseDescriptor, releaseDescriptorSize, releaseName, releaseNameSize, 0);

    /* Generate the instructions for the expression specified to the
     * synchronize statement.
     */
    k_ASTWalker_walk(astListener, context->m_expression);

    /* Make a duplicate of the resulting lock object. */
    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

    /* Log the emission of the duplicate instruction. */
    jtk_Logger_debug(logger, "Emitted duplicate");

    /* Store the duplicate reference in a local variable that only the
     * compiler has access to.
     */
    k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, 0);

    /* Log the emission of the store_a instruction. */
    jtk_Logger_debug(logger, "Emitted store_a 0 (dummy index)");

    /* Invoke the Lock#acquire() function. */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, acquireIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", acquireIndex);

    /* Save the index where the instruction section for the statement suite
     * specified to the synchronize statement begins (inclusive).
     */
    int32_t startIndex1 = k_DataChannel_getSize(parentChannel);

    /* Generate the instructions corresponding to the statement suite specified
     * to the synchronize statement.
     */
    k_ASTWalker_walk(astListener, context->m_statementSuite);

    /* Load the reference to the lock object from the local variable. */
    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);

    /* Log the emission of the load_a instruction. */
    jtk_Logger_debug(logger, "Emitted load_a 0 (dummy index)");

    /* Invoke the Lock#release() function. */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, releaseIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", releaseIndex);

    /* Save the index where the instruction section for the statement suite
     * specified to the synchronize statement ends (exclusive).
     */
    int32_t stopIndex1 = k_DataChannel_getSize(parentChannel);

    /* The synchronize statement generates an implicit finally clause that releases
     * the lock when an exception is triggered within the statement suite specified
     * to the synchronize statement. Therefore, generate a jump instruction to
     * skip the finally clause section.
     *
     * Given the implicity finally clause section has not been generated yet,
     * the jump offset cannot be evaluated right now. Therefore, emit the jump
     * instruction with a dummy offset.
     */
    k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);

    /* Log the emission of the jump instruction. */
    jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

    /* Save the index of the bytes where the dummy data was written. */
    int32_t skipIndex = k_DataChannel_getSize(parentChannel) - 2;

    /* Save the index where the instruction section for the implicit finally
     * clause begins (inclusive).
     */
    int32_t startIndex2 = k_DataChannel_getSize(parentChannel);

    /* The virtual machine pushes the exception that was thrown to the operand
     * stack before the control is passed to the implicit finally clause, thanks
     * to the exception table. Store this reference in a local variable.
     */
    k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, 0);

    /* Log the emission of the store_a instruction. */
    jtk_Logger_debug(logger, "Emitted store_a 0 (dummy index)");

    /* Load the reference to the lock object from the local variable. */
    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);

    /* Log the emission of the load_a instruction. */
    jtk_Logger_debug(logger, "Emitted load_a 0 (dummy index)");

    /* Invoke the Lock#release() function. */
    k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, releaseIndex);

    /* Log the emission of the invoke_virtual instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_virtual %d", releaseIndex);

    /* Save the index where the instruction section for the implicit finally
     * clause ends (exclusive).
     */
    int32_t stopIndex2 = k_DataChannel_getSize(parentChannel);

    /* Load the thrown exception from the local variable. */
    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);

    /* Log the emission of the store_a instruction. */
    jtk_Logger_debug(logger, "Emitted load_a 0 (dummy index)");

    /* Throw the exception again. */
    k_BinaryEntityBuilder_emitThrow(generator->m_builder);

    /* Log the emission of the throw instruction. */
    jtk_Logger_debug(logger, "Emitted throw");

    uint16_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
    parentChannel->m_bytes[skipIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
    parentChannel->m_bytes[skipIndex + 1] = newParentChannelSize & 0x000000FF;

    k_ExceptionHandlerSite_t* type1Handler = jtk_Memory_allocate(k_ExceptionHandlerSite_t, 1);
    type1Handler->m_startIndex = startIndex1;
    type1Handler->m_stopIndex = stopIndex1;
    type1Handler->m_handlerIndex = startIndex2;
    type1Handler->m_exceptionClassIndex = 0;

    k_ExceptionHandlerSite_t* type2Handler = jtk_Memory_allocate(k_ExceptionHandlerSite_t, 1);
    type2Handler->m_startIndex = startIndex2;
    type2Handler->m_stopIndex = stopIndex2;
    type2Handler->m_handlerIndex = startIndex2;
    type2Handler->m_exceptionClassIndex = 0;

    jtk_ArrayList_add(generator->m_exceptionHandlerSites, type1Handler);
    jtk_ArrayList_add(generator->m_exceptionHandlerSites, type2Handler);
}

// withStatement

/*
 * ALGORITHM FOR GENERATING INSTRUCTIONS CORRESPONDING TO WITH STATEMENT
 *
 * The following steps should be repeated for managing N objects.
 * Note that the closing of the resources is in the order opposite to that
 * specified to the with statement.
 *
 * The structure of the instructions generated for 3 expressions is shown below.
 * [expression-1]
 *     [expression-2]
 *         [expression-3]
 *             [statement suite]
 *         [FC1-3]
 *         [FC2-3]
 *     [FC1-2]
 *     [FC2-2]
 * [FC1-1]
 * [FC1-2]
 *
 * In the following paragraphs, I refer to anything in between expression-k
 * and FC1-k as the core section for the kth iteration.
 *
 * Repeat the following sections N times, where N is the number of expressions
 * specified to the with statement.
 *
 * [Primary]
 * 1. Generate the instructions corresponding to the kth expression specified to
 *    the with statement.
 * 2. Store the resulting objects in local variable, say Lk, which can be
 *    accessed only by the compiler.
 * 3. If k is equal to N, generate the instructions corresponding to the
 *    statement suite specified to the with statement.
 * 4. Load the object from the local variable Lk.
 * 5. Invoke the Closeable#close() function to close the resource.
 * 6. Jump to skip FC1-k and FC2-k sections.
 *
 * [Finally Clause 1 (FC1)]
 * This section handles the exceptions thrown by the core section.
 *
 * 1. The virtual machine pushes the thrown exception to the operand stack.
 *    Store this reference in a local variable, say L[k + x] (where x is the
 *    base local variable index for private FC1 local variables), which can
 *    be accessed only by the compiler.
 *    UPDATE: Unlike the previous algorithm, the new algorithm allocates only
 *    one local variable. It can be reused, given only one FC1 section is active
 *    at any given time and the exception objects are propogated by the virtual
 *    machine. This helps improve the economy of the local variables.
 * 2. Load the resource object from the local variable Lk.
 * 3. Invoke the Closeable#close() function to close the resource.
 * 4. Load the exception object that was thrown by the core section.
 * 5. Throw the exception again.
 *
 * [Finally Clause 2 (FC2)]
 * This section handles the exceptions thrown by the FC2 section.
 *
 * 1. The virtual machine pushes the thrown exception to the operand stack.
 *    Store this reference in a local variable, say L[k + y] (where y is the base
 *    local variable index for private FC2 local variables), which can be accessed
 *    only by the compiler.
 *    UPDATE: Unlike the previous algorithm, the new algorithm allocates only
 *    one local variable. It can be reused, given only one FC2 section is active
 *    at any given time and the exception objects are propogated by the virtual
 *    machine. This helps improve the economy of the local variables.
 * 2. Load the exception object that was thrown by the core section. In other
 *    words, load the object referenced by L[k + x].
 * 3. Load the exception object that was thrown by the Closeable#close() function
 *    in the FC1 section. In other words, load the object referenced by L[k + y].
 * 4. Invoke the Throwable#suppress() function to add the exception thrown by
 *    the FC1 section to the exception thrown by the core section.
 *    In other words, `L[k + x].suppress(L[k + y])`.
 * 5. Load the exception object that was thrown by the core section. In other
 *    words, load the object referenced by L[k + x].
 * 6. Throw the exception again.
 *
 * The following cases of exceptions can be summarized along with the behavior
 * of the with statement.
 * 1. The core section does not trigger any exception.
 *    The Closeable#close() function does not trigger any exception.
 *
 * 2. The core section does not trigger any exception.
 *    The Closeable#close() function triggers any exception.
 *
 * 3. The core section triggers an exception.
 *    The Closeable#close() function does not trigger any exception.
 *
 *    In such cases, the control is transferred to the FC1-k section, which
 *    closes the resource. The exception thrown by the core is
 *    thrown again.
 *
 * 4. The core triggers an exception.
 *    The Closeable#close() function triggers an exception.
 *
 *    The core section trigger an exception. This results in the control being
 *    transferred to the FC1-k section, which makes an attempt to close the
 *    resource. This triggers another exception. Thus, the control is transferred
 *    to the FC2-k section. In the FC2-k section, the exception thrown by the
 *    Closeable#close() function is suppressed by the exception thrown by the
 *    core section using the Throwable#suppress() function. The oldest exception,
 *    that is, the exception thrown by the core section, is thrown again.
 *
 * The following records are added to the exception table.
 * 1. The exceptions thrown by the core section are handled by the FC1-k section.
 * 2. The exceptions thrown by the FC1-k section are handled by the FC2-k section.
 *
 * Notice that for N expressions, N * 2 records are added to the exception table.
 * Similarly, 3N + 1 sections of instructions are generated, that is,
 * N expressions, N FC1 sections, N FC2 sections, and 1 statement suite.
 */
void k_BinaryEntityGenerator_onEnterWithStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;

    /* Retrieve the context of the AST node. */
    k_WithStatementContext_t* context = (k_WithStatementContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

#define ZEN_BINARY_ENTITY_GENERATOR_MAX_LOCAL_VARIABLES 255

int32_t k_BinaryEntityGenerator_allocateLocalVariables(k_BinaryEntityGenerator_t*
    generator, int32_t count) {
    jtk_Assert_assertObject(generator, "The specified binary entity generator is null.");
    jtk_Assert_assertTrue(count > 0, "The specified local variable count is invalid.");

    int32_t result = -1;
    if (generator->m_localVariableCount + count <= ZEN_BINARY_ENTITY_GENERATOR_MAX_LOCAL_VARIABLES) {
        result = generator->m_localVariableCount;
        generator->m_localVariableCount += count;
    }
    else {
        printf("[internal error] Too many local variables declared within the same function. Looks like the phases prior to the code generator have malfunctioned.\n");
    }
    return result;
}

/* As of version 1.0.0, the wide instruction is only part of the virtual machine
 * design. This goes to say that certain features of Zen are limited. One such
 * example is that local variables are limited to a maximum of 255 per function.
 * In other words, you cannot declare more than 255 local variables in a function.
 * Remember this includes all the implicit variables generated by the code
 * generated, such as "this" reference, implicit variables used by compound statements
 * like for/with statements, and so on.
 *
 * In particular, one should be cautious when using the with statement. Of all
 * the compound statements, it is the most complex. The old algorithm generated at
 * least 3N local variables, where N is the total number of resources specified
 * to the same. The new algorithm for generating instructions corresponding
 * to the with statement allocates a minimum of N + 2 local variables for internal
 * operations.
 *
 * [EXAMPLE]
 *
 * Consider the following program written in Zen.
 *
 * function main()
 *     with 1, 2
 *         ;
 *
 * Notice that the expressions specified to the with statement evaluate to
 * integers. In reality, one cannot specify integers to the same. However, for
 * the sake of demonstration I have used integers for expressions and an empty
 * statement as the statement suite. This allows us to focus on the instructions
 * that provide the functionality of the with statement.
 *
 * ; Expression 1
 * #1 push_i1
 * #2 store_a 0
 *
 * ; Expression 2
 * #4 push_i2
 * #5 store_a 1
 *
 * ; Statement suite
 * #7 nop
 *
 * ; The default instructions that make an attempt to close the resource obtained
 * ; from expression 1. If an exception is thrown here, it propogates without causing
 * ; transfer of control to FC1-1 or FC2-1 sections. In other words, any exception
 * ; thrown here will transfer control to FC1-2 section, without causing the FC1-1
 * ; and FC2-1 sections to execute.
 * #8 load_a 0
 * #10 invoke_virtual 6
 * #13 jump 38
 *
 * ; The FC1-1 section, which makes an attempt to close the resource obtained
 * ; from expression 1. It throws the exception that originated from the statement
 * ; suite. The control is transferred to FC1-2 section.
 * #16 store_a 2
 * #18 load_a 0
 * #20 invoke_virtual 6
 * #23 load_a 2
 * #25 throw
 *
 * ; The following instructions constitute the FC2-1 section. It supresses the
 * ; exception thrown by Closeable#close() function with the exception thrown by
 * ; the statement suite. It throws the exception that originated from the statement
 * ; suite. The control is transferred to FC1-2 section.
 * #26 store_a 3
 * #28 load_a 2
 * #30 load_a 3
 * #32 invoke_virtual 11
 * #35 load_a 2
 * #37 throw
 *
 * ; The default instructions that make an attempt to close the resource obtained
 * ; from expression 2. If an exception is thrown here, it propogates without causing
 * ; transfer of control to FC1-2 or FC2-2 sections. In other words, any exception
 * ; thrown here will transfer control to instructions beyond the with statement,
 * ; without causing the FC1-2 and FC2-2 sections to execute.
 * #38 load_a 1
 * #40 invoke_virtual 6
 * #43 jump 68
 *
 * ; The FC1-2 section, which makes an attempt to close the resource obtained
 * ; from expression 2. It throws the exception that originated from the FC2-1
 * ; section or expression 2. The control is transferred to instructions beyond
 * ; the with statement.
 * #46 store_a 2
 * #48 load_a 1
 * #50 invoke_virtual 6
 * #53 load_a 2
 * #55 throw
 *
 * ; The following instructions constitute the FC2-2 section. It supresses the
 * ; exception thrown by Closeable#close() function in FC1-2 section with the
 * ; exception thrown by the statement suite. It throws the exception that originated
 * ; from the FC2-1 section or expression 2. The control is transferred to
 * ; instructions beyond the with statement.
 * #56 store_a 3
 * #58 load_a 2
 * #60 load_a 3
 * #62 invoke_virtual 11
 * #65 load_a 2
 * #67 throw
 */
void k_BinaryEntityGenerator_onExitWithStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    /* Retrieve the context of the AST node. */
    k_WithStatementContext_t* context = (k_WithStatementContext_t*)node->m_context;

    const uint8_t* closeableClassName = "zen/core/Closeable";
    int32_t closeableClassNameSize = 18;
    const uint8_t* closeDescriptor = "v:v";
    int32_t closeDescriptorSize = 3;
    const uint8_t* closeName = "close";
    int32_t closeNameSize = 5;
    uint16_t closeIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, closeableClassName, closeableClassNameSize,
        closeDescriptor, closeDescriptorSize, closeName, closeNameSize, 0);

    const uint8_t* throwableClassName = "zen/core/Throwable";
    int32_t throwableClassNameSize = 18;
    const uint8_t* suppressDescriptor = "v:(zen/core/Throwable)";
    int32_t suppressDescriptorSize = 22;
    const uint8_t* suppressName = "suppress";
    int32_t suppressNameSize = 8;
    uint16_t suppressIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, throwableClassName, throwableClassNameSize,
        suppressDescriptor, suppressDescriptorSize, suppressName, suppressNameSize, 0);

    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
         generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
         generator->m_builder, parentChannelIndex);

    /* Retrieve the the AST node for with parameters rule. */
    k_ASTNode_t* withParameters = context->m_withParameters;
    /* Retrieve the context of the with parameters AST node. */
    k_WithParametersContext_t* withParametersContext =
        (k_WithParametersContext_t*)withParameters->m_context;

    int32_t withParameterCount = jtk_ArrayList_getSize(withParametersContext->m_withParameters);

    // int32_t* skipIndexes = jtk_Memory_allocate(int32_t, withParameterCount + 1);

    /* Allocate local variables for storing the resource objects. These local
     * variables can only be accessed by the compiler.
     */
    int32_t resourceBaseIndex = k_BinaryEntityGenerator_allocateLocalVariables(
        generator, withParameterCount);
    /* Allocate local variables for storing the exception objects thrown by
     * the core sections. These local variables can only be accessed by the
     * compiler.
     */
    // int32_t fc1BaseIndex = k_BinaryEntityGenerator_allocateLocalVariables(
    //    generator, withParameterCount);
    /* Allocate the local variables for storing the exception objects thrown
     * by the FC1-k sections. These local variables can only be accessed by
     * the compiler.
     */
    // int32_t fc2BaseIndex = k_BinaryEntityGenerator_allocateLocalVariables(
    //    generator, withParameterCount);

    /* Allocate a local variable for storing the exception objects thrown by
     * the core sections. Unlike the previous algorithm, the new algorithm
     * allocates only one local variable. It can be reused given only one FC1
     * section is active at any given time and the exception objects are
     * propogated by the virtual machine. This local variable can only be
     * accessed by the compiler.
     */
    int32_t fc1ExceptionIndex = k_BinaryEntityGenerator_allocateLocalVariables(
        generator, 1);
    /* Allocate a local variable for storing the exception objects thrown by
     * the FC1 sections. Unlike the previous algorithm, the new algorithm
     * allocates only one local variable. It can be reused given only one FC2
     * section is active at any given time and the exception objects are
     * propogated by the virtual machine. This local variable can only be
     * accessed by the compiler.
     */
    int32_t fc2ExceptionIndex = k_BinaryEntityGenerator_allocateLocalVariables(
        generator, 1);

    int32_t withParameterIndex;
    for (withParameterIndex = 0; withParameterIndex < withParameterCount;
        withParameterIndex++) {
        /* Retreive the current with parameter AST node. */
        k_ASTNode_t* withParameter = (k_ASTNode_t*)jtk_ArrayList_getValue(
            withParametersContext->m_withParameters, withParameterIndex);

        /* Retrieve the context of the current with parameter. */
        k_WithParameterContext_t* withParameterContext =
            (k_WithParameterContext_t*)withParameter->m_context;

        // TODO: Allocate a local variable if an identifier was specified.

        /* Generate the instructions corresponding to the expression specified to
         * the with statement.
         */
        k_ASTWalker_walk(astListener, withParameterContext->m_expression);

        /* Allocate a local variable only the compiler has access to. */
        uint8_t resourceIndex = resourceBaseIndex + withParameterIndex;

        /* Store the resulting object in the local variable allocated in the previous
         * step.
         *
         * TODO: Allocate a local variable!
         */
        k_BinaryEntityBuilder_emitStoreReference(generator->m_builder, resourceIndex);
        /* Log the emission of the store_a instruction. */
        jtk_Logger_debug(logger, "Emitted store_a %d", resourceIndex);

        int32_t coreStartIndex = k_DataChannel_getSize(parentChannel);
        k_ExceptionHandlerSite_t* type1Handler = jtk_Memory_allocate(
            k_ExceptionHandlerSite_t, 1);
        type1Handler->m_startIndex = coreStartIndex;
        type1Handler->m_exceptionClassIndex = 0;
        jtk_ArrayList_add(generator->m_exceptionHandlerSites, type1Handler);
    }

    /* Save the index where the instruction for the statement suite clause
     * begins (inclusive).
     */
    int32_t startIndex = k_DataChannel_getSize(parentChannel);

    /* Generate the instructions corresponding to the statement suite specified to
     * the with statement.
     */
    k_ASTWalker_walk(astListener, context->m_statementSuite);

    for (withParameterIndex = withParameterIndex - 1; withParameterIndex >= 0;
        withParameterIndex--) {
        int32_t normalizedIndex = withParameterCount - withParameterIndex - 1;
        uint8_t resourceIndex = resourceBaseIndex + normalizedIndex;
        // uint8_t fc1ExceptionIndex = fc1BaseIndex + normalizedIndex;
        // uint8_t fc2ExceptionIndex = fc2BaseIndex + normalizedIndex;

        int32_t coreStopIndex = k_DataChannel_getSize(parentChannel);
        k_ExceptionHandlerSite_t* type1Handler = (k_ExceptionHandlerSite_t*)
            jtk_ArrayList_getValue(generator->m_exceptionHandlerSites, withParameterIndex);
        type1Handler->m_stopIndex = coreStopIndex;

        /* Load the resulting object from the local variable we allocated in the
         * previous loop.
         */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, resourceIndex);
        /* Log the emission of the load_a instruction. */
        jtk_Logger_debug(logger, "Emitted load_a %d", resourceIndex);

        /* Invoke the Closeable#close() function to close the resource. */
        k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, closeIndex);
        /* Log the emission of the invoke_virtual instruction. */
        jtk_Logger_debug(logger, "Emitted invoke_virtual %d", closeIndex);

        /* Jump to skip FC1-k and FC2-k sections. */
        k_BinaryEntityBuilder_emitJump(generator->m_builder, 0);
        /* Log the emission of the jump instruction. */
        jtk_Logger_debug(logger, "Emitted jump 0 (dummy index)");

        /* Save the index of the bytes where the dummy data was written. */
        int32_t skipIndex = k_DataChannel_getSize(parentChannel) - 2;

        /* -- Finally Clause 1 -- */

        int32_t fc1StartIndex = k_DataChannel_getSize(parentChannel);
        type1Handler->m_handlerIndex = fc1StartIndex;

        /* The virtual machine pushes the thrown exception to the operand stack.
         * Store this reference in a local variable, say L[k + x] (where x is the
         * base local variable index for private FC1 local variables), which can
         * be accessed only by the compiler.
         */
        k_BinaryEntityBuilder_emitStoreReference(generator->m_builder,
            fc1ExceptionIndex);
        /* Log the emission of the store_a instruction. */
        jtk_Logger_debug(logger, "store_a %d", fc1ExceptionIndex);

        /* Load the resource object from the local variable Lk. */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder,
            resourceIndex);
        /* Log the emission of the load_a instruction. */
        jtk_Logger_debug(logger, "load_a %d", resourceIndex);

        /* Invoke the Closeable#close() function to close the resource. */
        k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, closeIndex);
        /* Log the emission of the invoke_virtual instruction. */
        jtk_Logger_debug(logger, "Emitted invoke_virtual %d", closeIndex);

        int32_t fc1StopIndex = k_DataChannel_getSize(parentChannel);

        /* Load the exception object that was thrown by the core section. */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder,
            fc1ExceptionIndex);
        /* Log the emission of the load_a instruction. */
        jtk_Logger_debug(logger, "Emitted load_a %d", fc1ExceptionIndex);

        /* Throw the exception again. */
        k_BinaryEntityBuilder_emitThrow(generator->m_builder);
        /* Log the emission of the throw instruction. */
        jtk_Logger_debug(logger, "Emitted throw");

        /* -- Finally Clause 2 -- */

        int32_t fc2StartIndex = k_DataChannel_getSize(parentChannel);

        /* The virtual machine pushes the thrown exception to the operand stack.
         * Store this reference in a local variable, say L[k + y] (where y is the base
         * local variable index for private FC2 local variables), which can be
         * accessed only by the compiler.
         */
        k_BinaryEntityBuilder_emitStoreReference(generator->m_builder,
            fc2ExceptionIndex);
        /* Log the emission of the store_a instruction. */
        jtk_Logger_debug(logger, "store_a %d", fc2ExceptionIndex);

        /* Load the exception object that was thrown by the core section. In other
         * words, load the object referenced by L[k + x].
         */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder,
            fc1ExceptionIndex);
        /* Log the emission of the load_a instruction. */
        jtk_Logger_debug(logger, "Emitted load_a %d", fc1ExceptionIndex);

        /* Load the exception object that was thrown by the Closeable#close()
         * function in the FC1 section. In other words, load the object referenced
         * by L[k + y].
         */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder,
            fc2ExceptionIndex);
        /* Log the emission of the load_a instruction. */
        jtk_Logger_debug(logger, "Emitted load_a %d", fc2ExceptionIndex);

        /* Invoke the Throwable#suppress() function to add the exception thrown
         * by the FC1 section to the exception thrown by the core section.
         * In other words, `L[k + x].suppress(L[k + y])`.
         */
        k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, suppressIndex);
        /* Log the emission of the invoke_virtual instruction. */
        jtk_Logger_debug(logger, "Emitted invoke_virtual %d", suppressIndex);

        /* Load the exception object that was thrown by the core section. In
         * other words, load the object referenced by L[k + x].
         */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder,
            fc1ExceptionIndex);
        /* Log the emission of the load_a instruction. */
        jtk_Logger_debug(logger, "Emitted load_a %d", fc1ExceptionIndex);

        /* Throw the exception again. */
        k_BinaryEntityBuilder_emitThrow(generator->m_builder);
        /* Log the emission of the throw instruction. */
        jtk_Logger_debug(logger, "Emitted throw");

        int32_t newParentChannelSize = k_DataChannel_getSize(parentChannel);
        parentChannel->m_bytes[skipIndex] = (newParentChannelSize & 0x0000FF00) >> 8;
        parentChannel->m_bytes[skipIndex + 1] = newParentChannelSize & 0x000000FF;

        k_ExceptionHandlerSite_t* type2Handler = jtk_Memory_allocate(k_ExceptionHandlerSite_t, 1);
        type2Handler->m_startIndex = fc1StartIndex;
        type2Handler->m_stopIndex = fc1StopIndex;
        type2Handler->m_handlerIndex = fc2StartIndex;
        type2Handler->m_exceptionClassIndex = 0;
        jtk_ArrayList_add(generator->m_exceptionHandlerSites, type2Handler);

        /* I just had an epiphany! All the "Emitted *" log messages could have been
         * placed in the binary entity builder. This way I would have to repeat the
         * code less often! I am almost done with the code generate and I do not
         * have the energy to refactor everything. :(
         */
    }
}

// classDeclaration

void k_BinaryEntityGenerator_onEnterClassDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;

    /* Retrieve the current scope from the symbol table. At this point, it is
     * the enclosing scope.
     */
    k_Scope_t* parentScope = k_SymbolTable_getCurrentScope(generator->m_symbolTable);

    /* Retrieve the context of the AST node. */
    k_ClassDeclarationContext_t* context =
        (k_ClassDeclarationContext_t*)node->m_context;

    /* Retrieve the scope associated with the class being declared. */
    k_Scope_t* scope = k_ASTAnnotations_get(generator->m_scopes, node);

    /* Update the current scope in the symbol table. */
    k_SymbolTable_setCurrentScope(generator->m_symbolTable, scope);

    generator->m_mainComponent = ZEN_AST_NODE_TYPE_CLASS_DECLARATION;

    k_ASTNode_t* identifier = context->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

    uint16_t* superclassIndexes = NULL;
    uint16_t superclassCount = 0;
    if (context->m_classExtendsClause != NULL) {
        /* Retrieve the extends clause context to extract information about the
         * superclasses.
         */
        k_ClassExtendsClauseContext_t* extendsClauseContext =
            (k_ClassExtendsClauseContext_t*)context->m_classExtendsClause->m_context;

        /* Calculate the total number of superclasses. */
        superclassCount = jtk_ArrayList_getSize(extendsClauseContext->m_typeNames);
        /* Allocate an array to store the constant pool indexes to the superclass
         * references.
         */
        superclassIndexes = jtk_Memory_allocate(uint16_t, superclassCount);

        /* The entity generator does not ensure the validity of the identifiers.
         * It is the responsibility of the resolution phase.
         */
        int32_t index;
        for (index = 0; index < superclassCount; index++) {
            k_ASTNode_t* typeNameNode = (k_ASTNode_t*)jtk_ArrayList_getValue(extendsClauseContext->m_typeNames, index);
            k_TypeNameContext_t* typeNameContext = (k_TypeNameContext_t*)typeNameNode->m_context;

            // TODO: Prepare a qualified name from the type name context.
            uint8_t* qualifiedName = NULL;
            int32_t qualifiedNameSize;

            /* Retrieve the symbol for the current superclass. Do not begin the resolution
             * from the current scope, which is this class. In the future, if Zen allows
             * inner classes, this would allow inner classes to be inherited by their enclosing
             * classes!
             */
            k_Symbol_t* symbol = k_Scope_resolveQualifiedSymbol(
                parentScope, qualifiedName, &qualifiedNameSize);
            if (k_Symbol_isClass(symbol)) {
                k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;

                uint16_t superclassIndex = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
                    generator->m_constantPoolBuilder, classSymbol->m_qualifiedName,
                    classSymbol->m_qualifiedNameSize);
                superclassIndexes[index] = superclassIndex;
            }
            else {
                printf("[error] Looks like the semantic anlysis failed.\n");
            }
        }
    }
    else {
        /* The extends clause has not been explicitly written. Therefore,
         * the compiler generates the default extends clause which inherits
         * the zen.core.Object class.
         */

        superclassCount = 1;
        superclassIndexes = jtk_Memory_allocate(uint16_t, 1);
        superclassIndexes[0] = k_ConstantPoolBuilder_getUtf8EntryIndexEx(
            generator->m_constantPoolBuilder, "zen/core/Object", 15);
    }

    k_BinaryEntityGenerator_initializeClassName(generator);
    k_BinaryEntityGenerator_prepareClass(generator, identifierToken->m_text,
        identifierToken->m_length, superclassIndexes, superclassCount);
}

void k_BinaryEntityGenerator_onExitClassDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the generator associated with the AST listener. */
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;

    /* Retrieve the context of the AST node. */
    k_ClassDeclarationContext_t* context =
        (k_ClassDeclarationContext_t*)node->m_context;

    /* Invalidate the current scope in the symbol table. */
    k_SymbolTable_invalidateCurrentScope(generator->m_symbolTable);

    /* Write the generated binary entity to the output stream. */
    k_BinaryEntityGenerator_writeEntity(generator);

    /* Reset all the fields used to generate the binary entity. */
    int32_t parentChannelIndex = k_BinaryEntityBuilder_getActiveChannelIndex(
         generator->m_builder);
    k_DataChannel_t* parentChannel = k_BinaryEntityBuilder_getChannel(
         generator->m_builder, parentChannelIndex);
    // Reset the parent data channel
    parentChannel->m_index = 0;

    // TODO: Reset m_entityFile
    k_ConstantPoolBuilder_reset(generator->m_constantPoolBuilder);
    k_BinaryEntityGenerator_clearFields(generator);
    k_BinaryEntityGenerator_clearFunctions(generator);
    k_BinaryEntityGenerator_clearExceptionHandlerSites(generator);

    if (generator->m_breakRecords != NULL) {
        jtk_Memory_deallocate(generator->m_breakRecords);
        generator->m_breakRecords = NULL;
    }

    generator->m_maxStackSize = 0;
    generator->m_localVariableCount = 0;
    generator->m_breakRecordsCount = 0;
    generator->m_nextLoopLabel = 0;
    generator->m_currentLoopLabel = -1;
}

// classExtends

void k_BinaryEntityGenerator_onEnterClassExtendsClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitClassExtendsClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// classSuite

void k_BinaryEntityGenerator_onEnterClassSuite(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitClassSuite(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// classMember

void k_BinaryEntityGenerator_onEnterClassMember(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitClassMember(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// void k_BinaryEntityGenerator_onEnterConstructorDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
// }

// void k_BinaryEntityGenerator_onExitConstructorDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
// }


// enumerationDeclaration

void k_BinaryEntityGenerator_onEnterEnumerationDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitEnumerationDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// enumerationBaseClass

void k_BinaryEntityGenerator_onEnterEnumrationBaseClass(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitEnumerationBaseClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// enumerationSuite

void k_BinaryEntityGenerator_onEnterEnumerationSuite(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitEnumerationSuite(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// enumerate

void k_BinaryEntityGenerator_onEnterEnumerate(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitEnumerate(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// expressions

void k_BinaryEntityGenerator_onEnterExpressions(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitExpressions(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// expression

void k_BinaryEntityGenerator_onEnterExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// assignmentExpression

// How to differentiate between function calls

/* The left hand side of an assignment may take the following forms:
 * - local variable
 * - local class member variable
 * - local class member using 'this' reference
 * - super class member
 * - super class member using 'super' reference
 * - instance member variable (where, the instance is an expression)
 * - static member variable
 *
 *    ...
 *    invoke_virtual #function_descriptor_index
 *    store_a #x
 *
 * NOTE: The binary entity generator assumes that the left values were verified
 * to be valid in the previous phases of the compiler.
 */
void k_BinaryEntityGenerator_onEnterAssignmentExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_AssignmentExpressionContext_t* context = (k_AssignmentExpressionContext_t*)node->m_context;

    /* I have no idea, but the following statement just fixed a bug magically.
     * However, it has something to do with which type of code is generated,
     * the LHS or RHS, for postfix expressions.
     */
    lhs = false;
    k_ASTNode_t* assignmentOperator = context->m_assignmentOperator;
    if (assignmentOperator != NULL) {
        lhs = false;
        k_ASTWalker_walk(astListener, context->m_assignmentExpression);
        lhs = true;
        k_ASTWalker_walk(astListener, context->m_conditionalExpression);

        k_ASTListener_skipChildren(astListener);
    }
}

void k_BinaryEntityGenerator_onExitAssignmentExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {/*
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_AssignmentExpressionContext_t* context = (k_AssignmentExpressionContext_t*)node->m_context;

    if (context->m_assignmentOperator != NULL) {
        k_ASTNode_t* assignmentOperator = context->m_assignmentOperator;
        k_Token_t* operatorToken = (k_Token_t*)assignmentOperator->m_context;
        switch (operatorToken->m_type) {
            /* The equal operator only stores a reference. Therefore, it requires no
             * special implementation.
             *
            /*
            case ZEN_TOKEN_EQUAL: {
                break;
            }
            *

            case ZEN_TOKEN_ASTERISK_EQUAL: {
                break;
            }

            case ZEN_TOKEN_FORWARD_SLASH_EQUAL: {
                break;
            }

            case ZEN_TOKEN_MODULUS_EQUAL: {
                break;
            }

            case ZEN_TOKEN_PLUS_EQUAL: {
                break;
            }

            case ZEN_TOKEN_DASH_EQUAL: {
                break;
            }

            case ZEN_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL: {
                break;
            }

            case ZEN_TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL: {
                break;
            }

            case ZEN_TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL: {
                break;
            }

            case ZEN_TOKEN_AMPERSAND_EQUAL: {
                break;
            }

            case ZEN_TOKEN_CARET_EQUAL: {
                break;
            }

            case ZEN_TOKEN_VERTICAL_BAR_EQUAL: {
                break;
            }

            default: {
                // [internal error]
            }
        }
        // Depending on the first push reference
        k_BinaryEntityGenerator_emitStoreReference(generator, 10);
    }*/
}

// conditionalExpression

void k_BinaryEntityGenerator_onEnterConditionalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ConditionalExpressionContext_t* context = (k_ConditionalExpressionContext_t*)node->m_context;
}

void k_BinaryEntityGenerator_onExitConditionalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ConditionalExpressionContext_t* context = (k_ConditionalExpressionContext_t*)node->m_context;

    /* TODO */
}

// logicalOrExpression

void k_BinaryEntityGenerator_onEnterLogicalOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_LogicalOrExpressionContext_t* context = (k_LogicalOrExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitLogicalOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_LogicalOrExpressionContext_t* context = (k_LogicalOrExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_logicalAndExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* logicalAndExpressions = jtk_ArrayList_getValue(context->m_logicalAndExpressions, i);

        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * logical OR expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for logical OR
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, logicalAndExpressions);

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * logical OR operation is delegated to functions annotated with the
         * Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, "||", 2);

        // TODO: The instructions generated here are wrong.
    }
}

// logicalAndExpression

void k_BinaryEntityGenerator_onEnterLogicalAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_LogicalAndExpressionContext_t* context = (k_LogicalAndExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitLogicalAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_LogicalAndExpressionContext_t* context = (k_LogicalAndExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_inclusiveOrExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* inclusiveOrExpression = jtk_ArrayList_getValue(context->m_inclusiveOrExpressions, i);

        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * logical AND expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for logical AND
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, inclusiveOrExpression);

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * logical AND operation is delegated to functions annotated with the
         * Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, "&&", 2);

        // TODO: The instructions generated here are wrong.
    }
}

// inclusiveOrExpression

void k_BinaryEntityGenerator_onEnterInclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_InclusiveOrExpressionContext_t* context = (k_InclusiveOrExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitInclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_InclusiveOrExpressionContext_t* context = (k_InclusiveOrExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_exclusiveOrExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* exclusiveOrExpression = jtk_ArrayList_getValue(context->m_exclusiveOrExpressions, i);

        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * inclusive or expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for inclusive or
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, exclusiveOrExpression);

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * bitwise and operation is delegated to functions annotated with the
         * Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, "|", 1);
    }
}

// exclusiveOrExpression

void k_BinaryEntityGenerator_onEnterExclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ExclusiveOrExpressionContext_t* context = (k_ExclusiveOrExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitExclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ExclusiveOrExpressionContext_t* context = (k_ExclusiveOrExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_andExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* andExpression = jtk_ArrayList_getValue(context->m_andExpressions, i);

        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * and expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for and
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, andExpression);

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * bitwise and operation is delegated to functions annotated with the
         * Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, "^", 1);
    }
}

// andExpression

void k_BinaryEntityGenerator_onEnterAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_AndExpressionContext_t* context = (k_AndExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_AndExpressionContext_t* context = (k_AndExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_equalityExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* equalityExpression = jtk_ArrayList_getValue(context->m_equalityExpressions, i);

        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * shift expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for shift
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, equalityExpression);

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * bitwise and operation is delegated to functions annotated with the
         * Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, "&", 1);
    }
}

// equalityExpression

void k_BinaryEntityGenerator_onEnterEqualityExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_EqualityExpressionContext_t* context = (k_EqualityExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitEqualityExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_EqualityExpressionContext_t* context = (k_EqualityExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_relationalExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_relationalExpressions, i);

        /* Retrieve the equality operator. */
        k_ASTNode_t* equalityOperator = pair->m_left;
        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * equality expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for equality
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, (k_ASTNode_t*)pair->m_right);

        /* Retrieve the corresponding equality operator token from the AST
         * node.
         */
        k_Token_t* equalityOperatorToken = (k_Token_t*)equalityOperator->m_context;
        /* Retrieve the type of the equality operator. */
        k_TokenType_t equalityOperatorTokenType = k_Token_getType(equalityOperatorToken);

        /* The values of symbol and symbolSize are the only arbitrary variables
         * when invoking the k_BinaryEntityGenerator_invokeEvaluate() function.
         * Therefore, instead of rewriting the invocation expression multiple
         * times, I have factored it out.
         */
        uint8_t* symbol = NULL;
        int32_t symbolSize = 2;

        switch (equalityOperatorTokenType) {
            case ZEN_TOKEN_EQUAL_2: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '==' symbol.
                 */
                symbol = "==";

                break;
            }

            case ZEN_TOKEN_EXCLAMATION_MARK_EQUAL: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '!=' symbol.
                 */
                symbol = "!=";

                break;
            }

            default: {
                /* The generator should not reach this code! */
                printf("[error] Control should not reach here.\n");
            }
        }

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * the addition and subtraction operations are delegated to
         * functions annotated with the Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, symbol, symbolSize);
    }
}

// relationalExpression

void k_BinaryEntityGenerator_onEnterRelationalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_RelationalExpressionContext_t* context = (k_RelationalExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitRelationalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_RelationalExpressionContext_t* context = (k_RelationalExpressionContext_t*)node->m_context;

    /* NOTE: Relational operators have no associativity. In order to implement
     * this behavior, the parser first recognizes relational operators as if
     * they have left/right associativity. After which, it manually checks
     * for the number of operands on the right hand side of the very first
     * subexpression.
     *
     * At this point, we assume size of the shift expressions list is 1 or 0.
     */
    int32_t size = jtk_ArrayList_getSize(context->m_shiftExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_shiftExpressions, i);

        /* Retrieve the relational operator. */
        k_ASTNode_t* relationalOperator = pair->m_left;
        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * relational expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for relational
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, (k_ASTNode_t*)pair->m_right);

        /* Retrieve the corresponding relational operator token from the AST
         * node.
         */
        k_Token_t* relationalOperatorToken = (k_Token_t*)relationalOperator->m_context;
        /* Retrieve the type of the relational operator. */
        k_TokenType_t relationalOperatorTokenType = k_Token_getType(relationalOperatorToken);

        if (relationalOperatorTokenType == ZEN_TOKEN_KEYWORD_IS) {

        }
        else {
            /* The values of symbol and symbolSize are the only arbitrary variables
             * when invoking the k_BinaryEntityGenerator_invokeEvaluate() function.
             * Therefore, instead of rewriting the invocation expression multiple
             * times, I have factored it out.
             */
            uint8_t* symbol = NULL;
            int32_t symbolSize = 1;

            switch (relationalOperatorTokenType) {
                case ZEN_TOKEN_LEFT_ANGLE_BRACKET: {
                    /* The kernel should find a function annotated with the Operator
                     * annotation that handles the '+' symbol.
                     */
                    symbol = "<";

                    break;
                }

                case ZEN_TOKEN_RIGHT_ANGLE_BRACKET: {
                    /* The kernel should find a function annotated with the Operator
                     * annotation that handles the '>' symbol.
                     */
                    symbol = ">";

                    break;
                }

                case ZEN_TOKEN_LEFT_ANGLE_BRACKET_EQUAL: {
                    /* The kernel should find a function annotated with the Operator
                     * annotation that handles the '<=' symbol.
                     */
                    symbol = "<=";
                    /* All the other symbols are of length 1, except for the '<='
                     * and '>=' symbols. Therefore, update the symbol size.
                     */
                    symbolSize = 2;

                    break;
                }

                case ZEN_TOKEN_RIGHT_ANGLE_BRACKET_EQUAL: {
                    /* The kernel should find a function annotated with the Operator
                     * annotation that handles the '>=' symbol.
                     */
                    symbol = ">=";
                    /* All the other symbols are of length 1, except for the '<='
                     * and '>=' symbols. Therefore, update the symbol size.
                     */
                    symbolSize = 2;

                    break;
                }

                default: {
                    /* The generator should not reach this code! */
                    printf("[error] Control should not reach here.\n");
                }
            }

            /* Generate the instructions corresponding to invoking the
             * ZenKernel.evaluate() function. Since, Zen is dynamically typed
             * the compiler cannot determine the type of the operands. Therefore,
             * the left shift, right shift, and extended right shift operations are
             * delegated to functions annotated with the Operator annotation.
             */
            k_BinaryEntityGenerator_invokeEvaluate(generator, symbol, symbolSize);
        }
    }
}

// shiftExpression

void k_BinaryEntityGenerator_onEnterShiftExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ShiftExpressionContext_t* context = (k_ShiftExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitShiftExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_ShiftExpressionContext_t* context = (k_ShiftExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_additiveExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_additiveExpressions, i);

        /* Retrieve the shift operator. */
        k_ASTNode_t* shiftOperator = pair->m_left;
        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * shift expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for shift
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, (k_ASTNode_t*)pair->m_right);

        /* Retrieve the corresponding shift operator token from the AST
         * node.
         */
        k_Token_t* shiftOperatorToken = (k_Token_t*)shiftOperator->m_context;
        /* Retrieve the type of the shift operator. */
        k_TokenType_t shiftOperatorTokenType = k_Token_getType(shiftOperatorToken);

        /* The values of symbol and symbolSize are the only arbitrary variables
         * when invoking the k_BinaryEntityGenerator_invokeEvaluate() function.
         * Therefore, instead of rewriting the invocation expression multiple
         * times, I have factored it out.
         */
        uint8_t* symbol = NULL;
        int32_t symbolSize = 2;

        switch (shiftOperatorTokenType) {
            case ZEN_TOKEN_LEFT_ANGLE_BRACKET_2: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '+' symbol.
                 */
                symbol = ">>";

                break;
            }

            case ZEN_TOKEN_RIGHT_ANGLE_BRACKET_2: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '-' symbol.
                 */
                symbol = "<<";

                break;
            }

            case ZEN_TOKEN_RIGHT_ANGLE_BRACKET_3: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '-' symbol.
                 */
                symbol = "<<<";
                /* All the other symbols are of length 2, except for the '<<<' symbol.
                 * Therefore, update the symbol size.
                 */
                symbolSize = 3;

                break;
            }

            default: {
                /* The generator should not reach this code! */
                printf("[error] Control should not reach here.\n");
            }
        }

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * the left shift, right shift, and extended right shift operations are
         * delegated to functions annotated with the Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, symbol, symbolSize);
    }
}

// additiveExpression

void k_BinaryEntityGenerator_onEnterAdditiveExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_AdditiveExpressionContext_t* context = (k_AdditiveExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitAdditiveExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_AdditiveExpressionContext_t* context = (k_AdditiveExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_multiplicativeExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_multiplicativeExpressions, i);

        /* Retrieve the additive operator. */
        k_ASTNode_t* additiveOperator = pair->m_left;
        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * additive expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for additive
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, (k_ASTNode_t*)pair->m_right);

        /* Retrieve the corresponding additive operator token from the AST
         * node.
         */
        k_Token_t* additiveOperatorToken = (k_Token_t*)additiveOperator->m_context;
        /* Retrieve the type of the additive operator. */
        k_TokenType_t additiveOperatorTokenType = k_Token_getType(additiveOperatorToken);

        /* The values of symbol and symbolSize are the only arbitrary variables
         * when invoking the k_BinaryEntityGenerator_invokeEvaluate() function.
         * Therefore, instead of rewriting the invocation expression multiple
         * times, I have factored it out.
         */
        uint8_t* symbol = NULL;
        int32_t symbolSize = 1;

        switch (additiveOperatorTokenType) {
            case ZEN_TOKEN_PLUS: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '+' symbol.
                 */
                symbol = "+";

                break;
            }

            case ZEN_TOKEN_DASH: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '-' symbol.
                 */
                symbol = "-";

                break;
            }

            default: {
                /* The generator should not reach this code! */
                printf("[internal error] Control should not reach here.\n");
            }
        }

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * the addition and subtraction operations are delegated to
         * functions annotated with the Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, symbol, symbolSize);
    }
}

// multiplicativeExpression

void k_BinaryEntityGenerator_onEnterMultiplicativeExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_MultiplicativeExpressionContext_t* context = (k_MultiplicativeExpressionContext_t*)node->m_context;

    /* Generates the instructions corresponding to the very first child of
     * the node.
     */
    k_ASTListener_visitFirstChild(astListener);
}

void k_BinaryEntityGenerator_onExitMultiplicativeExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_MultiplicativeExpressionContext_t* context = (k_MultiplicativeExpressionContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(context->m_unaryExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_unaryExpressions, i);

        /* Retrieve the multiplicative operator. */
        k_ASTNode_t* multiplicativeOperator = pair->m_left;
        /* At this point, the instructions corresponding to the left operand
         * should be generated. The generation of the instructions for
         * multiplicative expressions follow the order: operand1 operand2 operator.
         * In other words, the compiler generates instructions for multiplicative
         * expressions in postfix order. Therefore, generate the instructions for
         * the right operand and invoking the ZenKernel.evaluate(...) function,
         * which takes care of *aggregating* the result.
         */
        k_ASTWalker_walk(astListener, (k_ASTNode_t*)pair->m_right);

        /* Retrieve the corresponding multiplicative operator token from the AST
         * node.
         */
        k_Token_t* multiplicativeOperatorToken = (k_Token_t*)multiplicativeOperator->m_context;
        /* Retrieve the type of the multiplicative operator. */
        k_TokenType_t multiplicativeOperatorTokenType = k_Token_getType(multiplicativeOperatorToken);

        /* The values of symbol and symbolSize are the only arbitrary variables
         * when invoking the k_BinaryEntityGenerator_invokeEvaluate() function.
         * Therefore, instead of rewriting the invocation expression multiple
         * times, I have factored it out.
         */
        uint8_t* symbol = NULL;
        int32_t symbolSize = 1;

        switch (multiplicativeOperatorTokenType) {
            case ZEN_TOKEN_ASTERISK: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '*' symbol.
                 */
                symbol = "*";

                break;
            }

            case ZEN_TOKEN_FORWARD_SLASH: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '/' symbol.
                 */
                symbol = "/";

                break;
            }

            case ZEN_TOKEN_MODULUS: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '%' symbol.
                 */
                symbol = "%";

                break;
            }

            default: {
                /* The generator should not reach this code! */
                printf("[error] Control should not reach here.\n");
            }
        }

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * the multiplication/division/modulus operations are delegated to
         * functions annotated with the Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, symbol, symbolSize);
    }
}

// unaryExpression

void k_BinaryEntityGenerator_onEnterUnaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_UnaryExpressionContext_t* context = (k_UnaryExpressionContext_t*)node->m_context;

    k_ASTNode_t* unaryOperator = context->m_unaryOperator;
    if (unaryOperator != NULL) {
        /* Generate the instructions corresponding to the unary expression. */
        k_ASTWalker_walk(astListener, context->m_unaryExpression);

        /* Retrieve the corresponding unary operator token from the AST
         * node.
         */
        k_Token_t* unaryOperatorToken = (k_Token_t*)unaryOperator->m_context;
        /* Retrieve the type of the unary operator. */
        k_TokenType_t unaryOperatorType = k_Token_getType(unaryOperatorToken);

        /* The values of symbol and symbolSize are the only arbitrary variables
         * when invoking the k_BinaryEntityGenerator_invokeEvaluate() function.
         * Therefore, instead of rewriting the invocation expression multiple
         * times, I have factored it out.
         */
        uint8_t* symbol = NULL;
        int32_t symbolSize = 1;

        switch (unaryOperatorType) {
            case ZEN_TOKEN_PLUS: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '1+' symbol.
                 */
                symbol = "1+";
                symbolSize = 2;

                break;
            }

            case ZEN_TOKEN_DASH: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '1-' symbol.
                 */
                symbol = "1-";
                symbolSize = 2;

                break;
            }

            case ZEN_TOKEN_TILDE: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '~' symbol.
                 */
                symbol = "~";

                break;
            }

            case ZEN_TOKEN_EXCLAMATION_MARK: {
                /* The kernel should find a function annotated with the Operator
                 * annotation that handles the '!' symbol.
                 */
                symbol = "!";

                break;
            }

            /*
            case ZEN_TOKEN_PLUS_2:
            case ZEN_TOKEN_DASH_2: {
                if (unaryOperatorType == ZEN_TOKEN_PLUS_2) {
                    /* The onPreIncrement() function is invoked against the object whose internal
                     * state has to be "incremented by 1". It returns an object with its internal
                     * state "incremented by 1".
                     *
                    // k_BinaryEntityGenerator_emitInvokeVirtual(generator, 0);
                }
                else {
                    /* The onPreDecrement() function is invoked against the object whose internal
                     * state has to be "incremented by 1". It returns an object with its internal
                     * state "incremented by 1".
                     *
                    // k_BinaryEntityGenerator_emitInvokeVirtual(generator, 0);
                }
                /* A copy of the "incremented" object is required on the operand stack for
                 * assignment.
                 */
                // k_BinaryEntityGenerator_emitDuplicate(generator);
                /* Assign the variable the object which represents the new state.
                 *
                 * TODO: Change store_a to store_a1 (and friends) and
                 *       store_field when necessary.
                 *
                // k_BinaryEntityGenerator_emitStoreReference(generator, 0);

                break;
            }
            */
        }

        // TODO: I don't think unary operators work.

        /* Generate the instructions corresponding to invoking the
         * ZenKernel.evaluate() function. Since, Zen is dynamically typed
         * the compiler cannot determine the type of the operands. Therefore,
         * the multiplication/division/modulus operations are delegated to
         * functions annotated with the Operator annotation.
         */
        k_BinaryEntityGenerator_invokeEvaluate(generator, symbol, symbolSize);

        /* The instructions corresponding to the children nodes have been generated.
         * Therefore, do not visit them again.
         */
        k_ASTListener_skipChildren(astListener);
    }
}

void k_BinaryEntityGenerator_onExitUnaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// postfixExpression

void k_BinaryEntityGenerator_invokeEvaluate(k_BinaryEntityGenerator_t* generator,
    uint8_t* symbol, int32_t symbolSize) {
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    // ZenKernel.evaluate(Object operand, String symbol)
    // ZenKernel.evaluate(Object operand1, Object operand2, String symbol)

    uint16_t symbolIndex = k_ConstantPoolBuilder_getStringEntryIndexEx(
        generator->m_constantPoolBuilder, symbol, symbolSize);
    k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder, symbolIndex);
    jtk_Logger_debug(logger, "Emitted load_cpr %d", symbolIndex);

    uint16_t evaluateIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_EVALUATE];

    /* Invoke the static function to evaluate the expression. */
    k_BinaryEntityBuilder_emitInvokeStatic(generator->m_builder,
        evaluateIndex);

    /* Log the emission of the invoke_static instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_static %d", evaluateIndex);
}

int64_t k_Long_convert(const uint8_t* text, int32_t length, int32_t radix) {
    jtk_Assert_assertObject(text, "The specified text is null.");

    bool error = false;
    int64_t result = -1;
    if ((length != 0) && (radix >= JTK_INTEGER_MIN_RADIX) &&
        (radix <= JTK_INTEGER_MAX_RADIX)) {
        bool negative = false;
        int32_t i = 0;
        int64_t limit = -JTK_LONG_MAX_VALUE;
        uint8_t first = text[0];
        result = 0;
        if (!((first >= '0') && (first <= '9'))) {
            if (first == '+') {
                negative = false;
            }
            else if (first == '-') {
                negative = true;
                limit = JTK_LONG_MIN_VALUE;
            }
            else {
                error = true;
            }

            if (length == 1) {
                error = true;
            }

            i++;
        }

        if (result != -1) {
            int64_t m = limit / radix;
            while (i < length) {
                uint8_t value = text[i++];
                if (value != '_') {
                    int32_t digit = jtk_Integer_digit(value, radix);
                    if ((digit < 0) || (result < m)) {
                        error = true;
                        break;
                    }
                    result *= radix;
                    if (result < (limit + digit)) {
                        error = true;
                        break;
                    }
                    result -= digit;
                }
            }
            if (error) {
                result = -1;
            }
            else {
                result = negative? result : -result;
            }
        }
    }
    return result;
}

void k_BinaryEntityGenerator_loadLong(k_BinaryEntityGenerator_t* generator,
    int64_t value) {
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    switch (value) {
        case 0: {
            /* Emit the push_l0 instruction. */
            k_BinaryEntityBuilder_emitPushLong0(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_l0");

            break;
        }

        case 1: {
            /* Emit the push_l1 instruction. */
            k_BinaryEntityBuilder_emitPushLong1(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_l1");

            break;
        }

        case 2: {
            /* Emit the push_l2 instruction. */
            k_BinaryEntityBuilder_emitPushLong2(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_l2");

            break;
        }

        default: {
            /* If the integer literal is less than or equal to 255, then it can be
             * pushed onto the operand stack with the push_b instruction. Another
             * instruction can pad the remaining 32 bits on the operand stack with
             * zeroes.
             *
             * The range of a byte is [-128, 127].
             *
             * As of now, the compiler generates negative values with two instructions.
             * One instruction pushes the value onto the operand stack. The other
             * instruction takes care of multiplying a value of -1 to the previously
             * pushed integer. This will be fixed in the future.
             */
            if ((value >= 6) && (value <= 127)) {
                /* Emit the push_i0 instruction in order to provide padding on the operand
                 * stack.
                 */
                k_BinaryEntityBuilder_emitPushInteger0(generator->m_builder);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted push_i0");

                /* Emit the push_b instruction. */
                k_BinaryEntityBuilder_emitPushByte(generator->m_builder,
                    value);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted push_b %d", value);
            }
            else if (value <= 32767) {
                /* Emit the push_i0 instruction in order to provide padding on the operand
                 * stack.
                 */
                k_BinaryEntityBuilder_emitPushInteger0(generator->m_builder);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted push_i0");

                /* If the integer literal is less than or equal to 32767, then it
                 * can be pushed onto the operand stack with the push_s instruction.
                 *
                 * The range of a short is [-32768, 32767].
                 *
                 * As of now, the compiler generates negative values with two instructions.
                 * One instruction pushes the value onto the operand stack. The other
                 * instruction takes care of multiplying a value of -1 to the previously
                 * pushed integer. This will be fixed in the future.
                 */
                /* Emit the push_s instruction. */
                k_BinaryEntityBuilder_emitPushShort(generator->m_builder,
                    value);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted push_s %d", value);
            }
            else {
                // TODO: Filter emission of values larger than the integer threshold.

                /* If the integer literal is larger than 32767, then it should be pushed
                 * the operand stack with the load_cpr instruction.
                 *
                 * As of now, the compiler generates negative values with two instructions.
                 * One instruction pushes the value onto the operand stack. The other
                 * instruction takes care of multiplying a value of -1 to the previously
                 * pushed integer. This will be fixed in the future.
                 */

                uint8_t longIndex = k_ConstantPoolBuilder_getLongEntryIndex(
                    generator->m_constantPoolBuilder, value);

                /* Emit the load_cpr instruction. */
                k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder,
                    longIndex);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted load_cpr %d", longIndex);
            }
        }
    }
}

void k_BinaryEntityGenerator_loadInteger(k_BinaryEntityGenerator_t* generator,
    int32_t value) {
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;

    switch (value) {
        case 0: {
            /* Emit the push_i0 instruction. */
            k_BinaryEntityBuilder_emitPushInteger0(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_i0");

            break;
        }

        case 1: {
            /* Emit the push_i1 instruction. */
            k_BinaryEntityBuilder_emitPushInteger1(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_i1");

            break;
        }

        case 2: {
            /* Emit the push_i2 instruction. */
            k_BinaryEntityBuilder_emitPushInteger2(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_i2");

            break;
        }

        case 3: {
            /* Emit the push_i3 instruction. */
            k_BinaryEntityBuilder_emitPushInteger3(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_i3");

            break;
        }

        case 4: {
            /* Emit the push_i4 instruction. */
            k_BinaryEntityBuilder_emitPushInteger4(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_i4");

            break;
        }

        case 5: {
            /* Emit the push_i5 instruction. */
            k_BinaryEntityBuilder_emitPushInteger5(generator->m_builder);

            /* Log the emission of the instruction. */
            jtk_Logger_debug(logger, "Emitted push_i5");

            break;
        }

        default: {
            /* If the integer literal is less than or equal to 255, then it can be
             * pushed onto the operand stack with the push_b instruction.
             *
             * The range of a byte is [-128, 127].
             *
             * As of now, the compiler generates negative values with two instructions.
             * One instruction pushes the value onto the operand stack. The other
             * instruction takes care of multiplying a value of -1 to the previously
             * pushed integer. This will be fixed in the future.
             */
            if ((value >= 6) && (value <= 127)) {
                /* Emit the push_b instruction. */
                k_BinaryEntityBuilder_emitPushByte(generator->m_builder,
                    value);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted push_b %d", value);
            }
            else if (value <= 32767) {
                /* If the integer literal is less than or equal to 32767, then it
                 * can be pushed onto the operand stack with the push_s instruction.
                 *
                 * The range of a short is [-32768, 32767].
                 *
                 * As of now, the compiler generates negative values with two instructions.
                 * One instruction pushes the value onto the operand stack. The other
                 * instruction takes care of multiplying a value of -1 to the previously
                 * pushed integer. This will be fixed in the future.
                 */
                /* Emit the push_s instruction. */
                k_BinaryEntityBuilder_emitPushShort(generator->m_builder,
                    value);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted push_s %d", value);
            }
            else {
                // TODO: Filter emission of values larger than the integer threshold.

                /* If the integer literal is larger than 32767, then it should be pushed
                 * the operand stack with the load_cpr instruction.
                 *
                 * As of now, the compiler generates negative values with two instructions.
                 * One instruction pushes the value onto the operand stack. The other
                 * instruction takes care of multiplying a value of -1 to the previously
                 * pushed integer. This will be fixed in the future.
                 */

                uint8_t integerIndex = k_ConstantPoolBuilder_getIntegerEntryIndex(
                    generator->m_constantPoolBuilder, value);

                /* Emit the load_cpr instruction. */
                k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder,
                    integerIndex);

                /* Log the emission of the instruction. */
                jtk_Logger_debug(logger, "Emitted load_cpr %d", integerIndex);
            }

            break;
        }
    }
}

/*
 * Zen allows operator overriding through a combination of functions and
 * annotations.
 *
 * All the operators in Zen are dispatched to a function call.
 * The ZenKernel.evaluate(...) function finds a suitable handler for the
 * operator defined within the operand object and dispatches it. In other words,
 * the compiler translates expressions with operators to equivalent
 * ZenKernel.evaluate(...) calls.
 *
 * For example, in the HashMap class the following annotation
 * overrides the subscript operator.
 *
 * @Operator symbol='[]'
 * function getValue(key)
 *     ...
 *
 * With that information, consider the following snippet of code.
 *
 * var emailAddresses = {
 *     'Samuel Rowe' : 'samuelrowe1999@gmail.com',
 *     'Joel E. Rego' : 'joelerego@gmail.com'
 * }
 * var myEmailAddress = emailAddresses['Samuel Rowe']
 *
 * The above code snippet is equivalent to the following expression statement.
 *
 * var emailAddresses = {
 *     'Samuel Rowe' : 'samuelrowe1999@gmail.com',
 *     'Joel E. Rego' : 'joelerego@gmail.com'
 * }
 * var myEmailAddress = ZenKernel.evaluate(emailAddresses, 'Samuel Rowe', '[]')
 *
 * In fact, when you compile the former code snippet the compiler generates
 * instructions as if the code was written in the latter form.
 */

void k_BinaryEntityGenerator_handleIntegerLiteral(k_BinaryEntityGenerator_t* generator,
    k_Token_t* token) {
    const uint8_t* integerClassName = "zen/core/Integer";
    int32_t integerClassNameSize = 16;
    uint16_t integerClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, integerClassName,
        integerClassNameSize);

    /* Emit the new instruction. */
    k_BinaryEntityBuilder_emitNew(generator->m_builder, integerClassIndex);
    /* Emit the duplicate instruction. */
    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

    uint8_t* integerText = k_Token_getText(token);
    int32_t actualIntegerLength = k_Token_getLength(token);
    int32_t integerLength = actualIntegerLength;

    int32_t radix = 10;
    if (integerLength > 2) {
        if ((integerText[0] == '0') && ((integerText[0] == 'x') || (integerText[0] == 'X'))) {
            radix = 16;
            integerText += 2;
            integerLength -= 2;
        }
        else if ((integerText[0] == '0') && ((integerText[0] == 'b') || (integerText[0] == 'B'))) {
            radix = 2;
            integerText += 2;
            integerLength -= 2;
        }
        else if ((integerText[0] == '0') && ((integerText[0] == 'c') || (integerText[0] == 'C'))) {
            /* TODO: Octal integer literals begin with 0c or 0C according
                * to the logic written here. Therefore, please modify the
                * lexer accordingly.
                */

            radix = 8;
            integerText += 2;
            integerLength -= 2;
        }
    }

    // bool longLiteral = (integerText[actualIntegerLength - 1] == 'L') ||
        // (integerText[actualIntegerLength - 1] == 'l');

    // if (longLiteral) {
        // integerLength--;
    // }

    int64_t value = k_Long_convert(integerText, integerLength, radix);

    // if (longLiteral) {
        k_BinaryEntityGenerator_loadLong(generator, value);
    // }
    // else {
        // k_BinaryEntityGenerator_loadInteger(generator, value);
    // }

    // TODO: Implement integer interning.

    const uint8_t* constructorName = "<initialize>";
    int32_t constructorNameSize = 12;
    // const uint8_t* constructorDescriptor = "v:i";
    const uint8_t* constructorDescriptor = "v:(zen/core/Object)";
    int32_t constructorDescriptorSize = 19;
    uint16_t constructorIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, integerClassName,
        integerClassNameSize, constructorDescriptor, constructorDescriptorSize,
        constructorName, constructorNameSize, 0);

    /* Invoke the constructor to initialize the new integer instance. */
    k_BinaryEntityBuilder_emitInvokeSpecial(generator->m_builder,
        constructorIndex);
}

void k_BinaryEntityGenerator_handleStringLiteral(k_BinaryEntityGenerator_t* generator,
    k_Token_t* token) {
    int32_t size = 0;
    int32_t limit = token->m_length - 1;
    uint8_t* string = jtk_Memory_allocate(uint8_t, limit);
    int32_t i;
    for (i = 1; i < limit; i++) {
        uint8_t next = token->m_text[i];
        if (token->m_text[i] == '\\') {
            switch (token->m_text[++i]) {
                case 'b': next = '\b'; break;
                case 'f': next = '\f'; break;
                case 'r': next = '\r'; break;
                case 'n': next = '\n'; break;
                case 't': next = '\t'; break;
                case '\'': next = '\''; break;
                case '\"': next = '\"'; break;
                // TODO: 'u'

                default: {
                    printf("[internal error] Control should not reach here.");
                }
            }
        }
        string[size++] = next;
    }

    /* Retrieve a valid index into the constant pool. The entry at
     * this index is a constant pool string. The token text encloses
     * the content within double quotes. Therefore, the first quote
     * is skipped using pointer arithmetic and the last quote
     * is skipped by subtracting 1 from the length of the text.
     * Another 1 is subtracted from the text length because the first
     * quote was skipped.
     */
    uint8_t stringIndex = k_ConstantPoolBuilder_getStringEntryIndexEx(
        generator->m_constantPoolBuilder, string, size);
    /* Emit load_cpr instruction. */
    k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder,
        stringIndex);

    jtk_Memory_deallocate(string);
}

void k_BinaryEntityGenerator_handleDirectFunction(
    k_BinaryEntityGenerator_t* generator, k_Symbol_t* symbol,
    k_FunctionArgumentsContext_t* functionArgumentsContext) {
    k_ASTNode_t* expressions = functionArgumentsContext->m_expressions;
    jtk_ArrayList_t* arguments = NULL;
    int32_t argumentCount = 0;
    if (expressions != NULL) {
        k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->m_context;
        arguments = expressionsContext->m_expressions;
        argumentCount = jtk_ArrayList_getSize(arguments);
    }

    k_FunctionSignature_t* signature = k_Symbol_getFunctionSignature(symbol,
        argumentCount);
    if (signature == NULL) {
        printf("[error] Cannot find a suitable static function. A previous phase in the compiler has failed.\n");
    }

    bool instance = !k_Modifier_hasStatic(signature->m_modifiers);
    if (instance) {
        /* The "this" reference is always stored at the zeroth position
        * in the local variable array. Further, we assume that the
        * class member and the expression being processed appear in
        * in the same class. Therefore, emit a load reference to the
        * this reference.
        */
        k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);
    }

    if (arguments != NULL) {
        int32_t argumentIndex;
        for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
            /* Retrieve the expression for the current argument. */
            k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(arguments,
                argumentIndex);

            /* Visit the expression node and generate the relevant instructions. */
            k_ASTWalker_walk(generator->m_astListener, argument);
        }
    }

    k_Scope_t* enclosingScope = symbol->m_enclosingScope;
    k_Symbol_t* classSymbol = enclosingScope->m_symbol;
    const uint8_t* classDescriptor = classSymbol->m_context.m_asClass.m_descriptor;
    uint16_t classDescriptorSize = classSymbol->m_context.m_asClass.m_descriptorSize;
    int32_t index = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, classDescriptor,
        classDescriptorSize, signature->m_descriptor, signature->m_descriptorSize,
        symbol->m_name, symbol->m_nameSize, signature->m_tableIndex);

    if (instance) {
        k_BinaryEntityBuilder_emitInvokeVirtual(generator->m_builder, index);
    }
    else {
        k_BinaryEntityBuilder_emitInvokeStatic(generator->m_builder, index);
    }
}

/* This function assumes that the reference for instance fields is already
 * pushed onto the operand stack.
 */
void k_BinaryEntityGenerator_handleDirectField(k_BinaryEntityGenerator_t* generator,
    k_Symbol_t* classSymbol, k_Symbol_t* targetSymbol, bool last) {
    k_ClassSymbol_t* classSymbolContext = &classSymbol->m_context.m_asClass;
    if (k_Symbol_isFunction(targetSymbol)) {
        jtk_ArrayList_t* signatures = targetSymbol->m_context.m_asFunction.m_signatures;
        if (jtk_ArrayList_getSize(signatures) > 1) {
            printf("[error] Cannot reference an overloaded function. Consider using the reflection package.\n");
        }
        else {
            k_FunctionSignature_t* signature = jtk_ArrayList_getValue(signatures, 0);

            int32_t cpIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
                generator->m_constantPoolBuilder, classSymbolContext->m_descriptor,
                classSymbolContext->m_descriptorSize, signature->m_descriptor, signature->m_descriptorSize,
                targetSymbol->m_name, targetSymbol->m_nameSize, signature->m_tableIndex);
            k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder, cpIndex);
        }
    }
    else if (k_Symbol_isConstant(targetSymbol) ||
        k_Symbol_isVariable(targetSymbol)) {
        // TODO: Add table indexes to fields!
        uint16_t cpIndex = k_ConstantPoolBuilder_getFieldEntryIndexEx(
            generator->m_constantPoolBuilder, classSymbolContext->m_descriptor,
                classSymbolContext->m_descriptorSize, "(zen/core/Object)", 17,
                targetSymbol->m_name, targetSymbol->m_nameSize);
        bool instance = !k_Symbol_isStatic(targetSymbol);
        if (last && lhs) {
            if (instance) {
                k_BinaryEntityBuilder_emitStoreInstanceField(generator->m_builder,
                    cpIndex);
            }
            else {
                k_BinaryEntityBuilder_emitStoreStaticField(generator->m_builder,
                    cpIndex);
            }
        }
        else {
            if (instance) {
                k_BinaryEntityBuilder_emitLoadInstanceField(generator->m_builder,
                    cpIndex);
            }
            else {
                k_BinaryEntityBuilder_emitLoadStaticField(generator->m_builder,
                    cpIndex);
            }
        }
    }
    else {
        printf("[internal error] Control should not reach here.");
    }
}

/* A direct member access can take one of the following forms.
 * primary.function()
 * primary.field
 * primary.function
 * primary.field =
 * primary.function = (TODO: The compiler should generate a compile-time error.)
 * primary.field() = (TODO: The compiler should generate a compile-time error.)
 * null.<anything> (TODO: The compiler should generate a compile-time error.)
 *
 * TODO: Decimal literal!
 *
 * Here, primary can an identifier referencing a class, integer literal, string
 * literal, Boolean literal, or this reference.
 */
void k_BinaryEntityGenerator_handleDirectAccess(k_BinaryEntityGenerator_t* generator,
    k_MemberAccessContext_t* context, k_Token_t* primaryToken,
    k_Symbol_t* primarySymbol, jtk_ArrayList_t* postfixParts, int32_t* index) {

    k_ASTNode_t* identifier = context->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

    k_Symbol_t* targetSymbol = NULL;
    k_Symbol_t* classSymbol = NULL;
    switch (primaryToken->m_type) {
        case ZEN_TOKEN_IDENTIFIER: {
            classSymbol = primarySymbol;
            break;
        }

        case ZEN_TOKEN_INTEGER_LITERAL: {
            classSymbol = k_Compiler_resolveSymbol(
                generator->m_compiler, "zen.core.Integer", 16);

            break;
        }

        case ZEN_TOKEN_STRING_LITERAL: {
            classSymbol = k_Compiler_resolveSymbol(generator->m_compiler,
                "zen.core.String", 15);
            break;
        }

        case ZEN_TOKEN_KEYWORD_TRUE:
        case ZEN_TOKEN_KEYWORD_FALSE: {
            classSymbol = k_Compiler_resolveSymbol(
                generator->m_compiler, "zen.core.Boolean", 16);
        }

        case ZEN_TOKEN_KEYWORD_NULL: {
            printf("[error] Why would you access a member of the null literal?\n");
            break;
        }

        case ZEN_TOKEN_KEYWORD_THIS: {
            classSymbol = generator->m_symbolTable->m_currentScope->m_enclosingScope->m_symbol;
            break;
        }
    }

    targetSymbol = k_Scope_resolve(primarySymbol->m_context.m_asClass.m_classScope,
        identifierToken->m_text);

    int32_t postfixPartCount = jtk_ArrayList_getSize(postfixParts);
    if ((*index + 1) < postfixPartCount) {
        k_ASTNode_t* nextPostfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
            postfixParts, *index + 1);
        k_ASTNodeType_t nextPostfixPartType = k_ASTNode_getType(nextPostfixPart);

        if (nextPostfixPartType == ZEN_AST_NODE_TYPE_FUNCTION_ARGUMENTS) {
            k_ASTNode_t* functionArguments = nextPostfixPart;
            *index++;

            k_FunctionArgumentsContext_t* functionArgumentsContext =
                (k_FunctionArgumentsContext_t*)functionArguments->m_context;

            k_ExpressionsContext_t* expressionsContext = NULL;
            k_ASTNode_t* expressions = functionArgumentsContext->m_expressions;
            if (expressions != NULL) {
                expressionsContext = (k_ExpressionsContext_t*)expressions->m_context;
            }

            k_BinaryEntityGenerator_handleDirectFunction(generator,
                targetSymbol, expressionsContext);
        }
    }
    else {
        // If lhs and last postfix part, data should be stored!
        k_BinaryEntityGenerator_handleDirectField(generator, classSymbol,
            targetSymbol, (*index + 1) == postfixPartCount);
    }
}

/* A dynamic member access can take one of the following forms.
 * parent.function()
 * parent.field
 * parent.function
 * parent.field =
 * parent.function = (the runtime throws an exception in this case)
 *
 * We assume that a reference to the field has been pushed onto the
 * operand stack.
 *
 * Methods and fields handled by this function cannot be static. The generator
 * ensures this assertion based on the fact that nested classes and
 * static methods and fields cannot be referenced via objects in Zen.
 */
void k_BinaryEntityGenerator_handleDynamicAccess(k_BinaryEntityGenerator_t* generator,
    k_MemberAccessContext_t* memberAccessContext, jtk_ArrayList_t* postfixParts,
    int32_t* index) {
    k_ASTNode_t* identifier = memberAccessContext->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

    /* The name of the function/field to invoke/load. */
    int32_t targetNameIndex = k_ConstantPoolBuilder_getStringEntryIndexEx(
        generator->m_constantPoolBuilder, identifierToken->m_text,
        identifierToken->m_length);

    int32_t postfixPartCount = jtk_ArrayList_getSize(postfixParts);
    int32_t previousIndex = *index;
    if ((*index + 1) < postfixPartCount) {
        k_ASTNode_t* nextPostfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
            postfixParts, *index + 1);
        k_ASTNodeType_t nextPostfixPartType = k_ASTNode_getType(nextPostfixPart);
        if (nextPostfixPartType == ZEN_AST_NODE_TYPE_FUNCTION_ARGUMENTS) {
            *index++;
            k_ASTNode_t* functionArguments = nextPostfixPart;
            k_FunctionArgumentsContext_t* functionArgumentsContext = (k_FunctionArgumentsContext_t*)functionArguments->m_context;

            int32_t argumentCount = 0;
            k_ASTNode_t* expressions = functionArgumentsContext->m_expressions;
            if (expressions != NULL) {
                k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->m_context;
                argumentCount = jtk_ArrayList_getSize(expressionsContext->m_expressions);
                int32_t argumentIndex;
                for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                    /* Retrieve the expression for the current argument. */
                    k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                        expressionsContext->m_expressions, argumentIndex);

                    /* Visit the expression node and generate the relevant instructions. */
                    k_ASTWalker_walk(generator->m_astListener, argument);
                }
            }

            // emitInvokeDynamic()
        }
    }

    /* If we did not generate instructions for a function invocation,
     * then generate instructions for loading/storing a field or function
     * reference.
     */
    if (previousIndex != *index) {
        /* Push the name of the target function on the operand stack. */
        k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder,
            targetNameIndex);

        if ((*index + 1 == postfixPartCount) && lhs) {
            uint16_t storeFieldIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_STORE_FIELD];
            k_BinaryEntityBuilder_emitInvokeStatic(generator->m_builder, storeFieldIndex);
        }
        else {
            /* We may be generating instructions either for LHS or RHS,
             * but it does not matter because at this point a field or method
             * reference should be loaded.
             */
            uint16_t loadFieldIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_LOAD_FIELD];
            k_BinaryEntityBuilder_emitInvokeStatic(generator->m_builder, loadFieldIndex);
        }
    }
}

void k_BinaryEntityGenerator_handleIdentifier(k_BinaryEntityGenerator_t* generator,
    k_Symbol_t* symbol) {
    k_ASTNode_t* identifier = symbol->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
    k_Scope_t* enclosingScope = k_Symbol_getEnclosingScope(symbol);

    uint16_t storeFieldIndex = generator->m_cpfIndexes[ZEN_BINARY_ENTITY_GENERATOR_ZEN_KERNEL_STORE_FIELD];
    if (k_Symbol_isVariable(symbol) || k_Symbol_isConstant(symbol)) {
        if (lhs) {
            if (k_Scope_isClassScope(enclosingScope)) {
                bool instance = !k_Symbol_isStatic(symbol);

                uint16_t identifierIndex = k_ConstantPoolBuilder_getStringEntryIndexEx(
                    generator->m_constantPoolBuilder, identifierToken->m_text,
                    identifierToken->m_length);

                if (instance) {
                    /* The this reference is always stored at the zeroth position
                        * in the local variable array. Further, we assume that the
                        * class member and the expression being processed appear in
                        * in the same class. Therefore, emit a load reference to the
                        * this reference.
                        */
                    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);
                    /* Load the name of the field. */
                    k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder, identifierIndex);
                    /* Invoke the ZenKernel.storeField() function to update
                     * the field.
                     */
                    k_BinaryEntityBuilder_emitInvokeStatic(generator->m_builder, storeFieldIndex);
                }
                else {
                    /* Emit the `load_cpr` instruction to load the reference of the
                        * class to which the static field belongs.
                        */
                    k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder, 0);
                    /* Load the name of the field. */
                    k_BinaryEntityBuilder_emitLoadCPR(generator->m_builder, identifierIndex);
                    /* Invoke the ZenKernel.storeField() function to update
                    * the field.
                    */
                    k_BinaryEntityBuilder_emitInvokeStatic(generator->m_builder, storeFieldIndex);
                }
            }
            else if (k_Scope_isLocalScope(enclosingScope)) {
                if (k_Symbol_isVariable(symbol)) {
                    /* Emit the duplicate instruction. */
                    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);
                    /* Emit the store_a instruction. */
                    k_BinaryEntityBuilder_emitStoreReference(generator->m_builder,
                        symbol->m_index);
                }
                else {
                    printf("[error] Invalid assignment of constant after declaration.\n");
                }
            }
        }
        else {
            if (k_Scope_isClassScope(enclosingScope)) {
                if (!k_Symbol_isStatic(symbol)) {
                    /* The this reference is always stored at the zeroth position
                        * in the local variable array. Further, we assume that the
                        * class member and the expression being processed appear in
                        * in the same class. Therefore, emit a load reference to the
                        * this reference.
                        */
                    k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);
                    /* Load the instance field. */
                    k_BinaryEntityBuilder_emitLoadInstanceField(generator->m_builder, 0);
                }
                else {
                    /* Load the static field. */
                    k_BinaryEntityBuilder_emitLoadStaticField(generator->m_builder, 0);
                }
            }
            else if (k_Scope_isLocalScope(enclosingScope) || k_Scope_isFunctionScope(enclosingScope)) {
                /* Emit the store_a instruction. */
                k_BinaryEntityBuilder_emitLoadReference(generator->m_builder,
                    symbol->m_index);
            }
        }
    }
    else if (k_Symbol_isFunction(symbol)) {
        printf("[TODO] Function references are yet to be implemented.\n");
    }
    else if (k_Symbol_isClass(symbol)) {
        printf("[TODO] Class references are yet to be implemented.\n");
    }
}

void k_BinaryEntityGenerator_handleSubscript(k_BinaryEntityGenerator_t* generator,
    k_SubscriptContext_t* subscriptContext, bool lastPostfix) {
    /* Visit the index expression node and generate the relevant
     * instructions.
     */
    k_ASTWalker_walk(generator->m_astListener, subscriptContext->m_expression);

    /* Generate the instructions corresponding to invoking the
     * ZenKernel.evaluate() function. Since, Zen is dynamically typed
     * the compiler cannot determine the type of the operands. Therefore,
     * the subscript operation is delegated to the function annotated
     * with the Operator annotation.
     */
    const uint8_t* operator0 = "[]=";
    int32_t size = 3;
    if (!lhs || !lastPostfix) {
        operator0 = "[]";
        size = 2;
    }
    k_BinaryEntityGenerator_invokeEvaluate(generator, operator0, size);
    k_ASTListener_skipChildren(generator->m_astListener);
}

/*
 * The generation is divided into two categories:
 *  i. Only the primary expression is present.
 *  ii. The primary expression along with the postfix parts are present.
 *
 * ALGORITHM FOR GENERATION OF PRIMARY EXPRESSION
 *
 * 1. For an identifier
 *    a. Resolve the symbol for the given identifier.
 *    b. If there are no postfix parts and the symbol is local and variable/constant,
 *       then generate load_a for RHS and store_a for LHS.
 *    c. If the symbol is a class member and variable or constant, then
 *       generate load_instance_field, load_static_field, store_instance_field,
 *       or store_static_field depending on the type of the member and expression
 *       side.
 *    d. If the symbol is a class or annotation and there are no
 *       postfix parts then generate the load_cpr instruction.
 *       (This step applies only to RHS.)
 *       Otherwise, pass the reference of the symbol to the next phase, whose
 *       algorithm is described below.
 *    e. If the symbol is a function and there are no postfix parts then generate
 *       the load_cpr instruction. (This step applies only to RHS.)
 *       Otherwise, pass the reference of the symbol to the next phase, whose
 *       algorithm is described below.
 *    f. For all other symbols, print an error message that the previous phases
 *       have malfunctioned.
 *
 * 2. For literals (LHS requires at least one postfix part)
 *    a. For an integer value, generate one of the following instructions depending on
 *       value: load_cpr, push_b, push_s, push_i*, and push_l*.
 *    b. For a floating-point value, generate one of the following instructions
 *       depending on the value: load_cpr, push_f*, and push_d*.
 *    c. For a string value, generate the load_cpr instruction.
 *    d. For true and false literals generate push_i1 and push_i0, respectively.
 *    e. For null literal, generate the push_null instruction.
 *
 * 3. For expressions enclosed in parenthesis, simply walk through the expressions
 *    tree.
 *
 * 4. For map, list, and new expressions, please refer to the algorithms
 *    documented with their respective generators.
 *
 * 5. For this keyword, load the reference from the zeroth position in the local
 *    variable array. For LHS, at least one postfix part is required.
 *
 * ALGORITHM FOR GENERATION WHEN POSTFIX PARTS ARE PRESENT
 *
 * 1. The subscript operator requires an object that implements the operator
 *    on the operand stack, which becomes the first operand. Therefore,
 *    generate the instructions corresponding to the primary expression and the
 *    postfix parts prior to the current postfix part. The second operand should
 *    be evaluated by walking over the tree of the expression specified inside
 *    the square brackets. After which, the ZenKernel.evaluate(operand1, operand2, '[]')
 *    or ZenKernel.evaluate(operand1, operand2, operand3, '[]=') call should be
 *    made.
 * 2. For function arguments
 *    a. If function arguments is the very first postfix part, then use the
 *       function symbol resolved in the previous phase. Generate instructions
 *       to invoke the ZenKernel.dispatch() function to simulate the function
 *       call.
 *    b. When function arguments postfix part occurs at a position other than the
 *       beginning of the list of postfix parts, it is processed along with the
 *       member access postfix part.
 *    In any case, the generator walks through each expression that constitutes
 *    a function argument generating instructions. The arguments are processed
 *    from left to right.
 * 3. For member access postfix parts, the subsequent postfix part is checked to
 *    see if it is a function arguments postfix part. If true, the algorithm
 *    generates instructions to invoke the ZenKernel.dispatch() function to
 *    simulate the function call.
 *    However, if the subsequent postfix part is not a function arguments postfix
 *    part, then either the load_static_field or  the load_instance_field instruction
 *    is generated.
 *    It should be noted that the identifiers in the member access are not resolved
 *    for their declaration in the symbol table. They are intended to be verified
 *    at runtime due to the dynamic nature of Zen.
 */
void k_BinaryEntityGenerator_onExitPostfixExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_PostfixExpressionContext_t* context = (k_PostfixExpressionContext_t*)node->m_context;
    k_ASTNode_t* primaryExpression = context->m_primaryExpression;
    k_PrimaryExpressionContext_t* primaryExpressionContext = (k_PrimaryExpressionContext_t*)primaryExpression->m_context;
    k_ASTNode_t* expression = primaryExpressionContext->m_expression;

    k_Symbol_t* primarySymbol = NULL;
    int32_t postfixPartCount = jtk_ArrayList_getSize(context->m_postfixParts);
    k_Token_t* primaryToken = NULL;
    k_TokenType_t primaryTokenType = ZEN_TOKEN_UNKNOWN;

    if (k_ASTNode_isTerminal(expression)) {
        /* Retrieve the token that the primary expression represents. */
        primaryToken = (k_Token_t*)expression->m_context;

        primaryTokenType = k_Token_getType(primaryToken);
        switch (primaryTokenType) {
            case ZEN_TOKEN_IDENTIFIER: {
                /* Resolve the symbol and pass it to the next phase. */
                primarySymbol = k_SymbolTable_resolve(generator->m_symbolTable,
                    primaryToken->m_text);
                break;
            }

            case ZEN_TOKEN_INTEGER_LITERAL: {
                k_BinaryEntityGenerator_handleIntegerLiteral(generator, primaryToken);
                break;
            }

            case ZEN_TOKEN_KEYWORD_TRUE: {
                /* Emit push_i1. In the operand stack, 1 represents true. */
                k_BinaryEntityBuilder_emitPushInteger1(generator->m_builder);
                break;
            }

            case ZEN_TOKEN_KEYWORD_FALSE: {
                /* Emit push_i0 instruction. In the operand stack, 0 represents false. */
                k_BinaryEntityBuilder_emitPushInteger0(generator->m_builder);
                break;
            }

            case ZEN_TOKEN_STRING_LITERAL: {
                k_BinaryEntityGenerator_handleStringLiteral(generator, primaryToken);
                break;
            }

            case ZEN_TOKEN_KEYWORD_NULL: {
                /* Emit the push_null instruction. */
                k_BinaryEntityBuilder_emitPushNull(generator->m_builder);
                break;
            }

            case ZEN_TOKEN_KEYWORD_THIS: {
                /* Emit the load_a instruction. */
                k_BinaryEntityBuilder_emitLoadReference(generator->m_builder, 0);
                break;
            }
        }
    }
    else if ((expression->m_type == ZEN_AST_NODE_TYPE_MAP_EXPRESSION) ||
        (expression->m_type == ZEN_AST_NODE_TYPE_LIST_EXPRESSION) ||
        (expression->m_type == ZEN_AST_NODE_TYPE_EXPRESSION) ||
        (expression->m_type == ZEN_AST_NODE_TYPE_NEW_EXPRESSION)) {
        k_ASTWalker_walk(astListener, expression);
    }
    else {
        printf("[internal error] What node do we have here?\n");
    }

    const uint8_t* objectClassName = "zen/core/Object";
    int32_t objectClassNameSize = 15;
    uint16_t objectClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, objectClassName, objectClassNameSize);

    if ((postfixPartCount == 0) && (primaryTokenType == ZEN_TOKEN_IDENTIFIER)) {
        k_BinaryEntityGenerator_handleIdentifier(generator, primarySymbol);
    }
    else {
        int32_t i;
        for (i = 0; i < postfixPartCount; i++) {
            k_ASTNode_t* postfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
                context->m_postfixParts, i);
            k_ASTNodeType_t type = k_ASTNode_getType(postfixPart);

            /* When code written in a statically typed ZVM language is integrated
             * with code written in a dynamically typed ZVM language, such as Zen,
             * special care should be taken. The operand stack is vulnerable to
             * pollution. For example, assume the following function written in a
             * hypothetical ZVM language that is statically typed.
             *
             * int getIndex()
             *     ...
             *
             * Now, consider the following code written in Zen.
             *
             * function main(...arguments)
             *     var index = getIndex()
             *     index += 1
             *
             * When getIndex() function is invoked from Zen, the primitive value
             * is passed around the code. The compound assignment operator causes
             * the invocation of ZenEnvironment.invokeOperator(...) against a primitive
             * value, given the operand stack does not store the type of its entries.
             * Therefore, the compiler of the dynamically typed language should take
             * care of wrapping and unwrapping primitive values to their corresponding
             * wrapper class objects.
             */
            switch (type) {
                case ZEN_AST_NODE_TYPE_SUBSCRIPT: {
                    k_SubscriptContext_t* subscriptContext = (k_SubscriptContext_t*)postfixPart->m_context;
                    k_BinaryEntityGenerator_handleSubscript(generator, (k_SubscriptContext_t*)postfixPart->m_context,
                        i + 1 == postfixPartCount);

                    break;
                }

                case ZEN_AST_NODE_TYPE_FUNCTION_ARGUMENTS: {
                    k_FunctionArgumentsContext_t* functionArgumentsContext =
                        (k_FunctionArgumentsContext_t*)postfixPart->m_context;
                    k_BinaryEntityGenerator_handleDirectFunction(generator,
                        primarySymbol, functionArgumentsContext);

                    break;
                }

                case ZEN_AST_NODE_TYPE_MEMBER_ACCESS: {
                    k_MemberAccessContext_t* memberAccessContext =
                        (k_MemberAccessContext_t*)postfixPart->m_context;

                    /* The primary symbol should be a class for the current member
                     * access to be considered as direct. Otherwise, the expression
                     * `variable.field` will crash the compiler.
                     */
                    if ((i == 0) && !k_Symbol_isVariable(primarySymbol) &&
                        !k_Symbol_isConstant(primarySymbol)) {
                        k_BinaryEntityGenerator_handleDirectAccess(generator,
                            memberAccessContext, primaryToken, primarySymbol,
                            context->m_postfixParts, &i);
                    }
                    else {
                        k_BinaryEntityGenerator_handleDynamicAccess(generator,
                            memberAccessContext, context->m_postfixParts, &i);
                    }

                    break;
                }

                default: {
                    printf("[error] Invalid AST node type %d encountered.\n", type);
                }
            }
        }
    }
}

void k_BinaryEntityGenerator_onEnterPostfixExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_PostfixExpressionContext_t* context = (k_PostfixExpressionContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the generator to
     * emit instructions in an undesirable fashion. Therefore, we partially
     * switch from the listener to visitor design pattern. The AST walker
     * can be guided to switch to this mode via k_ASTListener_skipChildren()
     * function which causes the AST walker to skip iterating over the children
     * nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

// subscript

void k_BinaryEntityGenerator_onEnterSubscript(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitSubscript(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// functionArguments

void k_BinaryEntityGenerator_onEnterFunctionArguments(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitFunctionArguments(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// memberAccess

void k_BinaryEntityGenerator_onEnterMemberAccess(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitMemberAccess(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// postfixOperator

void k_BinaryEntityGenerator_onEnterPostfixOperator(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitPostfixOperator(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// primaryExpression

void k_BinaryEntityGenerator_onEnterPrimaryExpression(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
}

void k_BinaryEntityGenerator_onExitPrimaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// mapExpression

/**
 * load_cpr size ; Push the size of the key array onto the operand stack.
 * new_array_a ; It is more efficient to create a temporary array before creating
 *             ; the hash map. Otherwise, the HashMap#putValue() should be invoked
 *             ; n number of times, where n is the number of the entries.
 *             ; The temporary array created by this instruction will store keys.
 * duplicate ; Duplicate the reference to the temporary key array.
 * load_cpr keyIndex ; Push the index at which the result of the key expression will be stored.
 * (expression) ; Evaluate the result of the key expression.
 * store_aa ; Store the result of the key expression in the temporary key array.
 *
 * load_cpr size ; Push the size of the value array onto the operand stack.
 * new_array_a ; It is more efficient to create a temporary array before creating
 *             ; the hash map. Otherwise, the HashMap#putValue() should be invoked
 *             ; n number of times, where n is the number of the entries.
 *             ; The temporary array created by this instruction will store values.
 * duplicate ; Duplicate the reference to the temporary value array.
 * load_cpr valueIndex ; Push the index at which the result of the value expression will be stored.
 * (expression) ; Evaluate the result of the value expression.
 * store_aa ; Store the result of the value expression in the temporary value array.
 *
 * new classIndex ; Create an instance of the HashMap class.
 * duplicate ; Duplicate the reference of the newly created map.
 * invoke_special functionIndex ; Invoke the constructor to initialize the new map instance.
 */
void k_BinaryEntityGenerator_onEnterMapExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_MapExpressionContext_t* context = (k_MapExpressionContext_t*)node->m_context;
    k_MapEntriesContext_t* entriesContext = (k_MapEntriesContext_t*)context->m_mapEntries->m_context;

    /* Retrieve the size of the map. */
    int32_t size = jtk_ArrayList_getSize(entriesContext->m_mapEntries);

    /* Push the size of the map onto the operand stack. The map size here indicates
     * the size of the temporary key array.
     */
    k_BinaryEntityGenerator_loadInteger(generator, size);

    const uint8_t* objectClassName = "zen/core/Object";
    int32_t objectClassNameSize = 15;
    uint16_t objectClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, objectClassName, objectClassNameSize);

    /* It is more efficient to create a temporary array before creating
     * the hash map. Otherwise, the HashMap#putValue() function
     * should be invoked n number of times, where n is the size of the array.
     *
     * Emit the new_array_a instruction to create the temporary key array.
     */
    k_BinaryEntityBuilder_emitNewReferenceArray(generator->m_builder,
        objectClassIndex);

    /* Log the emission of the new_array_a instruction. */
    jtk_Logger_debug(logger, "Emitted new_array_a %d", objectClassIndex);

    int32_t i;
    for (i = 0; i < size; i++) {
        /* Retrieve the map entry for the current iteration. */
        k_ASTNode_t* mapEntry = (k_ASTNode_t*)jtk_ArrayList_getValue(entriesContext->m_mapEntries, i);

        /* Retrieve the context for the current map entry. */
        k_MapEntryContext_t* mapEntryContext = (k_MapEntryContext_t*)mapEntry->m_context;

        /* Duplicate the reference to the temporary key array. */
        k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

        /* Log the emission of the duplicate instruction. */
        jtk_Logger_debug(logger, "Emitted duplicate");

        /* Push the index at which the result of the key expression will be stored. */
        k_BinaryEntityGenerator_loadInteger(generator, i);


        /* Visit the key expression node and generate the relevant instructions. */
        k_ASTWalker_walk(astListener, mapEntryContext->m_keyExpression);

        /* Store the result in the temporary key array. */
        k_BinaryEntityBuilder_emitStoreArrayReference(generator->m_builder);

        /* Log the emission of the store_aa instruction. */
        jtk_Logger_debug(logger, "Emitted store_aa");
    }

    /* Push the size of the map onto the operand stack. The map size here indicates
     * the size of the temporary value array.
     */
    k_BinaryEntityGenerator_loadInteger(generator, size);

    /* It is more efficient to create a temporary array before creating
     * the hash map. Otherwise, the HashMap#putValue() function
     * should be invoked n number of times, where n is the size of the value
     * array.
     *
     * TODO: Can we optimize map expressions by invoking HashMap#putValue()
     * when the number of entries is less than a certain threshold?
     *
     * Emit the new_array_a instruction to create the temporary array.
     */
    k_BinaryEntityBuilder_emitNewReferenceArray(generator->m_builder,
        objectClassIndex);

    /* Log the emission of the new_array_a instruction. */
    jtk_Logger_debug(logger, "Emitted new_array_a %d", objectClassIndex);

    int32_t j;
    for (j = 0; j < size; j++) {
        /* Retrieve the map entry for the current iteration. */
        k_ASTNode_t* mapEntry = (k_ASTNode_t*)jtk_ArrayList_getValue(entriesContext->m_mapEntries, j);

        /* Retrieve the context for the current map entry. */
        k_MapEntryContext_t* mapEntryContext = (k_MapEntryContext_t*)mapEntry->m_context;

        /* Duplicate the reference to the temporary value array. */
        k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

        /* Log the emission of the duplicate instruction. */
        jtk_Logger_debug(logger, "Emitted duplicate");

        /* Push the index at which the result of the value expression will be stored. */
        k_BinaryEntityGenerator_loadInteger(generator, j);

        /* Visit the value expression node and generate the relevant instructions. */
        k_ASTWalker_walk(astListener, mapEntryContext->m_valueExpression);

        /* Store the result in the temporary value array. */
        k_BinaryEntityBuilder_emitStoreArrayReference(generator->m_builder);

        /* Log the emission of the store_aa instruction. */
        jtk_Logger_debug(logger, "Emitted store_aa");
    }

    const uint8_t* hashMapClassName = "zen/collection/map/HashMap";
    int32_t hashMapClassNameSize = 26;
    uint16_t hashMapClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, hashMapClassName, hashMapClassNameSize);

    /* Create an instance of the HashMap class. */
    k_BinaryEntityBuilder_emitNew(generator->m_builder, hashMapClassIndex);

    /* Log the emission of the new instruction. */
    jtk_Logger_debug(logger, "Emitted new %d", hashMapClassIndex);

    /* Duplicate the reference of the newly created map. */
    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

    /* Log the emission of the duplicate instruction. */
    jtk_Logger_debug(logger, "Emitted duplicate");

    const uint8_t* constructorDescriptor = "v:@(zen/core/Object)@(zen/core/Object)";
    int32_t constructorDescriptorSize = 38;
    const uint8_t* constructorName = "<constructor>";
    int32_t constructorNameSize = 13;
    uint16_t hashMapConstructorIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, hashMapClassName, hashMapClassNameSize,
        constructorDescriptor, constructorDescriptorSize, constructorName,
        constructorNameSize, 0);

    /* Invoke the constructor to initialize the new map instance. */
    k_BinaryEntityBuilder_emitInvokeSpecial(generator->m_builder,
        hashMapConstructorIndex);

    /* Log the emission of the invoke_special instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_special %d", hashMapConstructorIndex);

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitMapExpression(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// mapEntries

void k_BinaryEntityGenerator_onEnterMapEntries(k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    k_MapEntriesContext_t* context = (k_MapEntriesContext_t*)node->m_context;
}

void k_BinaryEntityGenerator_onExitMapEntries(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// mapEntry

void k_BinaryEntityGenerator_onEnterMapEntry(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_BinaryEntityGenerator_onExitMapEntry(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// listExpression

/*
 * load_cpr size ; Push the size of the list onto the operand stack.
 * new_array_a ; It is more efficient to create a temporary array before creating
 *             ; the array list. Otherwise, the ArrayList#setValue() or ArrayList#add() functions
 *             ; should be invoked n number of times, where n is the size of the array.
 *
 * duplicate ; Duplicate the reference to the temporary array.
 * load_cpr index ; Push the index at which the result of the expression will be stored.
 * (expression) ; Evaluate the result of the expression.
 * store_aa ; Store the result in the temporary array.
 * ...
 *
 * new classIndex ; Create an instance of the ArrayList class.
 * duplicate ; Duplicate the reference of the newly created list.
 * invoke_special functionIndex ; Invoke the constructor to initialize the new list instance.
 */
void k_BinaryEntityGenerator_onEnterListExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_ListExpressionContext_t* context = (k_ListExpressionContext_t*)node->m_context;
    k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)context->m_expressions->m_context;

    /* Retrieve the size of the list. */
    int32_t size = jtk_ArrayList_getSize(expressionsContext->m_expressions);

    /* Push the size of the list onto the operand stack. */
    k_BinaryEntityGenerator_loadInteger(generator, size);

    const uint8_t* objectClassName = "zen/core/Object";
    int32_t objectClassNameSize = 15;
    uint16_t objectClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, objectClassName, objectClassNameSize);

    /* It is more efficient to create a temporary array before creating
     * the array list. Otherwise, the ArrayList#setValue() or ArrayList#add() functions
     * should be invoked n number of times, where n is the size of the array.
     *
     * Emit the new_array_a instruction to create the temporary array.
     */
    k_BinaryEntityBuilder_emitNewReferenceArray(generator->m_builder,
        objectClassIndex);

    /* Log the emission of the new_array_a instruction. */
    jtk_Logger_debug(logger, "Emitted new_array_a %d", objectClassIndex);

    int32_t i;
    for (i = 0; i < size; i++) {
        /* Retrieve the expression for the current element. */
        k_ASTNode_t* expression = (k_ASTNode_t*)jtk_ArrayList_getValue(expressionsContext->m_expressions, i);

        /* Duplicate the reference to the temporary array. */
        k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

        /* Log the emission of the duplicate instruction. */
        jtk_Logger_debug(logger, "Emitted duplicate");

        /* Push the index at which the result of the expression will be stored. */
        k_BinaryEntityGenerator_loadInteger(generator, i);

        /* Visit the expression node and generate the relevant instructions. */
        k_ASTWalker_walk(astListener, expression);

        /* Store the result in the temporary array. */
        k_BinaryEntityBuilder_emitStoreArrayReference(generator->m_builder);

        /* Log the emission of the store_aa instruction. */
        jtk_Logger_debug(logger, "Emitted store_aa");
    }

    const uint8_t* arrayListClassName = "zen.collection.list.ArrayList";
    int32_t arrayListClassNameSize = 29;
    uint16_t arrayListClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, arrayListClassName, arrayListClassNameSize);

    /* Create an instance of the ArrayList class. */
    k_BinaryEntityBuilder_emitNew(generator->m_builder, arrayListClassIndex);

    /* Log the emission of the new instruction. */
    jtk_Logger_debug(logger, "Emitted new %d", arrayListClassIndex);

    /* Duplicate the reference of the newly created list. */
    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

    /* Log the emission of the duplicate instruction. */
    jtk_Logger_debug(logger, "Emitted duplicate");

    const uint8_t* constructorDescriptor = "v:@(zen/core/Object)";
    int32_t constructorDescriptorSize = 20;
    const uint8_t* constructorName = "<constructor>";
    int32_t constructorNameSize = 13;
    uint16_t arrayListConstructorIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, arrayListClassName, arrayListClassNameSize,
        constructorDescriptor, constructorDescriptorSize, constructorName,
        constructorNameSize, 0);

    /* Invoke the constructor to initialize the new list instance. */
    k_BinaryEntityBuilder_emitInvokeSpecial(generator->m_builder,
        arrayListConstructorIndex);

    /* Log the emission of the invoke_special instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_special %d", arrayListConstructorIndex);

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitListExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// New Expression

/*
 * new classIndex ; Create an instance of the specified class.
 * duplicate ; Duplicate the reference to the newly created instance.
 * invoke_special functionIndex ; Invoke the constructor to initialize the instance.
 */
void k_BinaryEntityGenerator_onEnterNewExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    // TODO: Debug this function when superclasses are implemented!

    k_BinaryEntityGenerator_t* generator = (k_BinaryEntityGenerator_t*)astListener->m_context;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = generator->m_compiler->m_logger;
    k_NewExpressionContext_t* context = (k_NewExpressionContext_t*)node->m_context;

    /* Retrieve the scope within which the new expression appears. */
    // k_Scope_t* scope = k_SymbolTable_getCurrentScope(generator->m_symbolTable);

    /* Retrieve the string equivalent of the type name node. */
    int32_t typeNameSize;
    uint8_t* typeNameText = k_ASTNode_toCString(context->m_typeName, &typeNameSize);

    /* Resolve the class symbol for the type name. */
    k_Symbol_t* symbol = k_SymbolTable_resolve(generator->m_symbolTable, typeNameText);

    if (symbol == NULL) {
        printf("[error] Undeclared class %s\n", typeNameText);
        printf("[warning] Looks like a resolution phase failure was detected.\n");
    }

    // if (k_Symbol_isExternal(symbol)) {
    //     symbol = symbol->m_context.m_asExternal;
    // }

    if (!k_Symbol_isClass(symbol)) {
        printf("[error] %s is a non-class symbol\n", typeNameText);
        printf("[warning] Looks like the syntactical phase or the resolution phase failed.\n");
    }

    k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;
    /* Retrieve the scope corresponding to the class symbol. */
    k_Scope_t* scope = classSymbol->m_classScope;

    if (!k_Scope_isClassScope(scope)) {
        printf("[error] %s is a non-class scope\n", typeNameText);
        printf("[warning] Looks like the syntactical phase or the resolution phase failed.\n");
    }

    /* Retrieve the constructor declared in this class. */
    k_Symbol_t* constructorSymbol = k_Scope_resolve(scope, "new");

    if (k_Symbol_getEnclosingScope(constructorSymbol) != scope) {
        printf("[error] No constructor defined in class %s, neither explicitly nor implicity.\n", typeNameText);
        printf("[warning] Looks like a resolution phase failure was detected.\n");
    }

    if (!k_Symbol_isFunction(constructorSymbol)) {
        printf("[error] 'new' declared as non-constructor symbol in class %s.\n", typeNameText);
        printf("[warning] Looks like the syntactical phase or the resolution phase failed.\n");
    }

    uint8_t* qualifiedName = jtk_CString_newEx(classSymbol->m_qualifiedName,
        classSymbol->m_qualifiedNameSize);
    /* The binary entity format requires the identifiers of a class to be separated
     * using the forward slash character.
     */
    int32_t i;
    for (i = 0; i < classSymbol->m_qualifiedNameSize; i++) {
        if (qualifiedName[i] == '.') {
            qualifiedName[i] = '/';
        }
    }
    /* Retrieve the class entry index for the type name. */
    uint16_t typeNameIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
        generator->m_constantPoolBuilder, qualifiedName, classSymbol->m_qualifiedNameSize);

    /* Create an instance of the specified class. */
    k_BinaryEntityBuilder_emitNew(generator->m_builder, typeNameIndex);

    /* Log the emission of the new instruction. */
    jtk_Logger_debug(logger, "Emitted new %d", typeNameIndex);

    /* Duplicate the reference of the newly created instance. */
    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

    /* Log the emission of the duplicate instruction. */
    jtk_Logger_debug(logger, "Emitted duplicate");

    k_FunctionSymbol_t* functionSymbol = &constructorSymbol->m_context.m_asFunction;

    uint8_t* constructorName = "<initialize>";
    int32_t constructorNameSize = 12;
    uint8_t* constructorDescriptor0 = NULL;
    const uint8_t* constructorDescriptor = "v:v";
    int32_t constructorDescriptorSize = 3;

    k_ASTNode_t* functionArguments = context->m_functionArguments;
    if (functionArguments != NULL) {
        k_FunctionArgumentsContext_t* functionArgumentsContext
            = (k_FunctionArgumentsContext_t*)functionArguments->m_context;

        if (functionArgumentsContext->m_expressions != NULL) {
            k_ASTNode_t* expressions = functionArgumentsContext->m_expressions;
            k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->m_context;
            int32_t numberOfArguments = jtk_ArrayList_getSize(expressionsContext->m_expressions);
            int32_t parameterThreshold = k_FunctionSymbol_getParameterThreshold(functionSymbol);

            /* NOTE: This function assumes that the previous phases were successful.
             * Therefore, it blindly generates the descriptor of the constructor.
             */
            jtk_StringBuilder_t* builder = jtk_StringBuilder_new();
            jtk_StringBuilder_appendEx_z(builder, "v:", 2);

            int32_t j;
            int32_t numberOfFixedArguments = numberOfArguments;
            int32_t numberOfVariableArguments = 0;

            if (parameterThreshold != -1) {
                numberOfFixedArguments = jtk_Integer_min(numberOfArguments, parameterThreshold);
                numberOfVariableArguments = jtk_Integer_max(0, numberOfArguments - parameterThreshold);
            }

            for (j = 0; j < numberOfFixedArguments; j++) {
                jtk_StringBuilder_appendEx_z(builder, "(zen/core/Object)", 17);

                k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                    expressionsContext->m_expressions, j);
                k_ASTWalker_walk(astListener, argument);
            }

            /* When one of the versions of a function has a variable parameter,
             * then the function has a parameter threshold.
             */
            if ((parameterThreshold != -1) && (numberOfArguments >= parameterThreshold)) {
                // Generate the array for the variable argument.
                jtk_StringBuilder_appendEx_z(builder, "@(zen/core/Object)", 18);

                /* Evaluate the number of the variable arguments. */
                int32_t size = numberOfArguments - parameterThreshold;

                /* Push the size of the list onto the operand stack. */
                k_BinaryEntityGenerator_loadInteger(generator, size);

                const uint8_t* objectClassName = "zen/core/Object";
                int32_t objectClassNameSize = 15;
                uint16_t objectClassIndex = k_ConstantPoolBuilder_getClassEntryIndexEx(
                    generator->m_constantPoolBuilder, objectClassName, objectClassNameSize);

                /* Emit the new_array_a instruction to create an array to
                 * represent the variable arguments.
                 */
                k_BinaryEntityBuilder_emitNewReferenceArray(generator->m_builder,
                    objectClassIndex);

                /* Log the emission of the new_array_a instruction. */
                jtk_Logger_debug(logger, "Emitted new_array_a %d", objectClassIndex);

                while (j < numberOfArguments) {
                    /* Retrieve the expression for the current argument. */
                    k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                        expressionsContext->m_expressions, j);

                    /* Duplicate the reference to the variable argument array. */
                    k_BinaryEntityBuilder_emitDuplicate(generator->m_builder);

                    /* Log the emission of the duplicate instruction. */
                    jtk_Logger_debug(logger, "Emitted duplicate");

                    /* Push the index at which the result of the expression will be stored. */
                    k_BinaryEntityGenerator_loadInteger(generator, j - parameterThreshold);

                    /* Visit the argument expression node and generate the relevant
                     * instructions.
                     */
                    k_ASTWalker_walk(astListener, argument);

                    /* Store the result in the variable argument array. */
                    k_BinaryEntityBuilder_emitStoreArrayReference(generator->m_builder);

                    /* Log the emission of the store_aa instruction. */
                    jtk_Logger_debug(logger, "Emitted store_aa");

                    j++;
                }
            }

            constructorDescriptor = jtk_StringBuilder_toCString(builder, &constructorDescriptorSize);
            constructorDescriptor0 = constructorDescriptor;
            jtk_StringBuilder_delete(builder);
        }
    }

    uint16_t constructorIndex = k_ConstantPoolBuilder_getFunctionEntryIndexEx(
        generator->m_constantPoolBuilder, qualifiedName, classSymbol->m_qualifiedNameSize,
        constructorDescriptor, constructorDescriptorSize, constructorName,
        constructorNameSize, 0);

    /* Invoke the constructor to initialize the new instance. */
    k_BinaryEntityBuilder_emitInvokeSpecial(generator->m_builder,
        constructorIndex);

    /* Log the emission of the invoke_special instruction. */
    jtk_Logger_debug(logger, "Emitted invoke_special %d", constructorIndex);

    if (constructorDescriptor0 != NULL) {
        jtk_CString_delete(constructorDescriptor0);
    }

    // TODO: Generate arrays when variable parameters are encountered!
    /* Delete the type name text. The typeNameText is used to generate the constructor
     * entry in the constant pool. Therefore, we delay its destruction.
     */
    jtk_CString_delete(typeNameText);
    jtk_CString_delete(qualifiedName);

    /* The normal behaviour of the AST walker causes the generator to emit instructions
     * in an undesirable fashion. Therefore, we partially switch from the listener
     * to visitor design pattern. The AST walker can be guided to switch to this
     * mode via k_ASTListener_skipChildren() function which causes the AST walker
     * to skip iterating over the children nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_BinaryEntityGenerator_onExitNewExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// var j = [ 1, 2, 3, 4 ].freeze().clone().add(5).add(5).removeIndex(2)
// var size = [ 1, 2, 3, 4 ].freeze().size

// array(1, 2, 3, 4).reset(2)