# Friday, November 03, 2017

cmake_minimum_required (VERSION 3.4.1)

# Project Name

project (Kush)

# Packages

include(FindPkgConfig)

pkg_search_module(JTK REQUIRED jtk)

# Project Version Number

set (KUSH_PROJECT_VERSION_MAJOR 1)
set (KUSH_PROJECT_VERSION_MINOR 0)
set (KUSH_PROJECT_VERSION_PATCH 0)
set (KUSH_PROJECT_VERSION_TWEAK 0)
set (KUSH_PROJECT_VERSION "${KUSH_PROJECT_VERSION_MAJOR}.${KUSH_PROJECT_VERSION_MINOR}.${KUSH_PROJECT_VERSION_PATCH}.${KUSH_PROJECT_VERSION_TWEAK}")

# Options

option (BUILD_examples "Build the examples for ZEN." ON)
option (BUILD_shared "Build shared libraries instead of static libraries." ON)
option (BUILD_tests "Build the unit tests for ZEN." ON)

if (NOT BUILD_TYPE)
    message (WARNING "Build type not specified, falling back to 'Release' mode. To specify build type use: -DCMAKE_BUILD_TYPE=<mode> where <mode> is Debug or Release.")
    set (BUILD_TYPE "Release" CACHE STRING "The type of build: Debug or Release." FORCE)
endif (NOT BUILD_TYPE)

# Source

include_directories ("${PROJECT_SOURCE_DIR}/include")

set (KUSH_PUBLIC_HEADERS)
set (
    KUSH_PRIVATE_HEADERS
)

set (
    KUSH_COMMON_SOURCE

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/Configuration.c
)

set (
    KUSH_COMPILER_SOURCE

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/Compiler.c

    # Abstract Syntax Tree

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstAnnotation.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstAnnotations.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstHelper.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstListener.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstNode.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstPrinter.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/ast/AstWalker.c

    # Generator

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/generator/BinaryEntityGenerator.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/generator/BinaryEntityBuilder.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/generator/ConstantPoolBuilder.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/generator/DataChannel.c

    # Lexer

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/lexer/Lexer.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/lexer/LexerError.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/lexer/Token.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/lexer/TokenStream.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/lexer/TokenType.c

    # Parser

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/parser/Context.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/parser/Parser.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/parser/ParserError.c

    # Symbol Table

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/SymbolDefinitionListener.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/SymbolResolutionListener.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/SymbolTable.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/SymbolLoader.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/Modifier.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/scope/Scope.c

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/symbol/ClassSymbol.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/symbol/FunctionSignature.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/symbol/FunctionSymbol.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/symbol-table/symbol/Symbol.c

    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/support/Error.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/compiler/support/ErrorHandler.c
)

set (
    KUSH_COMPILER_TOOL_SOURCE

    # Instruction.c is from the virtual machine module. Remove references to it.
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/Instruction.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/FieldEntity.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/FunctionEntity.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/attribute/InstructionAttribute.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/Kush.c
)

set (
    KUSH_COMPILER_TOOL_SOURCE

    # Instruction.c is from the virtual machine module. Remove references to it.
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/Instruction.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/FieldEntity.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/FunctionEntity.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/virtual-machine/feb/attribute/InstructionAttribute.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/disassembler/BinaryEntityDisassembler.c
    ${PROJECT_SOURCE_DIR}/source/com/onecube/zen/Kush.c
)

# Compile

set(CMAKE_VERBOSE_MAKEFILE off)

link_directories(${JTK_LIBRARY_DIRS})

message(WARNING ${JTK_CFLAGS})

add_executable(zc
    ${KUSH_PUBLIC_HEADERS}
    ${KUSH_COMMON_SOURCE}
    ${KUSH_COMPILER_SOURCE}
    ${KUSH_COMPILER_TOOL_SOURCE}
    ${KUSH_PRIVATE_HEADERS}
target_link_libraries(zc ${JTK_LIBRARIES} m)
target_include_directories(zc SYSTEM PUBLIC ${JTK_INCLUDE_DIRS})
# target_compile_options(zc PUBLIC -Wall -Wswitch)
target_compile_options(zc PUBLIC -g -w ${JTK_CFLAGS} ${JTK_CFLAGS_OTHER})