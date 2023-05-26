#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

typedef char B;
typedef intptr_t I;
typedef double F;		/* This should be float in 32 bit system */

enum {
	/* */
	ANY = 1,
	/* Stack types */
	INT = 2,
	FLOAT = 3,
	MANAGED = 5,
	/* Representation types */
	CHAR = 11,
	NUMBER = 13,
	STRING = 17,
	ARRAY = 19
} T;

typedef struct { I t, c; union { I i; F f; } v; } O;

#define STACK_SIZE							64
#define RSTACK_SIZE							64

typedef struct { O s[STACK_SIZE]; I sp; B* r[RSTACK_SIZE]; I rp; B* ip; B* tib; I in; I tr; I err; } X;

typedef void (*FUNC)(X*);
void P_inner(X*);

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_DIVIDE_BY_ZERO			-3

I error(X* x) {
	/* TODO: Find in exception stack a handler for current error */
	printf("ERROR: %ld\n", x->err);
	return 1;
}

#define OF1(x)			if (x->sp == STACK_SIZE) { x->err = ERR_STACK_OVERFLOW; if (error(x)) return; }
#define OF2(x)			if (x->sp >= STACK_SIZE - 1) { x->err = ERR_STACK_OVERFLOW; if (error(x)) return; }
#define UF1(x)			if (x->sp == 0) { x->err = ERR_STACK_UNDERFLOW; if (error(x)) return; }
#define UF2(x)			if (x->sp <= 1) { x->err = ERR_STACK_UNDERFLOW; if (error(x)) return; }
#define UF3(x)			if (x->sp <= 2) { x->err = ERR_STACK_UNDERFLOW; if (error(x)) return; }
#define UF4(x)			if (x->sp <= 3) { x->err = ERR_STACK_UNDERFLOW; if (error(x)) return; }
#define DZ(x)				if (TS(x).v.i == 0) { x->err = ERR_DIVIDE_BY_ZERO; if (error(x)) return; }

#define DO(x, f)		f(x); if (x->err) { return; }

X* init() {
	X* x = malloc(sizeof(X));
	x->sp = x->rp = x->in = x->err = x->tr = 0;
	x->ip = x->tib = 0;
	return x;
}

#define PK(_x, _i)						(_x->s[_i])
#define TS(_x)								(PK(_x, x->sp - 1))
#define NS(_x)								(PK(_x, x->sp - 2))

#define PUSH(_x, _v)					(_x->sp++, TS(_x).t = INT, TS(_x).c = 0, TS(x).v.i = (I)_v)
#define PUSHF(_x, _v)					(_x->sp++, TS(_x).t = FLOAT, TS(_x).c = 0, TS(x).v.f = (F)_v)

#define DROP(_x)							if (TS(_x).t % MANAGED == 0) { free((void*)TS(_x).v.i); }; x->sp--

/* Beware, POP will free managed references before returning them! */
I pop(X* x) { DROP(x); return PK(x, x->sp).v.i; }
#define POPF(_x)							(DROP(_x), PK(_x, _x->sp).v.f)

#define PUSHR(_x, _i)					(_x->r[_x->rp++] = _i)
#define POPR(_x)							(_x->r[--_x->rp])

#define CALL(x, d, t)					if (t || !(*(x->ip + 1) == 0 || *(x->ip + 1) == ']')) { PUSHR(x, x->ip); } x->ip = d

/* External representation */

I dump_o(B* s, O* o) {
	if (o->t % STRING == 0) { return sprintf(s, "\"%.*s\"", (int)o->c, (B*)o->v.i); } 
	else if (o->t % CHAR == 0) { return sprintf(s, "%c", (char)o->v.i); }
	else if (o->t % INT == 0) { return sprintf(s, "%ld", o->v.i); } 
	else if (o->t % FLOAT == 0) { return sprintf(s, "%g", o->v.f); }
}

I dump_stack(B* s, X* x, I nl) {
	I i, t, n = 0;
	for (i = 0; i < x->sp; i++) { 
		s += t = dump_o(s, &PK(x, i)); 
		*s++ = nl ? '\n' : ' '; 
		n += t + 1; 
	}
	return n;
}

#define DUMP_CODE(o) for (i = 0; *(o + i) != 0 && t > 0; i++) { n++; *s++ = *(o + i); if (*(o + i) == '[') t++; else if (*(o + i) == ']') t--; }
#define SEPARATOR	*s++ = ' '; *s++ = ':'; *s++ = ' '; n += 3;

I dump_rstack(B* s, X* x) {
	I i, j, t = 1, n = 0;
	DUMP_CODE(x->ip);
	for (j = x->rp - 1; j >= 0; j--) { SEPARATOR; t = 1; DUMP_CODE(x->r[j]); }

	return n;
}

I dump(B* s, X* x) {
	B r[255];
	I n, t = 0, p;

	memset(r, 0, sizeof r);
	n = dump_stack(r, x, 0);
	s += t = sprintf(s, "%40s: ", r);
	s += n = dump_rstack(s, x);
	t += n;
	if (x->tib != 0 && *x->tib != 0) {
		*s++ = ' '; *s++ = '<'; *s++ = '<'; *s++ = ' '; t += 4;
		for (n = 0; *x->tib != 0; x->tib++, n++) { *s++ = *x->tib; }
		t += n;
	}

	return t;
}

