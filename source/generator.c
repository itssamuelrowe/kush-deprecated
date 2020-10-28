#include <jtk/collection/Pair.h>
#include <kush/generator.h>

#define invalidate(generator) generator->scope = generator->scope->parent

static LLVMValueRef generateConditional(Generator* generator, ConditionalExpression* expression);
static LLVMValueRef generateUnary(Generator* generator, UnaryExpression* expression);
static LLVMValueRef generateSubscript(Generator* generator, Subscript* subscript);
static LLVMValueRef generateFunctionArguments(Generator* generator, FunctionArguments* arguments);
static LLVMValueRef generateMemberAccess(Generator* generator, MemberAccess* access);
static LLVMValueRef generatePostfix(Generator* generator, PostfixExpression* expression);
static LLVMValueRef generateToken(Generator* generator, Token* token);
static LLVMValueRef generateNewExpression(Generator* generator, NewExpression* expression);
static LLVMValueRef generateArray(Generator* generator, ArrayExpression* expression);
static LLVMValueRef generateExpression(Generator* generator, Context* context);

LLVMTypeRef getLLVMVariableType(Type* type) {
    return (type->tag == TYPE_STRUCTURE)?
        LLVMPointerType(type->llvmType, 0) : type->llvmType;
}

LLVMValueRef allocate_ref(Generator* generator) {
    LLVMTypeRef llvmReturnType = LLVMPointerType(LLVMInt8Type(), 0);
    LLVMTypeRef llvmParameterTypes[] = {
        LLVMInt64Type()
    };
    LLVMTypeRef llvmFunctionType = LLVMFunctionType(llvmReturnType, llvmParameterTypes, 1, false);
    LLVMValueRef llvmFunction = LLVMAddFunction(generator->llvmModule, "k_allocate", llvmFunctionType);
    return llvmFunction;
}

LLVMValueRef llvmAllocate;

void generateConstructor(Generator* generator, Structure* structure, LLVMTypeRef* llvmParameterTypes, int parameterCount) {
    LLVMTypeRef llvmStructure = structure->type->llvmType;
    LLVMTypeRef llvmStructurePtr = LLVMPointerType(llvmStructure, 0);
    LLVMTypeRef llvmFunctionType = LLVMFunctionType(llvmStructurePtr, llvmParameterTypes, parameterCount, false);
    LLVMValueRef llvmFunction = LLVMAddFunction(generator->llvmModule, structure->name, llvmFunctionType);
    structure->llvmConstructor = llvmFunction;

    LLVMValueRef llvmParameters[parameterCount];
    for (int j = 0; j < parameterCount; j++) {
        llvmParameters[j] = LLVMGetParam(llvmFunction, j);
    }

    LLVMBasicBlockRef llvmBlock = LLVMAppendBasicBlock(llvmFunction, "");
    generator->llvmBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmBlock);

    LLVMTargetDataRef llvmTargetData = LLVMGetModuleDataLayout(generator->llvmModule);
    uint64_t size = LLVMABISizeOfType(llvmTargetData, llvmStructure);

    LLVMValueRef llvmAllocateArguments[] = {
        LLVMConstInt(LLVMInt64Type(), size, false)
    };
    LLVMValueRef llvmObject = LLVMBuildCall(generator->llvmBuilder, llvmAllocate, llvmAllocateArguments, 1, "");
    LLVMValueRef llvmSelf = LLVMBuildPointerCast(generator->llvmBuilder, llvmObject, llvmStructurePtr, "");

    for (int i = 0; i < parameterCount; i++) {
        LLVMValueRef llvmField = LLVMBuildStructGEP2(generator->llvmBuilder, llvmStructure, llvmSelf, i, "");
        LLVMBuildStore(generator->llvmBuilder, llvmParameters[i], llvmField);
    }

    LLVMBuildRet(generator->llvmBuilder, llvmSelf);
}

