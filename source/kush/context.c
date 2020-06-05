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

// Saturday, November 25, 2017

/*
 * The children list expected by the AST walker contains no null elements.
 * The enumeration function filters out nodes which are null and relevant to
 * the grammar. Unfiltered null values appear only within an erroneous
 * context. In order to avoid undefined behaviour due to such contexts,
 * the tree walker distinguishes between erroneously and correctly formed
 * constructs. Where corresponding listener callbacks are invoked for
 * correctly formed constructs, a special callback is invoked for erroneous
 * contexts.
 *
 * The AST destructor anticipates null values in the children list.
 * Thus, appropriately destroys only non-null nodes.
 *
 * All children are instances of k_ASTNode_t. This applies to tokens, too.
 * Tokens are marked as KUSH_AST_NODE_TYPE_TERMINAL.
 */

#include <jtk/core/Object.h>
#include <jtk/collection/Pair.h>

#include <kush/ast/ASTNode.h>
#include <kush/ast/context/Context.h>

/*******************************************************************************
 * Context                                                                     *
 *******************************************************************************/

void k_Context_initialiKUSHode(
    k_ASTNode_t* node,
    k_ASTNodeType_t type,
    void* context,
    k_ContextDestructorFunction_t contextDestructor,
    k_EnumerateContextChildrenFunction_t enumerateContextChildren) {
    node->m_type = type;
    node->m_context = context;
    node->m_contextDestructor = contextDestructor;
    node->m_enumerateContextChildren = enumerateContextChildren;
}

/*******************************************************************************
 * CompilationUnitContext                                                      *
 *******************************************************************************/

k_CompilationUnitContext_t* k_CompilationUnitContext_new(k_ASTNode_t* node) {
    k_CompilationUnitContext_t* context = k_Memory_allocate(k_CompilationUnitContext_t, 1);
    context->m_node = node;
    context->m_importDeclarations = jtk_ArrayList_new();
    context->m_annotatedComponentDeclarations = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_COMPILATION_UNIT, context,
        (k_ContextDestructorFunction_t)k_CompilationUnitContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_CompilationUnitContext_getChildren);

    return context;
}

void k_CompilationUnitContext_delete(k_CompilationUnitContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_importDeclarations);
    jtk_ArrayList_delete(context->m_annotatedComponentDeclarations);
    jtk_Memory_deallocate(context);
}

void k_CompilationUnitContext_getChildren(k_CompilationUnitContext_t* context, jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_importDeclarations);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_annotatedComponentDeclarations);
}

/*******************************************************************************
 * ImportDeclarationContext                                                    *
 *******************************************************************************/

k_ImportDeclarationContext_t* k_ImportDeclarationContext_new(k_ASTNode_t* node) {
    k_ImportDeclarationContext_t* context = k_Memory_allocate(k_ImportDeclarationContext_t, 1);
    context->m_node = node;
    context->m_identifiers = jtk_ArrayList_new();
    context->m_wildcard = false;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_IMPORT_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_ImportDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ImportDeclarationContext_getChildren);

    return context;
}

void k_ImportDeclarationContext_delete(k_ImportDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_identifiers);
    jtk_Memory_deallocate(context);
}

void k_ImportDeclarationContext_getChildren(k_ImportDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_identifiers);
}

/*******************************************************************************
 * AnnotatedComponentDeclaration                                               *
 *******************************************************************************/

k_AnnotatedComponentDeclarationContext_t* k_AnnotatedComponentDeclarationContext_new(k_ASTNode_t* node) {
    k_AnnotatedComponentDeclarationContext_t* context = k_Memory_allocate(k_AnnotatedComponentDeclarationContext_t, 1);
    context->m_node = node;
    context->m_annotations = NULL;
    context->m_componentDeclaration = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ANNOTATED_COMPONENT_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_AnnotatedComponentDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AnnotatedComponentDeclarationContext_getChildren);

    return context;
}

void k_AnnotatedComponentDeclarationContext_delete(k_AnnotatedComponentDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_AnnotatedComponentDeclarationContext_getChildren(k_AnnotatedComponentDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    // if (context->m_annotations != NULL) {
        // jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_annotations);
    // }


    jtk_ArrayList_add(children, context->m_annotations);
    jtk_ArrayList_add(children, context->m_componentDeclaration);
}

/*******************************************************************************
 * AnnotationsContext                                                          *
 *******************************************************************************/

k_AnnotationsContext_t* k_AnnotationsContext_new(k_ASTNode_t* node) {
    k_AnnotationsContext_t* context = k_Memory_allocate(k_AnnotationsContext_t, 1);
    context->m_node = node;
    context->m_annotations = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ANNOTATIONS, context,
        (k_ContextDestructorFunction_t)k_AnnotationsContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AnnotationsContext_getChildren);

    return context;
}

void k_AnnotationsContext_delete(k_AnnotationsContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_annotations);
    jtk_Memory_deallocate(context);
}

void k_AnnotationsContext_getChildren(k_AnnotationsContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_annotations);
}

/*******************************************************************************
 * AnnotationContext                                                           *
 *******************************************************************************/

k_AnnotationContext_t* k_AnnotationContext_new(k_ASTNode_t* node) {
    k_AnnotationContext_t* context = k_Memory_allocate(k_AnnotationContext_t, 1);
    context->m_node = node;
    context->m_annotationType = NULL;
    context->m_annotationAttributes = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ANNOTATION, context,
        (k_ContextDestructorFunction_t)k_AnnotationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AnnotationContext_getChildren);

    return context;
}

void k_AnnotationContext_delete(k_AnnotationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_annotationAttributes);
    jtk_Memory_deallocate(context);
}

void k_AnnotationContext_getChildren(k_AnnotationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_annotationType);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_annotationAttributes);
}

/*******************************************************************************
 * AnnotationTypeContext                                                       *
 *******************************************************************************/

k_AnnotationTypeContext_t* k_AnnotationTypeContext_new(k_ASTNode_t* node) {
    k_AnnotationTypeContext_t* context = k_Memory_allocate(k_AnnotationTypeContext_t, 1);
    context->m_node = node;
    context->m_identifiers = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ANNOTATION_TYPE, context,
        (k_ContextDestructorFunction_t)k_AnnotationTypeContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AnnotationTypeContext_getChildren);

    return context;
}

void k_AnnotationTypeContext_delete(k_AnnotationTypeContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_identifiers);
    jtk_Memory_deallocate(context);
}

void k_AnnotationTypeContext_getChildren(k_AnnotationTypeContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_identifiers);
}

/*******************************************************************************
 * AnnotationAttributeContext                                                  *
 *******************************************************************************/

k_AnnotationAttributeContext_t* k_AnnotationAttributeContext_new(k_ASTNode_t* node) {
    k_AnnotationAttributeContext_t* context = k_Memory_allocate(k_AnnotationAttributeContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;
    context->m_literal = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ANNOTATION_ATTRIBUTE, context,
        (k_ContextDestructorFunction_t)k_AnnotationAttributeContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AnnotationAttributeContext_getChildren);

    return context;
}

void k_AnnotationAttributeContext_delete(k_AnnotationAttributeContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_AnnotationAttributeContext_getChildren(k_AnnotationAttributeContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
    jtk_ArrayList_add(children, context->m_literal);
}

/*******************************************************************************
 * ComponentDeclarationContext                                                 *
 *******************************************************************************/

k_ComponentDeclarationContext_t* k_ComponentDeclarationContext_new(k_ASTNode_t* node) {
    k_ComponentDeclarationContext_t* context = k_Memory_allocate(k_ComponentDeclarationContext_t, 1);
    context->m_node = node;
    context->m_component = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_COMPONENT_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_ComponentDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ComponentDeclarationContext_getChildren);

    return context;
}

void k_ComponentDeclarationContext_delete(k_ComponentDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_ComponentDeclarationContext_getChildren(k_ComponentDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_component);
}

/*******************************************************************************
 * FunctionDeclarationContext                                                  *
 *******************************************************************************/

k_FunctionDeclarationContext_t* k_FunctionDeclarationContext_new(k_ASTNode_t* node) {
    k_FunctionDeclarationContext_t* context = k_Memory_allocate(k_FunctionDeclarationContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;
    context->m_functionParameters = NULL;
    context->m_functionBody = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FUNCTION_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_FunctionDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_FunctionDeclarationContext_getChildren);

    return context;
}

