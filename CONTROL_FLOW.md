# Control Flow
1. **Jumps:** `jmp` skips X bytes (by incrementing `rip`) and then resumes execution!
2. **Conditional Jumps:** Conditional jumps check conditions stored in the "flags" register: `rflags`.
Main conditional flags:
    - Carry Flag : was the 65th bit 1?
    - Zero Flag: was the result 0?
    - Overflow Flag: did the result "wrap" between positive to negative?  - - Signed Flag: was the result's signed bit set (i.e., was it negative)?
3. **Loops:** With conditional jumps, we can implement loops like for or while loop.
4. **Function Calls:** Assembly code is split into functions with `call` and `ret` instructions.
    - `call` pushes `rip` and jumps away! (to the start of called function)
    - `ret` pops `rip` and jumps to it! (continue where left off)
5. **Calling Conventions:** Callee and caller functions must agree on argument passing. 
    - Linux x86: push arguments (in reverse order), then call (which pushes return address), return value in eax
    - Linux amd64: rdi, rsi, rdx, rcx, r8, r9, return value in rax
    - Linux arm: r0, r1, r2, r3, return value in r0
    - **Linux amd64:**
        - rbx, rbp, r12, r13, r14, r15 are "callee-saved" (the function you call keeps their values safe on the stack).
        - Other registers are up for grabs (within reason; e.g., rsp must be maintained). Save their values (on the stack)!

---

## 1. Comparing Values
In x86, two values can be compared by using `cmp` instructions which compares two values by subtracting the second operand from the first and the result of subtraction is not stored anywhere. However, it updates the CPU's internal flags based on result. the CPU sets a special bit called the **Zero Flag (ZF)**. For example:
```assembly
cmp rdi, 42
```
From above code:
    - (Equal) If rdi contains 42, then ZF=1
    - (Not equal) Else: ZF=0

**Important Note:**
`cmp` can compare register with immediate value (`cmp rdi, 1337`) or memory location with immediate value (`cmp QWORD PTR [rsp], 1337`).
It cannot compare two memory locations at once, at most one operand can be a memory dereference.

**Writing 1-byte value to register:**
Since we can't directly access the value of flags register, we can use "set on conditions" instructions that can write 0 or 1 to a byte-sized destination based on the current flags.
- `setnz` ("Set if Not Zero")
- `setz` ("Set if Zero"):
    - If ZF = 1 (the values were equal, i.e., the subtraction result was zero), it writes 1 to dil.
    - If ZF = 0 (the values were not equal), it writes 0 to dil.
```
setz dil
```
With the above instruction, we are putting the 0 or 1 in the lowest byte of `rdi` using `dil`
- `setnz` ("Set if Not Zero")

The following code:
1. Compares argc with 42.
2. Uses setz dil to set the exit code: 1 if argc equals 42, 0 otherwise.
3. Exits.
```assembly
.intel_syntax noprefix
.global _start

_start:
        cmp QWORD PTR [rsp], 42     ; compares value at rsp address (arg2) with 42
        setz dil                    ; put 0 or 1 depending on the outcome of compare, in the lowest address of the rdi
        mov rax, 60
        syscall
```
**Note:** `QWORD PTR` tells the CPU we're working with a full 64-bit value, not a single byte.

### Comparing Characters
The following program that checks whether the first character of argv[1] is 'p'. Exit with 1 if it is, 0 if it isn't.
1. Load the argv[1] pointer from [rsp+16] into a register.
2. Compare BYTE PTR at that address against 'p'.
3. Use setz dil to capture the result.
4. Set up the exit syscall number.
5. syscall.
```asm
.intel_syntax noprefix
.global _start

_start:
        mov rax, [rsp+16]
        cmp BYTE PTR [rax], 'p'
        setz dil
        mov rax, 60
        syscall
```
**Remember:** `BYTE PTR` tells the CPU we're working with a single byte, not a full 64-bit value.

## 2. Conditional Jumps
Conditional jumps can be used to jump to a different part of the code based on the outcome of the comparision (`cmp`).
For this purpose, `jne` (Jump if Not Equal) instruciton can be used. Under the hood, `jne` checks the Zero Flag (`ZF`) that `cmp` set: `jne` jumps when `ZF = 0` (meaning the subtraction result was non-zero, i.e., the values differed). There's also `je` (Jump if Equal), which does the opposite: it jumps when the values are equal.

We can define a `label` in our code which is just a name given to a location. They don't generate any machine code and assembler resolves the label to an address.