void generateStructure(Generator* generator, Structure* structure) {
    int32_t declarationCount = jtk_ArrayList_getSize(structure->declarations);
    int32_t totalVariables = 0;
    for (int i = 0; i < declarationCount; i++) {
        VariableDeclaration* declaration =
            (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);
        totalVariables += jtk_ArrayList_getSize(declaration->variables);
    }

    LLVMTypeRef llvmVariableTypes[totalVariables];
    int k = 0;
    for (int i = 0; i < declarationCount; i++) {
        VariableDeclaration* declaration =
            (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);

        int32_t variableCount = jtk_ArrayList_getSize(declaration->variables);
        for (int j = 0; j < variableCount; j++) {
            Variable* variable = (Variable*)jtk_ArrayList_getValue(declaration->variables, j);
            variable->index = k++;
            llvmVariableTypes[variable->index] = getLLVMVariableType(variable->type);
        }
    }
    
    structure->type->llvmType = LLVMStructType(llvmVariableTypes, totalVariables, false);
    structure->type->llvmDefaultValue = LLVMConstNull(LLVMPointerType(structure->type->llvmType, 0));

    generateConstructor(generator, structure, llvmVariableTypes, totalVariables);
}

void generateStructures(Generator* generator, Module* module) {
    int32_t structureCount = jtk_ArrayList_getSize(module->structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        Structure* structure = (Structure*)jtk_ArrayList_getValue(
            module->structures, j);
        generateStructure(generator, structure);
    }
}

LLVMValueRef generateConditional(Generator* generator, ConditionalExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->condition);
    return left;
}

bool lhs = false;

LLVMValueRef generateAssignment(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left;
    int count = jtk_ArrayList_getSize(expression->others);
    bool previousLhs = lhs;
    if (count > 0) {
        lhs = false;
        jtk_Pair_t* pairRight = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, count - 1);
        LLVMValueRef right = generateExpression(generator, (Context*)pairRight->m_right);

        lhs = true;
        for (int i = count - 2; i >= 0; i++) {
            jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
            LLVMValueRef current = generateExpression(generator, (Context*)pair->m_right);
            right = LLVMBuildStore(generator->llvmBuilder, right, current);
        }
        left = generateExpression(generator, expression->left);
        LLVMBuildStore(generator->llvmBuilder, right, left);
    }
    else {
        lhs = false;
        left = generateExpression(generator, expression->left);
    }
    lhs = previousLhs;

    return left;
}

LLVMValueRef generateLogicalOr(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    /*int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        
        LLVMOpcode opcode;
        TokenType operatorToken = (Token*)pair->m_left->type;
        switch (operatorToken) {
            case TOKEN_LEFT_ANGLE_BRACKET_2: {
                opcode = LLVMShl;
                break;
            }

            case TOKEN_RIGHT_ANGLE_BRACKET_2: {
                opcode = LLVMShr;
                break;
            }

            default: {
                controlError();
                break;
            }
        }
        left = LLVMBuildBinOp(generator->llvmBuilder, opcode, left, right, "");
    }*/

    return left;
}

LLVMValueRef generateLogicalAnd(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    /*int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        
        LLVMOpcode opcode;
        TokenType operatorToken = (Token*)pair->m_left->type;
        switch (operatorToken) {
            case TOKEN_LEFT_ANGLE_BRACKET_2: {
                opcode = LLVMShl;
                break;
            }

            case TOKEN_RIGHT_ANGLE_BRACKET_2: {
                opcode = LLVMShr;
                break;
            }

            default: {
                controlError();
                break;
            }
        }
        left = LLVMBuildBinOp(generator->llvmBuilder, opcode, left, right, "");
    }*/

    return left;
}

LLVMValueRef generateInclusiveOr(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        left = LLVMBuildOr(generator->llvmBuilder, left, right, "");
    }
    return left;
}

LLVMValueRef generateExclusiveOr(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        left = LLVMBuildXor(generator->llvmBuilder, left, right, "");
    }
    return left;
}

LLVMValueRef generateAnd(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        left = LLVMBuildAnd(generator->llvmBuilder, left, right, "");
    }
    return left;
}

