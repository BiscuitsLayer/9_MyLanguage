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
sub rsp, 8d
push 0d
pop qword [rbp-8]
call get
mov [rbp-8], rax
mov rax, [rbp-8]
call put
mov rax, 0d ; return value in rax
add rsp, 8d
pop rbp
ret

put:
xor r8, r8 ; r8 <- extra char '-' if nuber is negative
add rax, 0d ; check if rax is negative
jns startput
neg rax ; rax *= -1
mov rcx, output ; string offset
mov byte [rcx], 02dh ; '-' char
inc r8 ; because the nuber is negative
startput:
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
add rdx, r8 ; +1 if the number is negative
add rcx, r8 ; +1 if the number is negative
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
xor r8, r8 ; r8 <- extra char '-' if number is negative
xor rax, rax ; rax = 0 <- result
mov rdi, 10d ; rdi = 10 <- decimal
mov rcx, input ; string offset
repget:
mov bl, byte [rcx] ; bl = next char
; sign
cmp bl, 02dh ; 02dh <- '-'
je signget
; sign
; end string
cmp bl, 0ah ; 0ah <- end string char 
je endget
; end string
; point
cmp bl, 02ch ; if (new_char != ',')
je pointget
cmp bl, 02eh ; if (new_char != ',')
je pointget
; point
sub bl, '0' ; bl (ascii) -> bl (digit)
mul rdi ; result *= 10
add rax, rbx ; result += new_char
inc rcx ; next char
jmp repget ; continue cycle
signget:
inc r8 ; number is negative
inc rcx ; next char
jmp repget
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
cmp r8, 1d ; if the number is negative
jne retget2
neg rax ; rax *= -1
retget2:
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
num dq 1
; FOR SQRT OPERATION
