    .section .rodata
fmt_bar:
    .string "|%s|\n"
fmt_int:
    .string "%d\n"

err_empty:
    .string "Error: empty line\n"
err_two:
    .string "Error: expected two numbers separated by a space\n"
err_second_missing:
    .string "Error: second number is missing\n"
err_invalid_first:
    .string "Error: first number is not a valid integer: %s\n"
err_invalid_second:
    .string "Error: second number is not a valid integer: %s\n"

    .text
    .globl main
main:
    addi sp, sp, -208
    sd ra, 200(sp)
    sd s0, 192(sp)
    sd s1, 184(sp)
    sd s2, 176(sp)
    sd s3, 168(sp)
    sd s4, 160(sp)
    sd s5, 152(sp)

    addi s0, sp, 208

    addi s1, sp, 0

    mv a0, s1
    li a1, 128
    call gets
    mv t0, s1

find_end_nl:
    lbu t1, 0(t0)
    beqz t1, check_last
    addi t0, t0, 1
    j find_end_nl

check_last:
    addi t0, t0, -1
    lbu t1, 0(t0)

    li t2, '\n'
    beq t1, t2, remove_last

    li t2, '\r'
    beq t1, t2, remove_last

    j strip_done

remove_last:
    sb zero, 0(t0)

strip_done:
    la a0, fmt_bar
    mv a1, s1
    call printf

    mv s2, s1

skip_spaces:
    lbu t0, 0(s2)
    beqz t0, empty_line
    li t1, ' '
    bne t0, t1, after_skip
    addi s2, s2, 1
    j skip_spaces

after_skip:
    mv s3, s2

find_space:
    lbu t0, 0(s3)
    beqz t0, need_two_numbers
    li t1, ' '
    beq t0, t1, space_found
    addi s3, s3, 1
    j find_space

space_found:
    sb zero, 0(s3)
    mv s4, s2
    addi s5, s3, 1

skip_spaces2:
    lbu t0, 0(s5)
    beqz t0, second_missing
    li t1, ' '
    bne t0, t1, after_skip2
    addi s5, s5, 1
    j skip_spaces2

after_skip2:
    mv t2, s5

find_end:
    lbu t0, 0(t2)
    beqz t0, trim_back
    addi t2, t2, 1
    j find_end

trim_back:
trim_loop:
    beq t2, s5, trim_done
    addi t3, t2, -1
    lbu t0, 0(t3)
    li t1, ' '
    bne t0, t1, trim_done
    sb zero, 0(t3)
    mv t2, t3
    j trim_loop

trim_done:
    mv a0, s4
    call is_valid_int_number
    beqz a0, invalid_first

    mv a0, s5
    call is_valid_int_number
    beqz a0, invalid_second

    mv a0, s4
    call atoi_signed
    mv t4, a0

    mv a0, s5
    call atoi_signed
    mv t5, a0

    add t6, t4, t5

    la a0, fmt_int
    mv a1, t6
    call printf

    li a0, 0
    call exit

empty_line:
    li a0, 2
    la a1, err_empty
    call fprintf
    li a0, 1
    call exit

need_two_numbers:
    li a0, 2
    la a1, err_two
    call fprintf
    li a0, 1
    call exit

second_missing:
    li a0, 2
    la a1, err_second_missing
    call fprintf
    li a0, 1
    call exit

invalid_first:
    li a0, 2
    la a1, err_invalid_first
    mv a2, s4
    call fprintf
    li a0, 1
    call exit

invalid_second:
    li a0, 2
    la a1, err_invalid_second
    mv a2, s5
    call fprintf
    li a0, 1
    call exit

    .globl is_valid_int_number
is_valid_int_number:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    mv s0, a0

    lbu t0, 0(s0)
    beqz t0, inv_bad
    
    li t1, '-'
    bne t0, t1, check_start_digits
    addi s0, s0, 1
    lbu t0, 0(s0)
    beqz t0, inv_bad

check_start_digits:
    li t1, '0'
    blt t0, t1, inv_bad
    li t2, '9'
    bgt t0, t2, inv_bad

scan_digits:
    addi s0, s0, 1
    lbu t0, 0(s0)
    beqz t0, inv_ok

    li t1, '0'
    blt t0, t1, inv_bad
    li t2, '9'
    bgt t0, t2, inv_bad
    j scan_digits

inv_ok:
    li a0, 1
    ld ra, 24(sp)
    ld s0, 16(sp)
    addi sp, sp, 32
    ret

inv_bad:
    li a0, 0
    ld ra, 24(sp)
    ld s0, 16(sp)
    addi sp, sp, 32
    ret

    .globl atoi_signed
atoi_signed:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)

    mv s0, a0
    li s1, 1
    lbu t0, 0(s0)
    li t1, '-'
    bne t0, t1, call_atoi
    li s1, -1
    addi s0, s0, 1

call_atoi:
    mv a0, s0
    call atoi

    li t0, -1
    bne s1, t0, atoi_done
    sub a0, zero, a0

atoi_done:
    ld ra, 24(sp)
    ld s0, 16(sp)
    ld s1, 8(sp)
    addi sp, sp, 32
    ret