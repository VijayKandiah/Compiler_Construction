# Compiler_Construction
End-End Compiler for a subset of the C language

## Prerequisites
C++14

## Build
To build all layers, run ```make``` from the root directory.
To build a specific layer, for example, IR: ```cd IR; make```

Make sure all layers below IR, i.e. L3,L2,L1 are built before invoking the IR compiler on .IR programs if you want to generate x86 binary.
This is required because the output of the IR compiler is a .L3 file which is the input to L3 compiler and so on.

## Run

Invoke your compiler, for example, IR compiler to compile a .IR program:

```
./IR/IRc IR/tests/test0.IR
```
This script works in the following way: 
1) Invokes your IR compiler (```IR/bin/IR```) to generate ```IR/prog.L3``` 

2) Invokes your L3 compiler (```L3/bin/L3```) on ``` IR/prog.L3```  to generate ``` L3/prog.L2``` 

3) Invokes your L2 compiler (```L2/bin/L2```) on ``` L3/prog.L2```  to generate ``` L2/prog.L1``` 

4) Invokes your L1 compiler (```L1/bin/L1```) on ``` L2/prog.L1```  to generate ``` L1/prog.S```  which is x86 assembly and invokes gcc to generate x86 binary ``` L1/a.out``` 

5) Moves a.out back to ```IR/``` directory.

## Tests
To compile the sample tests provided along with a compiler layer and compare generated output with oracle output for correctness: ```cd IR; make test```

## Performance
To compile the provided performance test ```LB/test/competition2019.b```  and measure execution-time: ```cd LB; make performance```

## Compiler Layers

Will update soon.
