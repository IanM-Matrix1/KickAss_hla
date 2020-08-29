/*
    Example: full_joystick_state.asm

    Demonstration of full coverage of the joystick controls.

    Note that the same down/up/pressed/not_pressed/released/not_released
    tests will work in exactly the same way for keys on the keyboard.

    Just run it and see all of the tests being performed for each joystick
    direction and the fire button.

    There are 9 lines in this code to deal with the joystick completely:
    - initialise_joystick_state
    - update_joystick2_state
    - if_key_down
    - if_key_up
    - if_key_pressed
    - if_key_not_pressed
    - if_key_released
    - if_key_not_released
    - create_joyState

*/

#import "hla_controller.inc"

// Text output routines
    .label CHROUT = $ffd2       // Kernal routine - output character (a=char)
    .label PLOT   = $fff0       // Kernel routine - position the cursor (x=row, y=col)
    .label STROUT = $ab1e       // Kernal routine - output a zero-terminated string (a=low,y=high)


    BasicUpstart2(start)

start:
    cls()
    
    initialize_joystick_state(joyState)
    
    do
        update_joystick2_state(joyState)

        setPos(0, 1)
        show_key_state(txt_joy_up,    joyState.up)
        show_key_state(txt_joy_down,  joyState.down)
        show_key_state(txt_joy_left,  joyState.left)
        show_key_state(txt_joy_right, joyState.right)
        show_key_state(txt_joy_fire,  joyState.fire)
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

// Joystick control
txt_joy_up:         ztext("   up ")
txt_joy_down:       ztext(" down ")
txt_joy_left:       ztext(" left ")
txt_joy_right:      ztext("right ")
txt_joy_fire:       ztext(" fire ")

// Each keystate state (!?)
txt_down:           ztext(" d ")
txt_up:             ztext(" u ")
txt_pressed:        ztext(" p  ")
txt_not_pressed:    ztext(" np ")
txt_released:       ztext(" r  ")
txt_not_released:   ztext(" nr ")
txt_blank:          ztext("    ")

txt_lineEnd:        ztext(@"\r")

joyState:
    create_joyState()

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
