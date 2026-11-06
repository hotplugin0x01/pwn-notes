# Understanding stack
- Grows toward lower memory addresses.
- RSP points to the current top of the stack.
- RBP is often used as a frame pointer, but it is optional.
- CALL pushes a return address onto the stack.
- RET pops the return address from the stack.

Remember:
```assembly
push rax     ; RSP = RSP - 8, then store RAX
pop rax      ; load [RSP], then RSP = RSP + 8
```
In x86-64, a normal push or pop operates on 8 bytes.

## Example function
```c
int foo(int a, int b) {
    int x = 10;
    int y = 20;
    return a + b + x + y;
}

int main() {
    int result = foo(5, 7);
    return 0;
}
```

A simplified assembly representation:

```assembly
main:
    mov edi, 5
    mov esi, 7
    call foo
    ; result in eax
    ret

foo:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov DWORD PTR [rbp-4], edi
    mov DWORD PTR [rbp-8], esi

    mov DWORD PTR [rbp-12], 10
    mov DWORD PTR [rbp-16], 20

    mov eax, DWORD PTR [rbp-4]
    add eax, DWORD PTR [rbp-8]
    add eax, DWORD PTR [rbp-12]
    add eax, DWORD PTR [rbp-16]

    leave
    ret
```
Here:
- EDI contains argument a = 5.
- ESI contains argument b = 7.
- EAX contains the return value.
- RBP is used as a frame pointer.
- RSP tracks the current stack top.

### Step-by-step stack changes
For simplicity, we assume the address is 16-byte aligned.
1. Before Call
The caller (main) prepares arguments:
```assembly
mov edi, 5
mov esi, 7
call foo
```

Arguments are in registers, not on the stack: RDI, RSI

2. CALL executes
```asm
call foo
```
The CPU performs:
```asm
RSP = RSP - 8
[RSP] = return address
RIP = address of foo
```

3. After Call: Function prologue
The function executes:
```asm
push rbp
mov rbp, rsp
sub rsp, 16
```
    1. `push rbp`: 
    - RSP -= 8
    - Save the caller's RBP on the stack.
    2. `mov rbp, rsp`
    - RBP now points to the saved RBP
    3. sub rsp, 16
    - Reserve 16 bytes for local variables.

4. For accessing variables
```asm
mov eax, DWORD PTR [rbp-4]
```
The CPU calculates: `Effective address = RBP - 4 = 0x7fffffffdfec`

Then reads 4 bytes from that address: `EAX = 10`

Similarly: `mov eax, DWORD PTR [rbp-16]` -> Reads b = 7 in our illustrative layout.

5. Function epilogue: LEAVE and RET
At the end:
```asm
leave
ret
```
`leave` is equivalent to:
```asm
mov rsp, rbp
pop rbp
```
where `pop` rbp is equivalent to the following:
```txt
RBP = [RSP]
RSP = RSP + 8
```
The caller's RBP is restored.

`ret` is equivalent to:
```txt
RIP = [RSP]
RSP = RSP + 8
```

The CPU loads the return address and execution returns to main.

---

## Stack Frame 

The call solve instruction does two things:
1. Pushes the return address onto the stack (8 bytes). Pushing decrements rsp, so the return address ends up at a smaller address than what was already on the stack.
2. Jumps to your code.

**That first step is critical:** the stack grows backwards from what you might expect. pop actually adds 8 to rsp, and push subtracts 8.

Initialization happens between allocation and deallocation of the stack frame:
```asm
sub rsp, 256       # allocate a 256-byte frame
    ...            # initialize and use [rsp] through [rsp+255]
add rsp, 256       # deallocate the frame
ret
```

That last step matters as much as the first. `ret` `pop`s its return address from `[rsp]`, so if `rsp` is not back where it started, `ret` will read the wrong bytes as an address and your program will crash.

---

**Challenge:** Count how many distinct byte values appear in a buffer. The natural way is to keep a table with one slot per possible byte value: 256 slots, indexed by the byte value itself. Start with the same 256-byte stack frame you built before and clear it to zero. Then, when you see byte value `b`, write `1` into slot `b`. After you have scanned the buffer, count how many table slots are marked.

Write a function called solve that takes a pointer to a buffer in `rdi` and a length in `rsi`, and returns, in `rax`, the number of distinct byte values among those `rsi` bytes. You might find the instruction `mov byte ptr [rsp+rcx], 1` useful for marking a given value (stored in `rcx`) as "present" in your scratch table (based at `rsp`).

```asm
.intel_syntax noprefix
.global solve

solve:
        sub rsp, 256

        xor rcx, rcx

clear:
        mov BYTE PTR [rsp + rcx], 0
        inc rcx
        cmp rcx, 256
        jne clear

        xor rcx, rcx

scan:
        cmp rcx, rsi
        je start_count

        movzx edx, BYTE PTR [rdi + rcx]
        mov BYTE PTR [rsp + rdx], 1

        inc rcx
        jmp scan

start_count:
        xor rcx, rcx
        xor rax, rax

count:
        cmp rcx, 256
        je done

        cmp BYTE PTR [rsp + rcx], 1
        jne next

        inc rax

next:
        inc rcx
        jmp count

done:
        add rsp, 256
        ret
```
Step 1: Allocate Table
- Reserve 256 bytes on the stack.
- Each position represents a byte value (0–255).

Step 2: Clear Table
- Initialize all 256 entries to zero.
- Use RCX as loop counter.
- Stop when RCX reaches 256.

Step 3: Scan Input
- Start RCX at zero.
- Iterate until RCX reaches RSI (input length).
- Read one byte from the input buffer.
- Zero-extend the byte into EDX using MOVZX.
- Use the byte value as an index into the table.
- Mark the corresponding slot as 1.
- Increment RCX.

Step 4: Count Distinct Values
- Reset RCX and RAX to zero.
- Iterate over all 256 table entries.
- If an entry is 1, increment RAX.
- Increment RCX for each entry.

Step 5: Return
- Restore the stack by releasing the 256 bytes.
- Return the result in RAX.

---

### Environment Variables on Stack
1. Both `argv` and envp are NULL-pointer-terminated: the kernel writes a `NULL` pointer at the end of each list of pointers. That's how programs (and you!) know where each list ends --- walk the pointers until you hit a `NULL`. In the diagram, you can see the `NULL` at `rsp+16` marking the end of `argv`, and another at `rsp+32` marking the end of `envp`.

2. The `envp` strings look like `NAME=VALUE` (e.g., `PATH=/usr/bin:/bin`). So `envp[0]` points to a string that starts with the first env var's name.

```text
     Address    │ Contents
   +────────────────────────+
   │ rsp + 0    │ 1         │ ◀─── argc
   +────────────────────────+
   │ rsp + 8    │ rsp + 128 │──┐  argv[0]: pointer to the program name
   +────────────────────────+  │
   │ rsp + 16   │ 0         │  │  NULL (end of argv)
   +────────────────────────+  │
   │ rsp + 24   │ rsp + 200 │──┼──┐  envp[0]: pointer to the first env var
   +────────────────────────+  │  │
   │ rsp + 32   │ 0         │  │  │  NULL (end of envp)
   +────────────────────────+  │  │
                               │  │
     Address    │ Contents     │  │
   +────────────────────────+  │  │
   │ rsp + 128  │ "/tmp/..."│◀─┘  │  the program name
   +────────────────────────+     │
   │ ...        │ ...       │     │
   +────────────────────────+     │
   │ rsp + 200  │ "FLAG=..."│◀────┘  the first env var: the `FLAG` variable
   +────────────────────────+
```