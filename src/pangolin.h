/* TODO: Add print arrays to dump */
/* TODO: Add markers (new type) on stack for easy creation of arrays and quotations */

#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<stdio.h>

typedef char B;
typedef int16_t W;
typedef int32_t H;
typedef int64_t C;
typedef double D;

typedef struct { W t, m, c, l; union { C i; D f; void* a; } v; } O;

enum { LIST, INT, INT_ARRAY, FLOAT, FLOAT_ARRAY, BYTE_ARRAY, STR } T;

#define STACK_SIZE	256
#define RSTACK_SIZE	256

typedef struct { O s[STACK_SIZE]; B* r[RSTACK_SIZE]; C sp, rp; B* ip; C trace; } X;

X* init() {
	X* x = malloc(sizeof(X));

	x->sp = x->rp = 0;
	x->ip = 0;
	x->trace = 1;

	return x;
}

typedef void (*F)(X*);

#define PEEK(_x, _i)			(_x->s[_i])

#define PUSH(_x, _v)			(_x->s[_x->sp].t = INT, _x->s[_x->sp++].v.i = (_v))
#define PUSHF(_x, _v)			(_x->s[_x->sp].t = FLOAT, _x->s[_x->sp++].v.f = (_v))

#define TOS(_x)						(_x->s[_x->sp - 1])
#define NOS(_x)						(_x->s[_x->sp - 2])
#define NNOS(_x)					(_x->s[_x->sp - 3])

#define POP(_x)						(_x->s[--_x->sp].v.i)
#define POPF(_x)					(_x->s[--_x->sp].v.f)

/* POPA/DROP is free */
#define DROP(_x)					(--_x->sp)	/* TODO: Free arrays! */

#define PUSHR(_x, _v)			(_x->r[_x->rp++] = _v)
#define POPR(_x)					(_x->r[--_x->rp])

#define ERR_OK									0
#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_DIVISION_BY_ZERO		-3
#define ERR_EXIT								-4

#define O1(_x)				if (_x->sp == STACK_SIZE) { return ERR_STACK_OVERFLOW; }
#define O2(_x)				if (_x->sp + 1 == STACK_SIZE) { return ERR_STACK_OVERFLOW; }
#define U1(_x)				if (_x->sp == 0) { return ERR_STACK_UNDERFLOW; }
#define U2(_x)				if (_x->sp == 1) { return ERR_STACK_UNDERFLOW; }
#define U3(_x)				if (_x->sp == 2) { return ERR_STACK_UNDERFLOW; }
#define ZD(_x)				if (TOS(_x).v.i == 0) { return ERR_DIVISION_BY_ZERO; }

char* dump_stack(char* s, X* x, C nl) {
	C i;
	O o;
	for (i = 0; i < x->sp; i++) {
		o = PEEK(x, i);
		switch (o.t) {
		case INT: sprintf(s, "%s%ld ", s, o.v.i); break;
		case FLOAT: sprintf(s, "%s%f ", s, o.v.f); break;
		case STR: sprintf(s, "%s\"%.*s\" ", s, o.l, (char*)o.v.a); break;
		}
		if (nl) sprintf(s, "%s\n", s);
	}

	return s;
}

char* dump_rstack(char* s, X* x) {
	B* i;
	for (i = x->ip; *(i - 1) != ';' && *i != 0 && *i != 10; i++) {
		if (*i == 'l') {
			sprintf(s, "%sl %08x ", s, (unsigned int)*((C*)(i + 1)));
			i += sizeof(C);
		} else {
			sprintf(s, "%s%c", s, *i);
		}
	}

	return s;
}

char* dump(char* s, X* x, C f) {
	char buf[255];
	char i;
	if (f) {
		buf[0] = 0;
		sprintf(buf, "%s", dump_stack(buf, x, 0));
		for (i = 0; i < f - strlen(buf); i++) { s[i] = ' '; }
		s[f - strlen(buf)] = 0;
	}
	sprintf(s, "%s: ", dump_stack(s, x, 0));
	s = dump_rstack(s, x);
	if (f) sprintf(s, "%s\n", s);
	return s;
}

#define OP2(_x, _op) \
	( (NOS(_x).t == INT && TOS(_x).t == INT) ? \
		(NOS(_x).v.i = NOS(_x).v.i _op TOS(_x).v.i) \
	: (NOS(_x).t == INT && TOS(_x).t == FLOAT) ? \
		(NOS(_x).t = FLOAT, NOS(_x).v.f = ((double)NOS(_x).v.i) _op TOS(_x).v.f) \
	: (NOS(_x).t == FLOAT && TOS(_x).t == INT) ? \
		(NOS(_x).v.f = NOS(_x).v.f _op ((double)TOS(_x).v.i)) \
	: (NOS(_x).v.f = NOS(_x).v.f _op TOS(_x).v.f) ) 