// TODO: Equality operators cannot be of the form: `a == b == c` or `a != b == c`.
LLVMValueRef generateEquality(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    if (count > 0) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, 0);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        TokenType operatorToken = ((Token*)pair->m_left)->type;

        if (/*expression->type->tag == TYPE_DECIMAL*/false) {
            LLVMRealPredicate predicate;
            switch (operatorToken) {
                case TOKEN_EQUAL_2: {
                    predicate = LLVMRealOEQ;
                    break;
                }

                case TOKEN_EXCLAMATION_MARK_EQUAL: {
                    predicate = LLVMRealONE;
                    break;
                }

                default: {
                    controlError();
                    break;
                }
            }
            left = LLVMBuildFCmp(generator->llvmBuilder, predicate, left, right, "");
        }
        else {
            LLVMIntPredicate predicate;
            switch (operatorToken) {
                case TOKEN_EQUAL_2: {
                    predicate = LLVMIntEQ;
                    break;
                }

                case TOKEN_EXCLAMATION_MARK_EQUAL: {
                    predicate = LLVMIntNE;
                    break;
                }

                default: {
                    controlError();
                    break;
                }
            }
            left = LLVMBuildICmp(generator->llvmBuilder, predicate, left, right, "");
        }

    }
    return left;
}

LLVMValueRef generateRelational(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        TokenType operatorToken = ((Token*)pair->m_left)->type;

        if (/*expression->type->tag == TYPE_DECIMAL*/false) {
            LLVMRealPredicate predicate;
            switch (operatorToken) {
                case TOKEN_LEFT_ANGLE_BRACKET: {
                    predicate = LLVMRealOLT;
                    break;
                }

                case TOKEN_RIGHT_ANGLE_BRACKET: {
                    predicate = LLVMRealOGT;
                    break;
                }

                case TOKEN_LEFT_ANGLE_BRACKET_EQUAL: {
                    predicate = LLVMRealOLE;
                    break;
                }

                case TOKEN_RIGHT_ANGLE_BRACKET_EQUAL: {
                    predicate = LLVMRealOGE;
                    break;
                }

                default: {
                    controlError();
                    break;
                }
            }
            left = LLVMBuildFCmp(generator->llvmBuilder, predicate, left, right, "");
        }
        else {
            LLVMIntPredicate predicate;
            switch (operatorToken) {
                case TOKEN_LEFT_ANGLE_BRACKET: {
                    predicate = LLVMIntSLT;
                    break;
                }

                case TOKEN_RIGHT_ANGLE_BRACKET: {
                    predicate = LLVMIntSGT;
                    break;
                }

                case TOKEN_LEFT_ANGLE_BRACKET_EQUAL: {
                    predicate = LLVMIntSLE;
                    break;
                }

                case TOKEN_RIGHT_ANGLE_BRACKET_EQUAL: {
                    predicate = LLVMIntSGE;
                    break;
                }

                default: {
                    controlError();
                    break;
                }
            }
            left = LLVMBuildICmp(generator->llvmBuilder, predicate, left, right, "");
        }
    }

    return left;
}

LLVMValueRef generateShift(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        
        LLVMOpcode opcode;
        TokenType operatorToken = ((Token*)pair->m_left)->type;
        switch (operatorToken) {
            case TOKEN_LEFT_ANGLE_BRACKET_2: {
                opcode = LLVMShl;
                break;
            }

            case TOKEN_RIGHT_ANGLE_BRACKET_2: {
                opcode = LLVMLShr;
                break;
            }

            default: {
                controlError();
                break;
            }
        }
        left = LLVMBuildBinOp(generator->llvmBuilder, opcode, left, right, "");
    }

    return left;
}

LLVMValueRef generateAdditive(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        
        LLVMOpcode opcode;
        bool decimalResult = false; // (expression->type->tag == TYPE_DECIMAL);
        TokenType operatorToken = ((Token*)pair->m_left)->type;
        switch (operatorToken) {
            case TOKEN_PLUS: {
                opcode = decimalResult? LLVMFAdd : LLVMAdd;
                break;
            }

            case TOKEN_DASH: {
                opcode = decimalResult? LLVMFSub : LLVMSub;
                break;
            }

            default: {
                controlError();
                break;
            }
        }
        left = LLVMBuildBinOp(generator->llvmBuilder, opcode, left, right, "");
    }

    return left;
}

