# Chip Programming Language

Chip is a compiled and interpreted programming language. The syntax was designed relatively close to JavaScript and Java but with some extra features.

The whole point of this project is to not only create a functioning compiler but to teach how compilers work in the simplest way possible.

The program uses a recursive descent parser to build a parse tree.

Unlike JVM, the bytecodes here will be stripped of any labels and the position of jumps will be calculated during the code generation stage. This is to enable to code to be more independent and also discourages decompilation. 

Fibonacci in Chip

```
class Main {
	method main() : [static] returns void {
		int n1 = 0;
		int n2 = 1;
		int n3;
		int i;
		int count = 10;    

		while(i < 30) {    
			n3 = n1 + n2;    

			Console.write(Convert.string(n3));
			Console.write("\n");    

			n1 = n2;    
			n2 = n3;  

			i = i + 1;
		}
	}
}
```

Compiles to

```
SUB_0x55f285537d00_main:
	nop
	push	0
	store	1
	push	1
	store	2
	push	10
	store	5
WB_42:
	nop
	load	4
	push	30
	cmplt
	push	0
	je	WE_43
	load	1
	load	2
	add
	store	3
	load	0
	load	0
	load	3
	push	1
	call	SUB_0x55f2855373a0_string
	push	1
	call	SUB_0x55f2855374e0_write
	pop	
	load	0
	loadconst	2	//2
	push	1
	call	SUB_0x55f2855374e0_write
	pop	
	load	2
	store	1
	load	3
	store	2
	load	4
	push	1
	add
	store	4
	jmp	WB_42
WE_43:
	nop
	push	0
	ret
```

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