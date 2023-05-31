# Pangolin

Concatenative imperative/functional language without garbage collection.

Tries to be like Joy/Factor, while maintaining low level features.

# External representation

## Continuation

		  <main stack> : <call stack>   << <text input>

			             : << 2 3 + dup *
                   : 2 << 3 + dup *
								 2 : 3 << + dup *
		           2 3 : + << dup *
							   5 : d << *
							 5 5 : *
							  25 :

# Data types

# Bytecode

	0      -> Pushes the value 0 to the stack
	1      -> Pushes the value 1 to the stack

	#nnn   -> Parses the number n as defined by strtod or strtol
	"..."  -> Pushes the address after first " as an (unmanaged) byte*array*string type
	[...]  -> Pushes the address after [ as an (unmanaged) byte*array

	x/y/z< -> Move top of the stack to x y or z
	x/y/z> -> Copy from x, y or z to top of the stack
	x/y/z+ -> Increment x, y or z (without copying it)
	x/y/z- -> Decrement x, y or z (without copying it)

# TODO / Notes

- 2023/05/29::i (exec) (and probably b, l, t and w) need a way to remove a managed block from 
              the stack without freeing it, and only when ending execution then it can be freed.
							If not, managed blocks are freed and its contents are not accesible for execution.
							Maybe a solution is to not DROP (free) on POP, just take the object and free when
							needed (from C code, of course).
							Another option is using the return stack as a dispose objects stack, and free
							anything that is not a RETURN address type.
							This last option will allow use of the same stack for S and R, using the top and
							the bottom, and also to implement locals easily on this same stack, using x, y, z
							as ways to call top of rstack, next of rstack and next of next of rstack. They
							will be freed when returning.

- 2023/05/29::I don't really see any advantage of using a STRING type. Even a CHAR type makes
              not much sense now. I just need them for printing purposes, to see on the data
							stack "test string" instead of a number, but I can analyze an I8 array anytime
							I print it and if all values are in ASCII viewable values (31<x<127) print
							both the array values (as maybe its not a string) and also the string at its
							side. Same concept for a char, if the value lies between 32 and 127, print
							the value and the char as its side.
							Types will change then to: INT, FLOAT, I8ARRAY, I16ARRAY, I32ARRAY and I64ARRAY.
							Maybe struct/record type should also be added to manage C structs and more
							advanced types implemented later.

# C API

    SP(x);
    MAX_DEPTH(x);

    PUSH(X* x, I v);
    PUSHF(X* x, F v);
    PUSHS(X* x, B* s, I l);

    I POP(X* x);
    F POPF(X* x);
    B* POPS(X* x);

    O* TO_R(X* x);

    I RP(X* x);
    I RMAX_DEPTH(X* x);
    