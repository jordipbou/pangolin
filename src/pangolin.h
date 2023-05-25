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

typedef struct { O s[64]; I sp; B* r[64]; I rp; B* ip; B* tib; } X;

typedef void (*FUNC)(X*);

X* init() {
	X* x = malloc(sizeof(X));
	x->sp = x->rp = 0;
	x->ip = 0;
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

void add(X* x) { 
	if (NS(x).t % INT == 0 && TS(x).t % INT == 0) {
		NS(x).v.i += TS(x).v.i; x->sp--;
	} else if (NS(x).t % FLOAT == 0 && TS(x).t % INT == 0) {
	} else if (NS(x).t % INT == 0 && TS(x).t % FLOAT == 0) {
	} else if (NS(x).t % FLOAT == 0 && TS(x).t % FLOAT == 0) {
	}
}

void sub(X* x) { 
	if (NS(x).t % INT == 0 && TS(x).t % INT == 0) {
		NS(x).v.i -= TS(x).v.i; x->sp--;
	} else if (NS(x).t % FLOAT == 0 && TS(x).t % INT == 0) {
	} else if (NS(x).t % INT == 0 && TS(x).t % FLOAT == 0) {
	} else if (NS(x).t % FLOAT == 0 && TS(x).t % FLOAT == 0) {
	}
}

void mul(X* x) { 
	if (NS(x).t % INT == 0 && TS(x).t % INT == 0) {
		NS(x).v.i *= TS(x).v.i; x->sp--;
	} else if (NS(x).t % FLOAT == 0 && TS(x).t % INT == 0) {
	} else if (NS(x).t % INT == 0 && TS(x).t % FLOAT == 0) {
	} else if (NS(x).t % FLOAT == 0 && TS(x).t % FLOAT == 0) {
	}
}

void lt(X* x) { NS(x).v.i = NS(x).v.i < TS(x).v.i; x->sp--; }
void eq(X* x) { NS(x).v.i = NS(x).v.i == TS(x).v.i; x->sp--; }

void dup(X* x) { I i = TS(x).v.i; PUSH(x, i); }
void swap(X* x) { I i = TS(x).v.i; TS(x).v.i = NS(x).v.i; NS(x).v.i = i; }
void gt(X* x) { NS(x).v.i = NS(x).v.i > TS(x).v.i; }
void exec_i(X* x) { B* q = (B*)pop(x); CALL(x, q, 0); }

I forward(X* x, I o, I c) {
	I t = 1, n = 0;
	while (t != 0 && *x->ip != 0) {
		x->ip++;
		n++;
		if (*(x->ip) == o) { t++; }
		if (*(x->ip) == c) { t--; }
	}

	return n - 1;
}

void ifthen(X* x) {
	B* e = (B*)pop(x);
	B* t = (B*)pop(x);
	if (pop(x)) { CALL(x, t, 0); }
	else { CALL(x, e, 0); }
}

void inner(X*);

void linrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i, 1); inner(x);
	if (pop(x)) { CALL(x, t, 1); inner(x); }
	else {
		CALL(x, r1, 1); inner(x);
		linrec(x, i, t, r1, r2);
		CALL(x, r2, 1); inner(x);
	}
}

void binrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i, 1); inner(x);
	if (pop(x)) { CALL(x, t, 1); inner(x); } 
	else {
		CALL(x, r1, 1); inner(x);
		binrec(x, i, t, r1, r2);
		swap(x);
		binrec(x, i, t, r1, r2);
		CALL(x, r2, 1); inner(x);
	}
}

void times(X* x, I t, B* q) { for(;t > 0; t--) { CALL(x, q, 1); inner(x); } }

void number(X* x) {
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

void inner(X* x) {
	B buf[255];
	I r = x->rp;
	do {
		memset(buf, 0, sizeof buf);
		dump(buf, x);
		printf("%s\n", buf);
		switch (*x->ip) {
			case 0: return;
			case '0': PUSH(x, 0); break;
			case '1': PUSH(x, 1); break;
			case '+': add(x); break;
			case '-': sub(x); break;
			case '*': mul(x); break;
			case '<': lt(x); break;
			case '=': eq(x); break;
			case 'd': dup(x); break;
			case 's': swap(x); break;
			case 'i': exec_i(x); break;
			case '[': PUSH(x, x->ip + 1); forward(x, '[', ']'); break;
			case ']': if (x->rp > r) { x->ip = POPR(x); } else { if (x->rp > 0) { x->ip = POPR(x); } return; } break;
			case '?': ifthen(x); break;
			case 'l': linrec(x, (B*)pop(x), (B*)pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 'b': binrec(x, (B*)pop(x), (B*)pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 't': times(x, pop(x), (B*)pop(x)); break;
			case '#': number(x); break;
			case '"': PUSH(x, x->ip + 1); TS(x).t *= STRING; TS(x).c = forward(x, 0, '"'); break;
		}
		x->ip++;
	} while(1);
}

#endif
