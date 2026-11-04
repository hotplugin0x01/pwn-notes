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
    - **Linux amd64**:
        - rbx, rbp, r12, r13, r14, r15 are "callee-saved" (the function you call keeps their values safe on the stack).
        - Other registers are up for grabs (within reason; e.g., rsp must be maintained). Save their values (on the stack)!

---

## Comparing Values
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
        cmp QWORD PTR [rsp], 42
        setz dil
        mov rax, 60
        syscall
```
