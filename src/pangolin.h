#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

typedef char B;
typedef intptr_t I;
typedef double F;

enum {
	ANY = 1,
	INT = 2,
	FLOAT = 3,
	MANAGED = 5,
	CHAR = 7,
	ARRAY = 11,
	I8 = 13,
	I16 = 17,
	I32 = 19,
	I64 = 23,
	STRING = 29,
  RETURN = 31
} T;

typedef struct { 
  I t;
  I c; 
  union { 
    I i; 
    F f; 
  } v; 
} O;

#define SETI(_o, _t, _c, _v)  (_o->t = _t, _o->c = _c, _o->v.i = _v, _o)
#define SETF(_o, _t, _c, _v)  (_o->t = _t, _o->c = _c, _o->v.f = _v, _o)

#define STACK_SIZE				128

typedef struct _X { 
	O s[STACK_SIZE]; 
	I sp, rp;
	B* ip; 
	I tr; 
	I err; 
} X;

typedef void (*FUNC)(struct _X*);

void P_inner(X*);

#define ERR_OK									0
#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_RSTACK_UNDERFLOW    -3
#define ERR_DIVIDE_BY_ZERO			-4
#define ERR_EXIT								-5

I error(X* x) {
	/* TODO: Find in exception stack a handler for current error */
	printf("ERROR: %ld\n", x->err);
	return 1;
}

#define ERR(_x, _c, _e)   if (_c) { _x->err = _e; if (error(_x)) return; }

#define OF1(_x)			      ERR(_x, _x->sp == _x->rp, ERR_STACK_OVERFLOW)
#define OF2(_x)			      ERR(_x, _x->sp >= _x->rp - 1, ERR_STACK_OVERFLOW)
#define UF1(_x)			      ERR(_x, _x->sp == 0, ERR_STACK_UNDERFLOW)
#define UF2(_x)			      ERR(_x, _x->sp <= 1, ERR_STACK_UNDERFLOW)
#define UF3(_x)			      ERR(_x, _x->sp <= 2, ERR_STACK_UNDERFLOW)
#define UF4(_x)			      ERR(_x, _x->sp <= 3, ERR_STACK_UNDERFLOW)
#define RUF1(_x)          ERR(_x, _x->rp < STACK_SIZE, ERR_RSTACK_UNDERFLOW)
#define DZ(_x)				    ERR(_x, TS(_x)->v.i == 0, ERR_DIVIDE_BY_ZERO)

#define DO(_x, _f)		    _f(_x); if (_x->err) { return; }

X* init() {
	X* x = malloc(sizeof(X));
	x->sp = x->err = x->tr = 0;
  x->tr = STACK_SIZE;
	x->ip = 0;
	return x;
}

#define PK(_x, _i)						(&_x->s[_i])

#define TS(_x)								(PK(_x, x->sp - 1))
#define NS(_x)								(PK(_x, x->sp - 2))

#define TR(_x)                (PK(_x, x->rp))

#define PUSH(_x, _v)					OF(_x); SETI(PK(_x, _x->sp++), INT, 0, (I)_v)
#define PUSHF(_x, _v)					OF(_x); SETF(PK(_x, _x->sp++), FLOAT, 0, (F)_v)

O* dispose(X* x) {
  O* o = TS(x);
  x->sp--;
  x->rp--;
  TR(x)->t = o->t;
  TR(x)->c = o->c;
  TR(x)->v.i = o->v.i;
  return TR(x);
}

I pop(X* x) { 
  O* o;
  I i;
  UF1(x);
  o = TS(x); 
  i = o->v.i; 
  if (o->t % MANAGED == 0) { 
    free((void*)o->v.i); 
  }
  return i;
}

#define POPF(_x)							(UF1(_x), PK(_x, --_x->sp).v.f)

#define PUSHR(_x, _i)					SETI(PK(_x, _x->rp--), RETURN, 0, (I)_i)

