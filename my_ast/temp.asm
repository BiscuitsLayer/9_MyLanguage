CALL main
END

main:
PUSH 0
POP [0]
PUSH 0
POP [1]
PUSH 0
POP [2]
IN
POP [0]
IN
POP [1]
IN
POP [2]
PUSH BP
POP [SP]
PUSH [0]
POP [SP]
PUSH [1]
POP [SP]
PUSH [2]
POP [SP]
PUSH SP
PUSH 3
SUB
POP BP
CALL disc
PUSH [-1]
PUSH BP
PUSH 1
SUB
POP SP
POP BP
POP [3]
PUSH [3]
OUT
PUSH [3]
PUSH 0
JAE elseif_1
PUSH 0
OUT
JMP endif_1
elseif_1:
PUSH [3]
PUSH 0.5
POW
POP [3]
PUSH [3]
PUSH 0
JNE elseif_2
PUSH 1
OUT
PUSH -1
PUSH [1]
MUL
PUSH 2
PUSH [0]
MUL
DIV
POP [4]
PUSH [4]
OUT
JMP endif_2
elseif_2:
PUSH 2
OUT
PUSH -1
PUSH [1]
MUL
PUSH [3]
ADD
PUSH 2
PUSH [0]
MUL
DIV
POP [4]
PUSH -1
PUSH [1]
MUL
PUSH [3]
SUB
PUSH 2
PUSH [0]
MUL
DIV
POP [5]
PUSH [4]
OUT
PUSH [5]
OUT
endif_2:
endif_1:
PUSH 0
RET

disc:
PUSH [1]
PUSH 2
POW
PUSH 4
PUSH [0]
MUL
PUSH [2]
MUL
SUB
POP [3]
PUSH [3]
RET
