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
mov rax, 10000d
call put
;push rax
pop rbp
ret

put:
mov rdi, 10d ; rdi = 10 <- decimal
repput:
xor rdx, rdx ; or divident will be dx_ax
div rdi ; rax /= 10, rdx %= 10
add rdx, '0' ; rdx -> ascii
push rdx
inc rsi ; ++digits counter
cmp rax, 0 ; while rax != 0
jne repput ; continue cycle
mov rcx, output ; string offset
repput2:
pop rbx ; pop digit ascii
mov byte [rcx], bl
inc rcx ; next char
dec rsi ; --digits counter
cmp rsi, 2d ; it's point time
je pointput
cmp rsi, 0d
jne repput2
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
mov rdx, 16d ; output length
int 0x80
; PUT FUNCTION
ret

section .data
; FOR PUT
output dw 1
; FOR PUT
