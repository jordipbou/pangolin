# Pangolin

Embeddable C89 virtual machine.

Features:
  * Easily write/read bytecode with printable ASCII characters
  * Header only library, easy integrated in C programs
  * Two typed stacks
  * No garbage collector, memory is managed thru the data stack
  * Array operations
  * Quotations
  * Easily extensible with C functions

Work in progress.

# Bytecode specification

    k key ( <gets char> -- c )
    e emit ( n -- <prints char> )
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

    ? ifelse ( b [P] [Q] -- <P if b else Q> )
    
    t times ( n [P] -- <do P n times> )
    w while ( [C] [P] -- <while C do P> )

    i exec ( [P] -- P )
    x      ( [P] -- [P] P )

    l linear recursion ( [C] [P] [R1] [R2] -- <>)
    b binary recursion ( [C] [P] [R1] [R2] -- <> )
  
    'c char literal ( -- c )
    #nnn number literal ( -- nnn )
    "s" string literal ( -- s )
    [P] quotation literal ( -- [P] )
    
# External representation
## Numbers
### Integers
### Floating point
## Arrays

    "test string"

## Continuation

		  <main stack> : <call stack>   << <text input>

			             : << 2 3 + dup *
                   : 2 << 3 + dup *
								 2 : 3 << + dup *
		           2 3 : + << dup *
							   5 : d << *
							 5 5 : *
							  25 :

    