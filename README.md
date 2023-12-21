# Chip Programming Language

Chip is a compiled and interpreted programming language. The syntax was designed relatively close to JavaScript and Java but with some extra features.

The whole point of this project is to not only create a functioning compiler but to teach how compilers work in the simplest way possible.

The program uses a recursive descent parser to build a parse tree for the input expression, which can then be used for further processing such as evaluation or code generation.

Unlike JVM, the bytecodes here will be stripped of any symbols and the position of jumps will be calculated during the code generation stage. This is to enable to code to be more independent and also discourages decompilation. 

# Examples
Examples are in the ```examples``` folder

# Compiling it

```$ make```<br />
This will generate the binary ```chip```


# Running it
```$ ./chip examples/socket.chip```<br />
This will compile and run the sample test code in the folder ```examples```

# Data types

Here are the built-in data types of Chip

|  Data Type     |Description                                       |
|----------------|--------------------------------------------------|
|string          | will be evaluated to char\[\] during compilation |
|int             | 64 bit signed integer                            |
|char            | 8 bit signed char (treated as integer)           |
|\<type\>\[\]    | array of type                                    |

# Internals
Chip consist of the following stages:
```mermaid
graph TD
A["Lexer (tokenize.c)"]  
B["Parser (parse.c, parse_expr.c)"]
C["Semantic Analyzer (semantic.c)"]
D["Code Generation (codegen.c)"]
E["Code Optimizer (optimize.c)"]
F["Interpreter (intepreter.c)"]

OUTPUT["Output Bytecode file"]
INPUT["Input Bytecode file"]

A --> B --> C --> D --> E
E --> OUTPUT
INPUT --> F