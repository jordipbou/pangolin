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
			sprintf(buf, "%s", dump_stack_stack(buf, x, 1));
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

/*
void fib(X* x) {
	B buf[255];
	DUP(x);
	PUSHI(x, NUMBER, 1);
	GT(x);
	if (POPI(x)) {
		PUSHI(x, NUMBER, 1);
		SUB(x);
		DUP(x);
		PUSHI(x, NUMBER, 1);
		SUB(x);
		fib(x);
		SWAP(x);
		fib(x);
		ADD(x);
	}
}

int main() {
	X* x = init();

	PUSHI(x, NUMBER, 36);
	fib(x);

	printf("OUT: %ld\n", TS(x).v.i);
}
*/

int main() {
	B buf[255];
	X* x = init();
	x->tr = 1;

	/* x->ip = "#36[d11+<][][1-d1-][+]b"; */ /* Fibonacci sequence */
	/* x->ip = "11111++++[d0=][1+][d1-][*]l"; */ /* Factorial */ 
	/* x->ip = "1[11+][111++]?"; */ /* ifthen */
	x->ip = "\"test string\"";
	/* x->ip = "#5[11+]t"; */
	/* x->ip = "#5[d1>][1-]w"; */
	/* x->ip = "[11+]i111++"; */
	/* x->ip = "#36.00"; */
	/* x->ip = "#5[d0=][1+][d1-][*]l"; */
	/* x->ip = "[111++]i"; */
	P_inner(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x, 1);
	printf("--- Data stack\n%s\n", buf);
}
