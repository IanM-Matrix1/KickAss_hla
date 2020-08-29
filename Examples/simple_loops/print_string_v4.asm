/*
    Example: print_string_v4.asm

    This is the final version - it turns out that the C64 Kernal already
    supports zero-terminated strings, so even though this code doesn't
    demonstrate the HLA library, I've added it for completeness.

    Of course, if you decide to bank out the ROMs, you can fall back to using
    one of the earlier methods.
*/

BasicUpstart2(start)

start: {
    print_string(text)
    rts
}

text:
    ztext(@"USING THE KERNAL THIS TIME\r")

// This macro provides automatically null-terminated strings
.macro ztext(val) {
    .text val
    .byte 0
}

    .label STROUT = $ab1e       // Kernal/BASIC routine - output a zero-terminated string (a=low,y=high)

// Front-end to the print_string routine.
.macro print_string(lbl)
{
    lda #<lbl
    ldy #>lbl
    jsr STROUT
}