/* Arithmetic operations */
void P_add(X* x) { /* ( n n -- n ) */ NS(x).v.i += TS(x).v.i; x->sp--; }
void P_sub(X* x) { /* ( n n -- n ) */ NS(x).v.i -= TS(x).v.i; x->sp--; }
void P_mul(X* x) { /* ( n n -- n ) */ NS(x).v.i *= TS(x).v.i; x->sp--; }
void P_div(X* x) { /* ( n n -- n ) */ NS(x).v.i /= TS(x).v.i; x->sp--; }
void P_mod(X* x) { /* ( n n -- n ) */ NS(x).v.i %= TS(x).v.i; x->sp--; }

/* Comparison operations */
void P_lt(X* x) { /* ( n n -- n ) */ NS(x).v.i = NS(x).v.i < TS(x).v.i; x->sp--; }
void P_eq(X* x) { /* ( n n -- n ) */ NS(x).v.i = NS(x).v.i == TS(x).v.i; x->sp--; }
void P_gt(X* x) { /* ( n n -- n ) */ NS(x).v.i = NS(x).v.i > TS(x).v.i; x->sp--; }

/* Stack operations */
/* TODO: Must ensure that stack operations work for every type */
void P_dup(X* x) { /* ( a -- a a ) */ I i = TS(x).v.i; PUSH(x, i); }
void P_swap(X* x) { /* ( a b -- b a ) */ I i = TS(x).v.i; TS(x).v.i = NS(x).v.i; NS(x).v.i = i; }

/* Execution */
void P_exec_i(X* x) { /* ( [P] -- P ) */ B* q = (B*)pop(x); CALL(x, q - 1, 0); }

/* Conditional and looping operations */
void P_ifthen(X* x, I c, B* t, B* e) { /* ( flag [P] [Q] -- P|Q ) */ CALL(x, (c ? t : e) - 1, 0); }
void P_times(X* x, I t, B* q) { /* ( n [P] -- %n times P% ) */ 
	for(;t > 0; t--) { 
		CALL(x, q, 1); DO(x, P_inner);
	} 
}
void P_while(X* x, B* c, B* q) { /* ( [C] [P] -- %P while C% ) */
	do { 
		CALL(x, c, 1); DO(x, P_inner);
		if (!pop(x)) { return; } 
		CALL(x, q, 1); DO(x, P_inner);
	} while(1); 
}
/* Recursion operations */
void P_linrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i, 1); DO(x, P_inner);
	if (pop(x)) { CALL(x, t, 1); DO(x, P_inner); }
	else {
		CALL(x, r1, 1); DO(x, P_inner);
		P_linrec(x, i, t, r1, r2);
		CALL(x, r2, 1); DO(x, P_inner);
	}
}

void P_binrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i, 1); P_inner(x);
	if (pop(x)) { CALL(x, t, 1); P_inner(x); } 
	else {
		CALL(x, r1, 1); P_inner(x);
		P_binrec(x, i, t, r1, r2);
		P_swap(x);
		P_binrec(x, i, t, r1, r2);
		CALL(x, r2, 1); P_inner(x);
	}
}

/* Parsing */
void P_number(X* x) {
	char *end, *end2;
	F f;
	I i;
	f = strtod(x->ip + 1, &end);
	if (f == 0 && end == x->ip + 1) {
		/* Conversion error */
	} else {
		i = strtol(x->ip + 1, &end2, 0);
		if (i == f && end == end2) {
			PUSH(x, i);
		} else {
			PUSHF(x, f);
		}
	}
	x->ip = end - 1;
}

I P_forward(X* x, I o, I c) {
	I t = 1, n = 0;
	while (t != 0 && *x->ip != 0) {
		x->ip++;
		n++;
		if (*(x->ip) == o) { t++; }
		if (*(x->ip) == c) { t--; }
	}

	return n - 1;
}

void P_inner(X* x) {
	B buf[255];
	I r = x->rp;
	while (*x->ip != 0 && x->err == 0) {
		if (x->tr) {
			memset(buf, 0, sizeof buf);
			dump(buf, x);
			printf("%s\n", buf);
		}
		switch (*x->ip) {
			case '0': PUSH(x, 0); break;
			case '1': PUSH(x, 1); break;
			case '+': UF2(x); P_add(x); break;
			case '-': UF2(x); P_sub(x); break;
			case '*': UF2(x); P_mul(x); break;
			case '/': UF2(x); DZ(x); P_div(x); break;
			case '%': UF2(x); P_mod(x); break;
			case '<': UF2(x); P_lt(x); break;
			case '=': UF2(x); P_eq(x); break;
			case '>': UF2(x); P_gt(x); break;
			case 'd': UF1(x); OF1(x); P_dup(x); break;
			case 's': UF2(x); P_swap(x); break;
			case 'i': UF1(x); P_exec_i(x); break;
			case '[': OF1(x); PUSH(x, x->ip + 1); P_forward(x, '[', ']'); break;
			case ']': if (x->rp > r) { x->ip = POPR(x); } else { if (x->rp > 0) { x->ip = POPR(x); } return; } break;
			case '?': UF2(x); P_ifthen(x, pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 'l': UF3(x); P_linrec(x, (B*)pop(x), (B*)pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 'b': UF3(x); P_binrec(x, (B*)pop(x), (B*)pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 't': UF2(x); P_times(x, pop(x), (B*)pop(x)); break;
			case 'w': UF2(x); P_while(x, (B*)pop(x), (B*)pop(x)); break;
			case '#': P_number(x); break;
			case '"': OF1(x); PUSH(x, x->ip + 1); TS(x).t *= STRING; TS(x).c = P_forward(x, 0, '"'); break;
		}
		x->ip++;
	} 
}

#endif
