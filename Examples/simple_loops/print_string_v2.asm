/*
    Example: print_string_v2.asm

    Just showing the structured loop example this time, but with a few extra
    macros I would also use and add to my library.

    The ztext macro is used to automatically zero-terminate the string provided
    as an argument.

    The print_string macro is used to move the detail of setting up the string
    pointer from the main code.

    It's all about moving assembly a half-step towards beinga higher-level
    language.
*/

#import "hla.inc"

BasicUpstart2(start)

start: {
    print_string(text)
    rts
}


text:
    ztext(@"JUST THE STRUCTURED LOOP THIS TIME\r")



.label temp_ptr = $fb       // temporary pointer location (unused in C64)
.label CHROUT = $ffd2       // Kernal routine

// This macro provides automatically null-terminated strings
.macro ztext(val) {
    .text val
    .byte 0
}

// Front-end to the print_string routine.
.macro print_string(lbl)
{
    lda #<lbl
    ldx #>lbl
    jsr print_string
}

print_string:
{
    sta temp_ptr            // Put the string ptr args into the temp_ptr
    stx temp_ptr + 1
    ldy #0

    do                      // Top of the loop
        lda (temp_ptr), y   // Get the next byte
    while_ne                // While it's not zero, we'll stay in the loop
        jsr CHROUT          // Pass the byte to CHROUT
        iny                 // Advance to the next byte
    loop                    // and repeat

    rts
}
