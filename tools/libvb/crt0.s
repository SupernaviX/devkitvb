    .section .text
    .align	2

    .global	_start
_start:
    # Initialize CPU
    ldsr r0, sr5
    sei
    ldsr r0, sr24

    # Delay to allow DRAM to warm up
    movhi 0x0002, r0, r10
dummyloop:
    add -1, r10
    bnz dummyloop

/* initiallize .data section */
    movhi	hi(__data_lma),r0,r4
    movea	lo(__data_lma),r4,r4
    movhi	hi(__data_end),r0,r5
    movea	lo(__data_end),r5,r5
    movhi	hi(__data_vma),r0,r6
    movea	lo(__data_vma),r6,r6

    jr	end_initdata
initdata:
    ld.b	0[r4],r7
    st.b	r7,0[r6]
    add	1,r4
    add	1,r6
end_initdata:
    cmp	r5,r6
    blt	initdata

/* clear .bss section and unintialized RAM */
    movhi	0x0501,r0,r4
    jr	loop_start1
loop_top1:
    st.h	r0,0[r5]
    add	2,r5
loop_start1:
    cmp	r4,r5
    blt	loop_top1

    # Configure global register pointers
    movhi hi(__stack_end), r0, sp
    movea lo(__gp), sp, gp

    # Call main()
    jr    main


    .global default_handler
default_handler:
    reti

    .section .rom_header,"a"
    .ascii "hello, world        "   # Title
    .byte  0x00,0x00,0x00,0x00,0x00 # Reserved
    .ascii "  "                     # Maker
    .ascii "VHWU"                   # Game
    .byte  0x00                     # Version

.macro VECTOR name
    .section .handlers.\name,"ax"
    jr \name
    .balign 16
.endm

    VECTOR isrGamePad
    VECTOR isrTimer
    VECTOR isrGamePak
    VECTOR isrCom
    VECTOR isrVIP
    VECTOR isrFpException
    VECTOR isrDivideByZero
    VECTOR isrIllegalOpcode
    VECTOR isrLoTrap
    VECTOR isrHiTrap
    VECTOR isrAddressTrap
    VECTOR isrDuplexed
    VECTOR _start
