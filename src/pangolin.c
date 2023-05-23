#include<stdio.h>
#include<string.h>
#include"pangolin.h"

/*
C repl(X* x) {
	char buf[255];
	C i;
	char k;

	do {
		printf("IN: ");
		for (i = 0; i < 255; i++) { buf[i] = 0; }
		fgets(buf, 255, stdin);
		x->ip = buf;
		i = inner(x);
		if (!x->trace && x->sp != 0) { 
			buf[0] = 0; 
			sprintf(buf, "%s", dump_stack(buf, x, 1));
			printf("%s", buf);
		}
		if (i == ERR_EXIT) { return ERR_EXIT; }
		if (i != ERR_OK) { 
			buf[0] = 0; 
			printf("ERROR: %ld\n", i);
		}
	} while(1);
}

int main() {
	X* x = init();

	repl(x);
}
*/

int main() {
}
