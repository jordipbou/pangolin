#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdlib.h>
#include<stdint.h>

#define p_malloc	malloc
#define p_free		free

enum { LIST, INT, FLOAT, INT_ARRAY } T;

typedef struct { 
	int16_t t, m, c, l; 
	union { 
		int64_t i; 
		double f; 
		void* a;
	} v; 
} O;

#define L(_o)		(((O*)(_o)->v.a))
#define IA(_o)	(((int64_t*)(_o)->v.a))

void *p_alloc(O* o, int t, int c, int s) {
	o->t = t;
	o->c = c;
	o->l = 0;
	o->m = 1;
	o->v.a = p_malloc(s * c);

	return o->v.a;
}

#define LPUSHI(_o, _v)		(L(_o)[(_o)->l].t = INT, L(_o)[(_o)->l++].v.i = _v)
#define LPUSHF(_o, _v)		(L(_o)[(_o)->l].t = FLOAT, L(_o)[(_o)->l++].v.f = _v)

#define LTOSI(_o)					(L(_o)[(_o)->l - 1].v.i)
#define LNOSI(_o)					(L(_o)[(_o)->l - 2].v.i)
#define LNNOSI(_o)				(L(_o)[(_o)->l - 3].v.i)

#define LTOSF(_o)					(L(_o)[(_o)->l - 1].v.f)
#define LNOSF(_o)					(L(_o)[(_o)->l - 2].v.f)
#define LNNOSF(_o)				(L(_o)[(_o)->l - 3].v.f)

#define IAPUSHI(_o, _v)		(IA(_o)[(_o)->l++] = _v)

#define IATOSI(_o)				(IA(_o)[(_o)->l - 1])
#define IANOSI(_o)				(IA(_o)[(_o)->l - 2])
#define IANNOSI(_o)				(IA(_o)[(_o)->l - 3])

/*
typedef union { 
	B b;		
	C i;		
	F f;		
	B* ba;	
	C* ia;	
	F* fa;	
} V;

typedef struct { 
	V v;
	B t;
} I;

typedef struct {
	V v;
} E;

typedef struct { I s[64]; C sp; } X;

void push_str(X* x, B* s) {
	B* d = malloc(strlen(s) + 1);
	strcpy(d, s);
	PUSH(s, STR, d);
}
*/

