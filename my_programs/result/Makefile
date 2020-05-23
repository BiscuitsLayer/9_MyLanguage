all: b l r
b:
	nasm -w+all -f elf64 -o 'nasm_main.o' 'my_main.asm';
l:       
	ld -o 'nasm_main' 'nasm_main.o'
r:
	./nasm_main                        
