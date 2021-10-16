#importonce
.filenamespace hla

/*
Supported defines:
    HLA_ARITH_COMPARE
        Enables unsigned comparison opcodes and the matching HLA structures.
*/

/////////////////////
// Structured code //
/////////////////////

// Storage and types for the structured statement jump-points
.var hla = List()
.struct jumpPoint_s { type, subtype, jump, addr }


// Information about opcodes for jumps/branches
.struct opcodeInfo_s { size }
.var opcodeInfo = Hashtable()

.eval opcodeInfo.put( "bcc", opcodeInfo_s( asmCommandSize(BCC_REL) ))
.eval opcodeInfo.put( "bcs", opcodeInfo_s( asmCommandSize(BCS_REL) ))
.eval opcodeInfo.put( "beq", opcodeInfo_s( asmCommandSize(BEQ_REL) ))
.eval opcodeInfo.put( "bne", opcodeInfo_s( asmCommandSize(BNE_REL) ))
.eval opcodeInfo.put( "bmi", opcodeInfo_s( asmCommandSize(BMI_REL) ))
.eval opcodeInfo.put( "bpl", opcodeInfo_s( asmCommandSize(BPL_REL) ))
.eval opcodeInfo.put( "bvc", opcodeInfo_s( asmCommandSize(BVC_REL) ))
.eval opcodeInfo.put( "bvs", opcodeInfo_s( asmCommandSize(BVS_REL) ))
.eval opcodeInfo.put( "jmp", opcodeInfo_s( asmCommandSize(JMP_ABS) ))
.eval opcodeInfo.put( "jsr", opcodeInfo_s( asmCommandSize(JSR_ABS) ))


// Helper to aid in constructing and pushing a new entry onto the hla stack
.macro push_hla( type, subtype, jump, addr ) {
    .eval hla.add( jumpPoint_s(type, subtype, jump, addr) )
}


// ====================================
// Add missing stack handling functions
// ====================================
.function pop(stk) {
    .var last = stk.size()-1
    .errorif last < 0, "Invalid scoping reached"

    .var item = stk.get(last)
    .eval stk.remove(last)
    .return item
}

.function top(stk) {
    .var last = stk.size()-1
    .errorif last < 0, "Invalid scoping reached"

    .var item = stk.get(last)
    .return item
}

// Pull from the names stack, until the named type is found.
// Returns a stack of all popped items in reverse order.
.function search_type(inStk, type, subtype) {
    .var stack = List();

    // Pull from the HLA stack until we get the type req'd - save everything we get
    .var top = pop(inStk)
    .while (top.type != type || top.subtype != subtype) {
        .eval stack.add(top)

        .errorif (inStk.size() == 0), "Unable to find " + type + "/" + subtype
        
        .eval top = pop(inStk)
    }
    .eval stack.add(top)

    .return stack
}


// ============================
// Placement of jump statements
// ============================

// Place the named jump statement at the current pc
.macro jump_stmt(jump, target) {
    // Save the current pc
    .var savePc = *
    
    // Attempt to place the instruction to jump to the target
    .if (jump == "bcc")   bcc target
    .if (jump == "bcs")   bcs target
    .if (jump == "beq")   beq target
    .if (jump == "bne")   bne target
    .if (jump == "bmi")   bmi target
    .if (jump == "bpl")   bpl target
    .if (jump == "bvc")   bvc target
    .if (jump == "bvs")   bvs target
    .if (jump == "jmp")   jmp target
    .if (jump == "jsr")   jsr target

#if HLA_ARITH_COMPARE
    // Support the 4 additional arithmetic unsigned comparison branches
    .if (jump == "blt")   blt target
    .if (jump == "bgt")   bgt target
    .if (jump == "ble")   ble target
    .if (jump == "bge")   bge target
#endif

    // Validate that the jump statement was handled as the pc will have advanced from
    // the saved position.
    .errorif savePc == *, "Unsupported jump/branch statement: " + jump
}

// Place the named jump statement at the requested placement point.
// Will fail if it overlaps an existing statement (ie if you didn't use reserve_pc or equivalent).
.macro place_jump_stmt(placement, jump, target)
{
    .var savePc = *
    * = placement
    jump_stmt( jump, target )
    * = savePc
}


// Reserve space in the instruction stream for a later jump statement
// to be inserted - unfortunately, there doesn't appear to be a way
// for KickAss to allow overwrites, so a gap is left instead.
.macro reserve_pc(gap)
{
    * = * + gap
}

.macro @add_patch_point(jump, type, subtype) {
    .errorif !opcodeInfo.containsKey(jump), "Unsupported/unknown opcode: " + jump

    push_hla( type, subtype, jump, * )
    .var info = opcodeInfo.get(jump)
    reserve_pc( info.size )
}

.macro @verify_structure() {
    .if (hla.size() > 0) {
        .error "There's something left over on the HLA stack:"
        .var last = hla.size()-1
        .for (var i = 0; i < last; i++) {
            .print "- " + hla.get(i).type + "/" + hla.get(i).subtype
        }
    }
}




// ======================
// IF_xx...[ELSE]...ENDIF
// ======================
.pseudocommand @if_eq {
_if_eq:
    add_patch_point("bne", "if", "if")
}

.pseudocommand @if_zero {
_if_zero:
    add_patch_point("bne", "if", "if")
}

.pseudocommand @if_ne {
_if_ne:
    add_patch_point("beq", "if", "if")
}

.pseudocommand @if_not_zero {
_if_not_zero:
    add_patch_point("beq", "if", "if")
}

.pseudocommand @if_plus {
_if_plus:
    add_patch_point("bmi", "if", "if")
}

.pseudocommand @if_minus {
_if_minus:
    add_patch_point("bpl", "if", "if")
}

.pseudocommand @if_c_set {
_if_c_set:
    add_patch_point("bcc", "if", "if")
}

.pseudocommand @if_c_clr {
_if_c_clr:
    add_patch_point("bcs", "if", "if")
}

.pseudocommand @if_v_set {
_if_v_set:
    add_patch_point("bvc", "if", "if")
}

.pseudocommand @if_v_clr {
_if_v_clr:
    add_patch_point("bvs", "if", "if")
}

#if HLA_ARITH_COMPARE
// Support the 4 additional arithmetic unsigned comparison branches
.pseudocommand @if_lt {
_if_lt:
    add_patch_point("bge", "if", "if")
}

.pseudocommand @if_gt {
_if_gt:
    add_patch_point("ble", "if", "if")
}

.pseudocommand @if_le {
_if_le:
    add_patch_point("bgt", "if", "if")
}

.pseudocommand @if_ge {
_if_ge:
    add_patch_point("blt", "if", "if")
}
#endif

.pseudocommand @else {
    .var stack = search_type(hla, "if", "if")

    // Need a place for the endif to place a jmp for the end of the 'if'
    add_patch_point("jmp", "if", "else")

_else:
    .while (stack.size() > 0) {
        .var top = pop(stack)
        .if (top.type == "if") {
            .errorif top.subtype == "else", "Cannot have multiple else statements for a single if"
            .if (top.subtype == "if" || top.subtype == "test") {
                place_jump_stmt(top.get("addr"), top.get("jump"), *)
            } else {
                .error "Unknown statement subtype: " + top.subtype
            }
        } else {    // Don't deal with this here
            .eval hla.add(top)
        }
    }
}

.pseudocommand @endif {
    .var stack = List()

    .var top = pop(hla)
    .while (top.type != "if" || (top.subtype != "if" && top.subtype != "else")) {
        .eval stack.add(top)
        .eval top = pop(hla)
    }
    .eval stack.add(top)

_endif:
    .while (stack.size() > 0) {
        .eval top = pop(stack)
        .if (top.type == "if") {
            .if (top.subtype == "if" || top.subtype == "test" || top.subtype == "else") {
                place_jump_stmt(top.get("addr"), top.get("jump"), *)
            } else {
                .error "Unknown statement subtype: " + top.subtype
            }
        } else {    // Don't deal with this here
            .eval hla.add(top)
        }
    }
}


// =====================================
// DO...[WHILE_xx|EXIT_LOOP]...LOOP[_xx]
// =====================================
.pseudocommand @do {
_do:
    push_hla( "loop", "do", "", *)
    // DO doesn't have a jump command. The entry is just used to record a target for another
    // jump from somewhere else, ie from the tail end of the loop.
}

.pseudocommand @exit_loop {
_exit_loop:
    add_patch_point("jmp", "loop", "test")
}

.pseudocommand @while_eq {
_while_eq:
    add_patch_point("bne", "loop", "test")
}

.pseudocommand @while_zero {
_while_zero:
    add_patch_point("bne", "loop", "test")
}

.pseudocommand @while_ne {
_while_ne:
    add_patch_point("beq", "loop", "test")
}

.pseudocommand @while_not_zero {
_while_not_zero:
    add_patch_point("beq", "loop", "test")
}

.pseudocommand @while_plus {
_while_plus:
    add_patch_point("bmi", "loop", "test")
}

.pseudocommand @while_minus {
_while_minus:
    add_patch_point("bpl", "loop", "test")
}

.pseudocommand @while_c_set {
_while_c_set:
    add_patch_point("bcc", "loop", "test")
}

.pseudocommand @while_c_clr {
_while_c_clr:
    add_patch_point("bcs", "loop", "test")
}

.pseudocommand @while_v_set {
_while_v_set:
    add_patch_point("bvc", "loop", "test")
}

.pseudocommand @while_v_clr {
_while_v_clr:
    add_patch_point("bvs", "loop", "test")
}

#if HLA_ARITH_COMPARE
// Support the 4 additional arithmetic comparison branches
.pseudocommand @while_lt {
_while_lt:
    add_patch_point("bge", "loop", "test")
}

.pseudocommand @while_gt {
_while_gt:
    add_patch_point("ble", "loop", "test")
}

.pseudocommand @while_le {
_while_le:
    add_patch_point("bgt", "loop", "test")
}

.pseudocommand @while_ge {
_while_ge:
    add_patch_point("blt", "loop", "test")
}
#endif

// Helper macro to provide all the machinery to link back to the top
// of the loop, and to patch all of the while/exit loop statements
// to after the loop.
.macro @terminate_loop(jump) {
    .var stack = search_type(hla, "loop", "do")
    .var top = pop(stack)

    // This is the end of the loop, so jump back to the 'DO' location.
    jump_stmt( jump, top.get("addr") )

    // Now deal with everything else we got from the stack
    .while (stack.size() > 0) {
        .eval top = pop(stack)

        .if (top.type == "loop") {
            // Can deal with it, so do so, by placing a jump to the current pc
            place_jump_stmt(top.get("addr"), top.get("jump"), *)
        } else {
            .errorif (top.type == "else"), "else without endif"
            .errorif (top.type == "if"),   "if without endif"
            .errorif (top.type != "loop"), "Internal error - I got " + top.subtype
        }
    }
}

.pseudocommand @loop {
    terminate_loop("jmp")
_loop:
}

.pseudocommand @loop_if_eq {
    terminate_loop("beq")
_loop_if_eq:
}

.pseudocommand @loop_if_zero {
    terminate_loop("beq")
_loop_if_zero:
}

.pseudocommand @loop_if_ne {
    terminate_loop("bne")
_loop_if_ne:
}

.pseudocommand @loop_if_not_zero {
    terminate_loop("bne")
_loop_if_not_zero:
}

.pseudocommand @loop_if_plus {
    terminate_loop("bpl")
_loop_if_plus:
}

.pseudocommand @loop_if_minus {
    terminate_loop("bmi")
_loop_if_minus:
}

.pseudocommand @loop_if_c_set {
    terminate_loop("bcs")
_loop_if_c_set:
}

.pseudocommand @loop_if_c_clr {
    terminate_loop("bcc")
_loop_if_c_clr:
}

.pseudocommand @loop_if_v_set {
    terminate_loop("bvs")
_loop_if_v_set:
}

.pseudocommand @loop_if_v_clr {
    terminate_loop("bvc")
_loop_if_v_clr:
}

#if HLA_ARITH_COMPARE
// Support the 4 additional arithmetic unsigned comparison branches
.pseudocommand @loop_if_lt {
    terminate_loop("blt")
_loop_if_lt:
}

.pseudocommand @loop_if_gt {
    terminate_loop("bgt")
_loop_if_gt:
}

.pseudocommand @loop_if_le {
    terminate_loop("ble")
_loop_if_le:
}

.pseudocommand @loop_if_ge {
    terminate_loop("bge")
_loop_if_ge:
}
#endif


////////////////////////
// New pseudo opcodes //
////////////////////////

#if HLA_ARITH_COMPARE
// The 4 additional arithmetic unsigned comparison branches
// NOTE: Broken my own rules here - some of these branches are 2 instructions.

.eval opcodeInfo.put( "blt", opcodeInfo_s( asmCommandSize(BCC_REL) ))
.eval opcodeInfo.put( "bgt", opcodeInfo_s( asmCommandSize(BEQ_REL) + asmCommandSize(BCS_REL) ))
.eval opcodeInfo.put( "ble", opcodeInfo_s( asmCommandSize(BEQ_REL) + asmCommandSize(BCC_REL) ))
.eval opcodeInfo.put( "bge", opcodeInfo_s( asmCommandSize(BCS_REL) ))

.pseudocommand @blt target {
    bcc target
}

.pseudocommand @bgt target {
    beq !+
    bcs target
!:
}

.pseudocommand @ble target {
    bcc target
    beq target
}

.pseudocommand @bge target {
    bcs target
}
#endif