void dup(X* x) {
	switch (TOS(x).t) {
	case INT: PUSH(x, TOS(x).v.i); break;
	case FLOAT: PUSHF(x, TOS(x).v.f); break;
	default: /* TODO */ break;
	}
}

void swap(X* x) {
	O o;
	o.t = TOS(x).t; o.m = TOS(x).m; o.c = TOS(x).c; o.l = TOS(x).l; o.v.a = TOS(x).v.a;
	TOS(x).t = NOS(x).t; TOS(x).m = NOS(x).m; TOS(x).c = NOS(x).c; TOS(x).l = NOS(x).l; TOS(x).v.a = NOS(x).v.a;
	NOS(x).t = o.t; NOS(x).m = o.m; NOS(x).c = o.c; NOS(x).l = o.l; NOS(x).v.a = o.v.a;
}

void over(X* x) {
	switch (TOS(x).t) {
	case INT: PUSH(x, NOS(x).v.i); break;
	case FLOAT: PUSHF(x, NOS(x).v.f); break;
	default: /* TODO */ break;
	}
}

void rot(X* x) {
	O o;
	o.t = TOS(x).t; o.m = TOS(x).m; o.c = TOS(x).c; o.l = TOS(x).l; o.v.a = TOS(x).v.a;
	TOS(x).t = NOS(x).t; TOS(x).m = NOS(x).m; TOS(x).c = NOS(x).c; TOS(x).l = NOS(x).l; TOS(x).v.a = NOS(x).v.a;
	NOS(x).t = NNOS(x).t; NOS(x).m = NNOS(x).m; NOS(x).c = NNOS(x).c; NOS(x).l = NNOS(x).l; NOS(x).v.a = NNOS(x).v.a;
	NNOS(x).t = o.t; NNOS(x).m = o.m; NNOS(x).c = o.c; NNOS(x).l = o.l; NNOS(x).v.a = o.v.a;
}

#define STEP \
	switch (*(x->ip)) {	\
		case '0': /* Literal two */ O1(x); PUSH(x, 0); break;	\
		case '1': /* Literal one */ O1(x); PUSH(x, 1); break;	\
		case 'r': /* Relative literal -to next IP- */ PUSH(x, (C)(*(++x->ip) + x->ip + 1)); break; \
		case 'l': /* Literal */ PUSH(x, (*((C*)(x->ip + 1)))); x->ip += sizeof(C); break; \
		/* Arithmetics */	\
		case '+': /* add */ U2(x); OP2(x, +); DROP(x); break;	\
		case '-': /* subtract */ U2(x); OP2(x, -); DROP(x); break;	\
		case '*': /* mul */ U2(x); OP2(x, *); DROP(x); break;	\
		/* TODO: Zero division on float is a problem? */ \
		case '/': /* div */ U2(x); ZD(x); OP2(x, /); DROP(x); break; \
		case '%': /* mod */ U2(x); NOS(x).v.i %= TOS(x).v.i; DROP(x); break;	\
		/* Comparisons */	\
		case '>': /* greater than */ U2(x); OP2(x, >); DROP(x); break;	\
		case '<': /* less than */ U2(x); OP2(x, <); DROP(x); break;	\
		case '=': /* equal */ U2(x); OP2(x, ==); DROP(x); break; \
		/* Bits */ \
		case '&': /* and */ U2(x); NOS(x).v.i &= TOS(x).v.i; DROP(x); break; \
		case '|': /* or */ U2(x); NOS(x).v.i |= TOS(x).v.i; DROP(x); break;	\
		case '!': /* not */ U1(x); TOS(x).v.i = !TOS(x).v.i; break;	\
		case '~': /* invert */ U1(x); TOS(x).v.i = ~TOS(x).v.i; break;	\
		/* Stack manipulators */ \
		case 'd': /* dup */ U1(x); dup(x); break; \
		case 's': /* swap */ U2(x); swap(x); break; \
		case 'o': /* over */ U2(x); O1(x); over(x); break; \
		case '@': /* rot */ U3(x); rot(x); break; \
		case '\\': U1(x); DROP(x); break; \
		/* Calls & jumps */ \
		case 'j': /* Jump (absolute) */ U1(x); x->ip = ((B*)POP(x)) - 1; break; \
		case 'c': /* Call */ U1(x); PUSHR(x, x->ip); x->ip = ((B*)POP(x)) - 1; break;	\
		case 'n': /* Native (C) call */ U1(x); ((F)(POP(x)))(x); break; \
		case 'z': /* 0 jump */ U2(x); x->ip = NOS(x).v.i ? (DROP(x), x->ip) : ((B*)POP(x)) - 1; DROP(x); break; \
		/* REPL */ \
		case '$': /* Trace on/off */ U1(x); x->trace = POP(x); break; \
		case 'q': /* Quit repl */ return ERR_EXIT; break; \
		/* Helpers */	\
		case '^': /* Recurse */ PUSHR(x, x->ip); while (*(x->ip) != '[') x->ip--; break; \
		case '#':	/* Parsed number literal */ \
			O1(x); \
			t = 0; \
			while ((o = *(x->ip + 1) - '0', 0 <= o && o <= 9)) { \
				t = 10*t + o; \
				x->ip++; \
			}	\
			PUSH(x, t); \
			break; \
		case '?': /* If */ \
			U1(x); \
			t = 1; \
			if (!POP(x)) { \
				while (t) { \
					x->ip++; \
					/* IOB(x); */	\
					if (*x->ip == '(') t--; \
					if (*x->ip == '?') t++; \
				} \
			} \
			break; \
		case '(':	/* Else */ \
			t = 1; \
			while (t) { \
				x->ip++; \
				/* IOB(x); */ \
				if (*x->ip == '(') t++; \
				if (*x->ip == ')') t--; \
			} \
			break; \
		case ')': /* Then */ /* NOOP */ break; \
		case '{': /* Start of loop */ /* NOOP */ break;	\
		case '}': /* End of loop */ \
			t = 1; \
			while (t) { \
				x->ip--; \
				/* IOB(x); */ \
				if (*x->ip == '}') t++; \
				if (*x->ip == '{') t--; \
			} \
			break; \
		case '[': /* Block */ PUSH(x, ((C)(x->ip + 1))); while (*x->ip != ']') x->ip++; break; \
		case ']': /* End of block */ if (x->rp > 0) x->ip = POPR(x); else return ERR_OK; break; \
		/* Extensions */ \
		/* default: o = OP(x, IP(x)); if ('A' <= o && o <= 'Z') { ((F)EX(x, o))(x); } break; */ \
	}

