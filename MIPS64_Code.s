      .data

jump:    .word    0

String_:    .ascii    "BIGGER\0"

String_1:    .ascii    "SMALLER\0"
      .text
      .globl    main


      li    $9,10
      la    $8,jump
      sd    $9,($8)

      j    main

main:

      dsubi    $sp,$sp,4

      li    $2,5
      syscall
      sd    $2,-4($fp)

      li    $24,10
      ld    $25,-4($fp)
      blt    $24,$25,Function_

      la    $4,String_
      li    $2,4
      syscall

      j    Function_1


Function_:

      la    $4,String_1
      li    $2,4
      syscall


Function_1:

      li    $2,10
      syscall