For example:
```asm
main:
  [load and compare]
  jne fail          ← jump to fail if NOT equal

success:
  mov rdi, 0
  mov rax, 60
  syscall

fail:
  mov rdi, 1
  mov rax, 60
  syscall
```

The following program checks whether the first character of argv[1] is 'p', using conditional jumps:
1. Load the `argv[1]` pointer from `[rsp+16]` into a register.
2. Compare BYTE PTR at that address against `'p'`.
3. `jne fail` --- jump to the failure case if the characters aren't equal.
4. Write the "fall-through" success case (exit(0)).
5. Define the `fail:` label and write the fail case (exit(1)).

The tricky thing is that success case (jump not taken) is between `jne` instruction and the fail case that the jne instruction refers to.

```asm
.intel_syntax noprefix
.global _start

_start:
        mov rax, [rsp+16]
        cmp BYTE PTR [rax], 'p'
        jne fail
        mov rdi, 0
        mov rax, 60
        syscall

fail:
        mov rdi, 1
        mov rax, 60
        syscall
```

### Comparing Strings
The following program checks whether the first argument starts with the string "pwn", character by character:
```asm
.intel_syntax noprefix
.global _start

_start:
        mov rax, [rsp+16]
        cmp BYTE PTR [rax], 'p'
        jne fail
        cmp BYTE PTR [rax+1], 'w'
        jne fail
        cmp BYTE PTR [rax+2], 'n'
        jne fail
        mov rdi, 0
        mov rax, 60
        syscall 

fail:
        mov rdi, 1
        mov rax, 60
        syscall
```

### Jump Tables
A jump table is an array of addresses stored in memory, one for each possible destination (called a case).
Instead of comparing the input against every possibility, the program uses the input value as an index into the table, loads the address stored at that position, and jumps to it. This pattern is called a switch, and it's a fundamental building block in programs.
```asm
mov    rax, 0                    ; zero out rax
mov    al, BYTE PTR [rcx]        ; load the character into the low byte of rax
mov    rax, [rax*8+0x1234000]    ; load a stored address from the jump table at 0x1234000
jmp    rax                       ; jump to it
```


## 3. Loops
A sequence of instructions that jumps backward to repeat itself.

The following code compares argv[1] against a secret password using the loop:
```asm
loop:
  mov    al, BYTE PTR [rsi]       ; load next password character
  cmp    al, BYTE PTR [rdi]       ; compare against next argv[1] character
  jne    fail                     ; mismatch → jump to fail
  cmp    al, 0x0                  ; reached the null terminator?
  je     success                  ; yes → all characters matched!
  inc    rdi                      ; **inc**rement rdi to advance to next argv[1] character
  inc    rsi                      ; **inc**rement rsi to advance to next password character
  jmp    loop                     ; jump back to the top — repeat!
```

- The key instruction is `jmp loop` at the bottom. 
- Unlike `jne` (which only jumps when a condition is met), `jmp` unconditionally always jumps. 
- By jumping backward to the loop label, the program re-executes the same comparison logic on the next pair of characters. 
- The loop terminates when either a mismatch is found (`jne fail`) or the `null` terminator is reached at the end of the string after the other characters are successfully matched to the password (`je success`).

**Writing Loop:** A loop needs three pieces of state: where the current work is, how much progress has been made, and a test that decides when the loop is finished.

For a string, the natural finish line is the null terminator: the 0 byte after the last character.

Following program computes the length of `argv[1]` and exits with that length as its exit code. The program inspects the string one byte at a time and uses a backward jump to repeat the loop.
```asm
.intel_syntax noprefix
.global _start

_start:
        xor rdi, rdi
        mov rax, [rsp+16]

loop:
        cmp BYTE PTR [rax], 0
        je done
        inc rdi
        inc rax
        jmp loop
done:
        mov rax, 60
        syscall
```

## 4. Functions `call` / `ret`
`call <target>` is x86's function-call instruction. It does two things:
1. Pushes the address of the next instruction `rip` after the call instruction (the return address) onto the stack.
2. Jumps to `<target>`.

```asm
.global _start
_start:
    push 0x41414141   // put four 'A' bytes (0x41) on the stack to stand in for the flag
    mov rdi, rsp      // first argument: a pointer to those bytes
    mov rsi, 4        // second argument: how many bytes to print
    int3              // optional: gdb breaks here without setting a breakpoint
    call solve        // your solve runs, prints the bytes, and exits on its own
```

Defining function:
```asm
.intel_syntax noprefix
.global solve

solve:
    <your code>
```

The `.global solve` line tells the assembler "expose this code so other code can find it". The `solve:` label actually specifies where the code is.

