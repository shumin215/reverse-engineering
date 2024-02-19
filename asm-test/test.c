#include <stdio.h>

void foo()
{
    __asm__ ( "xorq %rax, %rax;"
              "movq %rax, %rbx;"
              "decq %rax;"
              "addq %rax, %rbx;"
            );
}

void bar()
{
    __asm__ ( "xorq %rax, %rax;"
              "incq %rax;"
              "movq $0x7FFFFFFFFFFFFFFF, %rbx;" 
              "addq %rbx, %rax;"                // To check overflow flag
              "movq $0x8000000000000000, %rax;"
              "movq $0x8000000000000000, %rbx;"
              "addq %rbx, %rax;"                // To check overflow and carry flags
            );
}

void nothing (void)
{
    return;
}

void popFunc()
{
    __asm__ ( "call %P0" : : "i"(nothing));
    __asm__ (
              "subq $8, %rsp;"
              "popq %rax;"
            );
}

int main()
{
    popFunc();
    foo();
    bar();
    return 0;
}
