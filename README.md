# Chip Programming Language

Chip is a compiled and interpreted programming language. The syntax was designed relatively close to JavaScript and Java but with some extra features.

The whole point of this project is to not only create a functioning compiler but to teach how compilers work in the simplest way possible.

# Examples
Examples are in the ```test``` folder

# Compiling it

```$ make```

# Running it
```$ ./eval test/expr.js```
This will run and compile the sample test code in the folder ```test```

# Internals
Chip consist of the following stages:
```mermaid
graph TD
A["Lexer (tokenize.c)"]  
B["Parser (parse.c)"]
C["Code Generation (gen.c)"]
D["Interpreter (intepreter.c)"]

OUTPUT["Output Bytecode file"]
INPUT["Input Bytecode file"]

A --> B --> C 
C --> OUTPUT
INPUT --> D