B* popr(X* x) {
  O* o;
  while (x->rp < STACK_SIZE) {
    o = PK(x, x->rp);
    if (o->t % RETURN == 0) {
      B* b = (B*)o->v.i;
      x->rp++;
      return b;
    } else {
      if (o->t % MANAGED == 0) {
        free((void*)o->v.i);
      }
      x->rp++;
    }
  }
}

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
		if (nl) { s += t = sprintf(s, "[%c] ", PK(x, i)->t % MANAGED == 0 ? 'M' : ' '); n += t; }
		if (nl) { s += t = sprintf(s, "%08X ", (unsigned int)PK(x, i)->v.i); n += t; }
		s += t = dump_o(s, PK(x, i)); 
		*s++ = nl ? '\n' : ' '; 
		n += t + 1; 
	}
	return n;
}

#define DUMP_CODE(op) for (i = 0; *(op + i) != 0 && t > 0; i++) { n++; *s++ = *(op + i); if (*(op + i) == '[') t++; else if (*(op + i) == ']') t--; }
#define SEPARATOR	*s++ = ' '; *s++ = ':'; *s++ = ' '; n += 3;

I dump_rstack(B* s, X* x) {
	I i, j, t = 1, n = 0;
	DUMP_CODE(x->ip);
	for (j = STACK_SIZE - 1; j >= x->rp; j--) { SEPARATOR; t = 1; DUMP_CODE((B*)PK(x, j)->v.i); }

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

	return t;
}

/* Arithmetic operations */
void P_add(X* x) { /* ( n n -- n ) */ NS(x)->v.i += TS(x)->v.i; x->sp--; }
void P_sub(X* x) { /* ( n n -- n ) */ NS(x)->v.i -= TS(x)->v.i; x->sp--; }
void P_mul(X* x) { /* ( n n -- n ) */ NS(x)->v.i *= TS(x)->v.i; x->sp--; }
void P_div(X* x) { /* ( n n -- n ) */ NS(x)->v.i /= TS(x)->v.i; x->sp--; }
void P_mod(X* x) { /* ( n n -- n ) */ NS(x)->v.i %= TS(x)->v.i; x->sp--; }

/* Comparison operations */
void P_lt(X* x) { /* ( n n -- n ) */ NS(x)->v.i = NS(x)->v.i < TS(x)->v.i; x->sp--; }
void P_eq(X* x) { /* ( n n -- n ) */ NS(x)->v.i = NS(x)->v.i == TS(x)->v.i; x->sp--; }
void P_gt(X* x) { /* ( n n -- n ) */ NS(x)->v.i = NS(x)->v.i > TS(x)->v.i; x->sp--; }

void P_not(X* x) { /* ( n -- n ) */ TS(x)->v.i = !TS(x)->v.i; }

/* Stack operations */
/* TODO: Do I need to clone arrays here or just duplicate its address? */
/* Only managed items can not be duplicated because if one its dropped the address will be
   freed */
void P_dup(X* x) { /* ( a -- a a ) */ 
	if (TS(x)->t % ARRAY == 0) {
		I sz;
    void* a;
		if (TS(x)->t % CHAR == 0 || TS(x)->t % I8 == 0) sz = 1;
		else if (TS(x)->t % I16 == 0) sz = 2;
		else if (TS(x)->t % I32 == 0) sz = 4;
		else if (TS(x)->t % I64 == 0) sz = 8;
		a = malloc(TS(x)->c * sz);
		PUSH(x, a);
		TS(x)->t = NS(x)->t % MANAGED == 0 ? NS(x)->t : NS(x)->t * MANAGED;
		TS(x)->c = NS(x)->c;
		strncpy((B*)TS(x)->v.i, (B*)NS(x)->v.i, NS(x)->c * sz);
	} else {
		I i = TS(x)->v.i; PUSH(x, i); TS(x)->t = NS(x)->t; TS(x)->c = NS(x)->c;
	}
}
void P_swap(X* x) { /* ( a b -- b a ) */ 
	I t = TS(x)->v.i; TS(x)->v.i = NS(x)->v.i; NS(x)->v.i = t; 
	t = TS(x)->t; TS(x)->t = NS(x)->t; NS(x)->t = t;
	t = TS(x)->c; TS(x)->c = NS(x)->c; NS(x)->c = t;
}