**The calling convention:** The caller functions pass arguments to the callee in registers. For example, for two arguments, `rdi` and `rsi`.

```asm
.intel_syntax noprefix
.global _start

_start:
        push 0x41424141
        mov rdi, rsp
        mov rsi, 4
        call solve

solve:
        mov rdx, rsi
        mov rsi, rdi
        mov rdi, 1
        mov rax, 1
        syscall
        mov rdi, 0
        mov rax, 60
        syscall
```
In the above code:
- `rdi`	First argument (a pointer to a buffer of bytes)
- `rsi`	Second argument (the length of that buffer)

Then, in the solve function, we swap the registers because `write(1)` syscall takes arguments in the order of: file descriptor (1 in `rdi` for stdout), buffer (pointer to memory, in `rsi`), and size (in `rdx`). This is different from the arguments function is called with.

### Returning value `ret`
In previous function code, the callee exits using exit syscall and the control is never returned to the caller. This is not ideal.

A callee function is supposed to hand back control to the caller function. This can be done by using the `ret` instruction.

The `call` instruction pushes the return address onto the stack before jumping into function code. `ret` is the matching half: it pops that saved return address off the stack and jumps to it. 

Together, `call` and `ret` form the basic function-call/function-return pair in x86

**Note:** In the Linux x86-64 calling convention, the return value of a function goes in `rax`.

**The working mechanics a function call:**
1. The caller executes callee, passing argument (for example: one argument in `rdi`). When it does this, the next instruction in the caller function after `call` instruction gets pushed onto the stack as the "saved return address".
2. Callee function does some work.
3. Callee function puts its result in `rax` (return value).
4. Callee function executes `ret`, which pops the saved return address off the stack and jumps back to the next instruction in caller code.
5. The caller reads `rax` as the callee function's result ("return value").

See following example of a function, which takes one argument via the `rdi` register and returns the same value via `rax` register. This value is then used as exit status code for the exit syscall.
```asm
.intel_syntax noprefix
.global _start
.global solve
_start:
        xor rdi, rdi
        mov rdi, 0x1337
        call solve
        mov rdi, rax
        mov rax, 60
        syscall

solve:
        mov rax, rdi
        ret
```

### Call through pointer (Function Pointer)
A function pointer is just an address: a 64-bit value that names the location of some code in memory. 
In the following code, the caller function receives a pointer to a function in `rdi` as first argument. Then, it calls it using `call rdi`:

```asm
.intel_syntax noprefix
.global caller

caller:
        call rdi
        mov rdi, 0
        mov rax, 60
        syscall
```

### Calling Convention
How functions pass arguments and share registers across function calls is determined by the Calling Convention of architecture.

Generally, a calling convention splits registers into two groups so that separately-written functions can cooperate:

- **caller-saved registers** may be freely overwritten by any function you call. If you have a value in one of these that you need after a call, it's your job (the caller's) to save it first and restore it afterward. Typically, this is done by `push`ing them to the stack before calling the callee and `pop`ing them off the stack later. On x86-64, these registers are `rax` (which callees will clobber by moving the return value to), `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, `r10`, `r11`.
- **callee-saved registers** must be left untouched by the functions you call (callees). _Rather, callees can touch them, but they must restore them back to their original state._ On x86-64, these are `rbx`, `rbp`, `r12`, `r13`, `r14`, `r15`.

In simple terms:
1. Caller-saved: Save these registers before calling another function: `rax`, `rcx`, `rdx`, `r8`, `r9`, `r10`, `r11`. This can be done by pushing then onto stack and then poping:
```asm
.intel_syntax noprefix
.global solve

solve:
        push rsi
        push rdx
        push rcx
        push rax
        push r8
        push r9
        push r10
        push r11
        call rdi
        pop r11
        pop r10
        pop r9
        pop r8
        pop rax
        pop rcx
        pop rdx
        pop rsi
        call rsi
```

2. Callee-saved: The callee function must return these registers un-modified: `rbp`, `rbx`, `r12`, `r13`, `r14`, `r15`. 
The rule is, when your function wants to use these registers, save (`push`) them on entry to your function, restore (`pop`) them before you `ret`.
See example below:
```asm
.intel_syntax noprefix
.global solve

solve:
        push rbp
        push rbx
        push r12
        push r13
        push r14
        push r15
        mov rbx, 0x1337
        mov rbp, 0x1337
        mov r12, 0x1337
        mov r13, 0x1337
        mov r14, 0x1337
        mov r15, 0x1337
        call rdi
        pop r15
        pop r14
        pop r13
        pop r12
        pop rbx
        pop rbp
        ret
```