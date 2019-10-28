# Compiler_Construction
End-End Compiler for a subset of the C language

## Build
To build all layers, run ```make``` from the root directory.
To build a specific layer, for example, IR: ```cd IR; make```

Make sure all layers below IR, i.e. L3,L2,L1 are built before invoking the IR compiler on .IR programs if you want to generate x86 binary.
This is required because the output of the IR compiler is a .L3 file which is the input to L3 compiler and so on.

## Run

Invoke your compiler, for example, IR compiler to compile a .IR program:

```
cd IR; ./IRc IR/tests/test0.IR
```
This script works in the following way: 
1) Invokes your IR compiler (```IR/bin/IR```) to generate ```IR/prog.L3``` 

2) Invokes your L3 compiler (```L3/bin/L3```) on ``` IR/prog.L3```  to generate ``` L3/prog.L2``` 

3) Invokes your L2 compiler (```L2/bin/L2```) on ``` L3/prog.L2```  to generate ``` L2/prog.L1``` 

4) Invokes your L1 compiler (```L1/bin/L1```) on ``` L2/prog.L1```  to generate ``` L1/prog.S```  which is x86_64 assembly and invokes gcc to generate x86_64 binary ``` L1/a.out``` 

5) Moves a.out back to ```IR/``` directory.

## Tests
To compile the sample tests provided along with a compiler layer and compare generated output with oracle output for correctness: ```cd IR; make test```

## Performance
To compile the provided performance test ```LB/test/competition2019.b```  and measure execution-time: ```cd LB; make performance```

## Compiler Layers
Here's a brief description of each compiler layer. Please refer to slides provided at the course's homepage, https://users.cs.northwestern.edu/~simonec/CC.html for more information. 

### L1
Uses PEGTL, https://github.com/taocpp/PEGTL, for parsing .L1 code and generates x86_64 assembly. Deals with stack pointer movement for function calls and handles the x86_64 calling convention. 

### L2
Computes IN and OUT sets for liveness analysis and performs graph-coloring based register allocation to minimize spilling variables to memory.

### L3 
Performs instruction selection by tiling and pattern-matching: Generates and merges tiles (patterns) of instructions and uses the Maximal Munch pattern-matching algorithm for optimized code generation. 

### IR
Handles explicit control flows and explicit data types. Linearizes multi-dimension arrays.

### LA
Performs value-encoding, generates code to check array accesses, and creates Basic Blocks.

### LB
Uses name binding to deal with scoped variables and handles 'If' and 'While' control structures.
