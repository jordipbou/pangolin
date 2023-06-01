# Pangolin

Embeddable C89 virtual machine.

Features:
  * ASCII based, easy writable/readable bytecode
  * Header only, easy integrated in C/C++ programs
  * Two typed stacks
  * No garbage collector, memory is managed implicitly thru the stack
  * Array operations
  * Quotations
  * Easily extensible with C functions

Work in progress.

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

    