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

#include <jtk/core/CString.h>
#include <jtk/collection/stack/LinkedStack.h>

#include <kush/ast/ASTNode.h>
#include <kush/lexer/Token.h>
#include <kush/symbol-table/FunctionSymbol.h>
#include <kush/symbol-table/Analyzer.h>
#include <kush/symbol-table/SymbolTable.h>

/*
 * As new features are implemented, the source code of a project grows in
 * complexity. It becomes hard to maintain the source code when everything is
 * crammed into a single source file. Therefore, an intuitive feature of a
 * programming language is to allow programmers to split their source code
 * into multiple files. The symbols defined in these source files can be imported
 * and exported outside a given source file or compilation unit as needed.
 *
 * In KUSH, the import statement allows a compilation unit to refer to external
 * entities such as functions and classes. Without the use of the import statement,
 * the only way to refer to an entity outside the current compilatiin unit is to
 * use a fully qualified name. Further, all top-level entities declared in KUSH
 * are exported by default. As of this writing, there is no way to override this
 * behaviour.
 *
 * Albeit the import/export mechanism is complicated, it is an interesting topic.
 * The following article describes the import/export mechanism employed by the
 * KUSH compiler behind the scenes.
 *
 * ## Entity Forms
 *
 * A compilation unit can import entities that are either compiled or non-compiled.
 * (Note that the design assumes that the form of an entity is transparent
 * to the end user, i.e., the programmer.)
 *  1. Entities whose equivalent ".feb" files exist on the filsystem in the
 *     entity lookup directories specified to the compiler. These entities are
 *     referred to as compiled entities.
 *  2. Entities that are part of the current compilation batch. These entites
 *     are referred to as non-compiled entities.
 *
 * ## The Global Symbol Cache
 *
 * Given we are dealing with two forms of entities (compiled and non-compiled),
 * the design should allow for an abstraction which allows layers above the
 * import mechanism to work seamlessly without having to worry about an entities
 * form. In order to implement such an abstraction, a central repository known
 * as the global symbol cache is maintained by the compiler.
 *
 * The global symbol cache holds symbols which represent external entities. Here,
 * an external entity refers to any entity that is defined outside a compilation
 * unit. Internally, the global symbol cache uses a hash map to keep track of
 * the registered symbols. The various phases of the compiler can request the
 * global symbol cache to acquire symbols corresponding to entities. However,
 * symbols exported from compilation units that are part of the current compilation
 * batch are not available until the definition phase of the compiler is over.
 * In other words, non-compiled entities are not available until the definition
 * phase is complete.
 *
 * The global symbol cache satifies a symbol request in one of the following ways:
 * 1. If the requested symbol is found in the internal hash map, it is
 *    immediately returned.
 * 2. If the requested symbol is not found in the internal hash map, the cache
 *    searches the entity lookup directories to find a binary entity. If found,
 *    it loads the binary entity using the embedded binary entity loader and a
 *    corresponding symbol is created, inserted in the internal hash map, and
 *    returned. Otherwise, the request fails and null is returned. This allows
 *    the compiler to load external entities from their binary counterpart without
 *    the requesting party to manually deal with external entities.
 *
 *    What about non-compiled external entities? We know that such entities are
 *    part of the current compilation batch. The compiler can take advantage of
 *    this fact. During the definition phase, the compiler registers any symbol
 *    that is considered as an external symbol to the global symbol cache. This allows
 *    compilation units in the current compilation batch to reference symbols
 *    declared in another compilation unit. For example, consider two compilation
 *    units `BirdWatch.KUSH` and `Sparrow.KUSH` in the current compilation batch which
 *    declare the `BirdWatch` and `Sparrow` classes, respectively.
 *    Further, assume that `BirdWatch.KUSH` imports `Sparrow`. During the definition
 *    phase corresponding to `Sparrow.KUSH`, a class symbol for `Sparrow` is registered
 *    in the global cache. This allows `BirdWatch` to refer to the external entity
 *    `Sparrow` which is part of the current compilation batch.
 *
 * ## Phases of the Compiler
 *
 * For multiple input source files, the compiler takes a source file and subjects
 * it to various phases in a parallel sequence, where the phases include, lexical
 * analysis, syntax analysis, semantic analysis (which is divided into the
 * defintion phase and the resolution phase), optimization, and code generation.
 * However, the same flow might not be suitable for the mechanism described here.
 * Why? If you think about it, the previous flow does not provide an oppurutunity
 * to the compiler to register non-compiled entities. In other words, the phases
 * of the compiler are specific  to each compilation unit which prevents exchange
 * of symbols between compilation units.
 *
 * Therefore, in order to accomodate the mechanism described here, the flow of
 * the compiler has to be altered. In the older flow, a source file was not
 * processed until the previous source file was completely processed, i.e.,
 * subjected to all the phases from lexical analysis to code generation. In the
 * new flow, all the input source files are subjected to a single phase before
 * moving on to the next phase. This allows the compiler to mediate the exchange
 * of symbols between compilation units.
 */

k_Analyzer_t* k_Analyzer_new(k_Compiler_t* compiler) {
    k_Analyzer_t* listener = k_Memory_allocate(k_Analyzer_t, 1);
    listener->m_compiler = compiler;
    listener->m_symbolTable = NULL;
    listener->m_package = NULL;
    listener->m_packageSize = -1;

    return listener;
}

void k_Analyzer_delete(k_Analyzer_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    jtk_Memory_deallocate(listener);
}

void k_Analyzer_reset(k_Analyzer_t* listener,
    k_SymbolTable_t* symbolTable, const uint8_t* package, int32_t packageSize) {
    listener->m_symbolTable = symbolTable;
    listener->m_package = package;
    listener->m_packageSize = packageSize;
}

// define

void define(k_Analyzer_t* analyzer, k_CompilationUnit_t* unit) {
    unit->m_scope = k_Scope_forCompilationUnit();
    analyzer->m_scope = unit->m_scope;

    int32_t structureCount = jtk_ArrayList_getSize(unit->m_structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        k_Structure_t* structure = (k_Structure_t*)jtk_ArrayList_getValue(
            unit->m_structures, j);
        defineStructure(analyzer, structure);
    }

    int32_t functionCount = jtk_ArrayList_getSize(unit->m_functions);
    int32_t i;
    for (i = 0; i < functionCount; i++) {
        k_Function_t* function = (k_Function_t*)jtk_ArrayList_getValue(
            unit->m_functions, i);
        defineFunction(analyzer, function);
    }
}

void defineStructure(k_Analyzer_t* analyzer, k_Structure_t* structure) {
    structure->m_parent = analyzer->m_scope;
    structure->m_scope = k_Scope_forStructure(analyzer->m_scope);
    k_Scope_addStructure(analyzer->m_scope, structure);

    int32_t limit = jtk_ArrayList_getSize(declaration->m_variables);
    int32_t i;
    for (i = 0; i < limit; i++) {
        k_Storage_t* storage =
            (k_Storage_t*)jtk_ArrayList_getValue(declaration->m_variables, i);
        declarator->m_parent = structure->m_scope;
        k_Scope_addStorage(structure->m_scope, storage);
    }
}

void defineFunction(k_Analyzer_t* analyzer, k_Function_t* function) {
    function->m_parent = analyzer->m_scope;
    function->m_scope = analyzer->m_scope =
        k_Scope_forFunction(analyzer->m_scope);
    k_Scope_addFunction(analyzer->m_scope, function);

    k_Scope_t* scope = defineLocals(analyzer, function->m_body);
    int32_t parameterCount = jtk_ArrayList_getSize(function->m_parameters);
    int32_t i;
    for (i = 0; i < parameterCount; i++) {
        k_Variable_t* parameter = (k_Storage_t*)jtk_ArrayList_getValue(function->m_parameters, i);
        k_Scope_addVariable(scope, parameter);
    }
}

