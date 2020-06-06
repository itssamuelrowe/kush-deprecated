/*
 * Copyright 2018-2020 Samuel Rowe
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

// Monday, March 16, 2020

#include <stdio.h>
#include <string.h>

/* The JTK_LOGGER_DISABLE constant is defined in Configuration.h. Therefore,
 * make sure it is included before any other file which may
 * include Logger.h!
 */
#include <kush/Configuration.h>

#include <jtk/collection/list/ArrayList.h>
#include <jtk/collection/array/Arrays.h>
#include <jtk/fs/FileInputStream.h>
#include <jtk/fs/Path.h>
#include <jtk/io/BufferedInputStream.h>
#include <jtk/io/InputStream.h>
#include <jtk/log/ConsoleLogger.h>
#include <jtk/core/CStringObjectAdapter.h>
#include <jtk/core/CString.h>

#include <kush/compiler.h>
#include <kush/lexer.h>
#include <kush/parser.h>
#include <kush/error-handler.h>

// Register

void k_Compiler_registerSymbol(k_Compiler_t* compiler, const uint8_t* identifier,
    int32_t identifierSize, k_Symbol_t* symbol) {
    // uint8_t* copy = jtk_CString_newEx(identifier, identifierSize);
    /* NOTE: Do not create a copy of the qualified name because the string belongs
     * to the symbol we are mapping. Therefore, the lifetime of both the qualified
     * name and the symbol are equivalent.
     */
    jtk_HashMap_put(compiler->repository, identifier, symbol);
}

k_Symbol_t* k_Compiler_resolveSymbol(k_Compiler_t* compiler,
    const uint8_t* name, int32_t nameSize) {
    k_Symbol_t* result = jtk_HashMap_getValue(compiler->repository,
        name);
    if (result == NULL) {
        result = k_SymbolLoader_findSymbol(compiler->symbolLoader,
            name, nameSize);
    }
    return result;
}

// Token

void k_Compiler_printToken(k_Token_t* token) {
    printf("[%d-%d:%d-%d:%s:%s]", token->startLine, token->stopLine, token->startColumn + 1, token->stopColumn + 1, token->channel == KUSH_TOKEN_CHANNEL_DEFAULT? "default" : "hidden", k_Lexer_getLiteralName(token->type));
    k_TokenType_t type = k_Token_getType(token);
    if ((type == KUSH_TOKEN_IDENTIFIER) || (type == KUSH_TOKEN_INTEGER_LITERAL) ||
        (type == KUSH_TOKEN_STRING_LITERAL)) {
        printf(" %.*s", token->length, token->text);
    }
    puts("");
}

void k_Compiler_k_Compiler_printTokens(k_Compiler_t* compiler, jtk_ArrayList_t* tokens) {
    int32_t defaultChannel = 0;
    int32_t hiddenChannel = 0;
    int32_t otherChannel = 0;

    int32_t limit = jtk_ArrayList_getSize(tokens);
    int32_t i;
    for (i = 0; i < limit; i++) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(tokens, i);
        k_TokenChannel_t channel = k_Token_getChannel(token);
        if (channel == KUSH_TOKEN_CHANNEL_DEFAULT) {
            defaultChannel++;
        }
        else if (channel == KUSH_TOKEN_CHANNEL_HIDDEN) {
            hiddenChannel++;
        }
        else {
            otherChannel++;
        }
        k_Compiler_printToken(token);
    }
    fflush(stdout);
    fprintf(stdout, "[info] %d tokens were recognized on the default channel.\n", defaultChannel);
    fprintf(stdout, "[info] %d tokens were recognized on the hidden channel.\n", hiddenChannel);
    fprintf(stdout, "[info] %d tokens were recognized on other channels.%s\n", otherChannel, (otherChannel > 0)? " This is surprising to me." : "");
    fprintf(stdout, "[info] %d tokens were recognized in total.\n", limit);
}


/******************************************************************************
 * PathHelper                                                                 *
 ******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jtk/core/CString.h>

bool jtk_PathHelper_exists(const uint8_t* path) {
    /*
    struct stat buffer;
    int32_t result = fstat(path, &buffer);
    return result == 0 && S_ISREG(buffer.st_mode);
    */
   return true;
}

