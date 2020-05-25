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

// Saturday, April 28, 2018

#include <jtk/collection/stack/LinkedStack.h>

#include <kush/lexer/Token.h>
#include <kush/ast/ASTHelper.h>

/*******************************************************************************
 * ASTHelper                                                                   *
 *******************************************************************************/

bool k_ASTHelper_isAncestor(k_ASTNode_t* node, k_ASTNode_t* subject) {
    jtk_Assert_assertObject(node, "The specified node is null.");
    jtk_Assert_assertObject(subject, "The specified subject is null.");

    k_ASTNode_t* parent = k_ASTNode_getParent(subject);
    bool result = false;
    while (parent != NULL) {
        if (parent == node) {
            result = true;
            break;
        }
        parent = k_ASTNode_getParent(parent);
    }
    return result;
}

k_ASTNode_t* k_ASTHelper_getAncestor(k_ASTNode_t* node, k_ASTNodeType_t type) {
    jtk_Assert_assertObject(node, "The specified node is null.");

    k_ASTNode_t* parent = k_ASTNode_getParent(node);
    k_ASTNode_t* result = NULL;
    while (parent != NULL) {
        if (k_ASTNode_getType(parent) == type) {
            result = parent;
            break;
        }
        parent = k_ASTNode_getParent(parent);
    }
    return result;
}

bool k_ASTHelper_isDescendant(k_ASTNode_t* node, k_ASTNode_t* subject) {
    return k_ASTHelper_isAncestor(subject, node);
}

void k_ASTHelper_getTokens(k_ASTNode_t* node, jtk_ArrayList_t* list) {
    k_ASTHelper_getNodes(node, list, -1, true, true);
}

void k_ASTHelper_getFilteredTokens(k_ASTNode_t* node, jtk_ArrayList_t* list, k_TokenType_t type) {
    k_ASTHelper_getNodes(node, list, (int32_t)type, true, true);
}

void k_ASTHelper_getTerminalNodes(k_ASTNode_t* node, jtk_ArrayList_t* list) {
    k_ASTHelper_getNodes(node, list, -1, true, false);
}

void k_ASTHelper_getFilteredTerminalNodes(k_ASTNode_t* node, jtk_ArrayList_t* list, k_TokenType_t type) {
    k_ASTHelper_getNodes(node, list, (int32_t)type, true, false);
}

void k_ASTHelper_getNodes(k_ASTNode_t* node, jtk_ArrayList_t* list,
    int32_t filter, bool captureTerminals, bool strip) {
    jtk_LinkedStack_t* stack = jtk_LinkedStack_new();
    jtk_LinkedStack_push(stack, node);

    while (!jtk_LinkedStack_isEmpty(stack)) {
        k_ASTNode_t* currentNode = (k_ASTNode_t*)jtk_LinkedStack_pop(stack);

        if (k_ASTNode_isTerminal(currentNode)) {
            if (captureTerminals) {
                k_Token_t* token = (k_Token_t*)currentNode->m_context;

                if ((filter >= 0) && (k_Token_getType(token) != (k_TokenType_t)filter)) {
                    continue;
                }

                if (strip) {
                    jtk_ArrayList_add(list, token);
                }
                else {
                    jtk_ArrayList_add(list, currentNode);
                }
            }
        }
        else if (k_ASTNode_isRule(currentNode)) {
            if (!captureTerminals) {
                if ((filter >= 0) && (k_ASTNode_getType(currentNode) != (k_ASTNodeType_t)filter)) {
                    continue;
                }

                if (strip) {
                    void* context = currentNode->m_context;
                    jtk_ArrayList_add(list, context);
                }
                else {
                    jtk_ArrayList_add(list, currentNode);
                }
            }

            jtk_ArrayList_t* children = k_ASTNode_getChildren(currentNode);
            int32_t size = jtk_ArrayList_getSize(children);
            int32_t i;
            for (i = 0; i < size; i++) {
                k_ASTNode_t* child = (k_ASTNode_t*)jtk_ArrayList_getValue(children, i);
                jtk_LinkedStack_push(stack, child);
            }
        }
        // What happens to erroneous nodes?
    }
}