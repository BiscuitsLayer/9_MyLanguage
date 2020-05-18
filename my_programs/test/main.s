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
push 82900d
pop qword [rbp-8]
push 50d
pop qword [rbp-16]
push qword [rbp-8]
push qword [rbp-16]
; POWER FUNCTION
pop rcx
pop rax
cmp rcx, 50d
je sqrt_1
call pow
jmp endpow_1
sqrt_1:
call sqrt
endpow_1:
push rax
; POWER FUNCTION
pop qword [rbp-24]
mov rax, 0d ; return value in rax
add rsp, 24d
pop rbp
ret

pow:
push rbp
mov rbp, rsp
mov rbx, rax
; ACCURACY
mov rdi, 100d ; rdx = ACCURACY
div rdi
xchg rax, rbx
; ACCURACY
sub rcx, rdi
.for:
mul rbx
sub rcx, rdi
jnz .for
pop rbp
ret

sqrt:
push rbp
mov rbp, rsp
mov rdx, num
mov qword [rdx], rax ; num = rax
fild qword [rdx] ; fpu_reg = num
fsqrt ; fpu_reg = sqrt (fpu_reg)
fistp qword [rdx] ; num = fpu_reg
mov rax, qword [rdx] ; rax = num
; ACCURACY
mov rdi, 10d
mul rdi
; ACCURACY
pop rbp
ret

section .data
; FOR SQRT OPERATION
num db 8
; FOR SQRT OPERATION