k_Scope_t* defineLocals(k_Analyzer_t* analzyer, k_BlockStatement_t* block) {
    block->m_scope = analyzer->m_scope = k_Scope_forLocal(analyzer->m_scope);

    int32_t limit = jtk_ArrayList_getSize(block->m_statements);
    int32_t i;
    for (i = 0; i < limit; i++) {
        k_Context_t* context = (k_Context_t*)jtk_ArrayList_getValue(
            block->m_statements, i);
        switch (context->m_type) {
            case K_CONTEXT_BLOCK_STATEMENT: {
                defineLocals(analyzer, (k_BlockStatement_t*)context);
                break;
            }

            case K_CONTEXT_ITERATIVE_STATEMENT: {
                k_IterativeStatement_t* statement = (k_IterativeStatement_t*)context;
                if (statement->m_label != NULL) {
                    jtk_Scope_addLabel(analyzer->m_scope, &statement->m_label);
                }
                if (statement->m_parameter != NULL) {
                    jtk_Scope_addVariable(scope, statement->m_parameter);
                }
                defineLocals(analyzer, statement->m_body);
                break;
            }

            case K_CONTEXT_IF_STATEMENT: {
                k_IfStatement_t* statement = (k_IfStatement_t*)context;
                defineLocals(analyzer, statement->m_ifClause->m_body);
                int32_t count = jtk_ArrayList_getSize(statement->m_elseIfClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_IfClause_t* clause = (k_IfClause_t*)jtk_ArrayList_getValue(
                        statement->m_elseIfClauses, j);
                    defineLocals(analyzer, clause->m_body);
                }
                if (statement->m_elseClause != NULL) {
                    defineLocals(analyzer, statement->m_elseClause);
                }
                break;
            }

            case K_CONTEXT_TRY_STATEMENT: {
                k_TryStatement_t* statement = (k_TryStatement_t*)context;
                defineLocals(analyzer, statement->m_tryClause);

                int32_t count = jtk_ArrayList_getSize(statement->m_catchClauses);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_CatchClause_t* clause = (k_CatchClause_t*)jtk_ArrayList_getValue(
                        statement->m_catchClauses, j);
                    k_Scope_t* scope = defineLocals(clause->m_body);
                    defineVariable(analyzer, scope, clause->m_parameter);
                }

                if (statement->m_finallyClause != NULL) {
                    defineLocals(analyzer, statement->m_finallyClause);
                }

                break;
            }

            case K_CONTEXT_VARIABLE_DECLARATION: {
                k_VariableDeclaration_t* statement = (k_VariableDeclaration_t*)context;
                int32_t count = jtk_ArrayList_getSize(statement->m_variables);
                int32_t j;
                for (j = 0; j < count; j++) {
                    k_Variable_t* variable = (k_Variable_t*)jtk_ArrayList_getValue(
                        statement->m_variables, j);
                    defineVariable(analyzer->m_scope, variable);
                }
                break;
            }
        }
    }

    analyzer->m_scope = analyzer->m_scope->m_parent;
}






#include <jtk/collection/stack/LinkedStack.h>
#include <jtk/collection/array/Arrays.h>
#include <jtk/core/StringBuilder.h>

#include <kush/lexer/Token.h>
#include <kush/ast/ASTNode.h>
#include <kush/ast/ASTWalker.h>
#include <kush/ast/context/Context.h>
#include <kush/symbol-table/Analyzer.h>
#include <kush/symbol-table/Symbol.h>

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

