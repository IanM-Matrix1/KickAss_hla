/*
    Example: print_string_v3.asm

    This is a demonstration of nested structured statements.

    This code uses an alternative string representation - the first byte is the
    number of bytes in the string, and is followed by the data.

    The stext macro is used to automatically prefix the string with its size.

    The print_string macro is used to move the detail of setting up the string
    pointer from the main code.
*/

#import "hla.inc"

BasicUpstart2(start)

start: {
    print_string(text)
    rts
}

text: stext(@"USING SIZE-PREFIXED STRINGS INSTEAD OF  ZERO TERMINATED ONES\r")

.label temp_ptr = $fb       // temporary pointer location (unused in C64)
.label CHROUT = $ffd2       // Kernal routine

// This macro provides automatically prefixes the string with its length
.macro stext(val) {
    .byte val.size()
    .text val
}

// Front-end to the print_string routine.
.macro print_string(lbl)
{
    lda #<lbl
    ldx #>lbl
    jsr print_string
}


print_string: {
    sta temp_ptr                // Put the string ptr args into the temp_ptr
    stx temp_ptr + 1
    ldy #0

    lda (temp_ptr), y           // Load the string length
    if_not_zero                 // If the string size isn't zero, then continue
        tax                     // Put the length in X
        do
            iny                 // Advance the string pointer
            lda (temp_ptr),y    // Get the character
            jsr CHROUT          // Pass the byte to CHROUT
            dex                 // Decrement x
        loop_if_not_zero        // Loop if it hasn't reached zero yet
    endif
    rts
}