LLVMValueRef generateMultiplicative(Generator* generator, BinaryExpression* expression) {
    LLVMValueRef left = generateExpression(generator, expression->left);
    int count = jtk_ArrayList_getSize(expression->others);
    for (int i = 0; i < count; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->others, i);
        LLVMValueRef right = generateExpression(generator, (Context*)pair->m_right);
        
        LLVMOpcode opcode;
        bool decimalResult = false; // (expression->type->tag == TYPE_DECIMAL);
        TokenType operatorToken = ((Token*)pair->m_left)->type;
        switch (operatorToken) {
            case TOKEN_ASTERISK: {
                opcode = decimalResult? LLVMFMul : LLVMMul;
                break;
            }

            case TOKEN_FORWARD_SLASH: {
                opcode = decimalResult? LLVMFDiv : LLVMSDiv;
                break;
            }

            case TOKEN_MODULUS: {
                opcode = decimalResult? LLVMFRem : LLVMSRem;
                break;
            }

            default: {
                controlError();
                break;
            }
        }
        left = LLVMBuildBinOp(generator->llvmBuilder, opcode, left, right, "");
    }

    return left;
}

LLVMValueRef generateUnary(Generator* generator, UnaryExpression* expression) {
    LLVMValueRef result;
    Token* operator = expression->operator;
    if (operator != NULL) {
        LLVMValueRef llvmValue = generateExpression(generator, expression->expression);
        switch (operator->type) {
            case TOKEN_DASH: {
                result = /*(expression->type->tag == TYPE_DECIMAL)?
                    LLVMBuildFNeg(generator, llvmValue, "") :*/ LLVMBuildNeg(generator, llvmValue, "");
                break;
            }

            case TOKEN_EXCLAMATION_MARK: {
                result = LLVMBuildNot(generator, llvmValue, "");
                break;
            }

            case TOKEN_TILDE: {
                result = LLVMBuildNot(generator, llvmValue, "");
                break;
            }

            default: {
                controlError();
                break;
            }
        }
    }
    else {
        result = generateExpression(generator, expression->expression);
    }
    return result;
}

// void generateSubscript(Generator* generator, Subscript* subscript) {
//     fprintf(generator->output, "->value[");
//     generateExpression(generator, (Context*)subscript->expression);
//     fprintf(generator->output, "]");
// }

LLVMValueRef generateFunctionCall(Generator* generator, Function* function, FunctionArguments* arguments) {
    int32_t count = jtk_ArrayList_getSize(arguments->expressions);
    LLVMValueRef* llvmArguments = allocate(LLVMValueRef, count);
    for (int i = 0; i < count; i++) {
        Context* context = (Context*)jtk_ArrayList_getValue(arguments->expressions, i);
        llvmArguments[i] = generateExpression(generator, context);
    }
    LLVMValueRef result = LLVMBuildCall(generator->llvmBuilder, function->llvmValue, llvmArguments, count, "");
    deallocate(llvmArguments);
    return result;
}

LLVMValueRef generatePostfix(Generator* generator, PostfixExpression* expression) {
    LLVMValueRef result;
    int32_t count = jtk_ArrayList_getSize(expression->postfixParts);

    Symbol* symbol = NULL;
    if (expression->token) {
        Token* token = (Token*)expression->primary;
        if (token->type == TOKEN_IDENTIFIER) {
            symbol = resolveSymbol(generator->scope, token->text);
            if (symbol->tag == CONTEXT_VARIABLE) {
                Variable* variable = (Variable*)symbol;
                if (lhs) {
                    result = variable->llvmValue;
                    if (count > 0) {
                        result = LLVMBuildLoad(generator->llvmBuilder, result, "");
                    }
                }
                else {
                    if (variable->parameter) {
                        result = variable->llvmValue;
                    }
                    else {
                        result = LLVMBuildLoad(generator->llvmBuilder, variable->llvmValue, "");
                    }
                }
            }
        }
        else {
            result = generateToken(generator, token);
        }
    }
    else {
        result = generateExpression(generator, (Context*)expression->primary);
    }

    for (int i = 0; i < count; i++) {
        Context* postfix = (Context*)jtk_ArrayList_getValue(
            expression->postfixParts, i);

        /*if (postfix->tag == CONTEXT_SUBSCRIPT) {
            generateSubscript(generator, (Subscript*)postfix);
        }
        else*/ if (postfix->tag == CONTEXT_FUNCTION_ARGUMENTS) {
            // assert(symbol->tag == CONTEXT_FUNCTION)
            result = generateFunctionCall(generator, (Function*)symbol, (FunctionArguments*)postfix);
        }
        else if (postfix->tag == CONTEXT_MEMBER_ACCESS) {
            MemberAccess* access = (MemberAccess*)postfix;
            Variable* variable = (Variable*)resolveSymbol(access->previous->structure->scope, access->identifier->text);
            result = LLVMBuildStructGEP2(generator->llvmBuilder, access->previous->structure->type->llvmType, result, variable->index, "");
            if ((i + 1 == count && !lhs) || (access->previous->tag == TYPE_STRUCTURE && i + 1 < count)) {
                result = LLVMBuildLoad(generator->llvmBuilder, result, "");
            }
        }
        else {
            controlError();
            break;
        }
    }
    return result;
}

