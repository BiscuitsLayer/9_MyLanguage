//REGISTERS
const int REGLEN = 1;
#define RAX 0b00000000
#define RCX 0b00000001
#define RDX 0b00000010
#define RBX 0b00000011
#define RSP 0b00000100
#define RBP 0b00000101
#define RSI 0b00000110
#define RDI 0b00000111

//PROREGISTERS
const int PROREGLEN = 1;
#define R8 0b00000000
#define R9 0b00000001
#define R10 0b00000010
#define R11 0b00000011
#define R12 0b00000100
#define R13 0b00000101
#define R14 0b00000110
#define R15 0b00000111

//MATH
const int MATHLEN = 3;
#define INC(REG) 0b01001000, 0b11111111, 0b11000000 | REG
#define DEC(REG) 0b01001000, 0b11111111, 0b11001000 | REG
#define ADD_R_R(REG1, REG2) 0b01001000, 0b00000001, 0b11000000 | REG1 | (REG2 << 3)
#define ADD_R_N ?
#define SUB_R_R(REG1, REG2) 0b01001000, 0b00101001, 0b11000000 | REG1 | (REG2 << 3)
#define SUB_R_N ?

#define IMUL(REG) 0b01001000, 0b11110111, 0b11101000 | REG
#define IDIV(REG) 0b01001000, 0b11110111, 0b11111000 | REG

//SYSTEM
const int SYSLEN = 2;
#define SYSCALL 0b00001111, 0b00000101
#define CALL ?
#define RET ?

//STACK
const int STACKLEN = 1;
#define PUSH_R(REG) 0b01010000 | REG
#define PUSH_N(NUM) ?
#define PUSH_M(M) ?
#define POP_R(REG) 0b01011000 | REG

//MEMORY
const int MEMLEN = 7;
#define MOV_R_N(REG, NUM) 0b01001000, 0b11000111, 0b11000000 | REG, (NUM & 0x000000FF), (NUM & 0x0000FF00) >> 8, (NUM & 0x00FF0000) >> 16, (NUM & 0xFF000000) >> 24
#define MOV_R_R(REG1, REG2) ?
#define MOV_R_M(REG, MEM) ?
#define MOV_M_R(MEM, REG) ?
#define XCHG_R_R(REG1, REG2) ?

//MOV BYTE BL, NUM ???
//MOV BYTE MEM, BL
//MOV BL, NUM

//LOGIC
#define NEGAT_R(REG) ? //Политкорректное имя
#define XOR_R_R(REG1, REG2) 0b01001000, 0b00110001, 0b11000000 | REG1 | (REG2 << 3)
#define TEST_R_R(REG1, REG2) 0b01001000, 0b10000101, 0b11000000 | REG1 | (REG2 << 3)
#define TEST_R_N(REG, NUM) ?
#define CMP_R_N(REG, NUM) ?
#define CMP_R_R(REG1, REG2) 0b01001000, 0b00111001, 0b11000000 | REG1 | (REG2 << 3)

//JUMPS
#define JMP ?
#define JE ?
#define JNE ?
#define JG ?
#define JGE ?
#define JL ?
#define JLE ?
#define JS ?
#define JNS ?

//JZ ?? JNZ ??

//SQRT
#define FINIT 0b10011011, 0b11011011, 0b11100011
#define FILD_MR(REG) 0b11011111, 0b00101000 | REG //Вместо указателя на память значение регистра
#define FSQRT 0b11011001, 0b11111010
#define FISTP_MR(REG) 0b11011111, 0b00111000 | REG //Вместо указателя на память значение регистра

//Разберись с section test И data