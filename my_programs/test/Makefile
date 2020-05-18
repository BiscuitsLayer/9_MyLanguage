all: b l r
b:
	nasm -w+all -f elf64 -o 'simple_asm.o' 'main.s';
l:       
	ld -o 'asm.out' 'simple_asm.o'
r:
	./asm.out                        