void k_FunctionDeclarationContext_delete(k_FunctionDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_FunctionDeclarationContext_getChildren(k_FunctionDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
    if (context->m_functionParameters != NULL) {
        jtk_ArrayList_add(children, context->m_functionParameters);
    }
    jtk_ArrayList_add(children, context->m_functionBody);
}

/*******************************************************************************
 * FunctionParametersContext                                                   *
 *******************************************************************************/

k_FunctionParametersContext_t* k_FunctionParametersContext_new(k_ASTNode_t* node) {
    k_FunctionParametersContext_t* context = k_Memory_allocate(k_FunctionParametersContext_t, 1);
    context->m_node = node;
    context->m_fixedParameters = jtk_ArrayList_new();
    context->m_variableParameter = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FUNCTION_PARAMETERS, context,
        (k_ContextDestructorFunction_t)k_FunctionParametersContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_FunctionParametersContext_getChildren);

    return context;
}

void k_FunctionParametersContext_delete(k_FunctionParametersContext_t* context) {
    jtk_Assert_assertObject(context, "The specified object is null.");

    jtk_ArrayList_delete(context->m_fixedParameters);
    jtk_Memory_deallocate(context);
}

void k_FunctionParametersContext_getChildren(k_FunctionParametersContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_fixedParameters);
    if (context->m_variableParameter != NULL) {
        jtk_ArrayList_add(children, context->m_variableParameter);
    }
}

/*******************************************************************************
 * FunctionBodyContext                                                         *
 *******************************************************************************/

k_FunctionBodyContext_t* k_FunctionBodyContext_new(k_ASTNode_t* node) {
    k_FunctionBodyContext_t* context = k_Memory_allocate(k_FunctionBodyContext_t, 1);
    context->m_node = node;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FUNCTION_BODY, context,
        (k_ContextDestructorFunction_t)k_FunctionBodyContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_FunctionBodyContext_getChildren);

    return context;
}

void k_FunctionBodyContext_delete(k_FunctionBodyContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_FunctionBodyContext_getChildren(k_FunctionBodyContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * StatementSuiteContext                                                       *
 *******************************************************************************/

k_StatementSuiteContext_t* k_StatementSuiteContext_new(k_ASTNode_t* node) {
    k_StatementSuiteContext_t* context = k_Memory_allocate(k_StatementSuiteContext_t, 1);
    context->m_node = node;
    // context->m_simpleStatement = NULL;
    context->m_statements = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_STATEMENT_SUITE, context,
        (k_ContextDestructorFunction_t)k_StatementSuiteContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_StatementSuiteContext_getChildren);

    return context;
}

void k_StatementSuiteContext_delete(k_StatementSuiteContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_statements);

    jtk_Memory_deallocate(context);
}

void k_StatementSuiteContext_getChildren(k_StatementSuiteContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    /*if (context->m_simpleStatement != NULL) {
        jtk_ArrayList_add(children, context->m_simpleStatement);
    }
    else {*/
        jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_statements);
    /* } */
}

/*******************************************************************************
 * SimpleStatementContext                                                      *
 *******************************************************************************/

k_SimpleStatementContext_t* k_SimpleStatementContext_new(k_ASTNode_t* node) {
    k_SimpleStatementContext_t* context = k_Memory_allocate(k_SimpleStatementContext_t, 1);
    context->m_node = node;
    context->m_statement = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_SIMPLE_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_SimpleStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_SimpleStatementContext_getChildren);

    return context;
}

void k_SimpleStatementContext_delete(k_SimpleStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_SimpleStatementContext_getChildren(k_SimpleStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_statement);
}

/*******************************************************************************
 * StatementContext                                                            *
 *******************************************************************************/

k_StatementContext_t* k_StatementContext_new(k_ASTNode_t* node) {
    k_StatementContext_t* context = k_Memory_allocate(k_StatementContext_t, 1);
    context->m_node = node;
    context->m_simpleStatement = NULL;
    context->m_compoundStatement = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_StatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_StatementContext_getChildren);

    return context;
}

void k_StatementContext_delete(k_StatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_StatementContext_getChildren(k_StatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_simpleStatement != NULL) {
        jtk_ArrayList_add(children, context->m_simpleStatement);
    }
    else {
        jtk_ArrayList_add(children, context->m_compoundStatement);
    }
}

/*******************************************************************************
 * EmptyStatementContext                                                       *
 *******************************************************************************/

k_EmptyStatementContext_t* k_EmptyStatementContext_new(k_ASTNode_t* node) {
    k_EmptyStatementContext_t* context = k_Memory_allocate(k_EmptyStatementContext_t, 1);
    context->m_node = node;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_EMPTY_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_EmptyStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_EmptyStatementContext_getChildren);

    return context;
}

void k_EmptyStatementContext_delete(k_EmptyStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_EmptyStatementContext_getChildren(k_EmptyStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");
}

/*******************************************************************************
 * VariableDeclarationContext                                                  *
 *******************************************************************************/

/*
 * TODO: I have accidentally written k_*Context_new() where I am supposed
 * to write k_Memory_allocate(k_*Context_t, 1). Please check for such
 * bugs.
 */
k_VariableDeclarationContext_t* k_VariableDeclarationContext_new(k_ASTNode_t* node) {
    k_VariableDeclarationContext_t* context = k_Memory_allocate(k_VariableDeclarationContext_t, 1);
    context->m_node = node;
    context->m_variableDeclarators = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_VARIABLE_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_VariableDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_VariableDeclarationContext_getChildren);

    return context;
}

void k_VariableDeclarationContext_delete(k_VariableDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_variableDeclarators);
    jtk_Memory_deallocate(context);
}

/*
 * TODO: I have accidentally written add() instead of addAll(). Please look
 * for such bugs.
 */
void k_VariableDeclarationContext_getChildren(k_VariableDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_variableDeclarators);
}

/*******************************************************************************
 * VariableDeclaratorContext                                                   *
 *******************************************************************************/

/*
 * TODO: Check if the constructors are returning.
 */
k_VariableDeclaratorContext_t* k_VariableDeclaratorContext_new(k_ASTNode_t* node) {
    k_VariableDeclaratorContext_t* context = k_Memory_allocate(k_VariableDeclaratorContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_VARIABLE_DECLARATOR, context,
        (k_ContextDestructorFunction_t)k_VariableDeclaratorContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_VariableDeclaratorContext_getChildren);

    return context;
}

void k_VariableDeclaratorContext_delete(k_VariableDeclaratorContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_VariableDeclaratorContext_getChildren(k_VariableDeclaratorContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
    if (context->m_expression != NULL) {
        jtk_ArrayList_add(children, context->m_expression);
    }
}

/*******************************************************************************
 * ConstantDeclarationContext                                                  *
 *******************************************************************************/

k_ConstantDeclarationContext_t* k_ConstantDeclarationContext_new(k_ASTNode_t* node) {
    k_ConstantDeclarationContext_t* context = k_Memory_allocate(k_ConstantDeclarationContext_t, 1);
    context->m_node = node;
    context->m_constantDeclarators = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CONSTANT_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_ConstantDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ConstantDeclarationContext_getChildren);

    return context;
}

void k_ConstantDeclarationContext_delete(k_ConstantDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_constantDeclarators);
    jtk_Memory_deallocate(context);
}

void k_ConstantDeclarationContext_getChildren(k_ConstantDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_constantDeclarators);
}

/*******************************************************************************
 * ConstantDeclaratorContext                                                   *
 *******************************************************************************/

k_ConstantDeclaratorContext_t* k_ConstantDeclaratorContext_new(k_ASTNode_t* node) {
    k_ConstantDeclaratorContext_t* context = k_Memory_allocate(k_ConstantDeclaratorContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CONSTANT_DECLARATOR, context,
        (k_ContextDestructorFunction_t)k_ConstantDeclaratorContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ConstantDeclaratorContext_getChildren);

    return context;
}