#define JTK_FILE_OPEN_MODE_BUFFERED (1 << 0)

jtk_InputStreat* jtk_PathHelper_read(const uint8_t* path);
jtk_InputStreat* jtk_PathHelper_readEx(const uint8_t* path, uint32_t flags);

jtk_InputStreat* jtk_PathHelper_read(const uint8_t* path) {
    return jtk_PathHelper_readEx(path, JTK_FILE_OPEN_MODE_BUFFERED);
}

jtk_InputStreat* jtk_PathHelper_readEx(const uint8_t* path, uint32_t flags) {
    jtk_InputStreat* result = NULL;

    jtk_FileInputStreat* fileStream = jtk_FileInputStreanewFromString(path);
    result = fileStream->inputStream;

    if ((flags & JTK_FILE_OPEN_MODE_BUFFERED) != 0) {
        jtk_BufferedInputStreat* bufferedStream = jtk_BufferedInputStreanew(fileStream->inputStream);
        result = bufferedStream->inputStream;
    }

    return result;
}

uint8_t* jtk_PathHelper_getParent(const uint8_t* path, int32_t size,
    int32_t* resultSize) {
    size = size < 0? jtk_CString_getSize(path) : size;

    int32_t index = jtk_CString_findLast_c(path, size, JTK_PATH_ELEMENT_SEPARATOR);
    if (resultSize != NULL) {
        *resultSize = index < 0? 0 : index;
    }
    return index < 0? NULL : jtk_CString_substringEx(path, size, 0, index);
}

/******************************************************************************
 * Compiler                                                                   *
 ******************************************************************************/

// Constructor

k_Compiler_t* k_Compiler_new() {
    jtk_ObjectAdapter_t* stringObjectAdapter = jtk_CStringObjectAdapter_getInstance();

    k_Compiler_t* compiler = jtallocate(k_Compiler_t, 1);
    compiler->dumpTokens = false;
    compiler->dumpNodes = false;
    compiler->footprint = false;
    compiler->dumpInstructions = false;
    compiler->inputFiles = jtk_ArrayList_new();
    compiler->currentFileIndex = -1;
    compiler->errorHandler = k_ErrorHandler_new();
    compiler->compilationUnits = NULL;
    compiler->symbolTables = NULL;
    compiler->scopes = NULL;
    compiler->packages = NULL;
    compiler->packageSizes = NULL;
    compiler->symbolLoader = k_SymbolLoader_new(compiler);
    compiler->repository = jtk_HashMap_new(stringObjectAdapter, NULL);
    compiler->trash = NULL;
    compiler->coreApi = false;
#ifdef JTK_LOGGER_DISABLE
    compiler->logger = NULL;
#else
    compiler->logger = jtk_Logger_new(jtk_ConsoleLogger_log);
    jtk_Logger_setLevel(compiler->logger, JTK_LOG_LEVEL_NONE);
#endif

    return compiler;
}

// Destructor

void k_Compiler_delete(k_Compiler_t* compiler) {
    jtk_Assert_assertObject(compiler, "The specified compiler is null.");

    if (compiler->compilationUnits != NULL) {
        jtdeallocate(compiler->compilationUnits);
    }

    if (compiler->symbolTables != NULL) {
        jtdeallocate(compiler->symbolTables);
    }

    if (compiler->scopes != NULL) {
        jtdeallocate(compiler->scopes);
    }

    if (compiler->trash != NULL) {
        int32_t size = jtk_ArrayList_getSize(compiler->trash);
        int32_t i;
        for (i = 0; i < size; i++) {
            /* The ownership of the tokens produced by the lexer
            * is transfered to the token stream which demanded
            * their creation. Therefore, the token stream
            * has to destroy the buffered tokens.
            */
            k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(compiler->trash, i);
            k_Token_delete(token);
        }
        jtk_ArrayList_delete(compiler->trash);
    }

    if (compiler->packages != NULL) {
        int32_t i;
        int32_t inputCount = jtk_ArrayList_getSize(compiler->inputFiles);
        for (i = 0; i < inputCount; i++) {
            jtk_CString_delete(compiler->packages[i]);
        }
        jtdeallocate(compiler->packages);
        jtdeallocate(compiler->packageSizes);
    }

    jtk_Iterator_t* iterator = jtk_HashMap_getKeyIterator(compiler->repository);
    while (jtk_Iterator_hasNext(iterator)) {
        uint8_t* key = (uint8_t*)jtk_Iterator_getNext(iterator);
        jtk_CString_delete(key);
    }
    jtk_Iterator_delete(iterator);
    jtk_HashMap_delete(compiler->repository);

    k_ErrorHandler_delete(compiler->errorHandler);

#ifndef JTK_LOGGER_DISABLE
    jtk_Logger_delete(compiler->logger);
#endif
    jtk_ArrayList_delete(compiler->inputFiles);
    jtdeallocate(compiler);
}

void k_Compiler_onLexerError(k_LexerError_t* error) {
    fprintf(stderr, "[error] %s:%d:%d -- %s\n", error->path, error->line, error->column, error->message);
    fflush(stderr);
}

const uint8_t* k_ErrorCode_messages[] = {
    "None",

    // Lexical Errors

    "Unterminated string literal",
    "Unterminated multi-line comment",
    "Malformed unicode character sequence; expected four hexadecimal digits",
    "Invalid escape sequence",
    "Unknown character",
    "Invalid prefix in integer literal",
    "Expected digit after underscore in integer literal",

    // Syntactical Errors

    "Unexpected token",
    "The try statement expects at least one catch or finally clause.",

    // Semantical Errors

    "Undeclared class",
    "Variable required on left-hand side of assignment",
    "Name refers to a non-class symbol",
    "No suitable constructor found",
    "Undeclared identifier",
    "Variable treated as function",
    "Static initializer with parameters",
    "Redeclaration of symbol as function",
    "Redeclaration of symbol as parameter",
    "Redeclaration of symbol as variable parameter",
    "Multiple function overloads with variable parameter",
    "Duplicate function overload",
    "Function declaration exceeds parameter threshold",
    "Redeclaration of symbol as variable",
    "Redeclaration of symbol as constant",
    "Redeclaration of symbol as label",
    "Redeclaration of symbol as loop parameter",
    "Redeclaration of symbol as catch parameter",
    "Redeclaration of symbol as class",
    "Unknown class",
    "Redeclaration of symbol previously imported",
    "Cannot declare function and class in the same compilation unit",
    "Function declaration causes another function to exceed parameter threshold",

    // General Errors

    "Corrupted binary entity",
    "Binary entity encoded in unrecognizable FEB version"
};

void k_Compiler_printErrors(k_Compiler_t* compiler) {
    jtk_ArrayList_t* errors = k_ErrorHandler_getErrors(compiler->errorHandler);
    int32_t errorCount = jtk_ArrayList_getSize(errors);
    int32_t i;
    for (i = 0; i < errorCount; i++) {
        k_Error_t* error = (k_Error_t*)jtk_ArrayList_getValue(errors, i);
        k_Token_t* token = error->token;
        const char* message = k_ErrorCode_messages[(int32_t)error->code];
        char message0[128];

        char lineNumbers[100];
        if (token->startLine != token->stopLine) {
            sprintf(lineNumbers, "%d-%d", token->startLine, token->stopLine);
        }
        else {
            sprintf(lineNumbers, "%d", token->startLine);
        }

        if (error->expected != KUSH_TOKEN_UNKNOWN) {
            const uint8_t* expectedName = k_Lexer_getLiteralName(error->expected);
            const uint8_t* actualName = k_Lexer_getLiteralName(token->type);
            sprintf(message0, "Expected token '%s', encountered token '%s'",
                expectedName, actualName);
            message = message0;
        }
        fprintf(stderr, "\033[1;31m[error]\033[0m %s:%s:%d-%d: %s\n",
            token->file, lineNumbers, token->startColumn,
            token->stopColumn, message);
    }
}

void k_Compiler_initialize(k_Compiler_t* compiler) {
    int32_t size = jtk_ArrayList_getSize(compiler->inputFiles);
    compiler->compilationUnits = jtallocate(k_ASTNode_t*, size);
    compiler->symbolTables = jtallocate(k_SymbolTable_t*, size);
    compiler->scopes = jtallocate(k_ASTAnnotations_t*, size);
    compiler->packages = jtallocate(uint8_t*, size);
    compiler->packageSizes = jtallocate(int32_t, size);
}

void k_Compiler_buildAST(k_Compiler_t* compiler) {
    k_Lexer_t* lexer = k_Lexer_new(compiler);
    k_TokenStreat* tokens = k_TokenStreanew(compiler, lexer, KUSH_TOKEN_CHANNEL_DEFAULT);
    k_Parser_t* parser = k_Parser_new(compiler, tokens);
    k_ASTPrinter_t* astPrinter = k_ASTPrinter_new();
    k_ASTListener_t* astPrinterASTListener = k_ASTPrinter_getASTListener(astPrinter);

    int32_t size = jtk_ArrayList_getSize(compiler->inputFiles);
    int32_t i;
    for (i = 0; i < size; i++) {
        compiler->currentFileIndex = i;

        const uint8_t* path = (const uint8_t*)jtk_ArrayList_getValue(compiler->inputFiles, i);
        if (!jtk_PathHelper_exists(path)) {
            fprintf(stderr, "[error] Path '%s' does not exist.\n", path);
        }
        else {
            int32_t packageSize;
            uint8_t* package = jtk_PathHelper_getParent(path, -1, &packageSize);
            compiler->packages[i] = package;
            compiler->packageSizes[i] = packageSize;
            jtk_Arrays_replace_b(package, packageSize, '/', '.');

            jtk_InputStreat* stream = jtk_PathHelper_read(path);
            k_Lexer_reset(lexer, stream);

            jtk_Logger_info(compiler->logger, "The lexical analysis phase has started.");

            int32_t previousLexicalErrors = k_ErrorHandler_getErrorCount(compiler->errorHandler);
            k_TokenStreareset(tokens);
            k_TokenStreafill(tokens);
            if (compiler->dumpTokens) {
                k_Compiler_k_Compiler_printTokens(compiler, tokens->tokens);
            }
            int32_t currentLexicalErrors = k_ErrorHandler_getErrorCount(compiler->errorHandler);

            jtk_Logger_info(compiler->logger, "The lexical analysis phase is complete.");

            /* Perform syntax analysis for the current input source file only if
             * there are no lexical errors.
             */
            if (previousLexicalErrors == currentLexicalErrors) {
                jtk_Logger_info(compiler->logger, "The syntactical analysis phase has started.");

                k_ASTNode_t* compilationUnit = k_ASTNode_new(NULL);
                /* We do not have to reset the parser because the same token stream
                 * provides the tokens to the parser. Further, as of this writing,
                 * the parser actually does not maintain any internal state.
                 * If the parser is extended to maintain some kind of internal
                 * state, please make sure that the k_Parser_reset() function
                 * is invoked.
                 */
                k_Parser_reset(parser, tokens);
                k_Parser_compilationUnit(parser, compilationUnit);
                compiler->compilationUnits[i] = compilationUnit;

                jtk_Logger_info(compiler->logger, "The syntactical analysis phase is complete.");

                if (compiler->dumpNodes) {
                    k_ASTWalker_walk(astPrinterASTListener, compilationUnit);
                }
            }

            jtk_InputStreadestroy(stream);
        }
    }

    compiler->trash = tokens->trash;

    k_ASTPrinter_delete(astPrinter);
    k_Parser_delete(parser);
    k_TokenStreadelete(tokens);
    k_Lexer_delete(lexer);

    k_Compiler_printErrors(compiler);
}

void k_Compiler_analyze(k_Compiler_t* compiler) {
    k_SymbolDefinitionListener_t* symbolDefinitionListener = k_SymbolDefinitionListener_new(compiler);
    k_ASTListener_t* symbolDefinitionASTListener = k_SymbolDefinitionListener_getASTListener(symbolDefinitionListener);

    k_SymbolResolutionListener_t* symbolResolutionListener = k_SymbolResolutionListener_new(compiler);
    k_ASTListener_t* symbolResolutionASTListener = k_SymbolResolutionListener_getASTListener(symbolResolutionListener);

    int32_t size = jtk_ArrayList_getSize(compiler->inputFiles);
    int32_t i;
    for (i = 0; i < size; i++) {
        compiler->currentFileIndex = i;
        k_ASTNode_t* compilationUnit = compiler->compilationUnits[i];

        k_SymbolTable_t* symbolTable = k_SymbolTable_new(compiler);
        k_ASTAnnotations_t* scopes = k_ASTAnnotations_new();

        jtk_Logger_info(compiler->logger, "Starting definition phase...");
        k_SymbolDefinitionListener_reset(symbolDefinitionListener, symbolTable,
            scopes, compiler->packages[i], compiler->packageSizes[i]);
        k_ASTWalker_walk(symbolDefinitionASTListener, compilationUnit);
        jtk_Logger_info(compiler->logger, "The symbol definition phase is complete.");

        compiler->symbolTables[i] = symbolTable;
        compiler->scopes[i] = scopes;
    }

    for (i = 0; i < size; i++) {
        compiler->currentFileIndex = i;
        k_ASTNode_t* compilationUnit = compiler->compilationUnits[i];

        k_SymbolTable_t* symbolTable = compiler->symbolTables[i];
        k_ASTAnnotations_t* scopes = compiler->scopes[i];

        jtk_Logger_info(compiler->logger, "Starting symbol resolution phase...");
        k_SymbolResolutionListener_reset(symbolResolutionListener, symbolTable, scopes);
        k_ASTWalker_walk(symbolResolutionASTListener, compilationUnit);
        jtk_Logger_info(compiler->logger, "The symbol resolution phase is complete.");
    }

    k_SymbolResolutionListener_delete(symbolResolutionListener);
    k_SymbolDefinitionListener_delete(symbolDefinitionListener);

    k_Compiler_printErrors(compiler);
}

void k_Compiler_generate(k_Compiler_t* compiler) {
    /*
    int32_t size = jtk_ArrayList_getSize(compiler->inputFiles);
    int32_t i;
    for (i = 0; i < size; i++) {
        compiler->currentFileIndex = i;

        jtk_Logger_info(compiler->logger, "Starting code generation phase...");

        k_SymbolTable_t* symbolTable = compiler->symbolTables[i];
        k_ASTAnnotations_t* scopes = compiler->scopes[i];
        k_ASTNode_t* compilationUnit = compiler->compilationUnits[i];
        k_BinaryEntityGenerator_reset(generator, symbolTable, scopes,
            compilationUnit, compiler->packages[i], compiler->packageSizes[i],
            NULL);

        k_BinaryEntityGenerator_generate(generator);

        jtk_Logger_info(compiler->logger, "The code generation phase is complete.");
    }
    */
}

void k_Compiler_destroyNestedScopes(k_ASTAnnotations_t* scopes) {
    jtk_Assert_assertObject(scopes, "The specified annotations is null.");

    /* There are three algorithms that can help us in the destruction of the
     * symbols and scopes.
     * 1. The symbols are destructed by their enclosing scopes when the scopes
     *    themselves are destroyed.
     * 2. The compilation unit scope can be queried for children. These children
     *    are destroyed in a recursive fashion. During each recursion a symbol
     *    is tested to see if it also represents a scope. In which case,
     *    the scope is queried for its children. The scope part of the symbol
     *    is also destroyed here. The cycle repeats until there are no symbols
     *    and scopes left to destroy.
     * 3. The AST annotations are iterated over to retrieve the scopes.
     *    In each iteration, the children symbols are retrieved and destroyed.
     *    It should be noted that the scopes are also destroyed here.
     *
     * Here, I have chosen to implement the third algorithm due to certain
     * circumstances.
     */

    jtk_ArrayList_t* temporary = jtk_ArrayList_new();
    jtk_Iterator_t* iterator = jtk_HashMap_getValueIterator(scopes->map);
    while (jtk_Iterator_hasNext(iterator)) {
        /* The scopes are created during the definition phase of the symbol table.
         * Therefore, we destroy them here.
         */
        k_Scope_t* scope = (k_Scope_t*)jtk_Iterator_getNext(iterator);

        /* Retrieve the children symbols declared in the current scope. */
        k_Scope_getChildrenSymbols(scope, temporary);

        /* Iterate over the children symbols and destroy them. */
        int32_t limit = jtk_ArrayList_getSize(temporary);
        int32_t i;
        for (i = 0; i < limit; i++) {
            k_Symbol_t* symbol = (k_Symbol_t*)jtk_ArrayList_getValue(temporary, i);
            // k_Symbol_delete(symbol);
        }

        /* At this point, the symbols retrieved form the scope are destroyed.
         * Therefore, remove references to them from the temporary list.
         */
        jtk_ArrayList_clear(temporary);

        /* Destroy the current scope. */
        k_Scope_delete(scope);
    }
    jtk_Iterator_delete(iterator);
    jtk_ArrayList_delete(temporary);
    k_ASTAnnotations_delete(scopes);
}

