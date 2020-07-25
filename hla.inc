#importonce
.filenamespace hla

// Structured code

// Storage and types for the structured statement jump-points
.var hla = List()
.struct jumpPoint_s { type, jump, addr }


// Information about opcodes for jumps/branches
.struct opcodeInfo_s { size, opcode }
.var opcodeInfo = Hashtable()

.eval opcodeInfo.put( "bcc", opcodeInfo_s( 2, BCC_REL ))
.eval opcodeInfo.put( "bcs", opcodeInfo_s( 2, BCS_REL ))
.eval opcodeInfo.put( "beq", opcodeInfo_s( 2, BEQ_REL ))
.eval opcodeInfo.put( "bne", opcodeInfo_s( 2, BNE_REL ))
.eval opcodeInfo.put( "bmi", opcodeInfo_s( 2, BMI_REL ))
.eval opcodeInfo.put( "bpl", opcodeInfo_s( 2, BPL_REL ))
.eval opcodeInfo.put( "bvc", opcodeInfo_s( 2, BVC_REL ))
.eval opcodeInfo.put( "bvs", opcodeInfo_s( 2, BVS_REL ))
.eval opcodeInfo.put( "jmp", opcodeInfo_s( 3, JMP_ABS ))
.eval opcodeInfo.put( "jsr", opcodeInfo_s( 3, JSR_ABS ))
// .eval opcodeInfo.put( "bra", opcodeInfo_s( 2, BRA_REL ))


// Helper to aid in constructing and pushing a new entry onto the hla stack
.macro push_hla( type, jump, addr ) {
    .eval hla.add( jumpPoint_s(type, jump, addr) )
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
.function search(inStk, type) {
    .var stack = List();

    // Pull from the HLA stack until we get the type req'd - save everything we get
    .var top = pop(inStk)
    .while (top.type != type) {
        .eval stack.add(top)

        .errorif (inStk.size() == 0), "Unable to find " + type
        
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
    // .if (type == "bra")   bra target

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

.macro add_patch_point(jump, type) {
    .errorif !opcodeInfo.containsKey(jump), "Unsupported/unknown opcode: " + jump

    push_hla( type, jump, * )
    .var info = opcodeInfo.get(jump)
    reserve_pc( info.size )
}

.macro @verify_structure() {
    .if (hla.size() > 0) {
        .var last = hla.size()-1
        .for (var i = 0; i < last; i++) {
            .print hla.get(i).type
        }
        .error "There's something left over on the HLA stack"
    }
}




// ======================
// IF_xx...[ELSE]...ENDIF
// ======================
.pseudocommand @if_eq {
_if_eq:
    add_patch_point("bne", "if")
}

.pseudocommand @if_zero {
_if_zero:
    add_patch_point("bne", "if")
}

.pseudocommand @if_ne {
_if_ne:
    add_patch_point("beq", "if")
}

.pseudocommand @if_not_zero {
_if_not_zero:
    add_patch_point("beq", "if")
}

.pseudocommand @if_plus {
_if_plus:
    add_patch_point("bmi", "if")
}

.pseudocommand @if_minus {
_if_minus:
    add_patch_point("bpl", "if")
}

.pseudocommand @if_c_set {
_if_c_set:
    add_patch_point("bcc", "if")
}

.pseudocommand @if_c_clr {
_if_c_clr:
    add_patch_point("bcs", "if")
}

.pseudocommand @if_v_set {
_if_v_set:
    add_patch_point("bvc", "if")
}

.pseudocommand @if_v_clr {
_if_v_clr:
    add_patch_point("bvs", "if")
}

.pseudocommand @else {
    .var stack = List()

    .var top = pop(hla)
    .while (top.type == "loop") {
        .eval stack.add(top)
        .eval top = pop(hla)
    }

    // Can't allow an additional else, or an else in a loop when the if is outside
    .errorif top.type != "if", "Expected match of if, but got " + top.type

    .while (stack.size() > 0) {
        .eval hla.add( pop(stack) )
    }

    // Need a place for the endif to place a jmp for the end of the 'if'
    add_patch_point("jmp", "else")

_else:
    // Update the previous if to jump to this point
    place_jump_stmt(top.get("addr"), top.get("jump"), *)
}

.pseudocommand @endif {
    .var stack = List()

    .var top = pop(hla)
    .while (top.type == "loop") {
        .eval stack.add(top)
        .eval top = pop(hla)
    }

    .errorif top.type != "if" && top.type != "else", "Expected match of if/else, but got " + top.type

    .while (stack.size() > 0) {
        .eval hla.add( pop(stack) )
    }

_endif:
    // Update the previous else/if to jump to this point
    place_jump_stmt(top.get("addr"), top.get("jump"), *)
}


// =====================================
// DO...[WHILE_xx|EXIT_LOOP]...LOOP[_xx]
// =====================================
.pseudocommand @do {
_do:
    push_hla( "do", "", *)
    // DO doesn't have a jump command. The entry is just used to record a target for another
    // jump from somewhere else, ie from the tail end of the loop.
}

.pseudocommand @exit_loop {
_exit_loop:
    add_patch_point("jmp", "loop")
}

.pseudocommand @while_eq {
_while_eq:
    add_patch_point("bne", "loop")
}

.pseudocommand @while_zero {
_while_zero:
    add_patch_point("bne", "loop")
}

.pseudocommand @while_ne {
_while_ne:
    add_patch_point("beq", "loop")
}

.pseudocommand @while_not_zero {
_while_not_zero:
    add_patch_point("beq", "loop")
}

.pseudocommand @while_plus {
_while_plus:
    add_patch_point("bmi", "loop")
}

.pseudocommand @while_minus {
_while_minus:
    add_patch_point("bpl", "loop")
}

.pseudocommand @while_c_set {
_while_c_set:
    add_patch_point("bcc", "loop")
}

.pseudocommand @while_c_clr {
_while_c_clr:
    add_patch_point("bcs", "loop")
}

.pseudocommand @while_v_set {
_while_v_set:
    add_patch_point("bvc", "loop")
}

.pseudocommand @while_v_clr {
_while_v_clr:
    add_patch_point("bvs", "loop")
}

// Helper macro to provide all the machinery to link back to the top
// of the loop, and to patch all of the while/exit loop statements
// to after the loop.
.macro loop_helper(jump) {
    .var stack = search(hla, "do")
    .var top = pop(stack)

    // This is the end of the loop, so jump back to the 'DO' location.
    jump_stmt( jump, top.get("addr") )

    // Now deal with everything else we got from the stack
    .while (stack.size() > 0) {
        .eval top = pop(stack)
        .errorif (top.type == "else" || top.type == "if"), top.type + " without endif"
        .errorif (top.type != "loop"), "Internal error - I got " + top.type

        // Can deal with it, so do so, by placing a jump to the current pc
        place_jump_stmt(top.get("addr"), top.get("jump"), *)
    }
}

.pseudocommand @loop {
    loop_helper("jmp")
_loop:
}

.pseudocommand @loop_if_eq {
    loop_helper("beq")
_loop_if_eq:
}

.pseudocommand @loop_if_zero {
    loop_helper("beq")
_loop_if_zero:
}

.pseudocommand @loop_if_ne {
    loop_helper("bne")
_loop_if_ne:
}

.pseudocommand @loop_if_not_zero {
    loop_helper("bne")
_loop_if_not_zero:
}

.pseudocommand @loop_if_plus {
    loop_helper("bpl")
_loop_if_plus:
}

.pseudocommand @loop_if_minus {
    loop_helper("bmi")
_loop_if_minus:
}

.pseudocommand @loop_if_c_set {
    loop_helper("bcs")
_loop_if_c_set:
}

.pseudocommand @loop_if_c_clr {
    loop_helper("bcc")
_loop_if_c_clr:
}

.pseudocommand @loop_if_v_set {
    loop_helper("bvs")
_loop_if_v_set:
}

.pseudocommand @loop_if_v_clr {
    loop_helper("bvc")
_loop_if_v_clr:
}
