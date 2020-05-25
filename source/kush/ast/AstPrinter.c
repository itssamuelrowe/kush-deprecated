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

// Sunday, March 11, 2018

#include <kush/ast/ASTPrinter.h>
#include <kush/parser/Parser.h>

k_ASTPrinter_t* k_ASTPrinter_new() {
    k_ASTPrinter_t* listener = k_Memory_allocate(k_ASTPrinter_t, 1);
    listener->m_astListener = k_ASTListener_newWithContext(listener);
    listener->m_depth = 0;

    k_ASTListener_t* astListener = listener->m_astListener;
    astListener->m_onEnterEveryRule = k_ASTPrinter_onEnterEveryRule;
    astListener->m_onExitEveryRule = k_ASTPrinter_onExitEveryRule;
    astListener->m_onVisitTerminal = k_ASTPrinter_onVisitTerminal;

    // TODO: Initialize astListener

    return listener;
}

void k_ASTPrinter_delete(k_ASTPrinter_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    k_ASTListener_delete(listener->m_astListener);
    jtk_Memory_deallocate(listener);
}

k_ASTListener_t* k_ASTPrinter_getASTListener(k_ASTPrinter_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");
    return listener->m_astListener;
}

void k_ASTPrinter_indent(int32_t depth) {
    int32_t i;
    for (i = 0; i < depth; i++) {
        printf("    ");
    }
}

void k_ASTPrinter_onVisitTerminal(k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_ASTPrinter_t* listener = (k_ASTPrinter_t*)astListener->m_context;

    listener->m_depth++;
    k_ASTPrinter_indent(listener->m_depth);
    printf("<%s>\n", k_Token_getText((k_Token_t*)node->m_context));
    listener->m_depth--;
}

void k_ASTPrinter_onEnterEveryRule(k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_ASTPrinter_t* listener = (k_ASTPrinter_t*)astListener->m_context;

    listener->m_depth++;
    k_ASTPrinter_indent(listener->m_depth);
    printf("%s\n", k_Parser_getRuleName(node->m_type));
}

void k_ASTPrinter_onExitEveryRule(k_ASTListener_t* astListener, k_ASTNode_t* node) {
    k_ASTPrinter_t* listener = (k_ASTPrinter_t*)astListener->m_context;
    listener->m_depth--;
}
