.intel_syntax noprefix
.code32

.section .text
.global start

start:
    call 1f
1:
    pop esi

    mov eax, 1
    lea ebx, [esi + hello_message - 1b]
    mov ecx, hello_message_end - hello_message
    int 0x80

    mov eax, 2
    xor ebx, ebx
    int 0x80

hang:
    jmp hang

hello_message:
    .ascii "hello from /boot/hello.elf via int 0x80\n"
hello_message_end:
