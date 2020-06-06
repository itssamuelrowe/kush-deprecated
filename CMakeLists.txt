# Friday, November 03, 2017

cmake_minimurequired (VERSION 3.4.1)

# Project Name
project (Kush)

# Packages
include(FindPkgConfig)
pkg_search_module(JTK REQUIRED jtk)

# Project Version Number
set (KUSH_PROJECT_VERSION_MAJOR 0)
set (KUSH_PROJECT_VERSION_MINOR 1)
set (KUSH_PROJECT_VERSION_PATCH 0)
set (KUSH_PROJECT_VERSION_TWEAK 0)
set (KUSH_PROJECT_VERSION "${KUSH_PROJECT_VERSION_MAJOR}.${KUSH_PROJECT_VERSION_MINOR}.${KUSH_PROJECT_VERSION_PATCH}.${KUSH_PROJECT_VERSION_TWEAK}")

# Source
include_directories ("${PROJECT_SOURCE_DIR}/include")

set (
    KUSH_COMPILER_SOURCE

    # General
    ${PROJECT_SOURCE_DIR}/source/kush/Configuration.c
    ${PROJECT_SOURCE_DIR}/source/kush/Compiler.c
    ${PROJECT_SOURCE_DIR}/source/kush/Kush.c

    # Abstract Syntax Tree
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstAnnotation.c
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstAnnotations.c
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstHelper.c
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstListener.c
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstNode.c
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstPrinter.c
    ${PROJECT_SOURCE_DIR}/source/kush/ast/AstWalker.c

    # Lexer
    ${PROJECT_SOURCE_DIR}/source/kush/lexer/Lexer.c
    ${PROJECT_SOURCE_DIR}/source/kush/lexer/LexerError.c
    ${PROJECT_SOURCE_DIR}/source/kush/lexer/Token.c
    ${PROJECT_SOURCE_DIR}/source/kush/lexer/TokenStream.c
    ${PROJECT_SOURCE_DIR}/source/kush/lexer/TokenType.c

    # Parser
    ${PROJECT_SOURCE_DIR}/source/kush/parser/Context.c
    ${PROJECT_SOURCE_DIR}/source/kush/parser/Parser.c
    ${PROJECT_SOURCE_DIR}/source/kush/parser/ParserError.c

    # Symbol Table
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/SymbolDefinitionListener.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/SymbolResolutionListener.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/SymbolTable.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/SymbolLoader.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/Modifier.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/scope/Scope.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/symbol/ClassSymbol.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/symbol/FunctionSignature.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/symbol/FunctionSymbol.c
    ${PROJECT_SOURCE_DIR}/source/kush/symbol-table/symbol/Symbol.c

    ${PROJECT_SOURCE_DIR}/source/kush/support/Error.c
    ${PROJECT_SOURCE_DIR}/source/kush/support/ErrorHandler.c
)

# Compile

set(CMAKE_VERBOSE_MAKEFILE off)
link_directories(${JTK_LIBRARY_DIRS})
add_executable(zc ${KUSH_COMPILER_SOURCE})
target_link_libraries(zc ${JTK_LIBRARIES} m)
target_include_directories(zc SYSTEM PUBLIC ${JTK_INCLUDE_DIRS})
# target_compile_options(zc PUBLIC -Wall -Wswitch)
target_compile_options(zc PUBLIC -g -w ${JTK_CFLAGS} ${JTK_CFLAGS_OTHER})