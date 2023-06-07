# Pangolin

Embeddable C89 virtual machine.

Features:
  * Easily write/read bytecode with printable ASCII characters
		* Allows easy scaffolding of new languages or direct use of Pangolin as a
		  programming language for simple tasks.
  * Header only C89 library, easy integrated in C programs
  * Two typed stacks
  * No garbage collector, memory is managed thru the data stack
  * Array operations
  * Quotations
  * Easily extensible with C functions

Work in progress.

# Bytecode specification

		q exit

    k key ( <gets char> -- c )
    e emit ( n -- <prints char> )
		_ print object ( n -- <prints object representation> )
    r read ( <k until enter> -- s )
    p print ( s -- <prints string> )
    
    + addition ( n2 n1 -- <n2+n1> )
    - substraction ( n2 n1 -- <n2-n1> )
    * multiplication ( n2 n1 -- <n2*n1> )
    / division ( n2 n1 -- <n2/n1> )
    % modulo ( n2 n1 -- <n2%n1> )

    < less than ( n2 n1 -- <n2 < n1> )
    = equal ( n2 n1 -- <n2 = n1> )
    > greater than ( n2 n1 -- <n2 > n1> )

    & and ( n2 n1 -- <n2 and n1> )
    | or ( n2 n1 -- <n2 or n1> )
    ! not ( n -- <not n> )

    \ drop ( i -- )
    s swap ( i2 i1 -- i1 i2 )
    d dup ( i -- i i )
    o over ( i2 i1 -- i2 i1 i2 )
    @ rot ( i3 i2 i1 -- i1 i3 i2 )

    i exec ( [P] -- P )
    x      ( [P] -- [P] P )

    ? ifelse ( b [P] [Q] -- <P if b else Q> )
    
    t times ( n [P] -- <do P n times> )
    w while ( [C] [P] -- <while C do P> )

    l linear recursion ( [C] [P] [R1] [R2] -- <>)
    b binary recursion ( [C] [P] [R1] [R2] -- <> )

    # iota ( n -- <array from 1 to n> )
		$ shape ( ? )
		m map ( [A] [P] -- <P applied to each item in A> )
		z zip ( [A] [B] [P] -- <P applied to each A and B pairs> )
		{ left fold ( [A] [P] -- n )
		.{ left fold with initial argument
		} right fold ( [A] [P] -- n )
		.} right fold with initial argument
		f filter ( [A] [P] -- <array with only the items that pass predicate P> )
    ; append ( a b -- [a... b...] )
    ( drop ( [A] n -- <[A] without the first n items> )
    ) take ( [A] n -- <first n items of A> )
 
    'c character literal ( -- c )
    nnn number literal ( -- nnn )
    [P] array literal ( -- [P] )
    
# External representation
## Numbers
### Integers
### Floating point
## Arrays

## Continuation

		  <main stack> : <call stack>   << <text input>

			             : << 2 3 + dup *
                   : 2 << 3 + dup *
								 2 : 3 << + dup *
		           2 3 : + << dup *
							   5 : d << *
							 5 5 : *
							  25 :

    