LLVMValueRef generateToken(Generator* generator, Token* token) {
    LLVMValueRef result;
    switch (token->type) {
        case TOKEN_KEYWORD_TRUE: 
        case TOKEN_KEYWORD_FALSE: {
            result = LLVMConstInt(LLVMInt1Type(), token->type == TOKEN_KEYWORD_TRUE, false);
            break;
        }

        case TOKEN_INTEGER_LITERAL: {
            // TODO
            result = LLVMConstIntOfStringAndSize(LLVMInt32Type(), token->text, token->length, 10);
            break;
        }

        case TOKEN_FLOATING_POINT_LITERAL: {
            result = LLVMConstRealOfStringAndSize(LLVMFloatType(), token->text, token->length);
            break;
        }

        // case TOKEN_STRING_LITERAL: {
        //     fprintf(generator->output, "makeString(runtime, \"%.*s\")", token->length - 2, token->text + 1);
        //     break;
        // }

        case TOKEN_KEYWORD_NULL: {
            result = LLVMConstNull(NULL);
            break;
        }

        default: {
            controlError();
            break;
        }
    }
    return result;
}

LLVMValueRef generateObjectExpression(Generator* generator, NewExpression* expression) {
    Type* type = expression->type;
    Structure* structure = type->structure;

    int totalVariables = 0;
    int declarationCount = jtk_ArrayList_getSize(structure->declarations);
    for (int i = 0; i < declarationCount; i++) {
        VariableDeclaration* declaration =
            (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);
        totalVariables += jtk_ArrayList_getSize(declaration->variables);
    }

    LLVMValueRef llvmArguments[totalVariables];
    for (int i = 0; i < totalVariables; i++) {
        llvmArguments[i] = NULL;
    }

    /* For each entry in the object intializer, we apply linear search on the declared variables
     * to find the index of the field. A better solution would be to use a hash map.
     */
    int32_t entryCount = jtk_ArrayList_getSize(expression->entries);
    for (int i = 0; i < entryCount; i++) {
        jtk_Pair_t* pair = (jtk_Pair_t*)jtk_ArrayList_getValue(expression->entries, i);
        Token* identifier = (Token*)pair->m_left;
        
        int index = 0;
        for (int j = 0; j < declarationCount; j++) {
            VariableDeclaration* declaration =
                (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, j);
            int variableCount = jtk_ArrayList_getSize(declaration->variables);
            for (int k = 0; k < variableCount; k++) {
                Variable* variable = (Variable*)jtk_ArrayList_getValue(declaration->variables, k);
                if (jtk_CString_equals(variable->name, variable->nameSize,
                    identifier->text, identifier->length)) {
                    goto found;
                }
                index++;
            }
        }
        found:

        if (index != totalVariables) {
            llvmArguments[index] = generateExpression(generator, (Context*)pair->m_right);
        }
    }

    int m = 0;
    for (int i = 0; i < declarationCount; i++) {
        VariableDeclaration* declaration =
            (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);
        int variableCount = jtk_ArrayList_getSize(declaration->variables);
        for (int j = 0; j < variableCount; j++) {
            if (llvmArguments[m] == NULL) {
                Variable* variable = jtk_ArrayList_getValue(declaration->variables, j);
                llvmArguments[m] = variable->type->llvmDefaultValue;
            }
            m++;
        }
    }

    return LLVMBuildCall(generator->llvmBuilder, structure->llvmConstructor, llvmArguments,
        totalVariables, "");
}

