# System Calls
It's an instruction that makes a call into the Operating System.
`syscall` triggers the system call specified by the value in `rax`.
Arguments in `rdi`, `rsi`, `rdx`, `r10`, `r8`, and `r9` and return value in `rax`.

---

- **Write(1):** The write syscall tells the operating system to print text to the screen (or write to a file). It is how your program produces output.
**Args:** what data to write and where to write it to.
**File Descriptors:**
    - FD 0: Standard Input is the channel through which the process takes input.
    - FD 1: Standard Output is the channel through which processes output normal data.
    - FD 2: Standard Error is the channel through which processes output error details. 
**C Like syntax for write syscall**
```c
write(file_descriptor, memory_address, number_of_characters_to_write)
```
Assembly implementation:
```assembly
.intel_syntax noprefix
.global _start

_start:
        mov rdi, 1          ; File descriptor
        mov rsi, [rsp+16]   ; Memory address -> Use the argv[1] provided when running program.
        mov rdx, 64          ; Number of chars to write, 64 bytes
        mov rax, 1          ; Write syscall
        syscall             ; invoke write syscall
        mov rdi, 0          ; set exit status
        mov rax, 60         ; exit syscall
        syscall             ; invoke exit syscall
```

---

- **read(0):** It reads some amount of bytes from the provided file descriptor and stores them in memory. 
The following will read 5 bytes from file descriptor 0 (stdin) into memory starting from some_address.
```c
read(0, some_address, 5);
```
**What address to use?** Somewhere that's valid and writable. One such place is the stack! The `rsp` register points to the top of the stack, and there's plenty of writable space there. So, we can use `rsp` as memory address: `mov rsi, rsp`.
The following program:
1. first read 128 bytes from stdin to your program's memory
2. write those 128 bytes from that memory location to stdout
3. finally, exit with the exit code 42.
```assembly
.intel_syntax noprefix
.global _start

_start:
        mov rdi, 0      ; File descriptor (FD=0) stdin
        mov rsi, rsp    ; Memory address to write (start of stack)
        mov rdx, 128    ; Bytes to write
        mov rax, 0      ; read syscall
        syscall
        mov rdi, 1      ; File descriptor (FD=1) stdout
        mov rax, 1      ; write syscall
        syscall
        mov rdi, 42     ; exit status code 42 
        mov rax, 60     ; exit syscall
        syscall
```

Read exact bytes intead of fixed: When a system call returns, Linux places its result in `rax`. For `read`, that result is the number of bytes it actually read. Ask it to read 128 bytes but only 50 are available, and it reads those 50 and leaves 50 in rax.

So `read` into the buffer using a count comfortably larger than expected, then `write` back exactly the number of bytes `read` returned. The only missing piece is that you need to move `read`'s return value (`rax`) into write's size argument (`rdx`):
```assembly
.intel_syntax noprefix
.global _start

_start:
        mov rdi, 0      ; File descriptor (FD=0) stdin
        mov rsi, rsp    ; Memory address to write (start of stack)
        mov rdx, 128    ; Bytes to write
        mov rax, 0      ; read syscall
        syscall
        mov rdi, 1      ; File descriptor (FD=1) stdout
        mov rdx, rax    ; Write only the number of bytes that were read and not more
        mov rax, 1      ; write syscall
        syscall
        mov rdi, 42     ; exit status code 42 
        mov rax, 60     ; exit syscall
        syscall
```

---

- **open(2):** The open system call (syscall number 2) takes a pointer to a filename string and returns a brand-new file descriptor referring to that file:
```c
open("/flag", 0);
```
The second argument specifies additional modes and permissions for the file, but `0` requests the default: `read-only`.

When `open` returns, `rax` contains the new file descriptor (fd) number, which can be used as first argument for `read` to read from the file.

The following program will read the first argument (argv[1]) file name and read it and write to stdout.
1. Load a pointer to the filename (stored at [rsp+16], the first argument) into rdi
2. Specify the default of read access for the second argument (set rsi to 0).
3. open it (syscall 2)
4. read from the returned fd into memory. The fd open returned is in rax. 
5. Read a comfortably large count (128)
6. write to stdout exactly the number of bytes read returned.
7. exit with code 42 (syscall 60)

```assembly
.intel_syntax noprefix
.global _start

_start:
        mov rdi, [rsp+16]   ; Load a pointer to the filename (argv[1])
        mov rsi, 0          ; Mode and Permissions (0 = read-only)
        mov rax, 2          ; open syscall
        syscall
        mov rdi, rax        ; Provide the file descriptor returned by open syscall as first arg to read
        mov rsi, rsp        ; memory address to read and write the content -> in this case the start of stack.
        mov rdx, 1024       ; number to bytes to read 
        mov rax, 0          ; read syscall
        syscall
        mov rdi, 1          ; file descriptor for write (stdout = 1)
        mov rdx, rax        ; Write only the number of bytes that were read (less than or equal to 1024)
        mov rax, 1          ; write syscall
        syscall
        mov rdi, 42         ; exit status code 42
        mov rax, 60         ; exit syscall
        syscall
```