# jcc

A compiler for my own small programming language, written in C17.

jcc is **not** a C compiler. It compiles my own language, and the compiler itself is written in C.

I started this project to learn C. Before this, I had only written really small single-file programs in C. At that time I was already building another compiler in Rust together with my brother ([rython](https://github.com/Jannis241/rython)), so I thought it would be a good idea to build a simpler version of the same thing in C. That way I could learn how bigger multi-file projects in C work and get more familiar with the language.

## Status

This is version 1. The goal of V1 is to get something working first, not to make it perfect. It's also my first bigger project in C, so some of the design choices I made at the beginning are not perfect.

| Stage | Status |
|---|---|
| Lexer | ✅ done |
| Parser (AST) | ✅ done |
| Semantic analysis | currently working on |
| Code generation | planned for later |

## The language

The language doesn't have one big idea behind it. It's just a mix of things I like from other languages, especially from Rust. For example, I use `=>` for the return type of a function because I think it makes the code clearer to read, and every variable has an explicit type.

```
struct Point {
    x: int,
    y: int,
}

enum Result {
    Ok,
    Err,
}

const answer: int = 42;

fn distance_squared(a: Point, b: Point) => int {
    let dx: int = a.x - b.x;
    let dy: int = a.y - b.y;
    return dx * dx + dy * dy;
}

fn loops(limit: int) => int {
    let i: int = 0;

    while (i < limit) {
        i++;
        if (i >= 5) {
            break;
        }
    }

    for (let j: int = 0; j < 3; j++) {
        print("for-let", j);
    }

    return i;
}
```

What the parser understands so far:

- types: `int`, `float`, `bool`, `char`, `String`, arrays like `[int]` and nested arrays like `[[int]]`
- structs, enums and constants
- functions with parameters and return types
- `let` variables, blocks and scopes
- `if` / `else if` / `else`, `while` and C-style `for` loops, `break` and `continue`
- arithmetic, comparison, logical and bitwise operators, `++` / `--`, `+=` and similar
- struct literals, array literals, field access and indexing

The full example with all the syntax is in [`examples/features.jc`](examples/features.jc).

## How it works

```
source code -> lexer -> tokens -> parser -> AST -> semantic analysis -> (code generation)
```

**Lexer** (`src/lexer.c`): goes through the source code character by character and turns it into tokens. It handles numbers, floats, chars with escape sequences like `'\n'`, strings and comments. If something is wrong, for example a string that is never closed, it reports an error with the exact position.

**Parser** (`src/parser.c`): a handwritten recursive descent parser that builds the AST. For expressions there is one function per precedence level. Each one calls the next, stronger one:

```
or -> and -> equality -> comparison -> bit or -> bit xor -> bit and
   -> bit shift -> additive -> multiplicative -> cast -> unary -> postfix -> primary
```

This way `1 + 2 * 3` is parsed as `1 + (2 * 3)` without needing any extra rules.

**Semantic analysis** (`src/semantic.c`, work in progress): this is what I am working on right now, and it's the most complicated part so far. It will use a symbol table to check things like:

- whether variables, functions and types actually exist
- whether the types match, for example when assigning a value or calling a function
- whether a function returns the right type

But i still need to figure out how exactly i want to built this stage of my compiler.

## Error messages

I wanted error messages that are actually helpful like rust:

```
error: expected expression, got Semicolon
 --> bad.jc:2:14
  |
2 |   let x = 5 +;
  |              ^
```

At the moment the compiler stops at the first error

## How I built it

I read a few online articles about how compilers are structured in general. Most of what I know comes from earlier attempts at building compilers and from figuring things out myself. I didn't follow a tutorial for this project.

## Build and run

You need a C compiler (gcc or clang) and `make`.

```sh
make                  # release build -> bin/release/app
make MODE=debug       # debug build
make MODE=sanitize    # build with AddressSanitizer and UBSan

./bin/release/app examples/features.jc
```

At the moment jcc prints the AST of the program.

## Project structure

```
src/lexer.c       turns the source code into tokens
src/parser.c      builds the AST from the tokens
src/ast.c         AST nodes and printing
src/semantic.c    semantic analysis (work in progress)
include/vector.h  generic vector, written with macros
examples/         example programs
```

## Some decisions I made for V1

- **Memory:** Everything that is created while compiling lives until the program ends. This can obviously be improved, but I decided to leak memory on purpose to keep V1 simple.
- **Vector macro:** The vector macro is used in a lot of places, which makes the binary bigger. For V1 that's okay.

## What I want to improve later

- Tokens should not copy their text anymore, but point into the source code (slices).
- AST names and literals can then point to tokens or source slices too.
- Allocate AST nodes in an arena instead of many single `malloc` calls.
- Choose the start size of vectors depending on where they are used.
- When the parser is more stable: proper `free` functions or one central `CompilerContext`.
- Report more than one error at a time.

## What I learned so far

- A lot more about C in general, since learning the language was the main reason for this project
- How to structure a bigger C project with multiple files and header files
- How a lexer and a recursive descent parser work, and how operator precedence can be handled with one function per level
- How to write generic data structures in C with macros
