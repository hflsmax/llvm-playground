# llvm-playground

This repository implements two llvm passes. FunctionInfo prints out function informations. LocalOps implements local optimizations including constant folding, algebraic identity simplication and strength reduction(x * 8 -> x << 3).

Here's the instruction to run the pass on test code.
1. Clong the repo.
2. Build passes
`mkdir build; cd build; cmake ..; make; cd ..`
3. Build test code
```
cd tests
# Build code with no optimization
clang-10 -Xclang -disable-O0-optnone -O0 -emit-llvm -c *.c
# Run mem2reg tool
opt-10 -mem2reg algebraic.bc -o algebraic-m2r.bc
# (Optional) run llvm-dis
llvm-dis-10 algebraic-m2r.bc
```
4. Run pass on the test code
`opt-10 -load build/LocalOps/libLocalOpsPass.so -local-ops tests/algebraic-m2r.bc -o out`


Acknowledgement

The boilerplate code is copied from https://www.cs.cornell.edu/~asampson/blog/llvm.html and https://www.cs.cmu.edu/~15745/assigns.html
