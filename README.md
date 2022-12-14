# Isothermal Speculative Partial Redundancy Elimination

This project partially fulfills the requirements for the EECS 583 Compilers course at the University of Michigan.

## Requirements

- LLVM Installation - see Step 1 in [Setup](#setup).
- Packages required by `get_statistics.sh`
    - tr
    - sed

## Setup

1. [Install LLVM](https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm) on your machine. Then, clone this repository to your machine.
2. Inside the newly cloned git repo, create a build directory, generate the Makefile, and make the project.
    ```
    $ mkdir build && cd build
    $ cmake ..
    $ make
    ```
3. Navigate to the `benchmarks` folder, and use the `get_statistics.sh` script to do profiling on any C file placed in the same folder. You can view documentation for the script by running `./get_statistics.sh -h`. Example:
    ```
    $ ./get_statistics.sh -D ispre_test1
    ```

## Benchmarks

The `get_statistics.sh` performs two different benchmarks, correctness and performance, on four levels. The four levels are:

    - O0 optimization 
    - O0 optimization plus global value numbering (LLVM's implementation of traditional PRE) and dead code elimination
    - O0 optimization plus our custom ISPRE pass and dead code elimination
    - O0 optimization plus multiple passes of our custom ISPRE pass and dead code elimination

The correctness benchmark will verify correct behavior of the custom pass by comparing the output of a compiled executable with our pass and the output of a compiled executable without our pass. 

The performance benchmark will then run profiling on the four different levels, comparing runtime and IR code size between all four.

## Results

The below results were obtained by running the benchmark script on an department server at the University of Michigan.

```
$ ./get_statistics.sh ispre_test1
=== Correctness Check ===
>> Does the custom pass maintain correct program behavior?
>> PASS

=== Performance Check ===
1. 
   a. Runtime performance of unoptimized code

real    0m0.294s
user    0m0.294s
sys     0m0.000s

   b. Code size (IR) of unoptimized code

      3480 bytes
2. 
   a. Runtime performance of GVN code

real    0m0.347s
user    0m0.347s
sys     0m0.000s

   b. Code size (IR) of optimized code

      3504 bytes, .006% change

3. 
   a. Runtime performance of ISPRE code

real    0m0.268s
user    0m0.264s
sys     0m0.004s

   b. Code size (IR) of optimized code

      3524 bytes, .012% change

4. 
   a. Runtime performance of Multipass ISPRE code

real    0m0.268s
user    0m0.264s
sys     0m0.004s

   b. Code size (IR) of optimized code

      3652 bytes, .049% change
```

bdgeorge@eecs583a:~/isothermal-speculative-pre/benchmarks$ 

## Bibliography

This project is inspired and informed by the following paper - a big thank you to the authors.

Horspool, R.N., Pereira, D.J. and Scholz, B. (2006) “Fast profile-based partial redundancy elimination,” Lecture Notes in Computer Science, pp. 362–376. Available at: https://doi.org/10.1007/11860990_22. 
