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

// Friday, November 24, 2017

#ifndef COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_CONTEXT_CONTEXT_H
#define COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_CONTEXT_CONTEXT_H

/*
 * According to my knowledge, I had two options to implement the
 * contexts.
 *
 * 1. A generic class which represents all contexts. Each subrule
 *    or token in the rule is stored in a list. Accessing would
 *    require a linear search. Usually, children of a context do
 *    not exceed more than, say, 10. Thus, the performance would
 *    be affected in a negligible way. Further, each rule is
 *    associated with an helper classes which helps us in the
 *    linear search.
 *    This technique provides a common ground for destruction of
 *    nodes with error checking, abstract syntax tree (AST) walking,
 *    and other such generic manipulative tasks.
 *
 * 2. A custom class represents a particular rule. Each subrule
 *    or token in the rule is stored in its class. Accessing would
 *    not require any special methods. You just access with the
 *    member access operator.
 *
 * Here, I have implemented the latter technique. Yes, I understand the
 * former technique is more robust. Unfortunately, the code base is
 * too big to modify now without hours of typing, compiling, and
 * debugging.
 */

#include <kush/Configuration.h>
#include <kush/ast/context/CompilationUnitContext.h>
#include <kush/ast/context/ImportDeclarationContext.h>
#include <kush/ast/context/AnnotatedComponentDeclarationContext.h>
#include <kush/ast/context/AnnotationContext.h>
#include <kush/ast/context/AnnotationsContext.h>
#include <kush/ast/context/AnnotationTypeContext.h>
#include <kush/ast/context/AnnotationAttributeContext.h>
#include <kush/ast/context/ComponentDeclarationContext.h>
#include <kush/ast/context/FunctionDeclarationContext.h>
#include <kush/ast/context/FunctionParametersContext.h>
#include <kush/ast/context/FunctionBodyContext.h>
#include <kush/ast/context/StatementSuiteContext.h>
#include <kush/ast/context/StatementContext.h>
#include <kush/ast/context/SimpleStatementContext.h>
#include <kush/ast/context/EmptyStatementContext.h>
#include <kush/ast/context/VariableDeclarationContext.h>
#include <kush/ast/context/VariableDeclaratorContext.h>
#include <kush/ast/context/ConstantDeclarationContext.h>
#include <kush/ast/context/ConstantDeclaratorContext.h>
#include <kush/ast/context/AssertStatementContext.h>
#include <kush/ast/context/BreakStatementContext.h>
#include <kush/ast/context/ContinueStatementContext.h>
#include <kush/ast/context/ReturnStatementContext.h>
#include <kush/ast/context/ThrowStatementContext.h>
#include <kush/ast/context/CompoundStatementContext.h>
#include <kush/ast/context/IfStatementContext.h>
#include <kush/ast/context/IfClauseContext.h>
#include <kush/ast/context/ElseIfClauseContext.h>
#include <kush/ast/context/ElseClauseContext.h>
#include <kush/ast/context/IterativeStatementContext.h>
#include <kush/ast/context/LabelClauseContext.h>
#include <kush/ast/context/WhileStatementContext.h>
#include <kush/ast/context/ForStatementContext.h>
#include <kush/ast/context/ForParameterContext.h>
#include <kush/ast/context/TryStatementContext.h>
#include <kush/ast/context/TryClauseContext.h>
#include <kush/ast/context/CatchClauseContext.h>
#include <kush/ast/context/CatchFilterContext.h>
#include <kush/ast/context/FinallyClauseContext.h>
#include <kush/ast/context/SynchronizeStatementContext.h>
#include <kush/ast/context/WithStatementContext.h>
#include <kush/ast/context/WithParametersContext.h>
#include <kush/ast/context/WithParameterContext.h>
#include <kush/ast/context/ClassDeclarationContext.h>
#include <kush/ast/context/ClassExtendsClauseContext.h>
#include <kush/ast/context/ClassSuiteContext.h>
#include <kush/ast/context/ClassMemberContext.h>
#include <kush/ast/context/EnumerationDeclarationContext.h>
#include <kush/ast/context/EnumerationBaseClauseContext.h>
#include <kush/ast/context/EnumerationSuiteContext.h>
#include <kush/ast/context/EnumerateContext.h>
#include <kush/ast/context/ExpressionsContext.h>
#include <kush/ast/context/ExpressionContext.h>
#include <kush/ast/context/AssignmentExpressionContext.h>
#include <kush/ast/context/ConditionalExpressionContext.h>
#include <kush/ast/context/LogicalAndExpressionContext.h>
#include <kush/ast/context/LogicalOrExpressionContext.h>
#include <kush/ast/context/InclusiveOrExpressionContext.h>
#include <kush/ast/context/ExclusiveOrExpressionContext.h>
#include <kush/ast/context/AndExpressionContext.h>
#include <kush/ast/context/EqualityExpressionContext.h>
#include <kush/ast/context/RelationalExpressionContext.h>
#include <kush/ast/context/ShiftExpressionContext.h>
#include <kush/ast/context/AdditiveExpressionContext.h>
#include <kush/ast/context/MultiplicativeExpressionContext.h>
#include <kush/ast/context/UnaryExpressionContext.h>
#include <kush/ast/context/PostfixExpressionContext.h>
#include <kush/ast/context/SubscriptContext.h>
#include <kush/ast/context/FunctionArgumentsContext.h>
#include <kush/ast/context/MemberAccessContext.h>
#include <kush/ast/context/PostfixOperatorContext.h>
#include <kush/ast/context/PrimaryExpressionContext.h>
#include <kush/ast/context/MapExpressionContext.h>
#include <kush/ast/context/MapEntriesContext.h>
#include <kush/ast/context/MapEntryContext.h>
#include <kush/ast/context/ListExpressionContext.h>
#include <kush/ast/context/NewExpressionContext.h>
#include <kush/ast/context/TypeNameContext.h>

#endif /* COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_CONTEXT_CONTEXT_H */

/*******************************************************************************
 * ConstructorDeclarationContext                                               *
 *******************************************************************************/

// struct zen_ConstructorDeclarationContext_t {
//     zen_ASTNode_t* m_node;
//     zen_ASTNode_t* m_identifier;
//     zen_ASTNode_t* m_functionParameters;
//     zen_ASTNode_t* m_statementSuite;
// };

// typedef struct zen_ConstructorDeclarationContext_t zen_ConstructorDeclarationContext_t;

// zen_ConstructorDeclarationContext_t* zen_ConstructorDeclarationContext_new(zen_ASTNode_t* node);
// void zen_ConstructorDeclarationContext_delete(zen_ConstructorDeclarationContext_t* context);
// void zen_ConstructorDeclarationContext_getChildren(zen_ConstructorDeclarationContext_t* context, jtk_ArrayList_t* children);
