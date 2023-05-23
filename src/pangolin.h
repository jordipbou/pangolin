#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdint.h>
#include<stdlib.h>

typedef char B;
typedef intptr_t I;
typedef double F;

enum {
	/* Stack types */
	INT = 2,
	FLOAT = 3,
	UREF = 5,
	MREF = 7,
	/* Representation types */
	CHAR = 11,
	NUMBER = 13,
	STRING = 17
} T;

typedef struct { I t, c; union { I i; F f; void* r; } v; } O;

typedef struct { O s[64]; I sp; B* r[64]; I rp; B* tib; } X;

typedef void (*FUNC)(X*);

X* init() {
	X* x = malloc(sizeof(X));
	return x;
}

#define PK(_x, _i)						(_x->s[_i])
#define SP(_x)								(_x->sp)
#define TS(_x)								(PK(_x, SP(_x) - 1))
#define NS(_x)								(PK(_x, SP(_x) - 2))

/* TODO: I don't like this because it does not allow pushing alternative return points to the
   return stack, and that will be useful for implementation of backtracking. */
#define IP(_x)								(_x->r[_x->rp - 1])

#define PUSHI(_x, _t, _v)			(SP(_x)++, TS(_x).t = INT*_t, TS(_x).c = 0, TS(x).v.i = _v)
#define PUSHF(_x, _v)					(SP(_x)++, TS(_x).t = FLOAT*NUMBER, TS(_x).c = 0, TS(x).v.f = _v)
#define PUSHU(_x, _t, _c, _r)	(SP(_x)++, TS(_x).t = UREF*_t, TS(_x).c = _c, TS(x).v.r = _r)
#define PUSHM(_x, _t, _c, _r)	(SP(_x)++, TS(_x).t = MREF*_t, TS(_x).c = _c, TS(x).v.r = _r)

#define PUSHR(_x, _i)					(_x->r[_x->rp++] = _i)

#define DROP(x)								if (TS(x).t % MREF == 0) { free(TS(x).v.r); }; SP(x)--

/* Arithmetic operations */

#define ADD(_x) \
	if (NS(_x).t % (INT*NUMBER) == 0 && TS(_x).t % (INT*NUMBER) == 0) { \
		NS(_x).v.i = NS(_x).v.i + TS(_x).v.i; SP(_x)--; \
	} else if (NS(_x).t % (INT*NUMBER) == 0 && TS(_x).t % (FLOAT*NUMBER) == 0) { \
		NS(_x).t = FLOAT*NUMBER; NS(_x).v.f = ((F)NS(_x).v.i) + TS(_x).v.f; SP(_x)--; \
	} else if (NS(_x).t % (FLOAT*NUMBER) == 0 && TS(_x).t % (INT*NUMBER) == 0) { \
		NS(_x).v.f = NS(_x).v.f + ((F)TS(_x).v.i); SP(_x)--; \
	} else if (NS(_x).t % (FLOAT*NUMBER) == 0 && TS(_x).t % (FLOAT*NUMBER) == 0) { \
		NS(_x).v.f = NS(_x).v.f + TS(_x).v.f; SP(_x)--; \
	} else { \
		/* TODO: No implementation for current parameters error */ \
	}

#define SUB(_x) \
	if (NS(_x).t % (INT*NUMBER) == 0 && TS(_x).t % (INT*NUMBER) == 0) { \
		NS(_x).v.i = NS(_x).v.i - TS(_x).v.i; SP(_x)--; \
	} else if (NS(_x).t % (INT*NUMBER) == 0 && TS(_x).t % (FLOAT*NUMBER) == 0) { \
		NS(_x).t = FLOAT*NUMBER; NS(_x).v.f = ((F)NS(_x).v.i) - TS(_x).v.f; SP(_x)--; \
	} else if (NS(_x).t % (FLOAT*NUMBER) == 0 && TS(_x).t % (INT*NUMBER) == 0) { \
		NS(_x).v.f = NS(_x).v.f - ((F)TS(_x).v.i); SP(_x)--; \
	} else if (NS(_x).t % (FLOAT*NUMBER) == 0 && TS(_x).t % (FLOAT*NUMBER) == 0) { \
		NS(_x).v.f = NS(_x).v.f - TS(_x).v.f; SP(_x)--; \
	} else { \
		/* TODO: No implementation for current parameters error */ \
	}

/* Stack operations */

#define DUP(_x) \
	if (TS(_x).t % INT == 0) { \
		SP(_x)++; TS(_x).t = NS(_x).t; TS(_x).c = 0; TS(_x).v.i = NS(_x).v.i; \
	} else if (TS(_x).t % FLOAT == 0) { \
		SP(_x)++; TS(_x).t = NS(_x).t; TS(_x).c = 0; TS(_x).v.f = NS(_x).v.f; \
	} else { \
		/* TODO: Reference to object that needs to be cloned */ \
	}

/* External representation */

I ext(B* s, O* o) {
	I n;
	if (o->t % CHAR == 0) {
		n = sprintf(s, "'%c'", (int)o->v.i);
	} else if (o->t % NUMBER == 0 && o->t % INT == 0) {
		n = sprintf(s, "%ld", o->v.i);
	} else if (o->t % NUMBER == 0 && o->t % FLOAT == 0) {
		n = sprintf(s, "%g", o->v.f);
	} else if (o->t % STRING == 0) {
		n = sprintf(s, "\"%.*s\"", (int)o->c, (B*)o->v.r);
	}

	return n;
}

I dump_stack(B* s, X* x) {
	I i, n, t = 0;
	for (i = 0; i < SP(x); i++) {
		s += n = ext(s, &PK(x, i));
		*s++ = ' ';
		t += n + 1;
	}

	return t;
}

I dump_rstack(B* s, X* x) {
	I i;
	for (i = 0; *(IP(x) + i) != 0; i++) {
		*s++ = *(IP(x) + i);
	}

	return i;
}

I dump(B* s, X* x) {
	I n, t = 0;
	s += n = dump_stack(s, x);
	*s++ = ':';
	*s++ = ' ';
	t += n + 2;
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

#endif
