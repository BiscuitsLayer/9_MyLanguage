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
sub rsp, 32d ; space for locals
push 0d ; push number
pop qword [rbp-8] ; equal
push 0d ; push number
pop qword [rbp-16] ; equal
push 0d ; push number
pop qword [rbp-24] ; equal
call get ; get_start
mov [rbp-8], rax ; get_end
call get ; get_start
mov [rbp-16], rax ; get_end
call get ; get_start
mov [rbp-24], rax ; get_end
push qword [rbp-24] ; push local variable
push qword [rbp-16] ; push local variable
push qword [rbp-8] ; push local variable
call solveSquare
add rsp, 24d ; clean space for arguments
push qword rax ; push the value just returned
pop qword [rbp-32] ; equal
mov rax, 0d ; return value in rax
add rsp, 32d ; clean space for locals
pop rbp ; typical function exit
ret

solveSquare:
push rbp ; typical function entry
mov rbp, rsp
sub rsp, 32d ; space for locals
push qword [rbp+16] ; push local variable
push 0d ; push number
pop rax ; jmp start
pop rbx
cmp rax, rbx
jne elseif_1
push qword [rbp+32] ; push local variable
push qword [rbp+24] ; push local variable
call solveLinear
add rsp, 16d ; clean space for arguments
push qword rax ; push the value just returned
pop qword [rbp-8] ; equal
jmp endif_1
elseif_1:
push qword [rbp+32] ; push local variable
push 0d ; push number
pop rax ; jmp start
pop rbx
cmp rax, rbx
jne elseif_2
push 200d ; put_start
pop rax
call put ; put_end
push 0d ; put_start
pop rax
call put ; put_end
push qword [rbp+24] ; push local variable
push qword [rbp+16] ; push local variable
call solveLinear
add rsp, 16d ; clean space for arguments
push qword rax ; push the value just returned
pop qword [rbp-8] ; equal
jmp endif_2
elseif_2:
push qword [rbp+32] ; push local variable
push qword [rbp+24] ; push local variable
push qword [rbp+16] ; push local variable
call Discriminant
add rsp, 24d ; clean space for arguments
push qword rax ; push the value just returned
pop qword [rbp-16] ; equal
push qword [rbp-16] ; push local variable
push 0d ; push number
pop rax ; jmp start
pop rbx
cmp rax, rbx
jle elseif_3
push 0d ; put_start
pop rax
call put ; put_end
jmp endif_3
elseif_3:
push qword [rbp-16] ; push local variable
push 50d ; push number
; POWER FUNCTION
pop rcx
pop rax
cmp rcx, 50d ; compare if sqrt or not
je sqrt_1
call pow ; just power
jmp endpow_1
sqrt_1:
call sqrt ; just sqrt
endpow_1:
push rax
; POWER FUNCTION
pop qword [rbp-16] ; equal
push qword [rbp-16] ; push local variable
push 0d ; push number
pop rax ; jmp start
pop rbx
cmp rax, rbx
jne elseif_4
push 100d ; put_start
pop rax
call put ; put_end
push -100d ; push number
push qword [rbp+24] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
push 200d ; push number
push qword [rbp+16] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
pop rbx ; div_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
mov rcx, rbx
mov rbx, 100d
call mymul
mov rbx, rcx
call mydiv
push rax ; div_end
pop qword [rbp-24] ; equal
mov rax, [rbp-24]
call put ; put_end
jmp endif_4
elseif_4:
push 200d ; put_start
pop rax
call put ; put_end
push -100d ; push number
push qword [rbp+24] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
push qword [rbp-16] ; push local variable
pop rcx ; sum_start
pop rbx
add rbx, rcx
push rbx ; sum_end
push 200d ; push number
push qword [rbp+16] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
pop rbx ; div_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
mov rcx, rbx
mov rbx, 100d
call mymul
mov rbx, rcx
call mydiv
push rax ; div_end
pop qword [rbp-24] ; equal
push -100d ; push number
push qword [rbp+24] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
push qword [rbp-16] ; push local variable
pop rcx ; sub_start
pop rbx
sub rbx, rcx
push rbx ; sub_end
push 200d ; push number
push qword [rbp+16] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
pop rbx ; div_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
mov rcx, rbx
mov rbx, 100d
call mymul
mov rbx, rcx
call mydiv
push rax ; div_end
pop qword [rbp-32] ; equal
mov rax, [rbp-24]
call put ; put_end
mov rax, [rbp-32]
call put ; put_end
endif_4:
endif_3:
endif_2:
endif_1:
mov rax, 0d ; return value in rax
add rsp, 32d ; clean space for locals
pop rbp ; typical function exit
ret

solveLinear:
push rbp ; typical function entry
mov rbp, rsp
sub rsp, 8d ; space for locals
push qword [rbp+16] ; push local variable
push 0d ; push number
pop rax ; jmp start
pop rbx
cmp rax, rbx
jne elseif_5
push qword [rbp+24] ; push local variable
push 0d ; push number
pop rax ; jmp start
pop rbx
cmp rax, rbx
jne elseif_6
push 100000d ; put_start
pop rax
call put ; put_end
jmp endif_6
elseif_6:
push 0d ; put_start
pop rax
call put ; put_end
endif_6:
jmp endif_5
elseif_5:
push 100d ; put_start
pop rax
call put ; put_end
push -100d ; push number
push qword [rbp+24] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
push qword [rbp+16] ; push local variable
pop rbx ; div_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
mov rcx, rbx
mov rbx, 100d
call mymul
mov rbx, rcx
call mydiv
push rax ; div_end
pop qword [rbp-8] ; equal
mov rax, [rbp-8]
call put ; put_end
endif_5:
mov rax, 0d ; return value in rax
add rsp, 8d ; clean space for locals
pop rbp ; typical function exit
ret

Discriminant:
push rbp ; typical function entry
mov rbp, rsp
sub rsp, 0d ; space for locals
push qword [rbp+24] ; push local variable
push 200d ; push number
; POWER FUNCTION
pop rcx
pop rax
cmp rcx, 50d ; compare if sqrt or not
je sqrt_2
call pow ; just power
jmp endpow_2
sqrt_2:
call sqrt ; just sqrt
endpow_2:
push rax
; POWER FUNCTION
push 400d ; push number
push qword [rbp+16] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
push qword [rbp+32] ; push local variable
pop rbx ; mul_start
pop rax
xor rdx, rdx ; or divident will be dx_ax
call mymul
xor rdx, rdx ; or divident will be dx_ax
mov rbx, 100d
call mydiv
push rax ; mul_end
pop rcx ; sub_start
pop rbx
sub rbx, rcx
push rbx ; sub_end
pop rax ; return value in rax
add rsp, 0d ; clean space for locals
pop rbp ; typical function exit
ret

mymul:
push r8 ; save r8
xor r8, r8 ; number digit
add rax, 0
jns secondmul
neg rax
inc r8
secondmul:
add rbx, 0
jns checkmul
neg rbx
dec r8
checkmul:
imul rbx
cmp r8, 0d
je endmul
neg rax
endmul:
pop r8 ; return saved r8
ret

mydiv:
push r8 ; save r8
xor r8, r8 ; number digit
add rax, 0
jns seconddiv
neg rax
inc r8
seconddiv:
add rbx, 0
jns checkdiv
neg rbx
dec r8
checkdiv:
idiv rbx
cmp r8, 0d
je enddiv
neg rax
enddiv:
pop r8 ; return saved r8
ret

