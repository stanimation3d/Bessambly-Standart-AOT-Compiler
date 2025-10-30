# Bessambly Programming Language Syntax

1. Introduction and Language Philosophy
Bessambly is designed with the goal of being the lowest-level programming language capable of writing hardware-agnostic code.

The core philosophy of the language is built upon absolute control and minimalism. Due to this philosophy:

* No Standard Library: All input/output and fundamental operations must be performed entirely manually through memory and register manipulation.

* No Module System: The entire program consists of a single flow and address space.

* Imperative Programming: Only a model focusing on state changes is supported.

* Manual Management: Memory management is entirely the programmer's responsibility.

2. Memory and Data Access
In Bessambly, data manipulation and access are carried out via registers and memory addresses.

2.1. Memory Addressing
Memory is accessed using the special keyword MEM followed by an address enclosed in parentheses. Addresses are typically specified in hexadecimal.

Syntax,           Description
MEM[0x100],       Accesses the memory cell at address 0x100.
MEM[0x20] = 5,    Writes the value 5 to address 0x20.
VAR = MEM[0x30],  Reads the value at address 0x30 into the VAR register.

2.2. Register Usage
Simple variables are treated as single-letter registers (A, B, C, etc.) or programmer-defined named registers.

Syntax,          Description
A = 10,          Assigns the value 10 to Register A.
MEM[0x50] = B,   Copies the value of Register B to address 0x50.

3. Core Operations (Infix Notation)
All basic arithmetic and logical operations are performed using infix notation. The result of the operation must always be assigned to a register or a memory address.

Syntax,                Description
C = A + B,             "Adds A and B, assigns the result to C."
MEM[0x10] = A - 5,     "Subtracts 5 from A, writes the result to address 0x10."
D = A * B,             Multiplication operation.
E = A / B,             Division operation.
F = A & B,             Bitwise AND.
G = A | B,             Bitwise OR.

4. Control Structures (Jumps)
Bessambly uses only the goto and if-goto jump commands to manage code flow. Block structures ({}) or loop structures (while, for) are not supported.

4.1. Label Definition
Code lines that are the target of jump commands are identified by a Label, defined using a name followed by a colon (:).

Syntax,        Description
LOOP_START:,   Defines a label to jump to this point in the code.

4.2. Unconditional Jump
Redirects the flow of execution directly to the specified label.

Syntax,           Description
goto LOOP_START,  Transfers program execution to the LOOP_START: label.

4.3. Conditional Jump
If a specified condition is true, the program flow is redirected to the given label. If the condition is false, execution continues with the next line.

Syntax,                        Description
if A > B goto LABEL_X,         Jumps to LABEL_X if A is greater than B.
if C == 0 goto END_PROGRAM,    Jumps to END_PROGRAM if C equals 0.

Supported Comparison Operators:

Operator,Description
>,       Greater Than
<,       Less Than
==,      Equal To
!=,      Not Equal To
>=,      Greater Than or Equal To
<=,      Less Than or Equal To

5. Example Code (Summation from 0 to N)
The example below calculates the sum of numbers from 0 up to the value N stored at memory address 0x10, using only goto and if-goto, and saves the result to address 0x20.

```
// --- Initial Values ---
// MEM[0x10]: The value of N (e.g., 10)
// MEM[0x20]: The sum result (initially 0)

// Initialize Registers
I = 0         // Counter Register
SUM = 0       // Sum Register
N_VALUE = MEM[0x10] // Read N value from memory address

// --- Loop Start ---
LOOP_START:
    // Condition: If I > N_VALUE, skip the loop
    if I > N_VALUE goto LOOP_END

    // Body: Update the Sum
    SUM = SUM + I

    // Increment the Counter
    I = I + 1

    // Unconditionally return to the loop start
    goto LOOP_START

// --- Loop End ---
LOOP_END:
    // Write the result to memory
    MEM[0x20] = SUM

    // Terminate Program (hypothetical halt command)
    HALT
``` 