jtk_ArrayList_t* k_CString_split_c(const uint8_t* sequence, int32_t size,
    uint8_t value, bool inclusive) {
    jtk_ArrayList_t* result = jtk_ArrayList_new();
    int32_t i;
    for (i = 0; i < size; i++) {
        int32_t startIndex = i;
        while ((i < size) && (sequence[i] != value)) {
            i++;
        }
        int32_t stopIndex = ((i < size) && inclusive) ? i + 1 : i;
        uint8_t* substring = jtk_CString_substringEx(sequence, size, startIndex,
            stopIndex);
        jtk_ArrayList_add(result, substring);
    }
    return result;
}

int32_t k_kVirtualMachine_main(char** arguments, int32_t length);

void k_Compiler_printHelp() {
    printf(
        "[Usage]\n"
        "    zc [--tokens] [--nodes] [--footprint] [--instructions] [--core-api] [--log <level>] [--help] <inputFiles> [--run <vmArguments>]\n\n"
        "[Options]\n"
        "    --tokens        Print the tokens recognized by the lexer.\n"
        "    --nodes         Print the AST recognized by the parser.\n"
        "    --footprint     Print diagnostic information about the memory footprint of the compiler.\n"
        "    --instructions  Disassemble the binary entity generated.\n"
        "    --core-api      Disables the internal constant pool function index cache. This flag is valid only when compiling foreign function interfaces.\n"
        "    --run           Run the virtual machine after compiling the source files.\n"
        "    --log           Generate log messages. This flag is valid only if log messages were enabled at compile time.\n"
        "    --help          Print the help message.\n"
        );
}