/* Execution */
/* TODO: This will not work with managed blocks because the block will be popped out before
   its executed!!!! */
/* This should be changed to O o = init from TS; CALL(...1); inner(x); free(o) if needed. */
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
	B op;
	I r = x->rp;
	while (*x->ip != 0 && x->err == ERR_OK) {
		if (x->tr) {
			memset(buf, 0, sizeof buf);
			dump(buf, x);
			printf("%s", buf);
		}
		switch (*x->ip) {
			case '$': UF1(x); x->tr = pop(x); break;
			case 'q': x->err = ERR_EXIT; return; break;
			case '0': PUSH(x, 0); break;
			case '1': PUSH(x, 1); break;
			case '#': P_number(x); break;
			case '+': UF2(x); P_add(x); break;
			case '-': UF2(x); P_sub(x); break;
			case '*': UF2(x); P_mul(x); break;
			case '/': UF2(x); DZ(x); P_div(x); break;
			case '%': UF2(x); P_mod(x); break;
			case '<': UF2(x); P_lt(x); break;
			case '=': UF2(x); P_eq(x); break;
			case '>': UF2(x); P_gt(x); break;
			case '!': UF1(x); P_not(x); break;
			case 'd': UF1(x); OF1(x); P_dup(x); break;
			case 's': UF2(x); P_swap(x); break;
			case '\\': UF1(x); DROP(x); break;
			case 'i': UF1(x); P_exec_i(x); break;
			case '[': 
				OF1(x); 
				PUSH(x, x->ip + 1); 
				TS(x)->t *= I8*ARRAY;
				TS(x)->c = P_forward(x, '[', ']'); 
				break;
			case ']': 
        if (x->rp < r) { 
          x->ip = popr(x); 
        } else { 
          if (x->rp < STACK_SIZE) { 
            x->ip = popr(x); 
          } 
          return; 
        } 
        break;
			case '?': UF2(x); P_ifthen(x, pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 'l': UF3(x); P_linrec(x, (B*)pop(x), (B*)pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 'b': UF3(x); P_binrec(x, (B*)pop(x), (B*)pop(x), (B*)pop(x), (B*)pop(x)); break;
			case 't': UF2(x); P_times(x, pop(x), (B*)pop(x)); break;
			case 'w': UF2(x); P_while(x, (B*)pop(x), (B*)pop(x)); break;
			case '\'': OF1(x); PUSH(x, *++x->ip); TS(x)->t *= CHAR; break;
			case '"': 
				OF1(x); 
				PUSH(x, x->ip + 1); 
				TS(x)->t *= I8*ARRAY*STRING; 
				TS(x)->c = P_forward(x, 0, '"'); 
				break;
      /*
			default:
				op = *x->ip;
				if (op >= 'A' && op <= 'Z') {
					DO(x, x->ext[op - 'A']);
				}
      */
		}
		x->ip++;
	} 
}

void P_repl(X* x) {
	B buf[255];

	do {
		printf("IN: ");
		fgets(buf, 255, stdin);
		x->ip = buf;
		P_inner(x);
		if (!x->tr) {
			memset(buf, 0, sizeof buf);
			dump_stack(buf, x, 1);
			printf("\n--- Data stack\n%s", buf);
		}
		if (x->err == ERR_EXIT) { return; }
		if (x->err != ERR_OK) {
			printf("ERROR: %ld\n", x->err);
			x->err = ERR_OK;
		}
	} while (1);
}

#endif