put:
xor r8, r8 ; r8 <- extra char '-' if number is negative
xor r9, r9 ; flag if pointput was called
add rax, 0d ; check if rax is negative
jns startput ; not negative -> startput
neg rax ; rax *= -1
mov rcx, output ; string offset
mov byte [rcx], 02dh ; '-' char
inc r8 ; because the number is negative
startput:
mov rdi, 10d ; rdi = 10 <- decimal
xor rsi, rsi ; rsi <- digits counter
repput:
xor rdx, rdx ; or divident will be dx_ax
xchg rbx, rdi ; to call mydiv
call mydiv ; rax /= 10, rdx = rax mod 10
xchg rbx, rdi ; to call mydiv
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
inc r9 ; pointput flag
mov bl, 02ch ; point char
mov byte [rcx], bl
inc rcx ; next char
jmp repput2
endput:
mov rcx, output ; string offset
test r9, r9 ; test pointput flag
jnz endput2
push rdx ; save rdx
push rcx ; save rcx
xor rdx, rdx ; just clean rdx (no mul or div)
mov rcx, service_pos; print '0,'
mov rdi, service_pos_len
mov dl, byte [rdi]
test r8, r8 ; check if we need '-' char
jz nextput ; if we don't
pop rcx ; slightly change the value of rcx saved
pop rdx ; slightly change the value of rdx saved
dec rdx ;  in the main put we don't print '-'
inc rcx ; starting from the number
push rdx ; slightly change the value of rdx saved
push rcx ; slightly change the value of rcx saved
mov rcx, service_neg; print '-0,'
mov rdi, service_neg_len
mov dl, byte [rdi]
nextput:
mov rax, 4 ; sys_write
mov rbx, 1 ; file descriptor = stdout
int 0x80
pop rcx ; return saved rcx
pop rdx ; return saved rdx
dec rdx ; in the main put we don't print point
endput2:
; PUT FUNCTION
mov rax, 4 ; sys_write
mov rbx, 1 ; file descriptor = stdout
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
xchg rbx, rdi ; to call mymul
call mymul ; result *= 10
xchg rbx, rdi ; to call mymul
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
xchg rbx, rdi ; to call mymul
call mymul ; result *= 10
xchg rbx, rdi ; to call mymul
add rax, rbx ; result += new_char
inc rcx ; next char
dec rsi ; --counter
jnz repget2 ; continue cycle
jmp retget
endget:
; ACCURACY
mov rcx, 100d
xchg rbx, rcx ; to call mymul
call mymul
xchg rbx, rcx ; to call mymul
; ACCURACY
jmp retget
endget2:
repget3:
xchg rbx, rdi ; to call mymul
call mymul
xchg rbx, rdi ; to call mymul
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
test rcx, 1d ; check if rcx is odd (mod 2 != 0) or even (mod 2 = 0)
jnz powstart
add rax, 0
jns powstart
neg rax ; if number is even (mod 2 = 0) rax *= -1
powstart:
mov rbx, rax ; rbx = factor
; ACCURACY
mov rdi, 100d ; rdi = ACCURACY
xor rdx, rdx ; or divident will be dx_ax
xchg rbx, rdi ; to call mydiv
call mydiv
xchg rbx, rdi ; to call mydiv
xchg rax, rbx ; rbx = rbx / ACCURACY
; ACCURACY
sub rcx, rdi
reppow:
call mymul ; multiply by a factor
sub rcx, rdi ; --rcx (because rcx is multiplied by REAL_ACCURACY)
jnz reppow ; continue cycle
pop rbp
ret

sqrt:
push rbp
mov rbp, rsp
mov rdx, num ; rdx = memory address for sqrt (num)
mov qword [rdx], rax ; num = rax
finit
fild qword [rdx] ; fpu_reg = num
fsqrt ; fpu_reg = sqrt (fpu_reg)
fistp qword [rdx] ; num = fpu_reg
mov rax, qword [rdx] ; rax = num
; ACCURACY
mov rdi, 10d
xchg rbx, rdi ; to call mymul
call mymul
xchg rbx, rdi ; to call mymul
; ACCURACY
pop rbp
ret

section .data
; FOR PUT
service_pos db "0,"
service_pos_len db $ - service_pos
service_neg db "-0,"
service_neg_len db $ - service_neg
output dq 1
; FOR PUT
; FOR GET
input dq 1
; FOR GET
; FOR SQRT OPERATION
num dq 1
; FOR SQRT OPERATION
