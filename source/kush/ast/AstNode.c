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

#include <stdio.h>
#include <jtk/core/StringBuilder.h>
#include <kush/ast/ASTNode.h>
#include <kush/parser/Parser.h>

/******************************************************************************
 * ASTNode																	  *
 ******************************************************************************/

k_ASTNode_t* k_ASTNode_new(k_ASTNode_t* parent) {
    k_ASTNode_t* node = k_Memory_allocate(k_ASTNode_t, 1);
    node->m_type = ZEN_AST_NODE_TYPE_UNKNOWN;
    node->m_context = NULL;
    node->m_parent = parent;
    node->m_children = NULL;
    node->m_enumerateContextChildren = NULL;
    node->m_contextDestructor = NULL;

    return node;
}

void k_ASTNode_delete(k_ASTNode_t* node) {
    if (k_ASTNode_isRule(node)) {
        jtk_ArrayList_t* children = k_ASTNode_getChildren(node);
        int32_t size = jtk_ArrayList_getSize(children);
        int32_t i;
        for (i = 0; i < size; i++) {
            k_ASTNode_t* child = (k_ASTNode_t*)jtk_ArrayList_getValue(children, i);
            if (child != NULL) {
                k_ASTNode_delete(child);
            }
        }
        node->m_contextDestructor(node->m_context);
    }
    if (node->m_children != NULL) {
        jtk_ArrayList_delete(node->m_children);
    }
    jtk_Memory_deallocate(node);
}

k_ASTNodeType_t k_ASTNode_getType(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");
    return node->m_type;
}

k_ASTNode_t* k_ASTNode_getParent(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");
    return node->m_parent;
}

void* k_ASTNode_getContext(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");
    return node->m_context;
}

bool k_ASTNode_isErroneous(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");
#warning "[TODO] k_ASTNode_isErroneous() is not implemented."
    fflush(stdout);
    return false;
    // return node->m_erroneous;
}

bool k_ASTNode_isTerminal(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");
    return node->m_type == ZEN_AST_NODE_TYPE_TERMINAL;
}

bool k_ASTNode_isRule(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");
    return /* !node->m_erroneous && */ (node->m_type != ZEN_AST_NODE_TYPE_TERMINAL);
}

int32_t k_ASTNode_getDepth(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");

    int32_t depth = 0;
    k_ASTNode_t* currentNode = node;
    while (currentNode != NULL) {
        depth++;
        currentNode = currentNode->m_parent;
    }
    return depth;
}

/* The returned array list should not be modified externally. */
jtk_ArrayList_t* k_ASTNode_getChildren(k_ASTNode_t* node) {
    jtk_Assert_assertObject(node, "The specified node is null.");

    if (node->m_children == NULL) {
        node->m_children = jtk_ArrayList_new();
        node->m_enumerateContextChildren(node->m_context, node->m_children);
    }
    return node->m_children;
}

void k_ASTNode_toString0(k_ASTNode_t* node, jtk_StringBuilder_t* builder) {
    jtk_Assert_assertObject(node, "The specified node is null.");

    if (node->m_type == ZEN_AST_NODE_TYPE_TERMINAL) {
        k_Token_t* token = (k_Token_t*)node->m_context;
        jtk_StringBuilder_appendEx_z(builder, token->m_text, token->m_length);
    }
    else {
        jtk_ArrayList_t* nodes = k_ASTNode_getChildren(node);
        int32_t size = jtk_ArrayList_getSize(nodes);
        int32_t i;
        for (i = 0; i < size; i++) {
            k_ASTNode_t* child = jtk_ArrayList_getValue(nodes, i);
            if (child->m_type == ZEN_AST_NODE_TYPE_TERMINAL) {
                k_Token_t* token = (k_Token_t*)child->m_context;
                jtk_StringBuilder_appendEx_z(builder, token->m_text, token->m_length);
            }
            else if (child->m_type == ZEN_AST_NODE_TYPE_UNKNOWN) {
                jtk_StringBuilder_appendEx_z(builder, "<unknown>", 9);
            }
            else {
                // TODO: I am not sure what happens when the child is erroneous.
                k_ASTNode_toString0(child, builder);
            }
        }
    }
}

uint8_t* k_ASTNode_toCString(k_ASTNode_t* node, int32_t* size) {
    jtk_Assert_assertObject(node, "The specified node is null.");

    /* Create a string builder to create the string equivalent of the given
     * node.
     */
    jtk_StringBuilder_t* builder = jtk_StringBuilder_new();

    /* Recursively construct the string equivalent of the given node. */
    k_ASTNode_toString0(node, builder);

    /* Build and retrieve the string. */
    uint8_t* result = jtk_StringBuilder_toCString(builder, size);

    /* Destroy the string builder. */
    jtk_StringBuilder_delete(builder);

    return result;
}