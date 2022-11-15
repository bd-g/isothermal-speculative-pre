# Isothermal Speculative Partial Redundancy Elimination

This project partially fulfills the requirements for the EECS 583 Compilers course at the University of Michigan.

## Setup

1. [Install LLVM](https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm) on your machine. Then, clone this repository to your machine.
2. Inside the newly cloned git repo, create a build directory, generate the Makefile, and make the project.
    ```
    $ mkdir build && cd build
    $ cmake ..
    $ make
    ```
3. Navigate to the `benchmarks` folder, and use the `get_statistics.sh` script to do profiling on any C file with and without ISPRE. You can view documentation for the script by running `./get_statistics.sh -h`. Example:
    ```
    $ ./get_statistics.sh -d hw1correct1
    ```

## Bibliography

This project is inspired and informed by the following paper - a big thank you to the authors.

Horspool, R.N., Pereira, D.J. and Scholz, B. (2006) “Fast profile-based partial redundancy elimination,” Lecture Notes in Computer Science, pp. 362–376. Available at: https://doi.org/10.1007/11860990_22. 