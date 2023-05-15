#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdlib.h>
#include<stdint.h>

typedef int8_t B;
typedef int32_t H;
typedef intptr_t C;
typedef double F;
typedef struct { H t; H l; union { C c; F f; char* s; } v;  } P;
typedef struct { P s[64]; C d; char* b; C bl, in, err; } X;
typedef void (*FUNC)(X*);

enum { CHAR, INT, STR, FLOAT } T;

#define ERR_UNDEFINED_WORD	-1

X* init() {
	X* x = malloc(sizeof(X));
	x->d = 0;
	x->err = 0;
	x->bl = 0;
	x->in = 0;
	x->b = 0;

	return x;
}

#define PEEK(_x, _i)			(_x->s[_i])
#define PUSHC(_x, _t, _v)	(_x->s[_x->d].t = _t, _x->s[_x->d++].v.c = _v)
#define PUSHS(_x, _s, _l)	(_x->s[_x->d].t = STR, _x->s[_x->d].l = _l, _x->s[_x->d++].v.s = _s)
#define PUSHF(_x, _t, _v)	(_x->s[_x->d].t = _t, _x->s[_x->d++].v.f = _v)
#define POP(_x)						(_x->s[--_x->d])
#define DROP(_x)					(--_x->d)

#define TOS(_x)						(_x->s[_x->d - 1])
#define NOS(_x)						(_x->s[_x->d - 2])
#define NNOS(_x)					(_x->s[_x->d - 3])

#define ADD(_x)						(NOS(_x).t = INT, NOS(_x).v.c = NOS(_x).v.c + TOS(_x).v.c, x->d--)
#define SUB(_x)						(NOS(_x).t = INT, NOS(_x).v.c = NOS(_x).v.c - TOS(_x).v.c, x->d--)
#define MUL(_x)						(NOS(_x).t = INT, NOS(_x).v.c = NOS(_x).v.c * TOS(_x).v.x, x->d--)

char* dump_stack(char* s, X* x, C nl) {
	C i;
	P p;
	for (i = 0; i < x->d; i++) {
		p = PEEK(x, i);	
		switch (p.t) {
		case CHAR: sprintf(s, "%s'%c' ", s, (char)p.v.c); break;
		case INT: sprintf(s, "%s%ld ", s, p.v.c); break;
		case STR: sprintf(s, "%s\"%.*s\" ", s, p.l, p.v.s); break;
		case FLOAT: sprintf(s, "%s%f ", s, p.v.f); break;
		}
		if (nl) sprintf(s, "%s\n", s);
	}

	return s;
}

#define TK(x)				(*(x->b + x->in))
#define PARSE(x, c)	while (TK(x) != 0 && x->in < x->bl && TK(x) != 13 && c) { x->in++; }

void parse_token(X* x) {
	char *s;
	PARSE(x, isspace(*(x->b + x->in))); 
	s = x->b + x->in;
	PARSE(x, !isspace(*(x->b + x->in))); 
	PUSHS(x, s, (x->b + x->in) - s);
}

void parse_until(X* x) {
	char *s = x->b + (++x->in);
	char c = (char)POP(x).v.c;
	PARSE(x, *(x->b + x->in) != c);
	PUSHS(x, s, (x->b + x->in) - s);

}

void find_token(X* x) {
	char* sptr,* eptrf,* eptri;
	double fnum;
	C inum;
	/* TODO: Find in the dictionary */
	sptr = POP(x).v.s;
	fnum = strtod(sptr, &eptrf); 
	if (sptr == eptrf) { x->err = ERR_UNDEFINED_WORD; return; }
	inum = strtol(sptr, &eptri, 10); 
	if (eptri == eptrf) { PUSHC(x, INT, inum); } else { PUSHF(x, FLOAT, fnum); }
}

void eval(X* x) {
	char* endptr;
	double num;
	C cnum;
	C e;
	x->bl = TOS(x).l;
	x->b = POP(x).v.s;
	x->in = 0;
	do {
		parse_token(x); if (TOS(x).l == 0) { DROP(x); return; }
		find_token(x); if (x->err) { return; }
	} while (1);
}

#endif