k_Analyzer_t* k_Analyzer_new(
    k_Compiler_t* compiler) {
    k_Analyzer_t* listener = k_Memory_allocate(k_Analyzer_t, 1);
    listener->m_compiler = compiler;
    listener->m_astListener = k_ASTListener_newWithContext(listener);
    listener->m_symbolTable = NULL;
    listener->m_scopes = NULL;
    listener->m_label = KUSH_EXPRESSION_ANNOTATION_UNKNOWN;
    listener->m_functionIndex = 0;
    listener->m_fieldIndex = 0;

    k_ASTListener_t* astListener = listener->m_astListener;

    astListener->m_onVisitErrorNode = k_Analyzer_onVisitErrorNode;
    astListener->m_onVisitTerminal = k_Analyzer_onVisitTerminal;

    astListener->m_onEnterEveryRule = k_Analyzer_onEnterEveryRule;
    astListener->m_onExitEveryRule = k_Analyzer_onExitEveryRule;

    astListener->m_onEnterCompilationUnit = k_Analyzer_onEnterCompilationUnit;
    astListener->m_onExitCompilationUnit = k_Analyzer_onExitCompilationUnit;

    astListener->m_onEnterImportDeclaration = k_Analyzer_onEnterImportDeclaration;
    astListener->m_onExitImportDeclaration = k_Analyzer_onExitImportDeclaration;

    astListener->m_onEnterAnnotatedComponentDeclaration = k_Analyzer_onEnterAnnotatedComponentDeclaration;
    astListener->m_onExitAnnotatedComponentDeclaration = k_Analyzer_onExitAnnotatedComponentDeclaration;

    astListener->m_onEnterAnnotations = k_Analyzer_onEnterAnnotations;
    astListener->m_onExitAnnotations = k_Analyzer_onExitAnnotations;

    astListener->m_onEnterAnnotation = k_Analyzer_onEnterAnnotation;
    astListener->m_onExitAnnotation = k_Analyzer_onExitAnnotation;

    astListener->m_onEnterAnnotationType = k_Analyzer_onEnterAnnotationType;
    astListener->m_onExitAnnotationType = k_Analyzer_onExitAnnotationType;

    astListener->m_onEnterAnnotationAttribute = k_Analyzer_onEnterAnnotationAttribute;
    astListener->m_onExitAnnotationAttribute = k_Analyzer_onExitAnnotationAttribute;

    astListener->m_onEnterComponentDeclaration = k_Analyzer_onEnterComponentDeclaration;
    astListener->m_onExitComponentDeclaration = k_Analyzer_onExitComponentDeclaration;

    astListener->m_onEnterFunctionDeclaration = k_Analyzer_onEnterFunctionDeclaration;
    astListener->m_onExitFunctionDeclaration = k_Analyzer_onExitFunctionDeclaration;

    astListener->m_onEnterFunctionParameters = k_Analyzer_onEnterFunctionParameters;
    astListener->m_onExitFunctionParameters = k_Analyzer_onExitFunctionParameters;

    astListener->m_onEnterFunctionBody = k_Analyzer_onEnterFunctionBody;
    astListener->m_onExitFunctionBody = k_Analyzer_onExitFunctionBody;

    astListener->m_onEnterStatementSuite = k_Analyzer_onEnterStatementSuite;
    astListener->m_onExitStatementSuite = k_Analyzer_onExitStatementSuite;

    astListener->m_onEnterSimpleStatement = k_Analyzer_onEnterSimpleStatement;
    astListener->m_onExitSimpleStatement = k_Analyzer_onExitSimpleStatement;

    astListener->m_onEnterStatement = k_Analyzer_onEnterStatement;
    astListener->m_onExitStatement = k_Analyzer_onExitStatement;

    astListener->m_onEnterEmptyStatement = k_Analyzer_onEnterEmptyStatement;
    astListener->m_onExitEmptyStatement = k_Analyzer_onExitEmptyStatement;

    astListener->m_onEnterVariableDeclaration = k_Analyzer_onEnterVariableDeclaration;
    astListener->m_onExitVariableDeclaration = k_Analyzer_onExitVariableDeclaration;

    astListener->m_onEnterVariableDeclarator = k_Analyzer_onEnterVariableDeclarator;
    astListener->m_onExitVariableDeclarator = k_Analyzer_onExitVariableDeclarator;

    astListener->m_onEnterConstantDeclaration = k_Analyzer_onEnterConstantDeclaration;
    astListener->m_onExitConstantDeclaration = k_Analyzer_onExitConstantDeclaration;

    astListener->m_onEnterConstantDeclarator = k_Analyzer_onEnterConstantDeclarator;
    astListener->m_onExitConstantDeclarator = k_Analyzer_onExitConstantDeclarator;

    astListener->m_onEnterAssertStatement = k_Analyzer_onEnterAssertStatement;
    astListener->m_onExitAssertStatement = k_Analyzer_onExitAssertStatement;

    astListener->m_onEnterBreakStatement = k_Analyzer_onEnterBreakStatement;
    astListener->m_onExitBreakStatement = k_Analyzer_onExitBreakStatement;

    astListener->m_onEnterContinueStatement = k_Analyzer_onEnterContinueStatement;
    astListener->m_onExitContinueStatement = k_Analyzer_onExitContinueStatement;

    astListener->m_onEnterReturnStatement = k_Analyzer_onEnterReturnStatement;
    astListener->m_onExitReturnStatement = k_Analyzer_onExitReturnStatement;

    astListener->m_onEnterThrowStatement = k_Analyzer_onEnterThrowStatement;
    astListener->m_onExitThrowStatement = k_Analyzer_onExitThrowStatement;

    astListener->m_onEnterCompoundStatement = k_Analyzer_onEnterCompoundStatement;
    astListener->m_onExitCompoundStatement = k_Analyzer_onExitCompoundStatement;

    astListener->m_onEnterIfStatement = k_Analyzer_onEnterIfStatement;
    astListener->m_onExitIfStatement = k_Analyzer_onExitIfStatement;

    astListener->m_onEnterIfClause = k_Analyzer_onEnterIfClause;
    astListener->m_onExitIfClause = k_Analyzer_onExitIfClause;

    astListener->m_onEnterElseIfClause = k_Analyzer_onEnterElseIfClause;
    astListener->m_onExitElseIfClause = k_Analyzer_onExitElseIfClause;

    astListener->m_onEnterElseClause = k_Analyzer_onEnterElseClause;
    astListener->m_onExitElseClause = k_Analyzer_onExitElseClause;

    astListener->m_onEnterIterativeStatement = k_Analyzer_onEnterIterativeStatement;
    astListener->m_onExitIterativeStatement = k_Analyzer_onExitIterativeStatement;

    astListener->m_onEnterLabelClause = k_Analyzer_onEnterLabel;
    astListener->m_onExitLabelClause = k_Analyzer_onExitLabel;

    astListener->m_onEnterWhileStatement = k_Analyzer_onEnterWhileStatement;
    astListener->m_onExitWhileStatement = k_Analyzer_onExitWhileStatement;

    astListener->m_onEnterForStatement = k_Analyzer_onEnterForStatement;
    astListener->m_onExitForStatement = k_Analyzer_onExitForStatement;

    astListener->m_onEnterForParameter = k_Analyzer_onEnterForParameters;
    astListener->m_onExitForParameter = k_Analyzer_onExitForParameters;

    astListener->m_onEnterTryStatement = k_Analyzer_onEnterTryStatement;
    astListener->m_onExitTryStatement = k_Analyzer_onExitTryStatement;

    astListener->m_onEnterTryClause = k_Analyzer_onEnterTryClause;
    astListener->m_onExitTryClause = k_Analyzer_onExitTryClause;

    astListener->m_onEnterCatchClause = k_Analyzer_onEnterCatchClause;
    astListener->m_onExitCatchClause = k_Analyzer_onExitCatchClause;

    astListener->m_onEnterCatchFilter = k_Analyzer_onEnterCatchFilter;
    astListener->m_onExitCatchFilter = k_Analyzer_onExitCatchFilter;

    astListener->m_onEnterFinallyClause = k_Analyzer_onEnterFinallyClause;
    astListener->m_onExitFinallyClause = k_Analyzer_onExitFinallyClause;

    astListener->m_onEnterSynchronizeStatement = k_Analyzer_onEnterSynchronizeStatement;
    astListener->m_onExitSynchronizeStatement = k_Analyzer_onExitSynchronizeStatement;

    astListener->m_onEnterWithStatement = k_Analyzer_onEnterWithStatement;
    astListener->m_onExitWithStatement = k_Analyzer_onExitWithStatement;

    astListener->m_onEnterClassDeclaration = k_Analyzer_onEnterClassDeclaration;
    astListener->m_onExitClassDeclaration = k_Analyzer_onExitClassDeclaration;

    astListener->m_onEnterClassExtendsClause = k_Analyzer_onEnterClassExtendsClause;
    astListener->m_onExitClassExtendsClause = k_Analyzer_onExitClassExtendsClause;

    astListener->m_onEnterClassSuite = k_Analyzer_onEnterClassSuite;
    astListener->m_onExitClassSuite = k_Analyzer_onExitClassSuite;

    astListener->m_onEnterClassMember = k_Analyzer_onEnterClassMember;
    astListener->m_onExitClassMember = k_Analyzer_onExitClassMember;

    // astListener->m_onEnterConstructorDeclaration = k_Analyzer_onEnterConstructorDeclaration;
    // astListener->m_onExitConstructorDeclaration = k_Analyzer_onExitConstructorDeclaration;

    astListener->m_onEnterEnumerationDeclaration = k_Analyzer_onEnterEnumerationDeclaration;
    astListener->m_onExitEnumerationDeclaration = k_Analyzer_onExitEnumerationDeclaration;

    astListener->m_onEnterEnumerationBaseClause = k_Analyzer_onEnterEnumrationBaseClass;
    astListener->m_onExitEnumerationBaseClause = k_Analyzer_onExitEnumerationBaseClause;

    astListener->m_onEnterEnumerationSuite = k_Analyzer_onEnterEnumerationSuite;
    astListener->m_onExitEnumerationSuite = k_Analyzer_onExitEnumerationSuite;

    astListener->m_onEnterEnumerate = k_Analyzer_onEnterEnumerate;
    astListener->m_onExitEnumerate = k_Analyzer_onExitEnumerate;

    astListener->m_onEnterExpressions = k_Analyzer_onEnterExpressions;
    astListener->m_onExitExpressions = k_Analyzer_onExitExpressions;

    astListener->m_onEnterExpression = k_Analyzer_onEnterExpression;
    astListener->m_onExitExpression = k_Analyzer_onExitExpression;

    astListener->m_onEnterAssignmentExpression = k_Analyzer_onEnterAssignmentExpression;
    astListener->m_onExitAssignmentExpression = k_Analyzer_onExitAssignmentExpression;

    astListener->m_onEnterConditionalExpression = k_Analyzer_onEnterConditionalExpression;
    astListener->m_onExitConditionalExpression = k_Analyzer_onExitConditionalExpression;

    astListener->m_onEnterLogicalOrExpression = k_Analyzer_onEnterLogicalOrExpression;
    astListener->m_onExitLogicalOrExpression = k_Analyzer_onExitLogicalOrExpression;

    astListener->m_onEnterLogicalAndExpression = k_Analyzer_onEnterLogicalAndExpression;
    astListener->m_onExitLogicalAndExpression = k_Analyzer_onExitLogicalAndExpression;

    astListener->m_onEnterInclusiveOrExpression = k_Analyzer_onEnterInclusiveOrExpression;
    astListener->m_onExitInclusiveOrExpression = k_Analyzer_onExitInclusiveOrExpression;

    astListener->m_onEnterExclusiveOrExpression = k_Analyzer_onEnterExclusiveOrExpression;
    astListener->m_onExitExclusiveOrExpression = k_Analyzer_onExitExclusiveOrExpression;

    astListener->m_onEnterAndExpression = k_Analyzer_onEnterAndExpression;
    astListener->m_onExitAndExpression = k_Analyzer_onExitAndExpression;

    astListener->m_onEnterEqualityExpression = k_Analyzer_onEnterEqualityExpression;
    astListener->m_onExitEqualityExpression = k_Analyzer_onExitEqualityExpression;

    astListener->m_onEnterRelationalExpression = k_Analyzer_onEnterRelationalExpression;
    astListener->m_onExitRelationalExpression = k_Analyzer_onExitRelationalExpression;

    astListener->m_onEnterShiftExpression = k_Analyzer_onEnterShiftExpression;
    astListener->m_onExitShiftExpression = k_Analyzer_onExitShiftExpression;

    astListener->m_onEnterAdditiveExpression = k_Analyzer_onEnterAdditiveExpression;
    astListener->m_onExitAdditiveExpression = k_Analyzer_onExitAdditiveExpression;

    astListener->m_onEnterMultiplicativeExpression = k_Analyzer_onEnterMultiplicativeExpression;
    astListener->m_onExitMultiplicativeExpression = k_Analyzer_onExitMultiplicativeExpression;

    astListener->m_onEnterUnaryExpression = k_Analyzer_onEnterUnaryExpression;
    astListener->m_onExitUnaryExpression = k_Analyzer_onExitUnaryExpression;

    astListener->m_onEnterPostfixExpression = k_Analyzer_onEnterPostfixExpression;
    astListener->m_onExitPostfixExpression = k_Analyzer_onExitPostfixExpression;

    astListener->m_onEnterSubscript = k_Analyzer_onEnterSubscript;
    astListener->m_onExitSubscript = k_Analyzer_onExitSubscript;

    astListener->m_onEnterFunctionArguments = k_Analyzer_onEnterFunctionArguments;
    astListener->m_onExitFunctionArguments = k_Analyzer_onExitFunctionArguments;

    astListener->m_onEnterMemberAccess = k_Analyzer_onEnterMemberAccess;
    astListener->m_onExitMemberAccess = k_Analyzer_onExitMemberAccess;

    astListener->m_onEnterPostfixOperator = k_Analyzer_onEnterPostfixOperator;
    astListener->m_onExitPostfixOperator = k_Analyzer_onExitPostfixOperator;

    astListener->m_onEnterPrimaryExpression = k_Analyzer_onEnterPrimaryExpression;
    astListener->m_onExitPrimaryExpression = k_Analyzer_onExitPrimaryExpression;

    astListener->m_onEnterMapExpression = k_Analyzer_onEnterMapExpression;
    astListener->m_onExitMapExpression = k_Analyzer_onExitMapExpression;

    astListener->m_onEnterMapEntries = k_Analyzer_onEnterMapEntries;
    astListener->m_onExitMapEntries = k_Analyzer_onExitMapEntries;

    astListener->m_onEnterMapEntry = k_Analyzer_onEnterMapEntry;
    astListener->m_onExitMapEntry = k_Analyzer_onExitMapEntry;

    astListener->m_onEnterListExpression = k_Analyzer_onEnterListExpression;
    astListener->m_onExitListExpression = k_Analyzer_onExitListExpression;

    astListener->m_onEnterNewExpression = k_Analyzer_onEnterNewExpression;
    astListener->m_onExitNewExpression = k_Analyzer_onExitNewExpression;

    return listener;
}