void k_ConstantDeclaratorContext_delete(k_ConstantDeclaratorContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ConstantDeclaratorContext_getChildren(k_ConstantDeclaratorContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
    jtk_ArrayList_add(children, context->m_expression);
}

/*******************************************************************************
 * AssertStatementContext                                                      *
 *******************************************************************************/

k_AssertStatementContext_t* k_AssertStatementContext_new(k_ASTNode_t* node) {
    k_AssertStatementContext_t* context = k_Memory_allocate(k_AssertStatementContext_t, 1);
    context->m_node = node;
    context->m_conditionExpression = NULL;
    context->m_messageExpression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ASSERT_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_AssertStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AssertStatementContext_getChildren);

    return context;
}

void k_AssertStatementContext_delete(k_AssertStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_AssertStatementContext_getChildren(k_AssertStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_conditionExpression);
    jtk_ArrayList_add(children, context->m_messageExpression);
}

/*******************************************************************************
 * BreakStatementContext                                                       *
 *******************************************************************************/

k_BreakStatementContext_t* k_BreakStatementContext_new(k_ASTNode_t* node) {
    k_BreakStatementContext_t* context = k_Memory_allocate(k_BreakStatementContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_BREAK_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_BreakStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_BreakStatementContext_getChildren);

    return context;
}

void k_BreakStatementContext_delete(k_BreakStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_BreakStatementContext_getChildren(k_BreakStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_identifier != NULL) {
        jtk_ArrayList_add(children, context->m_identifier);
    }
}

/*******************************************************************************
 * ContinueStatementContext                                                    *
 *******************************************************************************/

k_ContinueStatementContext_t* k_ContinueStatementContext_new(k_ASTNode_t* node) {
    k_ContinueStatementContext_t* context = k_Memory_allocate(k_ContinueStatementContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CONTINUE_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_ContinueStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ContinueStatementContext_getChildren);

    return context;
}

void k_ContinueStatementContext_delete(k_ContinueStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_ContinueStatementContext_getChildren(k_ContinueStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_identifier) {
        jtk_ArrayList_add(children, context->m_identifier);
    }
}

/*******************************************************************************
 * ReturnStatementContext                                                      *
 *******************************************************************************/

k_ReturnStatementContext_t* k_ReturnStatementContext_new(k_ASTNode_t* node) {
    k_ReturnStatementContext_t* context = k_Memory_allocate(k_ReturnStatementContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_RETURN_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_ReturnStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ReturnStatementContext_getChildren);

    return context;
}

void k_ReturnStatementContext_delete(k_ReturnStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ReturnStatementContext_getChildren(k_ReturnStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_expression != NULL) {
        jtk_ArrayList_add(children, context->m_expression);
    }
}

/*******************************************************************************
 * ThrowStatementContext                                                       *
 *******************************************************************************/

k_ThrowStatementContext_t* k_ThrowStatementContext_new(k_ASTNode_t* node) {
    k_ThrowStatementContext_t* context = k_Memory_allocate(k_ThrowStatementContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_THROW_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_ThrowStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ThrowStatementContext_getChildren);

    return context;
}

void k_ThrowStatementContext_delete(k_ThrowStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ThrowStatementContext_getChildren(k_ThrowStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_expression != NULL) {
        jtk_ArrayList_add(children, context->m_expression);
    }
}

/*******************************************************************************
 * CompoundStatementContext                                                    *
 *******************************************************************************/

k_CompoundStatementContext_t* k_CompoundStatementContext_new(k_ASTNode_t* node) {
    k_CompoundStatementContext_t* context = k_Memory_allocate(k_CompoundStatementContext_t, 1);
    context->m_node = node;
    context->m_statement = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_COMPOUND_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_CompoundStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_CompoundStatementContext_getChildren);

    return context;
}

void k_CompoundStatementContext_delete(k_CompoundStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_CompoundStatementContext_getChildren(k_CompoundStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_statement);
}

/*******************************************************************************
 * IfStatementContext                                                          *
 *******************************************************************************/

k_IfStatementContext_t* k_IfStatementContext_new(k_ASTNode_t* node) {
    k_IfStatementContext_t* context = k_Memory_allocate(k_IfStatementContext_t, 1);
    context->m_node = node;
    context->m_ifClause = NULL;
    context->m_elseIfClauses = jtk_ArrayList_new();
    context->m_elseClause = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_IF_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_IfStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_IfStatementContext_getChildren);

    return context;
}

/*
 * TODO: At places, I have forgotten to delete the context. Please fix
 *       such bugs.
 */
void k_IfStatementContext_delete(k_IfStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_elseIfClauses);
    jtk_Memory_deallocate(context);
}

void k_IfStatementContext_getChildren(k_IfStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_ifClause);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_elseIfClauses);
    if (context->m_elseClause != NULL) {
        jtk_ArrayList_add(children, context->m_elseClause);
    }
}

/*******************************************************************************
 * IfClauseContext                                                             *
 *******************************************************************************/

k_IfClauseContext_t* k_IfClauseContext_new(k_ASTNode_t* node) {
    k_IfClauseContext_t* context = k_Memory_allocate(k_IfClauseContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_IF_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_IfClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_IfClauseContext_getChildren);

    return context;
}

void k_IfClauseContext_delete(k_IfClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_IfClauseContext_getChildren(k_IfClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expression);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * ElseIfClauseContext                                                         *
 *******************************************************************************/

k_ElseIfClauseContext_t* k_ElseIfClauseContext_new(k_ASTNode_t* node) {
    k_ElseIfClauseContext_t* context = k_Memory_allocate(k_ElseIfClauseContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ELSE_IF_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_ElseIfClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ElseIfClauseContext_getChildren);

    return context;
}

void k_ElseIfClauseContext_delete(k_ElseIfClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ElseIfClauseContext_getChildren(k_ElseIfClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expression);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * ElseClauseContext                                                           *
 *******************************************************************************/

k_ElseClauseContext_t* k_ElseClauseContext_new(k_ASTNode_t* node) {
    k_ElseClauseContext_t* context = k_Memory_allocate(k_ElseClauseContext_t, 1);
    context->m_node = node;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ELSE_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_ElseClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ElseClauseContext_getChildren);

    return context;
}

void k_ElseClauseContext_delete(k_ElseClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ElseClauseContext_getChildren(k_ElseClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * IterativeStatementContext                                                   *
 *******************************************************************************/

k_IterativeStatementContext_t* k_IterativeStatementContext_new(k_ASTNode_t* node) {
    k_IterativeStatementContext_t* context = k_Memory_allocate(k_IterativeStatementContext_t, 1);
    context->m_node = node;
    context->m_labelClause = NULL;
    context->m_statement = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ITERATIVE_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_IterativeStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_IterativeStatementContext_getChildren);

    return context;
}

void k_IterativeStatementContext_delete(k_IterativeStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_IterativeStatementContext_getChildren(k_IterativeStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_labelClause != NULL) {
        jtk_ArrayList_add(children, context->m_labelClause);
    }
    jtk_ArrayList_add(children, context->m_statement);
}

/*******************************************************************************
 * LabelClauseContext                                                          *
 *******************************************************************************/

k_LabelClauseContext_t* k_LabelClauseContext_new(k_ASTNode_t* node) {
    k_LabelClauseContext_t* context = k_Memory_allocate(k_LabelClauseContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_LABEL_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_LabelClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_LabelClauseContext_getChildren);

    return context;
}

void k_LabelClauseContext_delete(k_LabelClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_LabelClauseContext_getChildren(k_LabelClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
}

/*******************************************************************************
 * WhileStatementContext                                                       *
 *******************************************************************************/

k_WhileStatementContext_t* k_WhileStatementContext_new(k_ASTNode_t* node) {
    k_WhileStatementContext_t* context = k_Memory_allocate(k_WhileStatementContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_WHILE_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_WhileStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_WhileStatementContext_getChildren);

    return context;
}

void k_WhileStatementContext_delete(k_WhileStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_WhileStatementContext_getChildren(k_WhileStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expression);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * ForStatementContext                                                         *
 *******************************************************************************/

k_ForStatementContext_t* k_ForStatementContext_new(k_ASTNode_t* node) {
    k_ForStatementContext_t* context = k_Memory_allocate(k_ForStatementContext_t, 1);
    context->m_node = node;
    context->m_forParameter = NULL;
    context->m_expression = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FOR_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_ForStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ForStatementContext_getChildren);

    return context;
}

void k_ForStatementContext_delete(k_ForStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ForStatementContext_getChildren(k_ForStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_forParameter);
    jtk_ArrayList_add(children, context->m_expression);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * ForParameterContext                                                         *
 *******************************************************************************/

k_ForParameterContext_t* k_ForParameterContext_new(k_ASTNode_t* node) {
    k_ForParameterContext_t* context = k_Memory_allocate(k_ForParameterContext_t, 1);
    context->m_node = node;
    context->m_declaration = false;
    context->m_variable = false;
    context->m_identifier = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FOR_PARAMETER, context,
        (k_ContextDestructorFunction_t)k_ForParameterContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ForParameterContext_getChildren);

    return context;
}

void k_ForParameterContext_delete(k_ForParameterContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ForParameterContext_getChildren(k_ForParameterContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_add(children, context->m_identifier);
}

/*******************************************************************************
 * TryStatementContext                                                         *
 *******************************************************************************/

k_TryStatementContext_t* k_TryStatementContext_new(k_ASTNode_t* node) {
    k_TryStatementContext_t* context = k_Memory_allocate(k_TryStatementContext_t, 1);
    context->m_node = node;
    context->m_tryClause = NULL;
    context->m_catchClauses = jtk_ArrayList_new();
    context->m_finallyClause = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_TRY_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_TryStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_TryStatementContext_getChildren);

    return context;
}

void k_TryStatementContext_delete(k_TryStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_catchClauses);
    jtk_Memory_deallocate(context);
}

void k_TryStatementContext_getChildren(k_TryStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_tryClause);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_catchClauses);
    if (context->m_finallyClause != NULL) {
        jtk_ArrayList_add(children, context->m_finallyClause);
    }
}

