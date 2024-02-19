; hello.asm
; include direction
segment .data 
L1    dd  1
L2    dd  2
segment .bss
ret  resd 1
segment .text
global asm_main
asm_main:
enter   0,0 
pusha   
mov     eax, [L1]
add     eax, [L2]
mov     [ret], eax 
popa
mov eax, [ret]
leave
ret
