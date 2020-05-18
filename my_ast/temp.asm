section .text
global _start
_start:
call main
mov eax, 60
mov rdi, 0
syscall
ret

main:
push rbp
mov rbp, rsp
sub rsp, 24d
push 2d
pop qword [rbp-8]
push 4d
pop qword [rbp-16]
push qword [rbp-8]
push qword [rbp-16]
jae elseif_1
push qword [rbp-16]
push qword [rbp-8]
call sum
add rsp, 16d
push qword rax ; push the value just returned
pop qword [rbp-24]
jmp endif_1
elseif_1:
push qword [rbp-16]
push qword [rbp-8]
call div
add rsp, 16d
push qword rax ; push the value just returned
pop qword [rbp-24]
endif_1:
mov rax, 0d ; return value in rax
add rsp, 24d
pop rbp
ret

sum:
push rbp
mov rbp, rsp
sub rsp, 8d
push qword [rbp+16]
push qword [rbp+24]
pop rbx
pop rcx
add rbx, rcx
push rbx
pop qword [rbp-8]
mov rax, qword [rbp-8] ; return value in rax
add rsp, 8d
pop rbp
ret

div:
push rbp
mov rbp, rsp
sub rsp, 0d
push qword [rbp+16]
push qword [rbp+24]
pop rax
pop rbx
div rbx
push rax
pop rax
add rsp, 0d
pop rbp
ret