/*******************************************************************************
 * TryClauseContext                                                            *
 *******************************************************************************/

k_TryClauseContext_t* k_TryClauseContext_new(k_ASTNode_t* node) {
    k_TryClauseContext_t* context = k_Memory_allocate(k_TryClauseContext_t, 1);
    context->m_node = node;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_TRY_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_TryClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_TryClauseContext_getChildren);

    return context;
}

void k_TryClauseContext_delete(k_TryClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_TryClauseContext_getChildren(k_TryClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * CatchClauseContext                                                          *
 *******************************************************************************/

k_CatchClauseContext_t* k_CatchClauseContext_new(k_ASTNode_t* node) {
    k_CatchClauseContext_t* context = k_Memory_allocate(k_CatchClauseContext_t, 1);
    context->m_node = node;
    context->m_catchFilter = NULL;
    context->m_identifier = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CATCH_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_CatchClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_CatchClauseContext_getChildren);

    return context;
}

void k_CatchClauseContext_delete(k_CatchClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_CatchClauseContext_getChildren(k_CatchClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_catchFilter);
    jtk_ArrayList_add(children, context->m_identifier);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * CatchFilterContext                                                          *
 *******************************************************************************/

k_CatchFilterContext_t* k_CatchFilterContext_new(k_ASTNode_t* node) {
    k_CatchFilterContext_t* context = k_Memory_allocate(k_CatchFilterContext_t, 1);
    context->m_node = node;
    context->m_typeNames = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CATCH_FILTER, context,
        (k_ContextDestructorFunction_t)k_CatchFilterContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_CatchFilterContext_getChildren);

    return context;
}

void k_CatchFilterContext_delete(k_CatchFilterContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_typeNames);
    jtk_Memory_deallocate(context);
}

void k_CatchFilterContext_getChildren(k_CatchFilterContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_typeNames);
}

/*******************************************************************************
 * TypeNameContext                                                             *
 *******************************************************************************/

k_TypeNameContext_t* k_TypeNameContext_new(k_ASTNode_t* node) {
    k_TypeNameContext_t* context = k_Memory_allocate(k_TypeNameContext_t, 1);
    context->m_node = node;
    context->m_identifiers = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_TYPE_NAME, context,
        (k_ContextDestructorFunction_t)k_TypeNameContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_TypeNameContext_getChildren);

    return context;
}

void k_TypeNameContext_delete(k_TypeNameContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_identifiers);
    jtk_Memory_deallocate(context);
}

void k_TypeNameContext_getChildren(k_TypeNameContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_identifiers);
}

/*******************************************************************************
 * FinallyClauseContext                                                        *
 *******************************************************************************/

k_FinallyClauseContext_t* k_FinallyClauseContext_new(k_ASTNode_t* node) {
    k_FinallyClauseContext_t* context = k_Memory_allocate(k_FinallyClauseContext_t, 1);
    context->m_node = node;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FINALLY_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_FinallyClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_FinallyClauseContext_getChildren);

    return context;
}

void k_FinallyClauseContext_delete(k_FinallyClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_FinallyClauseContext_getChildren(k_FinallyClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * SynchronizeStatementContext                                                 *
 *******************************************************************************/

k_SynchronizeStatementContext_t* k_SynchronizeStatementContext_new(k_ASTNode_t* node) {
    k_SynchronizeStatementContext_t* context = k_Memory_allocate(k_SynchronizeStatementContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_SYNCHRONIZE_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_SynchronizeStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_SynchronizeStatementContext_getChildren);

    return context;
}

void k_SynchronizeStatementContext_delete(k_SynchronizeStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_SynchronizeStatementContext_getChildren(k_SynchronizeStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expression);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * WithStatementContext                                                        *
 *******************************************************************************/

k_WithStatementContext_t* k_WithStatementContext_new(k_ASTNode_t* node) {
    k_WithStatementContext_t* context = k_Memory_allocate(k_WithStatementContext_t, 1);
    context->m_node = node;
    context->m_withParameters = NULL;
    context->m_statementSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_WITH_STATEMENT, context,
        (k_ContextDestructorFunction_t)k_WithStatementContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_WithStatementContext_getChildren);

    return context;
}

void k_WithStatementContext_delete(k_WithStatementContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_WithStatementContext_getChildren(k_WithStatementContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_withParameters);
    jtk_ArrayList_add(children, context->m_statementSuite);
}

/*******************************************************************************
 * WithParametersContext                                                       *
 *******************************************************************************/

k_WithParametersContext_t* k_WithParametersContext_new(k_ASTNode_t* node) {
    k_WithParametersContext_t* context = k_Memory_allocate(k_WithParametersContext_t, 1);
    context->m_node = node;
    context->m_withParameters = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_WITH_PARAMETERS, context,
        (k_ContextDestructorFunction_t)k_WithParametersContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_WithParametersContext_getChildren);

    return context;
}

void k_WithParametersContext_delete(k_WithParametersContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_withParameters);
    jtk_Memory_deallocate(context);
}

void k_WithParametersContext_getChildren(k_WithParametersContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_withParameters);
}

/*******************************************************************************
 * WithParameterContext                                                        *
 *******************************************************************************/

k_WithParameterContext_t* k_WithParameterContext_new(k_ASTNode_t* node) {
    k_WithParameterContext_t* context = k_Memory_allocate(k_WithParameterContext_t, 1);
    context->m_node = node;
    context->m_variable = true;
    context->m_identifier = NULL;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_WITH_PARAMETER, context,
        (k_ContextDestructorFunction_t)k_WithParameterContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_WithParameterContext_getChildren);

    return context;
}

