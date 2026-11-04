# Understanding stack:
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