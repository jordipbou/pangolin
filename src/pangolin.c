#include<stdio.h>
#include<string.h>
#include"pangolin.h"

int main() {
	char s[255];
	char b[255];
	X* x = init();

	do {
		printf("IN: ");
		fgets(s, 255, stdin);
		PUSHS(x, s, strlen(s));
		eval(x);
		if (x->err) { printf("ERROR: %ld\n", x->err); x->err = 0; }
		b[0] = 0; printf("%s", dump_stack(b, x, 1));
	} while (1);
}