void k_WithParameterContext_delete(k_WithParameterContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_WithParameterContext_getChildren(k_WithParameterContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
    jtk_ArrayList_add(children, context->m_expression);
}

/*******************************************************************************
 * ClassDeclarationContext                                                     *
 *******************************************************************************/

k_ClassDeclarationContext_t* k_ClassDeclarationContext_new(k_ASTNode_t* node) {
    k_ClassDeclarationContext_t* context = k_Memory_allocate(k_ClassDeclarationContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;
    context->m_classExtendsClause = NULL;
    context->m_classSuite = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CLASS_DECLARATION, context,
        (k_ContextDestructorFunction_t)k_ClassDeclarationContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ClassDeclarationContext_getChildren);

    return context;
}

void k_ClassDeclarationContext_delete(k_ClassDeclarationContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ClassDeclarationContext_getChildren(k_ClassDeclarationContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
    if (context->m_classExtendsClause != NULL) {
        jtk_ArrayList_add(children, context->m_classExtendsClause);
    }
    jtk_ArrayList_add(children, context->m_classSuite);
}

/*******************************************************************************
 * ClassExtendsClauseContext                                                        *
 *******************************************************************************/

k_ClassExtendsClauseContext_t* k_ClassExtendsClauseContext_new(k_ASTNode_t* node) {
    k_ClassExtendsClauseContext_t* context = k_Memory_allocate(k_ClassExtendsClauseContext_t, 1);
    context->m_node = node;
    context->m_typeNames = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_EXTENDS_CLAUSE, context,
        (k_ContextDestructorFunction_t)k_ClassExtendsClauseContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ClassExtendsClauseContext_getChildren);

    return context;
}

void k_ClassExtendsClauseContext_delete(k_ClassExtendsClauseContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_typeNames);
    jtk_Memory_deallocate(context);
}

void k_ClassExtendsClauseContext_getChildren(k_ClassExtendsClauseContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_typeNames);
}

/*******************************************************************************
 * ClassSuiteContext                                                           *
 *******************************************************************************/

k_ClassSuiteContext_t* k_ClassSuiteContext_new(k_ASTNode_t* node) {
    k_ClassSuiteContext_t* context = k_Memory_allocate(k_ClassSuiteContext_t, 1);
    context->m_node = node;
    context->m_classMembers = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CLASS_SUITE, context,
        (k_ContextDestructorFunction_t)k_ClassSuiteContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ClassSuiteContext_getChildren);

    return context;
}

void k_ClassSuiteContext_delete(k_ClassSuiteContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_classMembers);
    jtk_Memory_deallocate(context);
}

void k_ClassSuiteContext_getChildren(k_ClassSuiteContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_classMembers);
}

/*******************************************************************************
 * ClassMemberContext                                                          *
 *******************************************************************************/

k_ClassMemberContext_t* k_ClassMemberContext_new(k_ASTNode_t* node) {
    k_ClassMemberContext_t* context = k_Memory_allocate(k_ClassMemberContext_t, 1);
    context->m_node = node;
    context->m_annotations = NULL;
    context->m_modifiers = jtk_ArrayList_new();
    context->m_declaration = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CLASS_MEMBER, context,
        (k_ContextDestructorFunction_t)k_ClassMemberContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ClassMemberContext_getChildren);

    return context;
}

void k_ClassMemberContext_delete(k_ClassMemberContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_modifiers);
    jtk_Memory_deallocate(context);
}

void k_ClassMemberContext_getChildren(k_ClassMemberContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_annotations);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_modifiers);
    jtk_ArrayList_add(children, context->m_declaration);
}

/*******************************************************************************
 * ConstructorDeclarationContext                                               *
 *******************************************************************************/

// k_ConstructorDeclarationContext_t* k_ConstructorDeclarationContext_new(k_ASTNode_t* node) {
//    k_ConstructorDeclarationContext_t* context = k_Memory_allocate(k_ConstructorDeclarationContext_t, 1);
//     context->m_node = node;
//     context->m_identifier = NULL;
//     context->m_functionParameters = NULL;
//     context->m_statementSuite = NULL;

//     k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CONSTRUCTOR_DECLARATION, context,
//         (k_ContextDestructorFunction_t)k_ConstructorDeclarationContext_delete,
//         (k_EnumerateContextChildrenFunction_t)k_ConstructorDeclarationContext_getChildren);

//     return context;
// }

// void k_ConstructorDeclarationContext_delete(k_ConstructorDeclarationContext_t* context) {
//     jtk_Assert_assertObject(context, "The specified context is null.");

//     jtk_Memory_deallocate(context);
// }

// void k_ConstructorDeclarationContext_getChildren(k_ConstructorDeclarationContext_t* context,
//     jtk_ArrayList_t* children) {
//     jtk_Assert_assertObject(context, "The specified context is null.");
//     jtk_Assert_assertObject(children, "The specified children is null.");

//     jtk_ArrayList_add(children, context->m_identifier);
//     if (context->m_functionParameters != NULL) {
//         jtk_ArrayList_add(children, context->m_functionParameters);
//     }
//     jtk_ArrayList_add(children, context->m_statementSuite);
// }

/*******************************************************************************
 * EnumerationDeclarationContext                                               *
 *******************************************************************************/

/* TODO: Check if all the delete functions deallocate the context. */
// k_EnumerationDeclarationContext_t* k_EnumerationDeclarationContext_new(k_ASTNode_t* node) {
//     k_EnumerationDeclarationContext_t* context = k_Memory_allocate(k_EnumerationDeclarationContext_t, 1);
//     context->m_node = node;
//     context->m_identifier = NULL;
//     context->m_enumerationBaseClause = NULL;
//     context->m_enumerationSuite = NULL;

//     k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ENUMERATION_DECLARATION, context,
//         (k_ContextDestructorFunction_t)k_EnumerationDeclarationContext_delete,
//         (k_EnumerateContextChildrenFunction_t)k_EnumerationDeclarationContext_getChildren);

//     return context;
// }

// void k_EnumerationDeclarationContext_delete(k_EnumerationDeclarationContext_t* context) {
//     jtk_Assert_assertObject(context, "The specified context is null.");

//     jtk_Memory_deallocate(context);
// }

// void k_EnumerationDeclarationContext_getChildren(k_EnumerationDeclarationContext_t* context,
//     jtk_ArrayList_t* children) {
//     jtk_Assert_assertObject(context, "The specified context is null.");
//     jtk_Assert_assertObject(children, "The specified children is null.");

//     jtk_ArrayList_add(children, context->m_identifier);
//     if (context->m_enumerationBaseClause != NULL) {
//         jtk_ArrayList_add(children, context->m_enumerationBaseClause);
//     }
//     jtk_ArrayList_add(children, context->m_enumerationSuite);
// }

/*******************************************************************************
 * EnumerationBaseClauseContext                                                *
 *******************************************************************************/

// k_EnumerationBaseClauseContext_t* k_EnumerationBaseClauseContext_new(k_ASTNode_t* node) {
//     k_EnumerationBaseClauseContext_t* context = k_Memory_allocate(k_EnumerationBaseClauseContext_t, 1);
//     context->m_node = node;
//     context->m_typeName = NULL;

//     k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ENUMERATION_BASE_CLAUSE, context,
//         (k_ContextDestructorFunction_t)k_EnumerationBaseClauseContext_delete,
//         (k_EnumerateContextChildrenFunction_t)k_EnumerationBaseClauseContext_getChildren);

//     return context;
// }

// void k_EnumerationBaseClauseContext_delete(k_EnumerationBaseClauseContext_t* context) {
//     jtk_Assert_assertObject(context, "The specified context is null.");

//     jtk_Memory_deallocate(context);
// }

// void k_EnumerationBaseClauseContext_getChildren(k_EnumerationBaseClauseContext_t* context,
//     jtk_ArrayList_t* children) {
//     jtk_Assert_assertObject(context, "The specified context is null.");
//     jtk_Assert_assertObject(children, "The specified children is null.");

//     jtk_ArrayList_add(children, context->m_typeName);
// }

/*******************************************************************************
 * EnumerationSuiteContext                                                     *
 *******************************************************************************/

// k_EnumerationSuiteContext_t* k_EnumerationSuiteContext_new(k_ASTNode_t* node) {
//     k_EnumerationSuiteContext_t* context = k_Memory_allocate(k_EnumerationSuiteContext_t, 1);
//     context->m_node = node;
//     context->m_enumerates = jtk_ArrayList_new();

//     k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ENUMERATION_SUITE, context,
//         (k_ContextDestructorFunction_t)k_EnumerationSuiteContext_delete,
//         (k_EnumerateContextChildrenFunction_t)k_EnumerationSuiteContext_getChildren);

//     return context;
// }

// void k_EnumerationSuiteContext_delete(k_EnumerationSuiteContext_t* context) {
//     jtk_Assert_assertObject(context, "The specified context is null.");

