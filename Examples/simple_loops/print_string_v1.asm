/*
    Example: print_string_v1.asm

    Shows off two routines that have exaxtly the same functionality (just
    printing a string to the screen using the CHROUT Kernal routine), but with
    one written using plain assembly with labels, and the second using the
    structured do...loop commands.
    
    Both routines in this case will generate exactly the same opcodes.

    See example print_string_v2.asm to see how I'd write this really.
*/

#import "hla.inc"

BasicUpstart2(start)

start: {
    lda #<text_v1
    ldx #>text_v1
    jsr print_string_v1

    lda #<text_v2
    ldx #>text_v2
    jsr print_string_v2

    rts
}


text_v1:
    .text @"UNSTRUCTURED LOOP\r"
    .byte 0                         // zero byte to terminate the string


text_v2:
    .text @"STRUCTURED LOOP\r"
    .byte 0                         // zero byte to terminate the string


print_string_v1:
{
    // Set up zero page and registers
    sta $fb
    stx $fc
    ldy #0

    // Will read each char, and as long as the char value isn't zero, it will print it
loop_top:
    lda ($fb), y        // Get the next byte
    beq loop_end        // If it's zero, then jump out of the loop
    jsr $ffd2           // Pass the byte to CHROUT
    iny                 // Advance to the next byte
    jmp loop_top        // and repeat
loop_end:

    rts
}


print_string_v2:
{
    sta $fb
    stx $fc
    ldy #0

    do                  // Top of the loop
        lda ($fb), y    // Get the next byte
    while_ne            // While it's not zero, we'll stay in the loop
        jsr $ffd2       // Pass the byte to CHROUT
        iny             // Advance to the next byte
    loop                // and repeat

    rts
}
