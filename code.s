
.data

jump:    .word    0

String_:    .ascii    "BIGGER\0"

String_1:    .ascii    "SMALLER\0"

.text
.globl	main

#< const   ,int     ,        ,jump     >

#< =       ,10      ,        ,jump     >
      li    $9,10
      la    $8,jump
      sw    $9,($8)

#< func    ,        ,        ,main     >

main:

#< int     ,        ,        ,a        >
      subi    $sp,$sp,4

#< scanf   ,        ,        ,a        >
      li    $2,5
      syscall
      sw    $2,-4($fp)

#< <       ,10      ,a       ,Function_ >
      li    $24,10
      lw    $25,-4($fp)
      blt    $24,$25,Function_

#< printf  ,"BIGGER",        ,         >
      la    $4,String_
      li    $2,4
      syscall

#< jmp     ,        ,        ,Function_1 >
      j    Function_1

#< lab     ,        ,        ,Function_ >

Function_:

#< printf  ,"SMALLER",        ,         >
      la    $4,String_1
      li    $2,4
      syscall

#< lab     ,        ,        ,Function_1 >

Function_1:

#< exit    ,        ,        ,main     >
      li    $2,10
      syscall