/*
typedef struct Ws { struct Ws* l; B f; P n; P c; } W;
typedef struct { P s[64]; C d; P r[64]; C rd; char* b; C bl, in, err, c; W* l; B* ip; } X;
typedef void (*FUNC)(X*);

enum { CHAR, INT, STR, EXT_STR, FLOAT, WORD } T;

#define ERR_UNDEFINED_WORD	-1

X* init() {
	X* x = malloc(sizeof(X));
	x->d = 0;
	x->rd = 0;
	x->ip = 0;
	x->err = 0;
	x->bl = 0;
	x->in = 0;
	x->b = 0;
	x->l = 0;
	x->c = 0;

	return x;
}

char* allocSTR(X* x, char* s, C l) {
	char* d = malloc(l + 1);
	strncpy(d, s, l);
	return d;
}

W* allocWORD(X* x, B f, char* n, char* c) {
	W* w = malloc(sizeof(W));
	w->f = f;
	w->n.t = STR;
	w->n.l = strlen(n);
	w->n.v.s = n;
	w->c.t = STR;
	w->c.l = strlen(c);
	w->c.v.s = c;
}

#define TOS(_x)							(_x->s[_x->d - 1])
#define NOS(_x)							(_x->s[_x->d - 2])
#define NNOS(_x)						(_x->s[_x->d - 3])

#define PEEK(_x, _i)				(_x->s[_i])
#define PUSHC(_x, _t, _v)		(_x->s[_x->d].t = _t, _x->s[_x->d++].v.c = _v)
#define PUSHS(_x, _s, _l)		(_x->s[_x->d].t = STR, _x->s[_x->d].l = _l, _x->s[_x->d++].v.s = _s)
#define PUSHES(_x, _s, _l)	(_x->s[_x->d].t = EXT_STR, _x->s[_x->d].l = _l, _x->s[_x->d++].v.s = _s)
#define PUSHF(_x, _v)				(_x->s[_x->d].t = FLOAT, _x->s[_x->d++].v.f = _v)
#define POP(_x)							(_x->s[--_x->d])
void DROP(X* x) { if (TOS(x).t == STR) { free(TOS(x).v.s); } --x->d; }

#define IADD(_x)						(NOS(_x).v.c += TOS(_x).v.c, x->d--)
#define ISUB(_x)						(NOS(_x).v.c -= TOS(_x).v.c, x->d--)
#define IMUL(_x)						(NOS(_x).v.c *= TOS(_x).v.c, x->d--)

#define FADD(_x)						(NOS(_x).v.f += TOS(_x).v.f, x->d--)
#define FSUB(_x)						(NOS(_x).v.f -= TOS(_x).v.f, x->d--)
#define FMUL(_x)						(NOS(_x).v.f *= TOS(_x).v.f, x->d--)

#define PRIMITIVE						1
#define IMMEDIATE						2

#define NEXT(_w)						(w->l)
#define NFA(_w)							(w->n)
#define CODE(_w)						(w->c)
#define XT(_w)							(w->c.v.s)
#define IS_PRIMITIVE(_w)		(w->f && PRIMITIVE)
#define IS_IMMEDIATE(_w)		(w->f && IMMEDIATE)

char* dump_stack(char* s, X* x, C nl) {
	C i;
	P p;
	for (i = 0; i < x->d; i++) {
		p = PEEK(x, i);	
		switch (p.t) {
		case CHAR: sprintf(s, "%s'%c' ", s, (char)p.v.c); break;
		case INT: sprintf(s, "%s%ld ", s, p.v.c); break;
		case STR: sprintf(s, "%s\"%.*s\" ", s, p.l, p.v.s); break;
		case EXT_STR: sprintf(s, "%s&\"%.*s\" ", s, p.l, p.v.s); break;
		case FLOAT: sprintf(s, "%s%f ", s, p.v.f); break;
		}
		if (nl) sprintf(s, "%s\n", s);
	}

	return s;
}

#define STEP(x) \
	switch (*x->ip) { \
	case '+': IADD(x); break; \
	case 'd': PUSHC(x, TOS(x).t, TOS(x).v.c); break; \
	}

void exec_i(X* x) {
	C crd = x->rd;
	B* q = POP(x).v.s;
	while (x->rd != crd && !x->err) {
		STEP(x);
	}
}

#define TK(x)				(x->b + x->in)
#define PARSE(x, c)	while (*TK(x) != 0 && x->in < x->bl && *TK(x) != 13 && c) { x->in++; }

void parse_token(X* x) {
	char *s;
	PARSE(x, isspace(*TK(x))); s = TK(x);
	PARSE(x, !isspace(*TK(x))); PUSHES(x, s, TK(x) - s);
}

void parse_until(X* x) {
	char *s = x->b + (++x->in);
	char c = (char)POP(x).v.c;
	PARSE(x, *TK(x) != c); 
	PUSHES(x, s, TK(x) - s);
}

void do_token(X* x) {
	C l = TOS(x).l;
	char* sptr = POP(x).v.s;
	char* eptrf,* eptri;
	double fnum;
	C inum;
	W* w = x->l;

	while (w) {
		if (!strncmp(NFA(w).v.s, sptr, l)) { 
			if (x->c || IS_IMMEDIATE(w)) {
				PUSHC(x, INT, (C)XT(w));
				exec_i(x);
			} else {
			}
			return;
		}
		else { w = NEXT(w); }
	}

	fnum = strtod(sptr, &eptrf); 
	if (sptr == eptrf) { x->err = ERR_UNDEFINED_WORD; return; }
	inum = strtol(sptr, &eptri, 10); 
	if (eptri == eptrf) { PUSHC(x, INT, inum); } else { PUSHF(x, fnum); }
}

void eval(X* x) {
	x->bl = TOS(x).l;
	x->b = POP(x).v.s;
	x->in = 0;
	do {
		parse_token(x); if (TOS(x).l == 0) { DROP(x); return; }
		do_token(x); if (x->err) { return; }
	} while (1);
}

void add_primitive(X* x, char* n, char* c) {
	W* w = allocWORD(x, 0, n, c);
	w->f = PRIMITIVE;
	w->l = x->l;
	x->l = w;
}

X* bootstrap(X* x) {
	add_primitive(x, "+", "+");
	add_primitive(x, "dup", "d");

	return x;
}
*/

#endif
