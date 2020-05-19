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
sub rsp, 0d
mov rax, 0d ; return value in rax
add rsp, 0d
pop rbp
ret

put:
mov rdi, 10d ; rdi = 10 <- decimal
xor rsi, rsi
repput:
xor rdx, rdx ; or divident will be dx_ax
div rdi ; rax /= 10, rdx = rax mod 10
add rdx, '0' ; rdx -> ascii
push rdx
inc rsi ; ++digits counter
cmp rax, 0 ; while rax != 0
jne repput ; continue cycle
mov rcx, output ; string offset
mov rdx, rsi ; output length
add rdx, 2 ; point char and \n are printed too!
repput2:
pop rbx ; pop digit ascii
mov byte [rcx], bl
inc rcx ; next char
dec rsi ; --digits counter
cmp rsi, 2d ; it's point time
je pointput
cmp rsi, 0d
jne repput2
mov byte [rcx], 0ah ; \n symbol 
jmp endput
pointput:
mov bl, 02ch
mov byte [rcx], bl
inc rcx
jmp repput2
endput:
; PUT FUNCTION
mov rax, 4 ; sys_write
mov rbx, 1 ; file descriptor = stdout
mov rcx, output ; string offset
int 0x80
; PUT FUNCTION
ret

get:
; GET FUNCTION
mov rax, 3 ; sys_read
mov rbx, 1 ; file descriptor = stdin
mov rcx, input ; string offset
mov rdx, 16d ; input length
int 0x80
; GET FUNCTION
xor rax, rax ; rax = 0 <- result
mov rdi, 10d ; rdi = 10 <- decimal
mov rcx, input ; string offset
repget:
mov bl, byte [rcx] ; bl = next char
; end string
cmp bl, 0ah ; 0ah <- end string char 
je endget
; end string
; point
cmp bl, 02ch ; if (new_char != ',')
je pointget
; point
sub bl, '0' ; bl (ascii) -> bl (digit)
mul rdi ; result *= 10
add rax, rbx ; result += new_char
inc rcx ; next char
jmp repget ; continue cycle
pointget:
mov rsi, 2d ; rsi = number of digits after the point
inc rcx ; next char
repget2:
mov bl, byte [rcx] ; bl = next char
; end string
cmp bl, 0ah ; 0ah <- end string char 
je endget2
; end string
sub bl, '0' ; bl (ascii) -> bl (digit)
mul rdi ; result *= 10
add rax, rbx ; result += new_char
inc rcx ; next char
dec rsi ; --counter
jnz repget2 ; continue cycle
jmp retget
endget:
; ACCURACY
mov rcx, 100d
mul rcx
; ACCURACY
jmp retget
endget2:
repget3:
mul rdi
dec rsi
jnz repget3
retget:
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
reppow:
mul rbx
sub rcx, rdi
jnz reppow
pop rbp
ret

sqrt:
push rbp
mov rbp, rsp
mov rdx, num
mov qword [rdx], rax ; num = rax
finit
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
; FOR PUT
output dq 1
; FOR PUT
; FOR GET
input dq 1
; FOR GET
; FOR SQRT OPERATION
num dq 8
; FOR SQRT OPERATION