LLVMValueRef generateNew(Generator* generator, NewExpression* expression) {
    LLVMValueRef llvmResult;
    Type* type = expression->type;
    if (type->tag == TYPE_ARRAY) {
        /*
        int32_t limit = jtk_ArrayList_getSize(expression->expressions);
        int32_t i;
        for (i = 0; i < limit; i++) {
            Context* context = (Context*)jtk_ArrayList_getValue(expression->expressions, i);
            generateExpression(generator, context);
        }
        */
    }
    else {
        llvmResult = generateObjectExpression(generator, expression);
    }
    return llvmResult;
}

LLVMValueRef generateExpression(Generator* generator, Context* context) {
    switch (context->tag) {
        case CONTEXT_ASSIGNMENT_EXPRESSION: {
            return generateAssignment(generator, (BinaryExpression*)context);
            break;
        }

        case CONTEXT_LOGICAL_OR_EXPRESSION: {
            return generateLogicalOr(generator, (BinaryExpression*)context);
            break;
        }
        
        case CONTEXT_LOGICAL_AND_EXPRESSION: {
            return generateLogicalAnd(generator, (BinaryExpression*)context);
            break;
        }
        
        case CONTEXT_INCLUSIVE_OR_EXPRESSION: {
            return generateInclusiveOr(generator, (BinaryExpression*)context);
            break;
        }
        
        case CONTEXT_EXCLUSIVE_OR_EXPRESSION: {
            return generateExclusiveOr(generator, (BinaryExpression*)context);
            break;
        }
        
        case CONTEXT_AND_EXPRESSION: {
            return generateAnd(generator, (BinaryExpression*)context);
            break;
        }

        case CONTEXT_EQUALITY_EXPRESSION: {
            return generateEquality(generator, (BinaryExpression*)context);
            break;
        }

        case CONTEXT_RELATIONAL_EXPRESSION: {
            return generateRelational(generator, (BinaryExpression*)context);
            break;
        }
        
        case CONTEXT_SHIFT_EXPRESSION: {
            return generateShift(generator, (BinaryExpression*)context);
            break;
        }

        case CONTEXT_ADDITIVE_EXPRESSION: {
            return generateAdditive(generator, (BinaryExpression*)context);
            break;
        }
        
        case CONTEXT_MULTIPLICATIVE_EXPRESSION: {
            return generateMultiplicative(generator, (BinaryExpression*)context);
            break;
        }

        case CONTEXT_CONDITIONAL_EXPRESSION: {
            return generateConditional(generator, (ConditionalExpression*)context);
            break;
        }

        case CONTEXT_UNARY_EXPRESSION: {
            return generateUnary(generator, (UnaryExpression*)context);
            break;
        }

        case CONTEXT_POSTFIX_EXPRESSION: {
            return generatePostfix(generator, (PostfixExpression*)context);
            break;
        }

        case CONTEXT_NEW_EXPRESSION: {
            return generateNew(generator, (NewExpression*)context);
            break;
        }
/*
        case CONTEXT_ARRAY_EXPRESSION: {
            return generateArray(generator, (ArrayExpression*)context);
           break;
        }
*/
        default: {
            controlError();
            break;
        }
    }
    LLVMValueRef dummy;
    return dummy;
}

void generateIfClause(Generator* generator, IfClause* clause,
    LLVMBasicBlockRef llvmConditionBlock, LLVMBasicBlockRef llvmThenBlock,
    LLVMBasicBlockRef llvmElseBlock, LLVMBasicBlockRef llvmExitBlock) {
    if (llvmConditionBlock != NULL) {
        LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmConditionBlock);
    }    
    LLVMValueRef llvmCondition = generateExpression(generator, (Context*)clause->expression);
    LLVMBuildCondBr(generator->llvmBuilder, llvmCondition, llvmThenBlock, llvmElseBlock);

    LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmThenBlock);
    generateBlock(generator, clause->body);
    LLVMBuildBr(generator->llvmBuilder, llvmExitBlock);
}

