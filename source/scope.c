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
#include <jtk/core/CStringObjectAdapter.h>

#include <kush/scope.h>

Scope* k_Scope_new(const uint8_t* name, int32_t nameSize,
    k_ScopeType_t type, Scope* parent, Context* symbol) {
    // jtk_Assert_assertObject(name, "The specified name is null.");

    jtk_ObjectAdapter_t* stringObjectAdapter = jtk_CStringObjectAdapter_getInstance();

    Scope* scope = allocate(Scope, 1);
    scope->name = NULL; // jtk_CString_make(name, &nameSize);
    scope->nameSize = nameSize;
    scope->type = type;
    scope->parent = parent;
    scope->symbols = jtk_HashMap_new(stringObjectAdapter, NULL);
    scope->nextTicket = 0;
    scope->symbol = symbol;

    return scope;
}

Scope* k_Scope_forCompilationUnit() {
    return k_Scope_new(NULL, 0, KUSH_SCOPE_COMPILATION_UNIT, NULL, NULL);
}

Scope* k_Scope_forFunction(Scope* parent) {
    return k_Scope_new(NULL, 0, KUSH_SCOPE_FUNCTION, parent, NULL);
}

Scope* k_Scope_forLocal(Scope* parent) {
    return k_Scope_new(NULL, 0, KUSH_SCOPE_LOCAL, parent, NULL);
}

Scope* k_Scope_forStructure(Scope* parent) {
    return k_Scope_new(NULL, 0, KUSH_SCOPE_STRUCTURE, parent, NULL);
}

void k_Scope_delete(Scope* scope) {
    jtk_Assert_assertObject(scope, "The specified scope is null.");

    jtk_HashMap_delete(scope->symbols);
    jtk_CString_delete(scope->name);
    deallocate(scope);
}

// Children Symbols

void getScopeChildren(Scope* scope, jtk_ArrayList_t* childrenSymbols) {
    jtk_Assert_assertObject(scope, "The specified scope is null.");
    jtk_Assert_assertObject(childrenSymbols, "The specified list is null.");

    jtk_Iterator_t* iterator = jtk_HashMap_getValueIterator(scope->symbols);
    jtk_ArrayList_addAllFromIterator(childrenSymbols, iterator);
    jtk_Iterator_delete(iterator);
}

// void k_Scope_define(Scope* scope, Context* symbol) {
//     const uint8_t* text = ((Token*)symbol->identifier->context)->text;
//     k_Scope_defineEx(scope, text, -1, symbol);
// }

// void k_Scope_defineEx(Scope* scope, const uint8_t* descriptor,
//     int32_t descriptorSize, Context* symbol) {
//     if (!jtk_HashMap_putStrictly(scope->symbols, (void*)descriptor, symbol)) {
//         fprintf(stderr, "[internal error] k_Scope_define() invoked to redefine a symbol.\n");
//     }
// }

/*
 * Resolving a class member
 * ------------------------
 *
 * 0. We are given a class scope and an identifier.
 * 1. Retrieve the class symbol corresponding to class scope.
 * 2. The super classes of the class symbol are stored in an
 *    tree. Therefore, create a linked-stack for its traversal.
 * 3. Push the class symbol on to the stack.
 * 4. Repeatedly process the elements on the stack.
 * 5. Retrieve the class symbol on top of the stack and remove it.
 * 6. Retrieve the class scope associated with the class symbol.
 * 7. Look for a function, variable, constant, enumeration, or subclass symbol
 *    within the class scope.
 * 8. If such a symbol exists, retrieve the corresponding symbol. Fall
 *    through to the end.
 * 9. Otherwise, retrieve the super classes of the current class.
 *     Push the super classes on to the stack. GOTO step 4.
 * 10. Return the symbol, if found. Otherwise, null.
 */
// Context* k_Scope_resolveClassMember(Scope* scope,
//     const uint8_t* identifier) {
//     jtk_Assert_assertObject(scope, "The specified class scope is null.");
//     jtk_Assert_assertObject(identifier, "The specified identifier is null.");

//     Context* result = NULL;
//     Context* symbol = scope->symbol;

//     jtk_LinkedStack_t* stack = jtk_LinkedStack_new();
//     /* The things you have to do when you have no inheritance. (-__-) */
//     jtk_LinkedStack_push(stack, symbol);

//     while (!jtk_LinkedStack_isEmpty(stack)) {
//         symbol = (Context*)jtk_LinkedStack_pop(stack);

//         result = (Context*)jtk_HashMap_getValue(scope->symbols, identifier);
//         if (result != NULL) {
//             break;
//         }
//         else {
//             jtk_ArrayList_t* superClasses = symbol->context.asClass.superClasses;
//             int32_t size = jtk_ArrayList_getSize(superClasses);
//             int32_t i;
//             for (i = 0; i < size; i++) {
//                 Context* superClassSymbol = jtk_ArrayList_getValue(superClasses, i);
//                 jtk_LinkedStack_push(stack, superClassSymbol);
//             }
//         }
//     }

//     /* Destroy the stack; not required anymore. */
//     jtk_LinkedStack_delete(stack);

//     return result;
// }

// Context* k_Scope_resolve(Scope* scope, uint8_t* identifier) {
//     Context* result = NULL;
//     switch (scope->type) {
//         case KUSH_SCOPE_COMPILATION_UNIT:
//         case KUSH_SCOPE_FUNCTION:
//         case KUSH_SCOPE_LOCAL: {
//             result = jtk_HashMap_getValue(scope->symbols, (void*)identifier);
//             break;
//         }

//         case KUSH_SCOPE_CLASS: {
//             result = k_Scope_resolveClassMember(scope, identifier);
//             break;
//         }
//     }
//     return result;
// }