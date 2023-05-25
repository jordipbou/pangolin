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
	INTEGER = 2,
	FLOAT = 3,
	MANAGED = 5,
	/* Representation types */
	CHAR = 11,
	NUMBER = 13,
	STRING = 17
} T;

typedef struct { I t, c; union { I i; F f; } v; } O;

typedef struct { O s[64]; I sp; B* r[64]; I rp; B* ip; B* tib; } X;

typedef void (*FUNC)(X*);

X* init() {
	X* x = malloc(sizeof(X));
	x->sp = x->rp = 0;
	return x;
}

#define PK(_x, _i)						(_x->s[_i])
#define TS(_x)								(PK(_x, x->sp - 1))
#define NS(_x)								(PK(_x, x->sp - 2))

#define PUSH(_x, _v)					(_x->sp++, TS(_x).t = INTEGER, TS(_x).c = 0, TS(x).v.i = (I)_v)
#define PUSHF(_x, _t, _v)			(_x->sp++, TS(_x).t = FLOAT*_t, TS(_x).c = 0, TS(x).v.f = (F)_v)
#define PUSHT(_x, _t, _c, _v)	(_x->sp++, TS(_x).t = _t, TS(_x).c = _c, TS(x).v.i = (I)_v)

#define DROP(_x)							if (TS(_x).t % MANAGED == 0) { free((void*)TS(_x).v.i); }; x->sp--

/* Beware, POP will free managed references before returning them! */
I pop(X* x) {
	DROP(x);
	return PK(x, x->sp).v.i;
}
#define POPF(_x)							(DROP(_x), PK(_x, _x->sp).v.f)

#define PUSHR(_x, _i)					(_x->r[_x->rp++] = _i)
#define POPR(_x)							(_x->r[--_x->rp])


/* External representation */

I ext(B* s, O* o) {
	if (o->t % INTEGER == 0) {
		return sprintf(s, "%ld", o->v.i);
	} else {
		return sprintf(s, "%f", o->v.f);
	}
}

I dump_stack(B* s, X* x, I nl) {
	I i, t, n = 0;
	for (i = 0; i < x->sp; i++) {
		s += t = ext(s, &PK(x, i));
		if (nl) *s++ = '\n'; else *s++ = ' ';
		n += t + 1;
	}

	return n;
}

I dump_rstack(B* s, X* x) {
	I i, j, t = 1, n;
	for (i = 0; *(x->ip + i) != 0 && t > 0; i++) {
		*s++ = *(x->ip + i);
		if (*(x->ip + i) == '[') { t++; }
		if (*(x->ip + i) == ']') { t--; }
	}

	n = i;

	for (i = x->rp - 1; i >= 0; i--) {
		*s++ = ' '; *s++ = ':'; *s++ = ' '; n += 3;
		t = 1;
		for (j = 0; *(x->r[i] + j) != 0 && t > 0; j++) {
			*s++ = *(x->r[i] + j);
			n++;
			if (*(x->r[i] + j) == '[') { t++; }
			if (*(x->r[i] + j) == ']') { t--; }
		}
	}

	return i;
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
		*s++ = ' ';
		*s++ = '<';
		*s++ = '<';
		*s++ = ' ';
		t += 4;
		for (n = 0; *x->tib != 0; x->tib++, n++) {
			*s++ = *x->tib;
		}
		t += n;
	}

	return t;
}

/* Inner interpreter */

void add(X* x) { NS(x).v.i += TS(x).v.i; x->sp--; }
void sub(X* x) { NS(x).v.i -= TS(x).v.i; x->sp--; }
void lt(X* x) { NS(x).v.i = NS(x).v.i < TS(x).v.i; x->sp--; }
void dup(X* x) { I i = TS(x).v.i; PUSH(x, i); }
void swap(X* x) { I i = TS(x).v.i; TS(x).v.i = NS(x).v.i; NS(x).v.i = i; }
void gt(X* x) { NS(x).v.i = NS(x).v.i > TS(x).v.i; }
#define CALL(_x, _i)					if (!(*(_x->ip + 1) == ']' || *(_x->ip + 1) == 0)) { PUSHR(_x, _x->ip); } _x->ip = _i - 1
void exec_i(X* x) { B* q = (B*)pop(x); CALL(x, q); }

void forward(X* x, I o, I c) {
	I t = 1;
	while (t != 0 && *x->ip != 0) {
		x->ip++;
		if (*(x->ip) == o) { t++; }
		if (*(x->ip) == c) { t--; }
	}
}

void ifthen(X* x) {
	B* e = (B*)pop(x);
	B* t = (B*)pop(x);
	if (pop(x)) { CALL(x, t); }
	else { CALL(x, e); }
}

void inner(X*);

void _bin_rec(X* x, B* i, B* t, B* r1, B* r2) {
	PUSHR(x, x->ip);
	x->ip = i;
	inner(x);
	if (pop(x)) {
		PUSHR(x, x->ip);
		x->ip = t;
		inner(x);
	} else {
		PUSHR(x, x->ip);
		x->ip =	r1;
		inner(x);
		_bin_rec(x, i, t, r1, r2);
		swap(x);
		_bin_rec(x, i, t, r1, r2);
		PUSHR(x, x->ip);
		x->ip = r2;
		inner(x);
	}
}

void binrec(X* x) {
	B* r2 = (B*)pop(x);
	B* r1 = (B*)pop(x);
	B* t = (B*)pop(x);
	B* i = (B*)pop(x);
	_bin_rec(x, i, t, r1, r2);
}

void inner(X* x) {
	B buf[255];
	I r = x->rp;
	do {
		/*
		memset(buf, 0, sizeof buf);
		dump(buf, x);
		printf("%s\n", buf);
		*/
		switch (*x->ip) {
			case 0: return;
			case '0': PUSH(x, 0); break;
			case '1': PUSH(x, 1); break;
			case '+': add(x); break;
			case '-': sub(x); break;
			case '<': lt(x); break;
			case 'd': dup(x); break;
			case 's': swap(x); break;
			case 'i': exec_i(x); break;
			case '[': PUSH(x, x->ip + 1); forward(x, '[', ']'); break;
			case ']': if (x->rp > r) { x->ip = POPR(x); } else { if (x->rp > 0) { x->ip = POPR(x); } return; } break;
			case '?': ifthen(x); break;
			case 'b': binrec(x); break;
		}
		x->ip++;
	} while(1);
}

#endif