C inner(X* x) {
	char buf[255];
	C t, o, e;

	if (x->trace) {
		buf[0] = 0; printf("%s", dump(buf, x, 40));
		while (*x->ip != 0) { 
			STEP(x); 
			x->ip++; 
			buf[0] = 0; printf("%s", dump(buf, x, 40));
		}
	} else {
		while (*x->ip != 0) { 
			STEP(x); 
			x->ip++; 
		}
	}

	return ERR_OK;
}

/*
#define p_malloc	malloc
#define p_free		free

enum { LIST, INT, FLOAT, INT_ARRAY } T;

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

#define PUSH(_o, _v)		(L(_o)[(_o)->l].t = INT, L(_o)[(_o)->l++].v.i = _v)
#define PUSHF(_o, _v)		(L(_o)[(_o)->l].t = FLOAT, L(_o)[(_o)->l++].v.f = _v)

#define TOS(_o)					(L(_o)[(_o)->l - 1])
#define NOS(_o)					(L(_o)[(_o)->l - 2])

#define OP2(_o, _op) \
	( NOS(_o).t == INT && TOS(_o).t == INT ? \
		NOS(_o).v.i = NOS(_o).v.i op TOS(_o).v.i \
	: NOS(_o).t == INT && TOS(_o).t == FLOAT ? \
		(NOS(_o).t = FLOAT, NOS(_o).v.f = ((double)NOS(_o).v.i) op TOS(_o).v.f) \
	: NOS(_o).t == FLOAT && TOS(_o).t == INT ? \
		NOS(_o).v.f = NOS(_o).v.f op ((double)TOS(_o).v.i) \
	: NOS(_o).v.f = NOS(_o).v.f op TOS(_o).v.f) 

#define ADD(_o)					OP2(_o, +)
#define SUB(_o)					OP2(_o, -)
#define MUL(_o)					OP2(_o, *)
#define DIV(_o)					OP2(_o, /)
#define MOD(_o)					OP2(_o, %)

#define GT(_o)					OP2(_o, >)
#define LT(_o)					OP2(_o, <)
#define EQ(_o)					OP2(_o, ==)

#define AND(_o)					(NOS(_o).v.i = NOS(_o).v.i & TOS(_o).v.i)
#define OR(_o)					(NOS(_o).v.i = NOS(_o).v.i | TOS(_o).v.i)
#define NOT(_o)					(TOS(_o).v.i = !TOS(_o).v.i)
#define INVERT(_o)			(TOS(_o).v.i = ~TOS(_o).v.i)

#define DUP(_o)					((_o)->l++, TOS(_o).t = NOS(_o).t, TOS(_o).v.a = NOS(_o).v.a)
*/
/* Expects a temporary Object o_ defined */
/*
#define SWAP(_o) ( \
	o_.t = TOS(_o).t, o_.m = TOS(_o).m, o_.c = TOS(_o).c, o_.l = TOS(_o).l, o_.v.a = TOS(_o).v.a, \
	TOS(_o).t = NOS(_o).t, TOS(_o).m = NOS(_o).m, TOS(_o).c = NOS(_o).c, TOS(_o).l = NOS(_o).l, TOS(_o).v.a = NOS(_o).v.a, \
	NOS(_o).t = o_.t, NOS(_o).m = o_.m, NOS(_o).c = o_.c, NOS(_o).l = o_.l, NOS(_o).v.a = o_.v.a)
*/
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
