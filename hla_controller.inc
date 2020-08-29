#importonce
.filenamespace hla_keypress

#import "hla.inc"

///////////////////////////////
// Internal keyboard support //
///////////////////////////////

.struct keyboardInfo_s { row, col, rowMask, colMask }

.function keyboardInfo(row, col) {
    .if (row < 0 || row > 7) .error"Row value is out of range (0-7)"
    .if (col < 0 || col > 7) .error "Column value is out of range (0-7)"
    .return keyboardInfo_s( row, col, 1 << row, 1 << col )
}

.var keyboard = Hashtable()

// Put all of the C64 keyboard definitions into the keyboard hashtable
.eval keyboard.put("DEL",           keyboardInfo(0, 0))
.eval keyboard.put("RETURN",        keyboardInfo(0, 1))
.eval keyboard.put("LEFT",          keyboardInfo(0, 2))
.eval keyboard.put("F7",            keyboardInfo(0, 3))
.eval keyboard.put("F1",            keyboardInfo(0, 4))
.eval keyboard.put("F3",            keyboardInfo(0, 5))
.eval keyboard.put("F5",            keyboardInfo(0, 6))
.eval keyboard.put("UP",            keyboardInfo(0, 7))

.eval keyboard.put("3",             keyboardInfo(1, 0))
.eval keyboard.put("W",             keyboardInfo(1, 1))
.eval keyboard.put("A",             keyboardInfo(1, 2))
.eval keyboard.put("4",             keyboardInfo(1, 3))
.eval keyboard.put("Z",             keyboardInfo(1, 4))
.eval keyboard.put("S",             keyboardInfo(1, 5))
.eval keyboard.put("E",             keyboardInfo(1, 6))
.eval keyboard.put("LEFT SHIFT",    keyboardInfo(1, 7))

.eval keyboard.put("5",             keyboardInfo(2, 0))
.eval keyboard.put("R",             keyboardInfo(2, 1))
.eval keyboard.put("D",             keyboardInfo(2, 2))
.eval keyboard.put("6",             keyboardInfo(2, 3))
.eval keyboard.put("C",             keyboardInfo(2, 4))
.eval keyboard.put("F",             keyboardInfo(2, 5))
.eval keyboard.put("T",             keyboardInfo(2, 6))
.eval keyboard.put("X",             keyboardInfo(2, 7))

.eval keyboard.put("7",             keyboardInfo(3, 0))
.eval keyboard.put("Y",             keyboardInfo(3, 1))
.eval keyboard.put("G",             keyboardInfo(3, 2))
.eval keyboard.put("8",             keyboardInfo(3, 3))
.eval keyboard.put("B",             keyboardInfo(3, 4))
.eval keyboard.put("H",             keyboardInfo(3, 5))
.eval keyboard.put("U",             keyboardInfo(3, 6))
.eval keyboard.put("V",             keyboardInfo(3, 7))

.eval keyboard.put("9",             keyboardInfo(4, 0))
.eval keyboard.put("I",             keyboardInfo(4, 1))
.eval keyboard.put("J",             keyboardInfo(4, 2))
.eval keyboard.put("0",             keyboardInfo(4, 3))
.eval keyboard.put("M",             keyboardInfo(4, 4))
.eval keyboard.put("K",             keyboardInfo(4, 5))
.eval keyboard.put("O",             keyboardInfo(4, 6))
.eval keyboard.put("N",             keyboardInfo(4, 7))

.eval keyboard.put("+",             keyboardInfo(5, 0))
.eval keyboard.put("P",             keyboardInfo(5, 1))
.eval keyboard.put("L",             keyboardInfo(5, 2))
.eval keyboard.put("-",             keyboardInfo(5, 3))
.eval keyboard.put(".",             keyboardInfo(5, 4))
.eval keyboard.put(":",             keyboardInfo(5, 5))
.eval keyboard.put("@",             keyboardInfo(5, 6))
.eval keyboard.put(",",             keyboardInfo(5, 7))

.eval keyboard.put("£",             keyboardInfo(6, 0))
.eval keyboard.put("*",             keyboardInfo(6, 1))
.eval keyboard.put(";",             keyboardInfo(6, 2))
.eval keyboard.put("HOME",          keyboardInfo(6, 3))
.eval keyboard.put("RIGHT SHIFT",   keyboardInfo(6, 4))
.eval keyboard.put("=",             keyboardInfo(6, 5))
.eval keyboard.put("^",             keyboardInfo(6, 6))
.eval keyboard.put("/",             keyboardInfo(6, 7))

.eval keyboard.put("1",             keyboardInfo(7, 0))
.eval keyboard.put("ARROW",         keyboardInfo(7, 1))
.eval keyboard.put("CTRL",          keyboardInfo(7, 2))
.eval keyboard.put("2",             keyboardInfo(7, 3))
.eval keyboard.put("SPACE",         keyboardInfo(7, 4))
.eval keyboard.put("CBM",           keyboardInfo(7, 5))
.eval keyboard.put("Q",             keyboardInfo(7, 6))
.eval keyboard.put("RUNSTOP",       keyboardInfo(7, 7))

