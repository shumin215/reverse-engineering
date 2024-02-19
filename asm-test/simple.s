.global _start

.text
_start:
        movq $1, %rax
        movq $2, %rbx
        subq %rbx, %rax
        ret