//     jtk_ArrayList_delete(context->m_enumerates);
//     jtk_Memory_deallocate(context);
// }

// void k_EnumerationSuiteContext_getChildren(k_EnumerationSuiteContext_t* context,
//     jtk_ArrayList_t* children) {
//     jtk_Assert_assertObject(context, "The specified context is null.");
//     jtk_Assert_assertObject(children, "The specified children is null.");

//     jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_enumerates);
// }

/*******************************************************************************
 * EnumerateContext                                                            *
 *******************************************************************************/

// k_EnumerateContext_t* k_EnumerateContext_new(k_ASTNode_t* node) {
//     k_EnumerateContext_t* context = k_Memory_allocate(k_EnumerateContext_t, 1);
//     context->m_node = node;
//     context->m_identifier = NULL;
//     context->m_functionArguments = NULL;

//     k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ENUMERATE, context,
//         (k_ContextDestructorFunction_t)k_EnumerateContext_delete,
//         (k_EnumerateContextChildrenFunction_t)k_EnumerateContext_getChildren);

//     return context;
// }

// void k_EnumerateContext_delete(k_EnumerateContext_t* context) {
//     jtk_Assert_assertObject(context, "The specified context is null.");

//     jtk_Memory_deallocate(context);
// }

// void k_EnumerateContext_getChildren(k_EnumerateContext_t* context,
//     jtk_ArrayList_t* children) {
//     jtk_Assert_assertObject(context, "The specified context is null.");
//     jtk_Assert_assertObject(children, "The specified children is null.");

//     jtk_ArrayList_add(children, context->m_identifier);
//     if (context->m_functionArguments != NULL) {
//         jtk_ArrayList_add(children, context->m_functionArguments);
//     }
// }

/*******************************************************************************
 * ExpressionsContext                                                          *
 *******************************************************************************/

k_ExpressionsContext_t* k_ExpressionsContext_new(k_ASTNode_t* node) {
    k_ExpressionsContext_t* context = k_Memory_allocate(k_ExpressionsContext_t, 1);
    context->m_node = node;
    context->m_expressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_EXPRESSIONS, context,
        (k_ContextDestructorFunction_t)k_ExpressionsContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ExpressionsContext_getChildren);

    return context;
}

void k_ExpressionsContext_delete(k_ExpressionsContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_expressions);
    jtk_Memory_deallocate(context);
}

void k_ExpressionsContext_getChildren(k_ExpressionsContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_expressions);
}

/*******************************************************************************
 * ExpressionContext                                                           *
 *******************************************************************************/

k_ExpressionContext_t* k_ExpressionContext_new(k_ASTNode_t* node) {
    k_ExpressionContext_t* context = k_Memory_allocate(k_ExpressionContext_t, 1);
    context->m_node = node;
    context->m_assignmentExpression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_ExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ExpressionContext_getChildren);

    return context;
}

void k_ExpressionContext_delete(k_ExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ExpressionContext_getChildren(k_ExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_assignmentExpression);
}

/*******************************************************************************
 * AssignmentExpressionContext                                                 *
 *******************************************************************************/

k_AssignmentExpressionContext_t* k_AssignmentExpressionContext_new(k_ASTNode_t* node) {
    k_AssignmentExpressionContext_t* context = k_Memory_allocate(k_AssignmentExpressionContext_t, 1);
    context->m_node = node;
    context->m_conditionalExpression = NULL;
    context->m_assignmentOperator = NULL;
    context->m_assignmentExpression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ASSIGNMENT_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_AssignmentExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AssignmentExpressionContext_getChildren);

    return context;
}

void k_AssignmentExpressionContext_delete(k_AssignmentExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_AssignmentExpressionContext_getChildren(k_AssignmentExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_conditionalExpression);
    if (context->m_assignmentOperator != NULL) {
        jtk_ArrayList_add(children, context->m_assignmentOperator);
        jtk_ArrayList_add(children, context->m_assignmentExpression);
    }
}

/*******************************************************************************
 * ConditionalExpressionContext                                                *
 *******************************************************************************/

k_ConditionalExpressionContext_t* k_ConditionalExpressionContext_new(k_ASTNode_t* node) {
    k_ConditionalExpressionContext_t* context = k_Memory_allocate(k_ConditionalExpressionContext_t, 1);
    context->m_node = node;
    context->m_logicalOrExpression = NULL;
    context->m_thenExpression = NULL;
    context->m_elseExpression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_CONDITIONAL_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_ConditionalExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ConditionalExpressionContext_getChildren);

    return context;
}

void k_ConditionalExpressionContext_delete(k_ConditionalExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ConditionalExpressionContext_getChildren(k_ConditionalExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_logicalOrExpression);
    if (context->m_thenExpression != NULL) {
        jtk_ArrayList_add(children, context->m_thenExpression);
        jtk_ArrayList_add(children, context->m_elseExpression);
    }
}

/*******************************************************************************
 * LogicalOrExpressionContext                                                  *
 *******************************************************************************/

k_LogicalOrExpressionContext_t* k_LogicalOrExpressionContext_new(k_ASTNode_t* node) {
    k_LogicalOrExpressionContext_t* context = k_Memory_allocate(k_LogicalOrExpressionContext_t, 1);
    context->m_node = node;
    context->m_logicalAndExpression = NULL;
    context->m_logicalAndExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_LOGICAL_OR_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_LogicalOrExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_LogicalOrExpressionContext_getChildren);

    return context;
}

void k_LogicalOrExpressionContext_delete(k_LogicalOrExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_logicalAndExpressions);
    jtk_Memory_deallocate(context);
}

void k_LogicalOrExpressionContext_getChildren(k_LogicalOrExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_logicalAndExpression);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_logicalAndExpressions);
}

/*******************************************************************************
 * LogicalAndExpressionContext                                                 *
 *******************************************************************************/

k_LogicalAndExpressionContext_t* k_LogicalAndExpressionContext_new(k_ASTNode_t* node) {
    k_LogicalAndExpressionContext_t* context = k_Memory_allocate(k_LogicalAndExpressionContext_t, 1);
    context->m_node = node;
    context->m_inclusiveOrExpression = NULL;
    context->m_inclusiveOrExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_LOGICAL_AND_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_LogicalAndExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_LogicalAndExpressionContext_getChildren);

    return context;
}

void k_LogicalAndExpressionContext_delete(k_LogicalAndExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_inclusiveOrExpressions);
    jtk_Memory_deallocate(context);
}

void k_LogicalAndExpressionContext_getChildren(k_LogicalAndExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_inclusiveOrExpression);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_inclusiveOrExpressions);
}

/*******************************************************************************
 * InclusiveOrExpressionContext                                                *
 *******************************************************************************/

k_InclusiveOrExpressionContext_t* k_InclusiveOrExpressionContext_new(k_ASTNode_t* node) {
    k_InclusiveOrExpressionContext_t* context = k_Memory_allocate(k_InclusiveOrExpressionContext_t, 1);
    context->m_node = node;
    context->m_exclusiveOrExpression = NULL;
    context->m_exclusiveOrExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_INCLUSIVE_OR_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_InclusiveOrExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_InclusiveOrExpressionContext_getChildren);

    return context;
}

void k_InclusiveOrExpressionContext_delete(k_InclusiveOrExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_exclusiveOrExpressions);
    jtk_Memory_deallocate(context);
}

void k_InclusiveOrExpressionContext_getChildren(k_InclusiveOrExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_exclusiveOrExpression);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_exclusiveOrExpressions);
}

/*******************************************************************************
 * ExclusiveOrExpressionContext                                                *
 *******************************************************************************/

k_ExclusiveOrExpressionContext_t* k_ExclusiveOrExpressionContext_new(k_ASTNode_t* node) {
    k_ExclusiveOrExpressionContext_t* context = k_Memory_allocate(k_ExclusiveOrExpressionContext_t, 1);
    context->m_node = node;
    context->m_andExpression = NULL;
    context->m_andExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_EXCLUSIVE_OR_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_ExclusiveOrExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ExclusiveOrExpressionContext_getChildren);

    return context;
}

void k_ExclusiveOrExpressionContext_delete(k_ExclusiveOrExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_andExpressions);
    jtk_Memory_deallocate(context);
}

