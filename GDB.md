# GDB Usage
Useful Resorce: [Tudor's gdb crash course](https://web.archive.org/web/20250101052732/https://users.umiacs.umd.edu/~tdumitra/courses/ENEE757/Fall15/misc/gdb_tutorial.html)

You can use the command `start` to start a program, with a breakpoint set on `main`. You can use the command `starti` to start a program, with a breakpoint set on `_start`. You can use the command `run` to start a program, with no breakpoint set. You can use the command `attach <PID>` to attach to some other already running program. You can use the command `core <PATH>` to analyze the coredump of an already run program.

When starting or running a program, you can specify arguments in almost exactly the same way as you would on your shell.
For example, you can use `start <ARGV1> <ARGV2> <ARGVN> < <STDIN_PATH>`.

- Start a program with the starti command:
```bash
(gdb) starti
```
- Disassemble the running code:
```bash
(gdb) disassemble
# OR
(gdb) disass
```
- Stepping through assembly instrusctions:
```bash
(gdb) stepi
# OR
(gdb) si
```
- Reading register value:
```bash
(gdb) print $rdi
(gdb) print $rsp
# print $REGISTER
```
- Setting register value:
```bash
(gdb) set $rax = 42
```
- **Examining memory:** GDB's x (examine) command lets you look at the contents of memory:
- You can examine the contents of memory using the `x/<n><u><f> <address>` parameterized command. 
        - In this format `<u>` is the unit size to display, `<f>` is the format to display it in, and `<n>` is the number of elements to display. 
        - Valid unit sizes are `b` (1 byte), `h` (2 bytes), `w` (4 bytes), and `g` (8 bytes). 
        - Valid formats are `d` (decimal), `x` (hexadecimal), `s` (string) and `i` (instruction). 
        - The address can be specified using a register name, symbol name, or absolute address. 
        - Additionally, you can supply mathematical expressions when specifying the address.
```bash
(gdb) x $rsp
```
NOTE: x displays values in hexadecimal by default
Display format can be changed by appending `/` to the command.
For example to see in decimal format:
```bash
(gdb) x/d $rsp
```
- Examining stack pointers:
`/a` tells `x` to display the value as a memory address:
```bash
(gdb) x/a $rsp+32
``` 
`/s` tells `x` to display the value as a string. Replace the address with whatever you got from previous step:
```bash
(gdb) x/s <address>
```
- Examining intruction at a address (location)
The following command examines the instruction at the `rip`:
```bash
(gdb) x/i $rip
```
Or examine 10 instructions at `rip`:
```bash
(gdb) x/10i $rip
```
- Listing registers values
```bash
(gdb) info reg
```
- Listing values at the stack
```bash
# List 4 values at stack in hex
(gdb) x/4gx $rsp
# List 4 values at stack in decimal (signed)
(gdb) x/4d $rsp
# List 4 values at stack in decimal (unsigned)
(gdb) x/4u $rsp
```

- **step instruction (si) vs next instruction (ni):** `si` will step into the function call whereas `ni` will not got into function and stay at the current level.
        - If we stepped into the function call using `si`, then we can step out using the `finish` command.

- **Breakpoint:** In x86 architecture, the program can signal a desire to be debugged by using the `int3` (opcode 0xCC) instruction. If a debugger is attached when `int3` is executed, it stops the program. This is called a program breakpoint. 
```assembly
.intel_syntax noprefix
.global _start

_start:
        mov rdi, 1337   ; exit status code
        int3            ; breakpoint for debugger
        mov rax, 60     ; exit syscall
        syscall
```
- Running program with argmuments: `run` starts the program and lets it execute freely. We can pass the arguments to program as following:
```bash
(gdb) run <arg1> <arg2> <argn>
# OR
(gdb) r <arg1> <arg2> <argn>
```
- Redirecting STDIN: Programs can also read from stdin, and gdb lets redirecting stdin when using `run`. The syntax is the same redirection as in shell:
```bash
(gdb) run < /path/to/input
```
- Restart a program:
```bash
(gdb) run /r
```

- **Breakpoints:**

Set the breakpoint on any function using:
```bash
(gdb) b *main
(gdb) b *function
```
List all the breakpoints
```bash
(gdb) info break
```
Delete breakpoint using:
```bash
(gdb) del break 1
```
We can also set breakpoint on `rip` offset displayed in `disassembly` of function using:
```bash
#(gdb) b *function_name + offset
(gdb) b *function_name + 51
```

- Dereferencing a value at register:
In this case, dereference the address at `rip` as long:
```bash
(gdb) print *(long *) $rsp
```