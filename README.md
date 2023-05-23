# Pangolin

Concatenative imperative/functional language without garbage collection.

Tries to be like Joy/Factor, while maintaining low level features.

# Representation

		  <main stack> : <call stack>   << <text input>

			             : << 2 3 + dup *
                   : 2 << 3 + dup *
								 2 : 3 << + dup *
		           2 3 : + << dup *
							   5 : d << *
							 5 5 : *
							  25 :
