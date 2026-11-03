# GDB Usage
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
- Examining memory: GDB's x (examine) command lets you look at the contents of memory:
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