// Additional aliases
.eval keyboard.put("ESCAPE",        keyboardInfo(7, 7))
.eval keyboard.put("ENTER",         keyboardInfo(0, 1))
.eval keyboard.put("LSHIFT",        keyboardInfo(1, 7))
.eval keyboard.put("RSHIFT",        keyboardInfo(6, 4))
.eval keyboard.put("UP ARROW",      keyboardInfo(6, 6))
.eval keyboard.put("LEFT ARROW",    keyboardInfo(7, 1))
.eval keyboard.put(" ",             keyboardInfo(7, 4))

/////////////////////////////
// Public macros/functions //
/////////////////////////////

.function @keyInfo(keyName) {
    .errorif !keyboard.containsKey(keyName), "No definition found for key '" + keyName + "'"
    .return keyboard.get(keyName)
}

.macro @create_keyMatrix() {
    .fill 8, $ff
}

.macro @update_keyboard_matrix(keyMatrix) {
    // TODO: Validate that keyMatrix is large enough

    ldy #7
    lda #%01111111              // Initial row mask
    sec                         // Ensure carry is set to fill hi-bit
    do
        sta $dc00               // Apply row mask
        tax                     // Save the mask
        lda $dc01               // Load the keyboard column state
        sta keyMatrix, y        // Store it in the key matrix
        txa                     // Restore the row mask
        ror                     // Rotate the row mask for the next row
        dey                     // Decrement the row counter
    loop_if_plus
}

.macro @initialize_keystate_list(keyStateData) {
    .var listSize = keyStateData.end - keyStateData

    ldx #listSize-1
    lda #$ff
    do
        sta keyStateData, x
        dex
    loop_if_plus
}

.macro @update_keystate_list_from_matrix(keyStateList, keyStateData, keyMatrix) {
    .var listSize = keyStateList.hi - keyStateList.lo

    // Loop through the keyCheckList list backwards
    ldx #listSize-1
    do
        ldy keyStateList.lo, x      // Get the row number to check.
        lda keyStateList.hi, x      // Get the 'column' mask to check.
        
        and keyMatrix, y            // AND with the keymatrix row.
        if_eq                       // Zero means that the key is currently down.
            lsr keyStateData,x      // shift a 0 into the high bit
        else
            sec
            ror keyStateData,x      // shift a 1 into the high bit
        endif

        dex
    loop_if_plus
}

.macro @initialize_keystate(keyState) {
    lda #$ff
    sta keyState
}

.macro @update_keystate(keyState, keyName)
{
    lda #(~keyboard.get(keyName).rowMask)
    sta $dc00
    lda $dc01
    and #keyboard.get(keyName).colMask
    if_eq
        lsr keyState
    else
        sec
        ror keyState
    endif
}

.macro @update_keystate_from_matrix(keyState, keyName, keyMatrix)
{
    ldy #keyboard.get(keyName).row
    lda keyMatrix,y
    and #keyboard.get(keyName).colMask
    if_eq
        lsr keyState
    else
        sec
        ror keyState
    endif
}

.macro @create_joyState() {
    up:     .byte $ff
    down:   .byte $ff
    left:   .byte $ff
    right:  .byte $ff
    fire:   .byte $ff
}

.macro update_joystick_state_from_port(joyState, port) {
    lda #$ff
    sta port
    lda port
    lsr
    ror joyState.up
    lsr
    ror joyState.down
    lsr
    ror joyState.left
    lsr
    ror joyState.right
    lsr
    ror joyState.fire
}

.macro @initialize_joystick_state(joyState) {
    lda #$ff
    sta joyState.up
    sta joyState.down
    sta joyState.left
    sta joyState.right
    sta joyState.fire
}

.macro @update_joystick1_state(joyState) {
    update_joystick_state_from_port(joyState, $dc01)
}

.macro @update_joystick2_state(joyState) {
    update_joystick_state_from_port(joyState, $dc00)
}

.macro @if_key_pressed(keyState) {
_if_key_pressed:
    bit keyState
    add_patch_point("bmi", "if", "if")
    add_patch_point("bvc", "if", "test")
}

.macro @if_key_not_pressed(keyState) {
_if_key_not_pressed:
    bit keyState
    bmi !+
    add_patch_point("bvs", "if", "if")
!:
}

.macro @if_key_released(keyState) {
_if_key_released:
    bit keyState
    add_patch_point("bpl", "if", "if")
    add_patch_point("bvs", "if", "test")
}

.macro @if_key_not_released(keyState) {
_if_key_not_released:
    bit keyState
    bpl !+
    add_patch_point("bvc", "if", "if")
!:
}

.macro @if_key_down(keyState) {
_if_key_up:
    bit keyState
    add_patch_point("bmi", "if", "if")
}

.macro @if_key_up(keyState) {
_if_key_down:
    bit keyState
    add_patch_point("bpl", "if", "if")
}

.macro @while_key_not_pressed(keyState)
{
_while_key_not_pressed:
    bit keyState
    bmi !+
    add_patch_point("bvs", "loop", "test")
!:
}

.macro @loop_if_key_not_pressed(keyState)
{
    bit keyState
    bmi !+
    bvs _loop_if_key_not_pressed
!:
    terminate_loop("jmp")
_loop_if_key_not_pressed:
}

.macro @loop_if_key_up(keyState) {
    bit keyState
    terminate_loop("bmi")
}

.macro @loop_if_key_down(keyState) {
    bit keyState
    terminate_loop("bpl")
}
