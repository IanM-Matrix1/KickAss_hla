# KickAss_hla
Some structured statements for KickAssembler

I prefer structure in my code. KickAssembler doesn't seem to have anything like this, so I added it.

The commands provided do some simple checks to ensure that proper structure is maintained, but I may have missed something somewhere.

In addition, I made the choice to ensure that only a single instruction may be inserted into the instruction stream by any one of the keywords provided. I didn't want to add any hidden bloat to the code.

## Structures current available

### if ... [else] ... endif

The opening 'if' statement can be one of the following:
- if_eq
- if_zero
- if_ne
- if_not_zero
- if_plus
- if_minus
- if_c_set
- if_c_clr
- if_v_set
- if_v_clr

The 'else' part of the structure is optional.
The 'endif' must be present.

The 'if' will insert a relative branch instruction, branching to the 'else' if it's present, or to the 'endif' otherwise.
The 'else', if present, will insert a JMP instruction to the 'endif'
The 'endif' will not result in any new instruction being inserted.

### do ... [exit_loop|while] ... loop

The opening 'do' statement must be present.

Within the structure, then can be one or more of the 'exit_loop' or 'while' instructions.

The 'exit_loop' is an unconditional statement and will cause the loop to terminate immediately when reached.

The 'while' statement will cause the loop to terminate immediately if the condition it is checking is false, and can be one of the following:
- while_eq
- while_zero
- while_ne
- while_not_zero
- while_plus
- while_minus
- while_c_set
- while_c_clr
- while_v_set
- while_v_clr

The 'loop' statement marks the end of the loop and can be one of the following:
- loop
- loop_if_eq
- loop_if_zero
- loop_if_ne
- loop_if_not_zero
- loop_if_plus
- loop_if_minus
- loop_if_c_set
- loop_if_c_clr
- loop_if_v_set
- loop_if_v_clr

'loop' on it's own will unconditionally jump back to the previous 'do', while the loop with a condition will branch back to the 'do' only if the condition is true.

The 'do' will not result in any new instruction being inserted.
The 'exit_loop' will insert a JMP instruction pointing to just after the terminating 'loop' statement.
The 'while' will insert a relative branch instruction pointing to just after the terminating 'loop' statement.
The unconditional 'loop' statement will insert a JMP instruction to the location of the 'do' statement.
The conditional 'loop' statements will insert a relative branch back to the location of the 'do' statement.

## Examples

### Simple 'if'

    ...
    <operation that sets the zero flag or not>
    if_eq
        jsr do_something
    endif
    ...

### Infinite loop

    ...
    do
        jsr update_game
    loop
    ...

### A simple conditional loop

On the C64, this code will loop until raster line 255 ($ff) is reached. Note that although the code itself is less compact than the equivalent plain assembly, the structure makes the code a little more readable.

    ...
    lda #$ff
    do
        cmp $d012
    loop_if_ne
    ...

## Implementation notes

KickAss has a built-in phobia against overwriting memory locations it has already assembled bytes to. Even if you set the options for KickAss to allow overwriting, it will issue warnings.

The mechanism I used to avoid this restriction is to record the current PC location, skip the PC forward the appropriate amount for the expected instruction, and then to continue with assembly.

Later statements will then use that recorded location to later reset the PC back to that point, assemble the instruction with the appropriate jump address, then restore the PC to continue assembling.

Not all of the jump locations are known when a structured instruction is reached - for example the 'if' statements don't know the location of the 'else' or 'endif' statements are, or even which ones are present.

It also doesn't seem to be possible to generate a label name dynamically.

So the work-around is to back-patch on following structured instructions. For example, if an 'else' is encountered after an 'if', it is the responsibility of the 'else' to patch in the assembly code for the 'if' statement. The 'endif' is then responsible for patching in the assembly code for the 'else',, if there is one, or for the 'if' otherwise.

One thing that I tried to do is to include an actual label in each of the routines anyway as a debugging aid. KickAss very kindly makes each of these labels unique by appending a numeric value and will provide these to the VICE monitor if you set KickASS options appropriately.

## Credits

Inspired by Garth Wilson - http://wilsonminesco.com
