.intel_syntax noprefix
.code32

.section .text
.global start
.extern main
.extern exit

start:
    call main
    push eax
    call exit

hang:
    jmp hang