void k_ExclusiveOrExpressionContext_getChildren(k_ExclusiveOrExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_andExpression);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_andExpressions);
}

/*******************************************************************************
 * AndExpressionContext                                                        *
 *******************************************************************************/

k_AndExpressionContext_t* k_AndExpressionContext_new(k_ASTNode_t* node) {
    k_AndExpressionContext_t* context = k_Memory_allocate(k_AndExpressionContext_t, 1);
    context->m_node = node;
    context->m_equalityExpression = NULL;
    context->m_equalityExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_AND_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_AndExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AndExpressionContext_getChildren);

    return context;
}

void k_AndExpressionContext_delete(k_AndExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_equalityExpressions);
    jtk_Memory_deallocate(context);
}

void k_AndExpressionContext_getChildren(k_AndExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_equalityExpression);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_equalityExpressions);
}

/*******************************************************************************
 * EqualityExpressionContext                                                   *
 *******************************************************************************/

k_EqualityExpressionContext_t* k_EqualityExpressionContext_new(k_ASTNode_t* node) {
    k_EqualityExpressionContext_t* context = k_Memory_allocate(k_EqualityExpressionContext_t, 1);
    context->m_node = node;
    context->m_relationalExpression = NULL;
    context->m_relationalExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_EQUALITY_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_EqualityExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_EqualityExpressionContext_getChildren);

    return context;
}

void k_EqualityExpressionContext_delete(k_EqualityExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    int32_t size = jtk_ArrayList_getSize(context->m_relationalExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_relationalExpressions, i);
        jtk_Pair_delete(pair);
    }
    jtk_ArrayList_delete(context->m_relationalExpressions);
    jtk_Memory_deallocate(context);
}

void k_EqualityExpressionContext_getChildren(k_EqualityExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_relationalExpression);

    int32_t size = jtk_ArrayList_getSize(context->m_relationalExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_relationalExpressions, i);
        jtk_ArrayList_add(children, pair->m_left);
        jtk_ArrayList_add(children, pair->m_right);
    }
}

/*******************************************************************************
 * RelationalExpressionContext                                                 *
 *******************************************************************************/

k_RelationalExpressionContext_t* k_RelationalExpressionContext_new(k_ASTNode_t* node) {
    k_RelationalExpressionContext_t* context = k_Memory_allocate(k_RelationalExpressionContext_t, 1);
    context->m_node = node;
    context->m_shiftExpression = NULL;
    context->m_shiftExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_RELATIONAL_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_RelationalExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_RelationalExpressionContext_getChildren);

    return context;
}

void k_RelationalExpressionContext_delete(k_RelationalExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    int32_t size = jtk_ArrayList_getSize(context->m_shiftExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
            context->m_shiftExpressions, i);
        jtk_Pair_delete(pair);
    }
    jtk_ArrayList_delete(context->m_shiftExpressions);
    jtk_Memory_deallocate(context);
}

void k_RelationalExpressionContext_getChildren(k_RelationalExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_shiftExpression);

    int32_t size = jtk_ArrayList_getSize(context->m_shiftExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_shiftExpressions, i);
        jtk_ArrayList_add(children, pair->m_left);
        jtk_ArrayList_add(children, pair->m_right);
    }
}

/*******************************************************************************
 * ShiftExpressionContext                                                      *
 *******************************************************************************/

k_ShiftExpressionContext_t* k_ShiftExpressionContext_new(k_ASTNode_t* node) {
    k_ShiftExpressionContext_t* context = k_Memory_allocate(k_ShiftExpressionContext_t, 1);
    context->m_node = node;
    context->m_additiveExpression = NULL;
    context->m_additiveExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_SHIFT_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_ShiftExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ShiftExpressionContext_getChildren);

    return context;
}

void k_ShiftExpressionContext_delete(k_ShiftExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    int32_t size = jtk_ArrayList_getSize(context->m_additiveExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
            context->m_additiveExpressions, i);
        jtk_Pair_delete(pair);
    }
    jtk_ArrayList_delete(context->m_additiveExpressions);
    jtk_Memory_deallocate(context);
}

void k_ShiftExpressionContext_getChildren(k_ShiftExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_additiveExpression);

    int32_t size = jtk_ArrayList_getSize(context->m_additiveExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_additiveExpressions, i);
        jtk_ArrayList_add(children, pair->m_left);
        jtk_ArrayList_add(children, pair->m_right);
    }
}

/*******************************************************************************
 * AdditiveExpressionContext                                                   *
 *******************************************************************************/

k_AdditiveExpressionContext_t* k_AdditiveExpressionContext_new(k_ASTNode_t* node) {
    k_AdditiveExpressionContext_t* context = k_Memory_allocate(k_AdditiveExpressionContext_t, 1);
    context->m_node = node;
    context->m_multiplicativeExpression = NULL;
    context->m_multiplicativeExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_ADDITIVE_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_AdditiveExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_AdditiveExpressionContext_getChildren);

    return context;
}

void k_AdditiveExpressionContext_delete(k_AdditiveExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    int32_t size = jtk_ArrayList_getSize(context->m_multiplicativeExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(
            context->m_multiplicativeExpressions, i);
        jtk_Pair_delete(pair);
    }
    jtk_ArrayList_delete(context->m_multiplicativeExpressions);
    jtk_Memory_deallocate(context);
}

void k_AdditiveExpressionContext_getChildren(k_AdditiveExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_multiplicativeExpression);

    int32_t size = jtk_ArrayList_getSize(context->m_multiplicativeExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_multiplicativeExpressions, i);
        jtk_ArrayList_add(children, pair->m_left);
        jtk_ArrayList_add(children, pair->m_right);
    }
}

/*******************************************************************************
 * MultiplicativeExpressionContext                                             *
 *******************************************************************************/

k_MultiplicativeExpressionContext_t* k_MultiplicativeExpressionContext_new(k_ASTNode_t* node) {
    k_MultiplicativeExpressionContext_t* context = k_Memory_allocate(k_MultiplicativeExpressionContext_t, 1);
    context->m_node = node;
    context->m_unaryExpression = NULL;
    context->m_unaryExpressions = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_MULTIPLICATIVE_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_MultiplicativeExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_MultiplicativeExpressionContext_getChildren);

    return context;
}

void k_MultiplicativeExpressionContext_delete(k_MultiplicativeExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    int32_t size = jtk_ArrayList_getSize(context->m_unaryExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_unaryExpressions, i);
        jtk_Pair_delete(pair);
    }
    jtk_ArrayList_delete(context->m_unaryExpressions);
    jtk_Memory_deallocate(context);
}

void k_MultiplicativeExpressionContext_getChildren(k_MultiplicativeExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_unaryExpression);

    int32_t size = jtk_ArrayList_getSize(context->m_unaryExpressions);
    int32_t i;
    for (i = 0; i < size; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(context->m_unaryExpressions, i);
        jtk_ArrayList_add(children, pair->m_left);
        jtk_ArrayList_add(children, pair->m_right);
    }
}

/*******************************************************************************
 * UnaryExpressionContext                                                      *
 *******************************************************************************/

k_UnaryExpressionContext_t* k_UnaryExpressionContext_new(k_ASTNode_t* node) {
    k_UnaryExpressionContext_t* context = k_Memory_allocate(k_UnaryExpressionContext_t, 1);
    context->m_node = node;
    context->m_unaryOperator = NULL;
    context->m_unaryExpression = NULL;
    context->m_postfixExpression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_UNARY_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_UnaryExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_UnaryExpressionContext_getChildren);

    return context;
}

void k_UnaryExpressionContext_delete(k_UnaryExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_UnaryExpressionContext_getChildren(k_UnaryExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_unaryOperator != NULL) {
        jtk_ArrayList_add(children, context->m_unaryOperator);
        jtk_ArrayList_add(children, context->m_unaryExpression);
    }
    else {
        jtk_ArrayList_add(children, context->m_postfixExpression);
    }
}

/*******************************************************************************
 * PostfixExpressionContext                                                    *
 *******************************************************************************/

