/*
    Example: simple_fire_down.asm

    Demonstration of the port2 joystick fire button.

    While the fire button is down, the colour of the screen will fade from
    white, through the greys, to black, and then back again.

    There are just 3 lines in this code to deal with the joystick completely:
    - update_joystick2_state
    - if_key_down
    - create_joyState

*/

#import "hla_controller.inc"

    BasicUpstart2(start)

start:
    // Set the initial border colour
    ldx #0
    stx colour_table_position

    ldy colour_table,x
    sty $d020
    sty $d021

    do
        jsr wait_for_raster

        // This is a small macro, so it's relatively safe to just embed it here
        update_joystick2_state(joystick_state)

        // If the fire button is down, update the foreground and border colour.
        if_key_down(joystick_state.fire)

            // Increment the table position, and if it reaches the end of the
            // table, reset back to 0.
            ldx colour_table_position
            inx
            cpx #(colour_table_end - colour_table)
            if_eq
                ldx #0
            endif
            stx colour_table_position

            // Now update the screen colour from the table.
            ldy colour_table,x
            sty $d020
            sty $d021
        endif

    loop

wait_for_raster:
    lda #$ff
    do
        cmp $d012
    loop_if_ne
    rts

joystick_state:
    create_joyState()

colour_table_position:
    .byte 0

colour_table:
    .byte  1,  1,  1,  1
    .byte 15, 15, 15, 15
    .byte 12, 12, 12, 12
    .byte 11, 11, 11, 11
    .byte  0,  0,  0,  0
    .byte  0,  0,  0,  0    // Stay a little longer at black
    .byte 11, 11, 11, 11
    .byte 12, 12, 12, 12
    .byte 15, 15, 15, 15
    .byte  1,  1,  1,  1    // Stay a little longer at white
colour_table_end:
