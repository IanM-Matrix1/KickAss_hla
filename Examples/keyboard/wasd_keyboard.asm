/*
    Example: wasd_keyboard_state.asm

    Demonstration of coverage of the keyboard controls.

    Note that the same down/up/pressed/not_pressed/released/not_released
    tests will work in exactly the same way for directions and fire on the
    joystick.

    Just run it and see all of the tests being performed for the WASD keys and
    the spacebar.

    There are 10 lines in this code to deal with the keyboard completely:
    - initialize_keystate_list
    - update_keyboard_matrix
    - update_keystate_list_from_matrix
    - if_key_down
    - if_key_up
    - if_key_pressed
    - if_key_not_pressed
    - if_key_released
    - if_key_not_released
    - create_keyMatrix

*/

#import "hla_controller.inc"

// Text output routines
    .label CHROUT = $ffd2       // Kernal routine - output character (a=char)
    .label PLOT   = $fff0       // Kernel routine - position the cursor (x=row, y=col)
    .label STROUT = $ab1e       // Kernal routine - output a zero-terminated string (a=low,y=high)


    BasicUpstart2(start)

start:
    cls()
    
    initialize_keystate_list(keyStateData)
    
    do
        update_keyboard_matrix(keyMatrix)
        update_keystate_list_from_matrix(keyStateList, keyStateData, keyMatrix)

        setPos(0, 1)
        show_key_state(txt_key_space,   keyStateData.space)
        show_key_state(txt_key_w,       keyStateData.w)
        show_key_state(txt_key_a,       keyStateData.a)
        show_key_state(txt_key_s,       keyStateData.s)
        show_key_state(txt_key_d,       keyStateData.d)
    loop

// Interface to the subroutine to show each keyState status
.macro show_key_state(txt_desc, keyState) {
    printString(txt_desc)

    // Copy the keyState specified to the local storage for the subroutine to use
    lda keyState
    sta show_key_state.keyState

    // Call the code in the subroutine
    jsr show_key_state.code
}

// The actual subroutine where the work is done
show_key_state: {
code:
    if_key_down(keyState)
        printString(txt_down)
    endif

    if_key_up(keyState)
        printString(txt_up)
    endif
    
    if_key_pressed(keyState)
        printString(txt_pressed)
    else
        printString(txt_blank)
    endif
    
    if_key_not_pressed(keyState)
        printString(txt_not_pressed)
    else
        printString(txt_blank)
    endif
    
    if_key_released(keyState)
        printString(txt_released)
    else
        printString(txt_blank)
    endif
    
    if_key_not_released(keyState)
        printString(txt_not_released)
    else
        printString(txt_blank)
    endif
    
    printString(txt_lineEnd)
    
    rts

    // Local storage for the subroutine.
keyState:
    .byte 0
}

    .encoding("petscii_mixed")

// Keyboard control
txt_key_space:      ztext("space ")
txt_key_w:          ztext("    w ")
txt_key_a:          ztext("    a ")
txt_key_s:          ztext("    s ")
txt_key_d:          ztext("    d ")

// Each keystate state (!?)
txt_down:           ztext(" d ")
txt_up:             ztext(" u ")
txt_pressed:        ztext(" p  ")
txt_not_pressed:    ztext(" np ")
txt_released:       ztext(" r  ")
txt_not_released:   ztext(" nr ")
txt_blank:          ztext("    ")

txt_lineEnd:        ztext(@"\r")

// Storage for the keyboard state
keyMatrix:          create_keyMatrix()

// The list of keys we want data for
keyStateList: {
    lo:
        .byte keyInfo(" ").row
        .byte keyInfo("W").row
        .byte keyInfo("A").row
        .byte keyInfo("S").row
        .byte keyInfo("D").row
    hi:
        .byte keyInfo(" ").colMask
        .byte keyInfo("W").colMask
        .byte keyInfo("A").colMask
        .byte keyInfo("S").colMask
        .byte keyInfo("D").colMask
    end:
    .errorif hi-lo != end-hi, "keyStateList has a different number of lo and hi bytes"
}

// The current state of each key
keyStateData: {
    space:      .byte $ff
    w:          .byte $ff
    a:          .byte $ff
    s:          .byte $ff
    d:          .byte $ff
    end:
    .errorif keyStateData.end-keyStateData != keyStateList.hi-keyStateList.lo, "keyStateData size does not have the same number of keys as keyStateList"
}


.macro ztext(val) {
    .text val
    .byte 0
}

    // Output zero-terminated string
.macro printString(addr) {
    lda #<addr
    ldy #>addr
    jsr STROUT
}

.macro setPos(x, y) {
    clc
    ldy #x
    ldx #y
    jsr PLOT
}

.macro cls() {
    lda #147
    jsr CHROUT
}