k_PostfixExpressionContext_t* k_PostfixExpressionContext_new(k_ASTNode_t* node) {
    k_PostfixExpressionContext_t* context = k_Memory_allocate(k_PostfixExpressionContext_t, 1);
    context->m_node = node;
    context->m_primaryExpression = NULL;
    context->m_postfixParts = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_POSTFIX_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_PostfixExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_PostfixExpressionContext_getChildren);

    return context;
}

void k_PostfixExpressionContext_delete(k_PostfixExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_postfixParts);
    jtk_Memory_deallocate(context);
}

void k_PostfixExpressionContext_getChildren(k_PostfixExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_primaryExpression);
    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_postfixParts);
}

/*******************************************************************************
 * SubscriptContext                                                            *
 *******************************************************************************/

k_SubscriptContext_t* k_SubscriptContext_new(k_ASTNode_t* node) {
    k_SubscriptContext_t* context = k_Memory_allocate(k_SubscriptContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_SUBSCRIPT, context,
        (k_ContextDestructorFunction_t)k_SubscriptContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_SubscriptContext_getChildren);

    return context;
}

void k_SubscriptContext_delete(k_SubscriptContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_SubscriptContext_getChildren(k_SubscriptContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expression);
}

/*******************************************************************************
 * FunctionArgumentsContext                                                   *
 *******************************************************************************/

k_FunctionArgumentsContext_t* k_FunctionArgumentsContext_new(k_ASTNode_t* node) {
    k_FunctionArgumentsContext_t* context = k_Memory_allocate(k_FunctionArgumentsContext_t, 1);
    context->m_node = node;
    context->m_expressions = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_FUNCTION_ARGUMENTS, context,
        (k_ContextDestructorFunction_t)k_FunctionArgumentsContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_FunctionArgumentsContext_getChildren);

    return context;
}

void k_FunctionArgumentsContext_delete(k_FunctionArgumentsContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_FunctionArgumentsContext_getChildren(k_FunctionArgumentsContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    if (context->m_expressions != NULL) {
        jtk_ArrayList_add(children, context->m_expressions);
    }
}

/*******************************************************************************
 * MemberAccessContext                                                         *
 *******************************************************************************/

k_MemberAccessContext_t* k_MemberAccessContext_new(k_ASTNode_t* node) {
    k_MemberAccessContext_t* context = k_Memory_allocate(k_MemberAccessContext_t, 1);
    context->m_node = node;
    context->m_identifier = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_MEMBER_ACCESS, context,
        (k_ContextDestructorFunction_t)k_MemberAccessContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_MemberAccessContext_getChildren);

    return context;
}

void k_MemberAccessContext_delete(k_MemberAccessContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_MemberAccessContext_getChildren(k_MemberAccessContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_identifier);
}

/*******************************************************************************
 * PostfixOperatorContext                                                      *
 *******************************************************************************/

k_PostfixOperatorContext_t* k_PostfixOperatorContext_new(k_ASTNode_t* node) {
    k_PostfixOperatorContext_t* context = k_Memory_allocate(k_PostfixOperatorContext_t, 1);
    context->m_node = node;
    context->m_postfixOperator = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_POSTFIX_OPERATOR, context,
        (k_ContextDestructorFunction_t)k_PostfixOperatorContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_PostfixOperatorContext_getChildren);

    return context;
}

void k_PostfixOperatorContext_delete(k_PostfixOperatorContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_PostfixOperatorContext_getChildren(k_PostfixOperatorContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_postfixOperator);
}

/*******************************************************************************
 * PrimaryExpressionContext                                                    *
 *******************************************************************************/

k_PrimaryExpressionContext_t* k_PrimaryExpressionContext_new(k_ASTNode_t* node) {
    k_PrimaryExpressionContext_t* context = k_Memory_allocate(k_PrimaryExpressionContext_t, 1);
    context->m_node = node;
    context->m_expression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_PRIMARY_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_PrimaryExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_PrimaryExpressionContext_getChildren);
    return context;
}

void k_PrimaryExpressionContext_delete(k_PrimaryExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Memory_deallocate(context);
}

void k_PrimaryExpressionContext_getChildren(k_PrimaryExpressionContext_t* context, jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expression);
}

/*******************************************************************************
 * MapExpressionContext                                                        *
 *******************************************************************************/

k_MapExpressionContext_t* k_MapExpressionContext_new(k_ASTNode_t* node) {
    k_MapExpressionContext_t* context = k_Memory_allocate(k_MapExpressionContext_t, 1);
    context->m_node = node;
    context->m_mapEntries = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_MAP_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_MapExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_MapExpressionContext_getChildren);

    return context;
}

void k_MapExpressionContext_delete(k_MapExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_MapExpressionContext_getChildren(k_MapExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_mapEntries);
}

/*******************************************************************************
 * MapEntriesContext                                                           *
 *******************************************************************************/

k_MapEntriesContext_t* k_MapEntriesContext_new(k_ASTNode_t* node) {
    k_MapEntriesContext_t* context = k_Memory_allocate(k_MapEntriesContext_t, 1);
    context->m_node = node;
    context->m_mapEntries = jtk_ArrayList_new();

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_MAP_ENTRIES, context,
        (k_ContextDestructorFunction_t)k_MapEntriesContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_MapEntriesContext_getChildren);

    return context;
}

void k_MapEntriesContext_delete(k_MapEntriesContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_ArrayList_delete(context->m_mapEntries);
    jtk_Memory_deallocate(context);
}

void k_MapEntriesContext_getChildren(k_MapEntriesContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_addAll(children, JTK_COLLECTION_ARRAY_LIST, context->m_mapEntries);
}

/*******************************************************************************
 * MapEntryContext                                                             *
 *******************************************************************************/

k_MapEntryContext_t* k_MapEntryContext_new(k_ASTNode_t* node) {
    k_MapEntryContext_t* context = k_Memory_allocate(k_MapEntryContext_t, 1);
    context->m_node = node;
    context->m_keyExpression = NULL;
    context->m_valueExpression = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_MAP_ENTRY, context,
        (k_ContextDestructorFunction_t)k_MapEntryContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_MapEntryContext_getChildren);

    return context;
}

void k_MapEntryContext_delete(k_MapEntryContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_MapEntryContext_getChildren(k_MapEntryContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_keyExpression);
    jtk_ArrayList_add(children, context->m_valueExpression);
}

/*******************************************************************************
 * ListExpressionContext                                                       *
 *******************************************************************************/

k_ListExpressionContext_t* k_ListExpressionContext_new(k_ASTNode_t* node) {
    k_ListExpressionContext_t* context = k_Memory_allocate(k_ListExpressionContext_t, 1);
    context->m_node = node;
    context->m_expressions = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_LIST_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_ListExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_ListExpressionContext_getChildren);

    return context;
}

void k_ListExpressionContext_delete(k_ListExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_ListExpressionContext_getChildren(k_ListExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_expressions);
}

/*******************************************************************************
 * NewExpressionContext                                                        *
 *******************************************************************************/

k_NewExpressionContext_t* k_NewExpressionContext_new(k_ASTNode_t* node) {
    k_NewExpressionContext_t* context = k_Memory_allocate(k_NewExpressionContext_t, 1);
    context->m_node = node;
    context->m_typeName = NULL;
    context->m_functionArguments = NULL;

    k_Context_initialiKUSHode(node, KUSH_AST_NODE_TYPE_NEW_EXPRESSION, context,
        (k_ContextDestructorFunction_t)k_NewExpressionContext_delete,
        (k_EnumerateContextChildrenFunction_t)k_NewExpressionContext_getChildren);

    return context;
}

void k_NewExpressionContext_delete(k_NewExpressionContext_t* context) {
    jtk_Assert_assertObject(context, "The specified context is null.");

    jtk_Memory_deallocate(context);
}

void k_NewExpressionContext_getChildren(k_NewExpressionContext_t* context,
    jtk_ArrayList_t* children) {
    jtk_Assert_assertObject(context, "The specified context is null.");
    jtk_Assert_assertObject(children, "The specified children is null.");

    jtk_ArrayList_add(children, context->m_typeName);
    if (context->m_functionArguments != NULL) {
        jtk_ArrayList_add(children, context->m_functionArguments);
    }
}