void generateIf(Generator* generator, IfStatement* statement) {
    LLVMValueRef llvmFunction = generator->function->llvmValue;

    int elseIfClauseCount = jtk_ArrayList_getSize(statement->elseIfClauses);
    /* Break down:
     *     - 1 block => if clause
     *     - N * 2 blocks => else if clause blocks + condition blocks
     *     - 0 or 1 block => else clause
     *     - 1 block => outside if statement
     */
    int blockCount = 2 + elseIfClauseCount * 2 + (statement->elseClause != NULL? 1 : 0);
    LLVMBasicBlockRef* llvmBlocks = allocate(LLVMBasicBlockRef, blockCount);
    for (int i = 0; i < blockCount; i++) {
        llvmBlocks[i] = LLVMAppendBasicBlock(llvmFunction, "");
    }

    /* 0 => If clause block
     * 1 => Else clause block / Else if clause block / Exit block
     */
    LLVMBasicBlockRef llvmExitBlock = llvmBlocks[blockCount - 1];
    generateIfClause(generator, statement->ifClause, NULL, llvmBlocks[0], llvmBlocks[1], llvmExitBlock);

    for (int i = 0; i < elseIfClauseCount; i++) {
        IfClause* clause = (IfClause*)jtk_ArrayList_getValue(statement->elseIfClauses, i);
        /* base + 0 => Current condition block
         * base + 1 => Current clause block
         * base + 2 => Next clause block or exit block
         */
        int baseIndex = (i * 2) + 1;
        generateIfClause(generator, clause, llvmBlocks[baseIndex], llvmBlocks[baseIndex + 1],
            llvmBlocks[baseIndex + 2], llvmExitBlock);
    }

    if (statement->elseClause != NULL) {
        /* blockCount - 2 => Else clause
         * blockCount - 1 => Exit block
         */
        LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmBlocks[blockCount - 2]);
        generateBlock(generator, statement->elseClause);
        LLVMBuildBr(generator->llvmBuilder, llvmExitBlock);
    }

    LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmExitBlock);
}

void generateIterative(Generator* generator, IterativeStatement* statement) {
    // assert (statement->keyword->type == TOKEN_KEYWORD_WHILE);
    LLVMValueRef llvmFunction = generator->function->llvmValue;
    LLVMBasicBlockRef llvmConditionBlock = LLVMAppendBasicBlock(llvmFunction, "");
    LLVMBasicBlockRef llvmThenBlock = LLVMAppendBasicBlock(llvmFunction, "");
    LLVMBasicBlockRef llvmElseBlock = LLVMAppendBasicBlock(llvmFunction, "");

    /* Make sure that the current basic block has a terminator. */
    LLVMBuildBr(generator->llvmBuilder, llvmConditionBlock);

    LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmConditionBlock);
    LLVMValueRef llvmCondition = generateExpression(generator, (Context*)statement->expression);
    LLVMBuildCondBr(generator->llvmBuilder, llvmCondition, llvmThenBlock, llvmElseBlock);

    LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmThenBlock);
    generateBlock(generator, statement->body);
    LLVMBuildBr(generator->llvmBuilder, llvmConditionBlock);

    LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmElseBlock);
}

void generateVariableDeclaration(Generator* generator, VariableDeclaration* statement) {
    int count = jtk_ArrayList_getSize(statement->variables);
    for (int j = 0; j < count; j++) {
        Variable* variable = (Variable*)jtk_ArrayList_getValue(
            statement->variables, j);
        LLVMTypeRef llvmType = getLLVMVariableType(variable->type);
        variable->llvmValue = LLVMBuildAlloca(generator->llvmBuilder, llvmType, "");

        LLVMValueRef llvmInitializer = (variable->expression != NULL)?
            generateExpression(generator, (Context*)variable->expression) : variable->type->llvmDefaultValue;
        LLVMBuildStore(generator->llvmBuilder, llvmInitializer, variable->llvmValue);
    }
}

void generateBlock(Generator* generator, Block* block) {
    generator->scope = block->scope;
    int32_t statementCount = jtk_ArrayList_getSize(block->statements);
    for (int i = 0; i < statementCount; i++) {
        Context* context = (Context*)jtk_ArrayList_getValue(
            block->statements, i);
        switch (context->tag) {
            case CONTEXT_ITERATIVE_STATEMENT: {
                generateIterative(generator, (IterativeStatement*)context);
                break;
            }

            case CONTEXT_IF_STATEMENT: {
                generateIf(generator, (IfStatement*)context);
                break;
            }

            case CONTEXT_VARIABLE_DECLARATION: {
                generateVariableDeclaration(generator, (VariableDeclaration*)context);
                break;
            }

            case CONTEXT_ASSIGNMENT_EXPRESSION: {
                generateExpression(generator, context);
                break;
            }

            case CONTEXT_RETURN_STATEMENT: {
                ReturnStatement* statement = (ReturnStatement*)context;
                LLVMValueRef llvmValue = generateExpression(generator, (Context*)statement->expression);
                LLVMBuildRet(generator->llvmBuilder, llvmValue);
                break;
            }

            default: {
                controlError();
                break;
            }
        }
    }

    invalidate(generator);
}