void k_Analyzer_delete(k_Analyzer_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    k_ASTListener_delete(listener->m_astListener);
    jtk_Memory_deallocate(listener);
}

k_ASTListener_t* k_Analyzer_getASTListener(k_Analyzer_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");
    return listener->m_astListener;
}

// Reset

void k_Analyzer_reset(k_Analyzer_t* listener,
    k_SymbolTable_t* symbolTable, k_ASTAnnotations_t* scopes) {

    listener->m_symbolTable = symbolTable;
    listener->m_scopes = scopes;
    listener->m_label = KUSH_EXPRESSION_ANNOTATION_UNKNOWN;
    listener->m_fieldIndex = 0;
    listener->m_functionIndex = 0;
}

// Event Handlers

void k_Analyzer_onVisitErrorNode(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    fprintf(stderr, "[warning] Cannot resolve symbols on erroneous AST.\n");
}

void k_Analyzer_onVisitTerminal(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onEnterEveryRule(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitEveryRule(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

/* compilationUnit */

void k_Analyzer_applyDefaultImports(k_Analyzer_t* listener) {
    k_Compiler_t* compiler = listener->m_compiler;
    k_Symbol_t* symbol = k_Compiler_resolveSymbol(compiler, "KUSH.core.KUSHKernel", 18);
    k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;
    k_Scope_t* currentScope = listener->m_symbolTable->m_currentScope;

    jtk_Iterator_t* iterator = jtk_HashMap_getEntryIterator(classSymbol->m_classScope->m_symbols);
    while (jtk_Iterator_hasNext(iterator)) {
        jtk_HashMapEntry_t* entry = (jtk_HashMapEntry_t*)jtk_Iterator_getNext(iterator);
        const uint8_t* name = (const uint8_t*)entry->m_key;
        k_Symbol_t* functionSymbol = (k_Symbol_t*)entry->m_value;
        k_Scope_defineEx(currentScope, name, -1, functionSymbol);
    }
    jtk_Iterator_delete(iterator);
}

void k_Analyzer_onEnterCompilationUnit(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_Scope_t* scope = k_ASTAnnotations_get(listener->m_scopes, node);
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);

    if (!listener->m_compiler->m_coreApi) {
        k_Analyzer_applyDefaultImports(listener);
    }
}

void k_Analyzer_onExitCompilationUnit(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

// importDeclaration

k_Symbol_t* k_Analyzer_createExternalSymbol(
    k_Analyzer_t* listener, k_Symbol_t* symbol,
    k_Scope_t* enclosingScope, k_ASTNode_t* identifier) {
    k_Symbol_t* externalSymbol = jtk_Memory_allocate(k_Symbol_t, 1);
    jtk_Arrays_copyEx_b((const int8_t*)symbol, sizeof (k_Symbol_t), 0,
        (const int8_t*)externalSymbol, sizeof (k_Symbol_t), 0,
        sizeof (k_Symbol_t));
    externalSymbol->m_enclosingScope = enclosingScope;
    externalSymbol->m_identifier = identifier;
    externalSymbol->m_flags |= KUSH_SYMBOL_FLAG_EXTERNAL;

    return externalSymbol;
}

void k_Analyzer_onEnterImportDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_Analyzer_t* listener =
        (k_Analyzer_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ImportDeclarationContext_t* context = (k_ImportDeclarationContext_t*)node->m_context;

    if (context->m_wildcard) {
        // TODO
    }
    else {
        int32_t identifierCount = jtk_ArrayList_getSize(context->m_identifiers);
        k_ASTNode_t* lastIdentifier = jtk_ArrayList_getValue(context->m_identifiers,
            identifierCount - 1);
        k_Token_t* lastIdentifierToken = (k_Token_t*)lastIdentifier->m_context;

        jtk_StringBuilder_t* builder = jtk_StringBuilder_new();
        int32_t i;
        for (i = 0; i < identifierCount; i++) {
            k_ASTNode_t* identifier = (k_ASTNode_t*)jtk_ArrayList_getValue(context->m_identifiers, i);
            k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
            jtk_StringBuilder_appendEx_z(builder, identifierToken->m_text, identifierToken->m_length);
            if (i + 1 < identifierCount) {
                jtk_StringBuilder_append_c(builder, '.');
            }
        }
        int32_t qualifiedNameSize;
        uint8_t* qualifiedName = jtk_StringBuilder_toCString(builder, &qualifiedNameSize);
        jtk_StringBuilder_delete(builder);

        k_Symbol_t* symbol = k_Compiler_resolveSymbol(compiler, qualifiedName,
            qualifiedNameSize);
        if (symbol == NULL) {
            k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                KUSH_ERROR_CODE_UNKNOWN_CLASS, lastIdentifierToken);
        }
        else {
            k_Symbol_t* localSymbol = k_SymbolTable_resolve(listener->m_symbolTable,
                lastIdentifierToken->m_text);
            if (localSymbol == NULL) {
                k_Symbol_t* externalSymbol = k_Symbol_forExternal(lastIdentifier,
                    listener->m_symbolTable->m_currentScope, symbol);
                k_SymbolTable_define(listener->m_symbolTable, externalSymbol);
            }
            else {
                k_ErrorHandler_handleSemanticalError(errorHandler,
                    listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_PREVIOUSLY_IMPORTED,
                    (k_Token_t*)localSymbol->m_identifier->m_context);
            }
        }

        jtk_CString_delete(qualifiedName);
    }
}

void k_Analyzer_onExitImportDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// annotatedComponentDeclaration

void k_Analyzer_onEnterAnnotatedComponentDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitAnnotatedComponentDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// annotations

void k_Analyzer_onEnterAnnotations(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitAnnotations(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// annotation

void k_Analyzer_onEnterAnnotation(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitAnnotation(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// annotationType

void k_Analyzer_onEnterAnnotationType(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitAnnotationType(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// annotationAttribute

void k_Analyzer_onEnterAnnotationAttribute(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitAnnotationAttribute(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// componentDeclaration

void k_Analyzer_onEnterComponentDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitComponentDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onEnterFunctionDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_FunctionDeclarationContext_t* context = (k_FunctionDeclarationContext_t*)node->m_context;

    k_Scope_t* scope = k_ASTAnnotations_get(listener->m_scopes, node);
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
    k_Scope_t* enclosingScope = k_Scope_getEnclosingScope(scope);

    /* Retrieve the identifier associated with the function declaration. */
    k_ASTNode_t* identifier = context->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
    /* Retrieve the text representation of the identifier. */
    const uint8_t* identifierText = ((k_Token_t*)identifier->m_context)->m_text;

    k_FunctionParametersContext_t* functionParametersContext =
        (k_FunctionParametersContext_t*)context->m_functionParameters->m_context;
    /* Retrieve the fixed parameters associated with the context of the
     * function arguments.
     */
    jtk_ArrayList_t* fixedParameters = functionParametersContext->m_fixedParameters;
    int32_t fixedParameterCount = jtk_ArrayList_getSize(fixedParameters);

    /* Retrieve the symbol table associated with this listener. */
    k_SymbolTable_t* symbolTable = listener->m_symbolTable;
    k_Symbol_t* symbol = k_SymbolTable_resolve(symbolTable, identifierText);
    k_FunctionSignature_t* signature = k_Symbol_getFunctionSignature(symbol, fixedParameterCount);
    signature->m_tableIndex = ++listener->m_functionIndex;

    // TODO: In inheritance, the index should be generated after following up
    // the hierarchy to address overriding.

    /* NOTE: Previously a different approach was taken to resolve clashes in
     *       inheritance. The original idea was to apply BFS algorithm against
     *       the inheritance graph. The generated graph tree could be used in
     *       a  matrix to resolve clashes by comparing each class in the
     *       cardinal set. This algorithm is active such that the compiler
     *       quickly detects clashes.
     *
     *       The design of the virtual machine allows interchangeable references
     *       of classes and other symbols without the language barrier. Therefore,
     *       another check should be applied during the resolution of functions
     *       and other symbols.
     *
     *      This introduces two disadvantages to the original algorithm.
     *          - The compiler checks multiple times to detect the same error.
     *          - The complexity of the algorithm is more when compared to latter
     *            algorithm.
     *
     *      Therefore, I am dropping the implementation of the previous algorithm
     *      as it is, which is incomplete.
     */
    /* Resolve member functions with reference to superclasses. */
//    if (k_Scope_isClassScope(enclosingScope)) {
//        k_ASTNode_t* identifier = context->m_identifier;
//        k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
//        const uint8_t* identifierText = k_Token_getText(identifierToken);
//
//        k_ClassScope_t* classScope = (k_ClassSymbol_t*)k_Scope_getContext(enclosingScope);
//        k_ClassSymbol_t* classSymbol = (k_ClassSymbol_t*)classScope->m_classSymbol->m_context;
//
//        k_LinkedStack_t* stack = k_LinkedStack_new();
//
//        jtk_ArrayList_t* superClasses = k_ClassSymbol_getSuperClasses(classSymbol);
//        int32_t size = jtk_ArrayList_getSize(superClasses);
//        int32_t i;
//        for (i = 0; i < size; i++) {
//            k_Symbol_t* superClassSymbol = (k_Symbol_t*)jtk_ArrayList_getValue(superClasses, i);
//            k_ClassSymbol_t* superClassSymbol_ = (k_ClassSymbol_t*)superClassSymbol->m_context;
//            k_LinkedStack_push(stack, superClassSymbol_);
//        }
//
//        /* The function is not resolved against the current scope as it is redundant.
//         * Remember, the declaration phase takes care of this.
//         */
//
//        k_Symbol_t* currentScopeSymbol = (k_Symbol_t*)jtk_HashMap_getValue(classScope->m_symbols, identifierText);
//
//        while (!k_LinkedStack_isEmpty(stack)) {
//            k_ClassSymbol_t* classSymbol = (k_ClassSymbol_t*)k_LinkedStack_pop(stack);
//            k_Scope_t* ts = k_ClassSymbol_getClassScope(classSymbol);
//            classScope = (k_ClassScope_t*)ts->m_context;
//
//            k_Symbol_t* superScopeSymbol = (k_Symbol_t*)jtk_HashMap_getValue(classScope->m_symbols, identifierText);
//            if (superScopeSymbol != NULL) {
//                if (k_Symbol_isFunction(superScopeSymbol)) {
//                    /* Thanks to the declaration phase, we can be sure that the current scope symbol
//                     * is a function.
//                     */
//                    jtk_Assert_assertTrue(k_Symbol_isFunction(currentScopeSymbol), "The declaration phase declared a different symbols with the same identifier.");
//
//                    k_FunctionSymbol_t* currentScopeFunctionSymbol = (k_FunctionSymbol_t*)currentScopeSymbol->m_context;
//                    k_FunctionSymbol_t* superScopeFunctionSymbol = (k_FunctionSymbol_t*)superScopeSymbol->m_context;
//
//                    if ((currentScopeFunctionSymbol->m_parameterThreshold == -1) &&
//                        (superScopeFunctionSymbol->m_parameterThreshold == -1)) {
//                        printf("Either functions were overridden or overloaded.");
//                    }
//                    else if ((currentScopeFunctionSymbol->m_parameterThreshold != -1) &&
//                        (superScopeFunctionSymbol->m_parameterThreshold != -1)) {
//                        if (currentScopeFunctionSymbol->m_parameterThreshold != superScopeFunctionSymbol->m_parameterThreshold) {
//                            /* k_Error_t* error = */ k_ErrorHandler_reportError(NULL, "Multiple overloads with variable parameters across superclass and subclass.", (k_Token_t*)currentScopeSymbol->m_identifier->m_context);
//                            // k_Error_addExtra(error, "Previously valid here");
//                       }
//                    }
//                    else if ((currentScopeFunctionSymbol->m_parameterThreshold != -1) &&
//                        (superScopeFunctionSymbol->m_parameterThreshold == -1)) {
//                        jtk_ArrayList_t* superScopeSignatures = k_FunctionSymbol_getSignatures(superScopeFunctionSymbol);
//                        int32_t superScopeSignatureCount = jtk_ArrayList_getSize(superScopeSignatures);
//                        int32_t j;
//                        for (j = 0; j < superScopeSignatureCount; j++) {
//                            k_FunctionSignature_t* superScopeSignature = (k_FunctionSignature_t*)jtk_ArrayList_getValue(superScopeSignatures, j);
//                            int32_t fixedParameterCount = jtk_ArrayList_getSize(superScopeSignature->m_fixedParameters);
//                            if (fixedParameterCount >= currentScopeFunctionSymbol->m_parameterThreshold) {
//                                k_ErrorHandler_reportError(NULL, "Superclass function exceeds parameter threshold", (k_Token_t*)superScopeSymbol->m_identifier->m_context);
//                            }
//                        }
//                    }
//                    else if ((currentScopeFunctionSymbol->m_parameterThreshold == -1) &&
//                        (superScopeFunctionSymbol->m_parameterThreshold != -1)) {
//                        jtk_ArrayList_t* currentScopeSignatures = k_FunctionSymbol_getSignatures(currentScopeFunctionSymbol);
//                        int32_t currentScopeSignatureCount = jtk_ArrayList_getSize(currentScopeSignatures);
//                        int32_t j;
//                        for (j = 0; j < currentScopeSignatureCount; j++) {
//                            k_FunctionSignature_t* currentScopeSignature = (k_FunctionSignature_t*)jtk_ArrayList_getValue(currentScopeSignatures, j);
//                            int32_t fixedParameterCount = jtk_ArrayList_getSize(currentScopeSignature->m_fixedParameters);
//                            if (fixedParameterCount >= superScopeFunctionSymbol->m_parameterThreshold) {
//                                k_ErrorHandler_reportError(NULL, "Subclass function exceeds parameter threshold", (k_Token_t*)currentScopeSymbol->m_identifier->m_context);
//                            }
//                        }
//                    }
//                }
//            }
//            else {
//                superClasses = k_ClassSymbol_getSuperClasses(classSymbol);
//                size = jtk_ArrayList_getSize(superClasses);
//                for (i = 0; i < size; i++) {
//                    k_Symbol_t* rawSuperClassSymbol = jtk_ArrayList_getValue(superClasses, i);
//                    k_ClassSymbol_t* superClassSymbol = (k_ClassSymbol_t*)rawSuperClassSymbol->m_context;
//                    k_LinkedStack_push(stack, superClassSymbol);
//                }
//            }
//        }
//    }
}

void k_Analyzer_onExitFunctionDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

// functionParameters

void k_Analyzer_onEnterFunctionParameters(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitFunctionParameters(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// functionBody

void k_Analyzer_onEnterFunctionBody(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitFunctionBody(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onEnterStatementSuite(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;

    k_Scope_t* scope = k_ASTAnnotations_get(listener->m_scopes, node);
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
}

void k_Analyzer_onExitStatementSuite(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;

    k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

// simpleStatement

void k_Analyzer_onEnterSimpleStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitSimpleStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// statement

void k_Analyzer_onEnterStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// emptyStatement

void k_Analyzer_onEnterEmptyStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitEmptyStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// variableDeclaration

void k_Analyzer_onEnterVariableDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitVariableDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// variableDeclarator

void k_Analyzer_onEnterVariableDeclarator(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitVariableDeclarator(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// constantDeclaration

void k_Analyzer_onEnterConstantDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitConstantDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// constantDeclarator

void k_Analyzer_onEnterConstantDeclarator(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitConstantDeclarator(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// assertStatement

void k_Analyzer_onEnterAssertStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitAssertStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onEnterBreakStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// breakStatement

void k_Analyzer_onExitBreakStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// continueStatement

void k_Analyzer_onEnterContinueStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitContinueStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// returnStatement

void k_Analyzer_onEnterReturnStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitReturnStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// throwStatement

void k_Analyzer_onEnterThrowStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitThrowStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// compoundStatement

void k_Analyzer_onEnterCompoundStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitCompoundStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// ifStatement

void k_Analyzer_onEnterIfStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitIfStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// ifClause

void k_Analyzer_onEnterIfClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitIfClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// elseIfClause

void k_Analyzer_onEnterElseIfClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitElseIfClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// elseClause

void k_Analyzer_onEnterElseClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitElseClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// iterativeStatement

void k_Analyzer_onEnterIterativeStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitIterativeStatement(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// label

void k_Analyzer_onEnterLabel(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitLabel(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// whileStatement

void k_Analyzer_onEnterWhileStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitWhileStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// forStatement

void k_Analyzer_onEnterForStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitForStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// forParameters

void k_Analyzer_onEnterForParameters(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitForParameters(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// tryStatement

void k_Analyzer_onEnterTryStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_TryStatementContext_t* context = (k_TryStatementContext_t*)node->m_context;

    int32_t catchClauseCount = jtk_ArrayList_getSize(context->m_catchClauses);
    int32_t i;
    for (i = 0; i < catchClauseCount; i++) {
        k_ASTNode_t* catchClause = (k_ASTNode_t*)jtk_ArrayList_getValue(context->m_catchClauses, i);
        k_CatchClauseContext_t* catchClauseContext = (k_CatchClauseContext_t*)catchClause->m_context;

        k_Scope_t* scope = k_ASTAnnotations_get(listener->m_scopes, catchClause);
        k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);

        /* Visit the scopes of the statement suite specified to the
         * catch clause.
         */
        k_ASTWalker_walk(astListener, catchClauseContext->m_statementSuite);

        k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
    }

    k_ASTListener_skipChildren(astListener);
}

void k_Analyzer_onExitTryStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// tryClause

void k_Analyzer_onEnterTryClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitTryClause(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// catchClause

void k_Analyzer_onEnterCatchClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitCatchClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// catchFilter

void k_Analyzer_onEnterCatchFilter(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitCatchFilter(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// finallyClause

void k_Analyzer_onEnterFinallyClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitFinallyClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// synchronizeStatement

void k_Analyzer_onEnterSynchronizeStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitSynchronizeStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// withStatement

void k_Analyzer_onEnterWithStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");
}

void k_Analyzer_onExitWithStatement(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");
}

// classDeclaration

void k_Analyzer_onEnterClassDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    /* Retrieve the instance of the symbol resolution listener. */
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    /* Retrieve the scope associated with the class being declared. */
    k_Scope_t* scope = k_ASTAnnotations_get(listener->m_scopes, node);
    /* If the definition of the class is duplicate, a scope is not associated with
     * it.
     */
    if (scope != NULL) {
        /* Update the current scope in the symbol table. */
        k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);

        k_ClassDeclarationContext_t* classDeclarationContext = (k_ClassDeclarationContext_t*)node->m_context;
        if (classDeclarationContext->m_classExtendsClause != NULL) {
            k_ClassExtendsClauseContext_t* classExtendsClauseContext = classExtendsClauseContext = (k_ClassExtendsClauseContext_t*)classDeclarationContext->m_classExtendsClause->m_context;

            // TODO: Use a node annotation

            k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, k_Token_getText((k_Token_t*)classDeclarationContext->m_identifier->m_context));
            k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;
            jtk_ArrayList_t* superClasses = k_ClassSymbol_getSuperClasses(classSymbol);

            int32_t superClassCount = jtk_ArrayList_getSize(classExtendsClauseContext->m_typeNames);
            int32_t i;
            for (i = 0; i < superClassCount; i++) {
                k_ASTNode_t* typeName = (k_ASTNode_t*)jtk_ArrayList_getValue(classExtendsClauseContext->m_typeNames, i);
                // const uint8_t* qualifiedTypeName = k_Analyzer_getQualifiedTypeName(typeName);
                // TODO!!
                k_TypeNameContext_t* typeNameContext = (k_TypeNameContext_t*)(typeName->m_context);
                jtk_ArrayList_t* identifiers = typeNameContext->m_identifiers;
                k_Token_t* first = ((k_ASTNode_t*)jtk_ArrayList_getValue(identifiers, 0))->m_context;
                const uint8_t* firstText = k_Token_getText(first);
                k_Symbol_t* superClassSymbol = k_SymbolTable_resolve(listener->m_symbolTable, firstText);
                if (superClassSymbol == NULL) {
                    fprintf(stderr, "[semantic error] Unknown class\n");
                }
                else {
                    jtk_ArrayList_add(superClasses, superClassSymbol);
                }
            }
        }

        k_ASTWalker_walk(astListener, classDeclarationContext->m_classSuite);

        k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
    }
    k_ASTListener_skipChildren(astListener);
}

void k_Analyzer_onExitClassDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");
}

// classExtends

void k_Analyzer_onEnterClassExtendsClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitClassExtendsClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// classSuite

void k_Analyzer_onEnterClassSuite(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitClassSuite(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// classMember

void k_Analyzer_onEnterClassMember(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitClassMember(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// void k_Analyzer_onEnterConstructorDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
// }

// void k_Analyzer_onExitConstructorDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
// }

// enumerationDeclaration

void k_Analyzer_onEnterEnumerationDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitEnumerationDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// enumerationBaseClass

void k_Analyzer_onEnterEnumrationBaseClass(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitEnumerationBaseClause(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// enumerationSuite

void k_Analyzer_onEnterEnumerationSuite(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitEnumerationSuite(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// enumerate

void k_Analyzer_onEnterEnumerate(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitEnumerate(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// expressions

void k_Analyzer_onEnterExpressions(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitExpressions(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// expression

void k_Analyzer_onEnterExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_ExpressionContext_t* context = (k_ExpressionContext_t*)node->m_context;

    /* Use the function stack to save the previous label. This is
     * important in the case of nested assignment expressions. Consider
     * the expression `x = (y = z)`, if the previous label is unsaved,
     * the nested assignment expression can alter the label of the outer
     * assignment expression.
     */
    k_ExpressionAnnotation_t previous = listener->m_label;

    k_ASTWalker_walk(astListener, context->m_assignmentExpression);

    /* Restore the previous label saved on the function stack. */
    listener->m_label = previous;

    k_ASTListener_skipChildren(astListener);
}

void k_Analyzer_onExitExpression(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// assignmentExpression

void k_Analyzer_onExitAssignmentExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onEnterAssignmentExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_AssignmentExpressionContext_t* context = (k_AssignmentExpressionContext_t*)node->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;

    k_ASTNode_t* assignmentOperator = context->m_assignmentOperator;
    if (assignmentOperator != NULL) {
        k_ASTWalker_walk(astListener, context->m_conditionalExpression);
        if (listener->m_label == KUSH_EXPRESSION_ANNOTATION_VALUE) {
            k_Token_t* assignmentOperatorToken = (k_Token_t*)assignmentOperator->m_context;
            k_ErrorHandler_handleSemanticalError(errorHandler, listener,
            KUSH_ERROR_CODE_INVALID_LVALUE, assignmentOperatorToken);
        }
        else {
            /* Do not walk through the assignment expression when the left value
             * is invalid.
             */
            k_ASTWalker_walk(astListener, context->m_assignmentExpression);
        }
        k_ASTListener_skipChildren(astListener);
    }
}

// conditionalExpression

void k_Analyzer_onExitConditionalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_ConditionalExpressionContext_t* context = (k_ConditionalExpressionContext_t*)node->m_context;

    if (context->m_thenExpression != NULL) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterConditionalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// logicalOrExpression

void k_Analyzer_onExitLogicalOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_LogicalOrExpressionContext_t* context = (k_LogicalOrExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_logicalAndExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterLogicalOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// logicalAndExpression

void k_Analyzer_onExitLogicalAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_LogicalAndExpressionContext_t* context = (k_LogicalAndExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_inclusiveOrExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterLogicalAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// inclusiveOrExpression

void k_Analyzer_onExitInclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_InclusiveOrExpressionContext_t* context = (k_InclusiveOrExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_exclusiveOrExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterInclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// exclusiveOrExpression

void k_Analyzer_onExitExclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_ExclusiveOrExpressionContext_t* context = (k_ExclusiveOrExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_andExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterExclusiveOrExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// andExpression

void k_Analyzer_onExitAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_AndExpressionContext_t* context = (k_AndExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_equalityExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterAndExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// equalityExpression

void k_Analyzer_onExitEqualityExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_EqualityExpressionContext_t* context = (k_EqualityExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_relationalExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterEqualityExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// relationalExpression

void k_Analyzer_onExitRelationalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_RelationalExpressionContext_t* context = (k_RelationalExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_shiftExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterRelationalExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// shiftExpression

void k_Analyzer_onExitShiftExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_ShiftExpressionContext_t* context = (k_ShiftExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_additiveExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterShiftExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// additiveExpression

void k_Analyzer_onExitAdditiveExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_AdditiveExpressionContext_t* context = (k_AdditiveExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_multiplicativeExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterAdditiveExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// multiplicativeExpression

void k_Analyzer_onExitMultiplicativeExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_MultiplicativeExpressionContext_t* context = (k_MultiplicativeExpressionContext_t*)node->m_context;

    if (!jtk_ArrayList_isEmpty(context->m_unaryExpressions)) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterMultiplicativeExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// unaryExpression

void k_Analyzer_onExitUnaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_UnaryExpressionContext_t* context = (k_UnaryExpressionContext_t*)node->m_context;

    if (context->m_unaryOperator != NULL) {
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }
}

void k_Analyzer_onEnterUnaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// postfixExpression

void k_Analyzer_onEnterPostfixExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_PostfixExpressionContext_t* context = (k_PostfixExpressionContext_t*)node->m_context;

    /* The normal behaviour of the AST walker causes the listener to
     * visit the AST in an undesirable fashion. Therefore, we partially
     * switch from the listener to visitor design pattern. The AST walker
     * can be guided to switch to this mode via k_ASTListener_skipChildren()
     * function which causes the AST walker to skip iterating over the children
     * nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_Analyzer_onExitPostfixExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_PostfixExpressionContext_t* context = (k_PostfixExpressionContext_t*)node->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ASTNode_t* primaryExpression = context->m_primaryExpression;
    k_PrimaryExpressionContext_t* primaryExpressionContext = (k_PrimaryExpressionContext_t*)primaryExpression->m_context;

    k_Symbol_t* primarySymbol = NULL;
    int32_t postfixPartCount = jtk_ArrayList_getSize(context->m_postfixParts);

    k_ASTNode_t* expression = primaryExpressionContext->m_expression;
    k_Token_t* token = NULL;
    /* Check if the primary expression is a literal or an identifier. */
    if (k_ASTNode_isTerminal(expression)) {
        /* Retrieve the token that the primary expression represents. */
        token = (k_Token_t*)expression->m_context;

        switch (k_Token_getType(token)) {
            case KUSH_TOKEN_IDENTIFIER: {
                /* Retrieve the string equivalent to the identifier node. */
                const uint8_t* identifierText = k_Token_getText(token);
                /* Resolve the symbol in the symbol table. */
                k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);

                if (symbol != NULL) {
                    k_Scope_t* enclosingScope = k_Symbol_getEnclosingScope(symbol);
                    if (k_Symbol_isVariable(symbol) || k_Symbol_isConstant(symbol)) {
                        /* Annotate the AST node as placeholder. */
                        listener->m_label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;
                    }
                    else {
                        /* Pass the reference to the symbol to the next phase. */
                        primarySymbol = symbol;
                    }

                    if ((enclosingScope != NULL) && k_Scope_isLocalScope(enclosingScope)) {
                        k_Token_t* symbolToken = (k_Token_t*)symbol->m_identifier->m_context;
                        if (token->m_startIndex <= symbolToken->m_startIndex) {
                            k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                                    KUSH_ERROR_CODE_UNDECLARED_IDENTIFIER, token);
                        }
                    }
                }
                else {
                    k_ErrorHandler_handleSemanticalError(errorHandler, listener,
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
                listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;

                break;
            }
        }
    }
    else if ((expression->m_type == KUSH_AST_NODE_TYPE_MAP_EXPRESSION) ||
        (expression->m_type == KUSH_AST_NODE_TYPE_LIST_EXPRESSION) ||
        (expression->m_type == KUSH_AST_NODE_TYPE_EXPRESSION) ||
        (expression->m_type == KUSH_AST_NODE_TYPE_NEW_EXPRESSION)) {
        k_ASTWalker_walk(astListener, expression);

        /* Annotate the AST node as value. */
        listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;
    }

    int32_t i;
    for (i = 0; i < postfixPartCount; i++) {
        k_ASTNode_t* postfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
            context->m_postfixParts, i);
        k_ASTNodeType_t type = k_ASTNode_getType(postfixPart);

        switch (type) {
            case KUSH_AST_NODE_TYPE_SUBSCRIPT: {
                /* Annotate the AST node as placeholder. */
                listener->m_label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;

                k_SubscriptContext_t* subscriptContext = (k_SubscriptContext_t*)postfixPart->m_context;

                /* Visit the index expression node and analyze. */
                k_ASTWalker_walk(astListener, subscriptContext->m_expression);

                /* The normal behaviour of the AST walker causes the listener to
                 * visit the index in an undesirable fashion. Therefore, we partially
                 * switch from the listener to visitor design pattern. The AST walker
                 * can be guided to switch to this mode via k_ASTListener_skipChildren()
                 * function which causes the AST walker to skip iterating over the children
                 * nodes.
                 */
                k_ASTListener_skipChildren(astListener);

                break;
            }

            case KUSH_AST_NODE_TYPE_FUNCTION_ARGUMENTS: {
                /* Annotate the AST node as value. */
                listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;

                k_FunctionArgumentsContext_t* functionArgumentsContext =
                    (k_FunctionArgumentsContext_t*)postfixPart->m_context;

                if (i == 0) {
                    if (primarySymbol == NULL) {
                        k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                            KUSH_ERROR_CODE_VARIABLE_TREATED_AS_FUNCTION, token);
                    }
                    else if (k_Symbol_isFunction(primarySymbol)) {
                        k_FunctionSymbol_t* functionSymbol = &primarySymbol->m_context.m_asFunction;
                        k_ASTNode_t* expressions = functionArgumentsContext->m_expressions;
                        if (expressions != NULL) {
                            k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->m_context;
                            int32_t argumentCount = jtk_ArrayList_getSize(expressionsContext->m_expressions);
                            if (argumentCount > 0) {
                                int32_t argumentIndex;
                                for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                                    /* Retrieve the expression for the current argument. */
                                    k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                                        expressionsContext->m_expressions, argumentIndex);

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
                    (k_MemberAccessContext_t*)postfixPart->m_context;
                k_ASTNode_t* identifier = memberAccessContext->m_identifier;
                k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

                k_ASTNode_t* functionArguments = NULL;

                /* Annotate the AST node as placeholder. */
                listener->m_label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;

                if ((i + 1) < postfixPartCount) {
                    k_ASTNode_t* nextPostfixPart = (k_ASTNode_t*)jtk_ArrayList_getValue(
                        context->m_postfixParts, i + 1);
                    k_ASTNodeType_t nextPostfixPartType = k_ASTNode_getType(nextPostfixPart);
                    if (nextPostfixPartType == KUSH_AST_NODE_TYPE_FUNCTION_ARGUMENTS) {
                        functionArguments = nextPostfixPart;
                        i++;
                    }
                }

                if (functionArguments != NULL) {
                    /* Annotate the AST node as placeholder. */
                    listener->m_label = KUSH_EXPRESSION_ANNOTATION_VALUE;

                    k_FunctionArgumentsContext_t* functionArgumentsContext =
                        (k_FunctionArgumentsContext_t*)functionArguments->m_context;

                    k_ASTNode_t* expressions = functionArgumentsContext->m_expressions;
                    if (expressions != NULL) {
                        k_ExpressionsContext_t* expressionsContext = (k_ExpressionsContext_t*)expressions->m_context;
                        int32_t argumentCount = jtk_ArrayList_getSize(expressionsContext->m_expressions);
                        if (argumentCount > 0) {
                            int32_t argumentIndex;
                            for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                                /* Retrieve the expression for the current argument. */
                                k_ASTNode_t* argument = (k_ASTNode_t*)jtk_ArrayList_getValue(
                                    expressionsContext->m_expressions, argumentIndex);

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

// subscript

void k_Analyzer_onEnterSubscript(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_ExpressionContext_t* context = (k_ExpressionContext_t*)node->m_context;

    // listener->m_label = KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER;
}

void k_Analyzer_onExitSubscript(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// functionArguments

void k_Analyzer_onEnterFunctionArguments(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitFunctionArguments(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// memberAccess

void k_Analyzer_onEnterMemberAccess(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitMemberAccess(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// postfixOperator

void k_Analyzer_onEnterPostfixOperator(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitPostfixOperator(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// Primary Expression

void k_Analyzer_onEnterPrimaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitPrimaryExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onEnterMapExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitMapExpression(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// mapEntries

void k_Analyzer_onEnterMapEntries(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitMapEntries(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// mapEntry

void k_Analyzer_onEnterMapEntry(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

void k_Analyzer_onExitMapEntry(k_ASTListener_t* astListener, k_ASTNode_t* node) {
}

// listExpression

void k_Analyzer_onEnterListExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

void k_Analyzer_onExitListExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}

// New Expression

void k_Analyzer_onEnterNewExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    // TODO: Debug this function when superclasses are implemented!

    k_Analyzer_t* listener = (k_Analyzer_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    /* Retrieve the logger from the compiler. */
    jtk_Logger_t* logger = compiler->m_logger;

    k_NewExpressionContext_t* context = (k_NewExpressionContext_t*)node->m_context;

    /* Retrieve the scope within which the new expression appears. */
    // k_Scope_t* scope = k_SymbolTable_getCurrentScope(generator->m_symbolTable);

    /* Retrieve the string equivalent of the type name node. */
    int32_t typeNameSize;
    uint8_t* typeNameText = k_ASTNode_toCString(context->m_typeName, &typeNameSize);
    k_TypeNameContext_t* typeName = (k_TypeNameContext_t*)context->m_typeName->m_context;
    int32_t identifierCount = jtk_ArrayList_getSize(typeName->m_identifiers);
    k_ASTNode_t* lastIdentifier = jtk_ArrayList_getValue(typeName->m_identifiers, identifierCount - 1);
    k_Token_t* lastIdentifierToken = (k_Token_t*)lastIdentifier->m_context;

    /* Resolve the class symbol for the type name. */
    k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, typeNameText);

    if (symbol == NULL) {
        k_ErrorHandler_handleSemanticalError(errorHandler, listener,
            KUSH_ERROR_CODE_UNDECLARED_CLASS, lastIdentifierToken);
    }
    else {
        if (k_Symbol_isExternal(symbol)) {
            symbol = symbol->m_context.m_asExternal;
        }

        if (k_Symbol_isClass(symbol)) {
            k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;
            /* Retrieve the scope corresponding to the class symbol. */
            k_Scope_t* scope = classSymbol->m_classScope;
            if (k_Scope_isClassScope(scope)) {
                /* Retrieve the constructor declared in this class. */
                k_Symbol_t* constructorSymbol = k_Scope_resolve(scope, "new");

                if ((constructorSymbol == NULL) ||
                    (k_Symbol_getEnclosingScope(constructorSymbol) != scope)) {
                    k_ErrorHandler_handleSemanticalError(errorHandler, listener,
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
            k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                KUSH_ERROR_CODE_INSTANTIATION_OF_NON_CLASS_SYMBOL, lastIdentifierToken);
        }
    }

    /* Delete the type name text. The typeNameText is used to generate the constructor
     * entry in the constant pool. Therefore, we delay its destruction.
     */
    jtk_CString_delete(typeNameText);
}

void k_Analyzer_onExitNewExpression(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
}
