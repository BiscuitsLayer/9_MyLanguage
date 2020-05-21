section .text
global _start
_start: ; entry point
call main ; start main function
mov rax, 60 ; rax = exit
mov rdi, 0 ; rdi = 0 code
syscall ; program exit
ret

main:
push rbp ; typical function entry
mov rbp, rsp
add rax, 0
add r8, 0
push rax
push r8
pop rax
pop r8
nop
nop
push 0
push 1
push 2
push 100
nop
nop
mov rax, rax
mov rcx, rax
mov rax, rcx
xchg rax, rax
xchg rcx, rax
xchg rax, rcx
test rax, rax
test rcx, rax
test rax, rcx
cmp rax, rax
cmp rcx, rax
cmp rax, rcx
pop rbp ; typical function exit
ret