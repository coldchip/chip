# Chip Programming Language

Chip is a compiled and interpreted programming language. The syntax was designed relatively close to JavaScript and Java but with some extra features.

The whole point of this project is to not only create a functioning compiler but to teach how compilers work in the simplest way possible.

The program uses a recursive descent parser to build a parse tree for the input expression, which can then be used for further processing such as evaluation or code generation.

The function takes in a pointer to a pointer to a Token object, which represents the current token in the input expression. The function advances the pointer as it consumes tokens from the input, and it returns a pointer to a Node object representing the root of the parse tree.

For example, the parse_add_sub function is responsible for parsing addition and subtraction operations in the input expression. It first calls the parse_mul_div function to parse any multiplication and division operations that may be present. It then loops indefinitely, consuming + or - tokens from the input and creating a new binary node for each operator. The left operand of the binary node is the result of the previous parse_mul_div call, and the right operand is the result of calling parse_mul_div again on the remaining input. The function returns the final binary node, which represents the entire addition/subtraction expression.

given the input string "2 + 3 * 4 - 5", the parse_add_sub function would first call parse_mul_div to parse the "2" and "3 * 4" subexpress

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

|  Data Type     |Description                    |
|----------------|-------------------------------|
|String          |String of any length           |
|Number          |64 Bit Float Value             |
|Function        |Method of a class              |

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