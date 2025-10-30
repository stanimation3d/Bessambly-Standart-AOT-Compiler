# 📖 BESSAMBLY AOT COMPILER MANUAL (MANUAL.md)

This manual explains how to set up, run, and use the various command-line options of the Bessambly AOT (Ahead-of-Time) compiler.

---

## 1. Introduction: What is Bessambly?

Bessambly is a minimalist, low-level programming language similar to Three-Address Code (TAC). It focuses on direct management of registers and memory. This AOT Compiler translates Bessambly source code directly into **32-bit RISC-V (RV32I)** machine code.

## 2. Setup and Compilation

The compiler is written in C and can be easily compiled with any standard C compiler (e.g., GCC or Clang).

### 2.1 Prerequisites

* `gcc` or `clang` (C Compiler)
* `make` (For build automation)

### 2.2 Compiling

Run the following command in the project directory:

```bash
make
```
This command will compile all source files and create the main compiler executable (bessamblyc).

3. Running the Compiler
The basic usage format for the compiler is as follows:
```
./bessamblyc <input_file> [options]
```
3.1 Basic Usage
To compile a simple Bessambly file (e.g., program.bess) with default settings:
```
./bessamblyc program.bess
```
This reads program.bess and generates an output file named a.out, containing unoptimized (-O0), UNIX target machine code.

3.2 Specifying the Output File (-o)
Used to specify the name and path of the output file.

Option,            Description
-o <filename>,     Specifies the name of the generated machine code file.

Example: Creating an output file named compiled.bin. 
```
./bessamblyc program.bess -o compiled.bin
```
4. Optimization Levels (-O<level>)
The compiler supports a wide range of options to optimize your code for performance or size.

Option,                 Description,                                                                                 Focus
-O0,                    No Optimization. Best for debugging.,                                                        Speed (Compilation)
-O1,                   "Enables basic optimizations (NOP removal, simple Peephole).",                                Speed / Compilation Speed
-O2,                   "More comprehensive optimizations (DCE, jump chain flattening).",                             Performance
-O3,                    Maximum performance optimizations.,                                                          Maximum Performance
-Ofast,                 Includes aggressive optimizations beyond -O3 that may relax strict mathematical standards.,  Most Aggressive Performance
-Oflash,                "(Theoretical Maximum Performance) Includes future deep, time-consuming analyses.",          Theoretical Performance
-Os,                    Size-focused optimizations. Reduces size without significantly impacting performance.,       Size
-Oz,                    More aggressive size optimizations.,                                                         Maximum Size
-Onano,                (Theoretical Smallest Size) Focuses on the smallest possible output size.,                    Theoretical Minimum Size

Example: Compiling your code with the -O2 optimization level.
```
./bessamblyc program.bess -O2 -o optimized.out
```
5. Specifying the Target Platform (-target)
The compiler can generate output for two main target platforms.

Option,                     Description,                                                                                     Output Format
-target unix,               Generates output for Linux/UNIX-like operating systems.,                                        "Raw Binary (Not Executable, just machine code)"
-target baremetal,          Generates output for bare-metal (operating-system-free) hardware (RISC-V microcontrollers).,     Raw Binary (Flat Binary)

Note: Both unix and baremetal outputs are raw machine code intended to be run in a RISC-V simulator (QEMU, Spike) or loaded onto real hardware via a custom bootloader (they are not standard ELF executables).

Example: Preparing the output for a Bare-Metal environment.
```
./bessamblyc embedded_code.bess -target baremetal -o boot.bin
```
6. Debugging
The compiler will report all errors encountered during syntax, semantic, or code generation phases with the line number and a detailed message.

It is recommended to always use the -O0 optimization level when debugging, as unoptimized code directly maps to the instructions in your source code.

7. Resources
RISC-V Register Mapping: Bessambly registers are internally mapped to the RISC-V Saved (S) and Temporary (T) registers.

Memory Addressing: MEM[address] commands are interpreted as direct constant memory addresses.
