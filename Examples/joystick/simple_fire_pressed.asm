/*
    Example: simple_fire_pressed.asm

    Demonstration of the port2 joystick fire button pressed event.

    The joystick fire button is down as long as you hold it, however the fire
    button is only pressed at the point it switches from up to down.

    When the fire button is pressed, the colour of the screen will fade 1 step
    from white, through the greys, to black, and then back again.

    You will need to release and the press the joystick again to cycle through
    all of the colours. Or activate your auto-fire and hold the fire button
    down :)

    There are just 3 lines in this code to deal with the joystick completely:
    - update_joystick2_state
    - if_key_pressed
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
        if_key_pressed(joystick_state.fire)

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
    .byte  1
    .byte 15
    .byte 12
    .byte 11
    .byte  0
    .byte 11
    .byte 12
    .byte 15
colour_table_end:
