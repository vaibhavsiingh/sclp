# C-Like Language Compiler

A compiler implementation for a C-like programming language, developed as a series of **five assignments** covering the complete compilation pipeline—from lexical analysis and parsing to intermediate representations and MIPS assembly generation.

## Overview

The project implements a compiler supporting core C-like language features including:

* Nested scopes and symbol tables
* Functions and function calls
* Arrays
* Pointers
* Expressions, statements, and control flow
* Static semantic analysis
* Multiple intermediate representations
* MIPS assembly generation
* Register allocation

The compiler progressively transforms source code through several stages:

```text
C-like Source Code
        │
        ▼
 Lexical Analysis
     (Lex)
        │
        ▼
 Syntax Analysis
    (Yacc/Parser)
        │
        ▼
      AST
        │
        ▼
 Three Address Code
        │
        ▼
 Register Transfer Language
        │
        ▼
 Register Allocation
        │
        ▼
 MIPS Assembly
```

## Assignments

### Assignment 1 — Lexical and Syntax Analysis

Implemented the front end of the compiler using **Lex and Yacc**.

Key components:

* Lexical analysis and tokenization
* Bottom-up parsing
* Grammar specification
* Syntax error detection
* Recognition of C-like language constructs

### Assignment 2 — Abstract Syntax Tree and Semantic Analysis

Built the compiler's structural representation of source programs using **Abstract Syntax Trees (ASTs)**.

Key components:

* AST construction
* Symbol table management
* Nested scope handling
* Type and semantic checks
* Variables, expressions, arrays, pointers, and functions

### Assignment 3 — Three Address Code

Implemented generation of **Three Address Code (TAC)** as an intermediate representation.

Key components:

* Expression translation
* Temporary variable generation
* Control-flow representation
* Function calls and returns
* Array and pointer operations
* Translation of AST nodes into TAC

### Assignment 4 — Register Transfer Language

Translated the intermediate representation into a lower-level **Register Transfer Language (RTL)**.

Key components:

* Instruction-level representation
* Explicit data movement
* Arithmetic and logical operations
* Memory access operations
* Function call conventions
* Control-flow translation

### Assignment 5 — MIPS Code Generation and Register Allocation

Implemented the final backend to generate **MIPS assembly code**.

Key components:

* MIPS instruction generation
* Stack-frame management
* Function prologue and epilogue generation
* Memory and pointer operations
* Register allocation
* Translation of RTL into executable MIPS assembly

## Compiler Pipeline

The compiler follows a traditional multi-stage architecture:

### 1. Lexical Analysis

The lexer converts the source program into a stream of tokens using **Lex**.

```text
Source Program → Tokens
```

### 2. Parsing

A **Yacc-based bottom-up parser** validates the token stream against the language grammar and constructs the program structure.

```text
Tokens → Parse Tree / AST
```

### 3. Semantic Analysis

The compiler maintains symbol tables and scope information to validate declarations, identifiers, types, functions, arrays, and pointers.

Nested scopes are handled through hierarchical symbol-table structures.

### 4. Intermediate Representation

The AST is progressively lowered into **Three Address Code**, providing a simpler representation suitable for subsequent compiler passes.

Example:

```text
t1 = a + b
t2 = t1 * c
x  = t2
```

### 5. Lower-Level Representation

TAC is translated into **Register Transfer Language**, making data movement and machine-level operations explicit.

### 6. Code Generation

The backend converts RTL instructions into **MIPS assembly**, including stack management and function-call handling.

### 7. Register Allocation

Registers are assigned to intermediate values to reduce unnecessary memory accesses and produce more efficient MIPS code.

## Supported Language Features

The compiler supports a range of C-like constructs, including:

* Primitive variables and expressions
* Arithmetic and logical operations
* Conditional statements
* Loops
* Functions and function calls
* Return statements
* Nested blocks and scopes
* Arrays and indexed access
* Pointers and dereferencing
* Address-of operations
* Memory accesses
* Intermediate representations
* MIPS code generation

## Tools and Technologies

* **C/C++** — Compiler implementation
* **Lex/Flex** — Lexical analysis
* **Yacc/Bison** — Bottom-up parsing
* **MIPS** — Target instruction set
* **AST** — High-level intermediate representation
* **Three Address Code** — Intermediate representation
* **RTL** — Low-level intermediate representation

## Repository Structure

The repository is organized around the five compiler assignments. The exact file organization may vary depending on the assignment, but the overall progression is:

```text
.
├── Assignment 1/
│   └── Lexer & Parser
├── Assignment 2/
│   └── AST & Semantic Analysis
├── Assignment 3/
│   └── Three Address Code
├── Assignment 4/
│   └── Register Transfer Language
├── Assignment 5/
│   └── MIPS & Register Allocation
└── README.md
```

## Compilation

For assignments using Lex/Yacc, the typical build process is:

```bash
lex lexer.l
yacc -d parser.y
gcc lex.yy.c y.tab.c -o compiler
```

The exact build commands and dependencies may differ across assignments; refer to the respective assignment directories for implementation-specific instructions.

## Key Learning Outcomes

This project provided hands-on experience with:

* Compiler front-end and back-end design
* Lexical analysis and parsing
* Bottom-up parsing with Yacc
* AST-based program representation
* Symbol tables and scope management
* Intermediate representation design
* Code generation
* Function-call and stack-frame management
* Assembly-level programming
* Register allocation
* Translation between multiple abstraction levels

## Authors

Developed as part of a compiler construction coursework project.