void generateFunction(Generator* generator, Function* function) {
    generator->scope = function->scope;
    generator->function = function;

    int32_t parameterCount = jtk_ArrayList_getSize(function->parameters);
    LLVMTypeRef* llvmParameterTypes = allocate(LLVMTypeRef, parameterCount);

    for (int i = 0; i < parameterCount; i++) {
        Variable* parameter = (Variable*)jtk_ArrayList_getValue(function->parameters, i);
        llvmParameterTypes[i] = getLLVMVariableType(parameter->type);
    }

    // TODO: Variable parameter

    /* function->llvmType =  */
    LLVMTypeRef llvmFunctionType = LLVMFunctionType(function->returnType->llvmType, llvmParameterTypes, parameterCount, false);
    LLVMValueRef llvmFunction = LLVMAddFunction(generator->llvmModule, function->name, llvmFunctionType);
    function->llvmValue = llvmFunction;

    if (function->body != NULL) {
        for (int i = 0; i < parameterCount; i++) {
            Variable* parameter = (Variable*)jtk_ArrayList_getValue(function->parameters, i);
            parameter->llvmValue = LLVMGetParam(llvmFunction, i);
            parameter->parameter = true;
        }

        LLVMBasicBlockRef llvmBlock = LLVMAppendBasicBlock(llvmFunction, "");
        generator->llvmBuilder = LLVMCreateBuilder();
        LLVMPositionBuilderAtEnd(generator->llvmBuilder, llvmBlock);

        generateBlock(generator, function->body);
    }

    deallocate(llvmParameterTypes);
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

bool generateLLVM(Generator* generator, Module* module, const char* name) {
    LLVMContextRef context = LLVMContextCreate();
    // LLVMModuleRef llvmModule = LLVMModuleCreateWithNameInContext(name, context);
    LLVMModuleRef llvmModule = LLVMModuleCreateWithName(name);
	LLVMSetDataLayout(llvmModule, "");
	LLVMSetTarget(llvmModule, LLVMGetDefaultTargetTriple());

    // generator->context = context;
    generator->llvmModule = llvmModule;
    generator->llvmBuilder = LLVMCreateBuilder();

    llvmAllocate = allocate_ref(generator);
    
    generateStructures(generator, module);
    generateFunctions(generator, module);

	char *error = NULL;
	bool invalid = LLVMVerifyModule(generator->llvmModule, LLVMAbortProcessAction, &error);
	if (!invalid) {
		char* data = LLVMPrintModuleToString(generator->llvmModule);
		fprintf(generator->output, data);
		LLVMDisposeMessage(data);
	}
    else {
        fprintf(stderr, "[error] %s\n", error);
    }
    LLVMDisposeMessage(error);

    LLVMContextDispose(context);
	
	return !invalid;
}

void generateIR(Generator* generator, Module* module) {
    Compiler* compiler = generator->compiler;
    const uint8_t* path = (const uint8_t*)jtk_ArrayList_getValue(compiler->inputFiles,
        compiler->currentFileIndex);

    int pathSize = jtk_CString_getSize(path);
    uint8_t* sourceName = allocate(uint8_t, pathSize - 1);

    int32_t i;
    for (i = 0; i < pathSize - 4; i++) {
        sourceName[i] = path[i];
    }
    sourceName[i] = 'l';
    sourceName[i + 1] = 'l';
    sourceName[i + 2] = '\0';

    generator->output = fopen(sourceName, "w+");
    generateLLVM(generator, module, sourceName);

    fclose(generator->output);
    deallocate(sourceName);
}

Generator* newGenerator(Compiler* compiler) {
    Generator* generator = allocate(Generator, 1);
    generator->compiler = compiler;
    generator->scope = NULL;
    generator->function = NULL;
    return generator;
}

void deleteGenerator(Generator* generator) {
    deallocate(generator);
}