bool k_Compiler_compileEx(k_Compiler_t* compiler, char** arguments, int32_t length) {
    jtk_Assert_assertObject(compiler, "The specified compiler is null.");

    // TODO: Add the --path flag
    k_SymbolLoader_addDirectory(compiler->symbolLoader, ".");

    char** vmArguments = NULL;
    int32_t vmArgumentsSize = 0;

    bool invalidCommandLine = false;
    int32_t i;
    for (i = 1; i < length; i++) {
        if (arguments[i][0] == '-') {
            if (strcmp(arguments[i], "--tokens") == 0) {
                compiler->dumpTokens = true;
            }
            else if (strcmp(arguments[i], "--nodes") == 0) {
                compiler->dumpNodes = true;
            }
            else if (strcmp(arguments[i], "--footprint") == 0) {
                compiler->footprint = true;
            }
            else if (strcmp(arguments[i], "--instructions") == 0) {
                compiler->dumpInstructions = true;
            }
            else if (strcmp(arguments[i], "--core-api") == 0) {
                compiler->coreApi = true;
            }
            else if (strcmp(arguments[i], "--run") == 0) {
                vmArgumentsSize = length - i + 1;
                if (vmArgumentsSize > 0) {
                    vmArguments = arguments + (i + 1);
                    break;
                }
                else {
                    printf("[error] Please specify the main class.");
                    invalidCommandLine = true;
                }
            }
            else if (strcmp(arguments[i], "--help") == 0) {
                k_Compiler_printHelp();
                exit(0);
            }
            else if (strcmp(arguments[i], "--log") == 0) {
                if ((i + 1) < length) {
                    i++;
                    jtk_LogLevel_t level = JTK_LOG_LEVEL_NONE;
                    if (strcmp(arguments[i], "all") == 0) {
                        level = JTK_LOG_LEVEL_ALL;
                    }
                    else if (strcmp(arguments[i], "finest") == 0) {
                        level = JTK_LOG_LEVEL_FINEST;
                    }
                    else if (strcmp(arguments[i], "finer") == 0) {
                        level = JTK_LOG_LEVEL_FINER;
                    }
                    else if (strcmp(arguments[i], "fine") == 0) {
                        level = JTK_LOG_LEVEL_FINE;
                    }
                    else if (strcmp(arguments[i], "debug") == 0) {
                        level = JTK_LOG_LEVEL_DEBUG;
                    }
                    else if (strcmp(arguments[i], "configuration") == 0) {
                        level = JTK_LOG_LEVEL_CONFIGURATION;
                    }
                    else if (strcmp(arguments[i], "information") == 0) {
                        level = JTK_LOG_LEVEL_INFORMATION;
                    }
                    else if (strcmp(arguments[i], "warning") == 0) {
                        level = JTK_LOG_LEVEL_WARNING;
                    }
                    else if (strcmp(arguments[i], "severe") == 0) {
                        level = JTK_LOG_LEVEL_SEVERE;
                    }
                    else if (strcmp(arguments[i], "error") == 0) {
                        level = JTK_LOG_LEVEL_ERROR;
                    }
                    else if (strcmp(arguments[i], "none") == 0) {
                        level = JTK_LOG_LEVEL_NONE;
                    }
                    else {
                        printf("[error] Unknown log level '%s'\n", arguments[i]);
                        invalidCommandLine = true;
                    }

                    #ifdef JTK_LOGGER_DISABLE
                        printf("[warning] The logger was disabled at compile time. Please consider building k without the `JTK_LOGGER_DISABLE` constant in 'Configuration.h'.\n");
                    #else
                        jtk_Logger_setLevel(compiler->logger, level);
                    #endif
                }
                else {
                    printf("[error] The `--log` flag expects argument specifying log level.");
                    invalidCommandLine = true;
                }
            }
            else {
                printf("[error] Unknown flag `%s`\n", arguments[i]);
            }
        }
        else {
            // jtk_String_t* path = jtk_String_new(arguments[i]);
            // jtk_ArrayList_add(compiler->inputFiles, path);
            jtk_ArrayList_add(compiler->inputFiles, arguments[i]);
        }
    }

    int32_t size = jtk_ArrayList_getSize(compiler->inputFiles);
    bool noErrors = false;
    if (size == 0) {
        fprintf(stderr, "[error] Please specify input files.\n");
    }
    else {
        k_Compiler_initialize(compiler);
        k_Compiler_buildAST(compiler);
        if ((noErrors = !k_ErrorHandler_hasErrors(compiler->errorHandler))) {
            k_Compiler_analyze(compiler);

            if ((noErrors = !k_ErrorHandler_hasErrors(compiler->errorHandler))) {
                k_Compiler_generate(compiler);
            }
        }
    }

    for (i = 0; i < size; i++) {
        /* The ASTAnnotations that stores the scopes is not required anymore.
        * Therefore, destroy it and release the resources it holds.
        */
        if (compiler->scopes[i] != NULL) {
            k_Compiler_destroyNestedScopes(compiler->scopes[i]);
        }
    }

    for (i = 0; i < size; i++) {
        /* The symbol table is not required anymore. Therefore, destroy it
         * and release the resources it holds.
         */
        if (compiler->symbolTables[i] != NULL) {
            k_SymbolTable_delete(compiler->symbolTables[i]);
        }

        if (compiler->compilationUnits[i] != NULL) {
            k_ASTNode_delete(compiler->compilationUnits[i]);
        }
    }

    if (compiler->footprint) {
        int32_t footprint = k_Memory_getFootprint();
        printf("Memory Footprint = %.2f KB\n", footprint / 1024.0f);
    }

    if ((vmArguments != NULL) && noErrors) {
        k_kVirtualMachine_main(vmArguments, vmArgumentsSize);
    }

    // TODO: Return true only if the compilation suceeded.
    return true;
}

bool k_Compiler_compile(k_Compiler_t* compiler) {
    return k_Compiler_compileEx(compiler, NULL, -1);
}
