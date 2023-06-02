#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#ifndef Pmalloc
#define Pmalloc(n) malloc(n)
#endif

#ifndef Pfree
#define Pfree(p) free(p)
#endif

typedef char B;
typedef intptr_t I;
typedef double F;

enum { ANY = 1, INT = 2, FLOAT = 3, MANAGED = 5, ARRAY = 7, I8 = 11, I16 = 13, I32 = 17, I64 = 19, RETURN = 23 } T;

typedef struct { I t; I c; union { I i; F f; } v; } O;

#define SETI(_o, _t, _c, _v)  (_o->t = _t, _o->c = _c, _o->v.i = _v, _o)
#define SETF(_o, _t, _c, _v)  (_o->t = _t, _o->c = _c, _o->v.f = _v, _o)

#define STACK_SIZE 128
#define RSTACK_SIZE 256

typedef struct _X { 
	O s[STACK_SIZE]; 
	I sp;
  O r[RSTACK_SIZE];
  I rp;
	B* ip; 
	I tr; 
	I err;
  void (*k)(struct _X*);
  void (*e)(struct _X*);
  void (*ex[26])(struct _X*);
} X;

typedef void (*FUNC)(X*);

#define KEY(x) (x->k(x))
#define EMIT(x) (x->e(x))

#define ERROR(x) (x->err)

#define IP(X) (x->ip)

#define SP(x) (x->sp)
#define DEPTH(x) SP(x)
#define MAX_DEPTH(x) (STACK_SIZE - 1)

#define PK(x, i) (&x->s[i])
#define TS(x) (PK(x, SP(x) - 1))
#define NS(x)	(PK(x, SP(x) - 2))

#define RP(x) (x->rp)
#define RDEPTH(x) RP(x)
#define RMAX_DEPTH(x) (RSTACK_SIZE - 1)

#define RPK(x, i) (&x->r[i])
#define TR(x) (RPK(x, RP(x) - 1))

void P_inner(X*);

#define ERR_OK									0
#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_RSTACK_OVERFLOW     -3
#define ERR_RSTACK_UNDERFLOW    -4
#define ERR_DIVIDE_BY_ZERO			-5
#define ERR_STRING_EXPECTED     -6
#define ERR_ALLOCATION          -7
#define ERR_EXIT								-8

I handle(X* x, I err) {
  ERROR(x) = err;
	return 1;
}

#ifndef ERR
#define ERR(x, c, e, b)   if (c) { if (handle(x, e)) { return; } else b } else b
#endif

#define OF1(x, b) ERR(x, SP(x) == MAX_DEPTH(x), ERR_STACK_OVERFLOW, b)
#define OF2(x, b) ERR(x, SP(x) >= MAX_DEPTH(x) - 1, ERR_STACK_OVERFLOW, b)
#define UF1(x, b) ERR(x, SP(x) == 0, ERR_STACK_UNDERFLOW, b)
#define UF2(x, b) ERR(x, SP(x) <= 1, ERR_STACK_UNDERFLOW, b)
#define UF3(x, b) ERR(x, SP(x) <= 2, ERR_STACK_UNDERFLOW, b)
#define UF4(x, b) ERR(x, SP(x) <= 3, ERR_STACK_UNDERFLOW, b)
#define ROF1(x, b) ERR(x, RP(x) == RMAX_DEPTH(x), ERR_RSTACK_OVERFLOW, b)
#define RUF1(x, b) ERR(x, RP(x) == 0, ERR_RSTACK_UNDERFLOW, b)
#define DZ(x, b) ERR(x, TS(x)->v.i == 0, ERR_DIVIDE_BY_ZERO, b)

#define DO(x, f) f(x); if (ERROR(x)) { return; }
#define DO1(x, f, a) f(x, a); if (ERROR(x)) { return; }
#define DO2(x, f, a, b) f(x, a, b); if (ERROR(x)) { return; }

#define PUSH(x, v) OF1(x, { SP(x)++; SETI(TS(x), INT, 0, (I)v); })
#define PUSHF(x, v) OF1(x, { SP(x)++; SETF(TS(x), FLOAT, 0, (F)v); })
#define PUSHM(x, p) OF1(x, { SP(x)++; SETI(TS(x), INT*MANAGED, 0, (I)p); })

O* TO_R(X* x) {
  O* s, * d;
  UF1(x, { 
    ROF1(x, {
      x->rp++;
      s = TS(x);
      d = TR(x);
      d->t = s->t;
      d->c = s->c;
      d->v.i = s->v.i;
      x->sp--;
      return d;
    })
  })
}

I POP(X* x) { 
  O* o;
  I i;
  UF1(x, {
    o = TS(x);
    i = o->v.i;
    SP(x)--;
    if (o->t % MANAGED == 0) { 
      Pfree((void*)o->v.i);
      return 0;
    } else {
      return i;
    }
  })
}

#define POPF(x) UF1(x, { PK(x, --SP(x))->v.f); })

#define RPUSH(x, v) ROF1(x, { RP(x)++; SETI(TR(x), RETURN, 0, (I)v); })

B* RPOP(X* x) {
  O* o;
  RUF1(x, {
    while (RP(x) > 0) {
      o = TR(x);
      if (o->t % RETURN == 0) {
        B* b = (B*)o->v.i;
        RP(x)--;
        return b;
      } else {
        if (o->t % MANAGED == 0) {
          Pfree((void*)o->v.i);
        }
        RP(x)--;
      }
    }
  })
}

#define CALL(x, d, t) if (t || !(*(x->ip + 1) == 0 || *(x->ip + 1) == ']')) { RPUSH(x, x->ip); } x->ip = d

X* init() {
	X* x = malloc(sizeof(X));
	x->sp = x->err = x->tr = 0;
  x->tr = STACK_SIZE;
	x->ip = 0;
	return x;
}

/* External representation */

#define DUMP_CODE(op) for (i = 0; *(op + i) != 0 && t > 0; i++) { n++; *s++ = *(op + i); if (*(op + i) == '[') t++; else if (*(op + i) == ']') t--; }

I dump_o(B* s, O* o) {
	if (o->t % ARRAY == 0) {
		I i, n = 0, t;
		if (o->t % I8 == 0) {
			I r = 1; 
			for (i = 0; i < o->c; i++) {
				if (((B*)o->v.i)[i] < 31 || ((B*)o->v.i)[i] > 126) { r = 0; }
			}
			if (r) {
				return sprintf(s, "[%.*s]", (unsigned int)o->c, (B*)o->v.i);
			} else {
				s += t = sprintf(s, "["); n += t;
				for (i = 0; i < o->c; i++) {
					s += t = sprintf(s, "%d ", ((B*)o->v.i)[i]); n += t;
				}
				s += t = sprintf(s, "]"); n += t;
				return n;
			}
		}
	}
  else if (o->t % RETURN == 0) { I i, t = 1, n = 0; DUMP_CODE((B*)o->v.i); return n; }
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

#define SEPARATOR	*s++ = ' '; *s++ = ':'; *s++ = ' '; n += 3;

I dump_rstack(B* s, X* x) {
  I i, j, t = 1, n = 0;
  DUMP_CODE(x->ip);
  for (j = RDEPTH(x) - 1; j >= 0; j--) {
    if (RPK(x, j)->t % RETURN == 0) {
      SEPARATOR; t = 1; DUMP_CODE((B*)RPK(x, j)->v.i);
    }
  }

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
void P_add(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i += TS(x)->v.i; x->sp--; }); }
void P_sub(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i -= TS(x)->v.i; x->sp--; }); }
void P_mul(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i *= TS(x)->v.i; x->sp--; }); }
void P_div(X* x) { /* ( n n -- n ) */ UF2(x, { DZ(x, { NS(x)->v.i /= TS(x)->v.i; x->sp--; }); }); }
void P_mod(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i %= TS(x)->v.i; x->sp--; }); }

/* Comparison operations */
void P_lt(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i = NS(x)->v.i < TS(x)->v.i; x->sp--; }); }
void P_eq(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i = NS(x)->v.i == TS(x)->v.i; x->sp--; }); }
void P_gt(X* x) { /* ( n n -- n ) */ UF2(x, { NS(x)->v.i = NS(x)->v.i > TS(x)->v.i; x->sp--; }); }

void P_and(X* x) { /* ( n n -- b ) */ UF2(x, { NS(x)->v.i = NS(x)->v.i & TS(x)->v.i; x->sp--; }); }
void P_or(X* x) { /* ( n n -- b ) */ UF2(x, { NS(x)->v.i = NS(x)->v.i | TS(x)->v.i; x->sp--; }); }
void P_not(X* x) { /* ( n -- n ) */ UF1(x, { TS(x)->v.i = !TS(x)->v.i; }); }

/* Stack operations */
void P_dup(X* x) { /* ( a -- a a ) */ 
  UF1(x, {
    OF1(x, {
	    if (TS(x)->t % ARRAY == 0) {
	      I sz;
        void* a;
	      if (TS(x)->t % I8 == 0) sz = 1;
	      else if (TS(x)->t % I16 == 0) sz = 2;
	      else if (TS(x)->t % I32 == 0) sz = 4;
	      else if (TS(x)->t % I64 == 0) sz = 8;
	      a = Pmalloc(TS(x)->c * sz);
	      PUSH(x, a);
	      TS(x)->t = NS(x)->t % MANAGED == 0 ? NS(x)->t : NS(x)->t * MANAGED;
	      TS(x)->c = NS(x)->c;
	      strncpy((B*)TS(x)->v.i, (B*)NS(x)->v.i, NS(x)->c * sz);
	    } else {
	      I i = TS(x)->v.i; PUSH(x, i); TS(x)->t = NS(x)->t; TS(x)->c = NS(x)->c;
	    }
    });
  });
}
void P_swap(X* x) { /* ( a b -- b a ) */ 
  UF2(x, {
	  I t = TS(x)->v.i; TS(x)->v.i = NS(x)->v.i; NS(x)->v.i = t; 
	  t = TS(x)->t; TS(x)->t = NS(x)->t; NS(x)->t = t;
	  t = TS(x)->c; TS(x)->c = NS(x)->c; NS(x)->c = t;
  });
}
void P_over(X* x) { /* ( a b -- a b a ) */
  UF2(x, {
    /* TODO: Arrays must be cloned !! */
    I i = NS(x)->v.i;
    PUSH(x, i);
  })
}
/* TODO: Implement and rot */

/* Execution */
void P_exec_i(X* x) { /* ( [P] -- P ) */ B* q = (B*)TO_R(x)->v.i; CALL(x, q - 1, 0); }

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
		if (!POP(x)) { return; } 
		CALL(x, q, 1); DO(x, P_inner);
	} while(1); 
}

/* Recursion operations */
void P_linrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i, 1); DO(x, P_inner);
	if (POP(x)) { CALL(x, t, 1); DO(x, P_inner); }
	else {
		CALL(x, r1, 1); DO(x, P_inner);
		P_linrec(x, i, t, r1, r2);
		CALL(x, r2, 1); DO(x, P_inner);
	}
}

void P_binrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i, 1); P_inner(x);
	if (POP(x)) { CALL(x, t, 1); P_inner(x); } 
	else {
		CALL(x, r1, 1); P_inner(x);
		P_binrec(x, i, t, r1, r2);
		P_swap(x);
		P_binrec(x, i, t, r1, r2);
		CALL(x, r2, 1); P_inner(x);
	}
}

/* Arrays */
void P_map(X* x, B* q) {
  OF1(x, {
    B* b = (B*)TS(x)->v.i;
    I i;
    I l = TS(x)->c;
    /* Let's assume I8 */
    for (i = 0; i < l; i++) {
      PUSH(x, b[i]);
      CALL(x, q, 1); DO(x, P_inner);
      b[i] = (B)POP(x);
    }
  });
}

void P_zip(X* x, B* q) {
  OF2(x, {
    B* a = (B*)NS(x)->v.i;
    B* b = (B*)TS(x)->v.i;
    I i;
    I l = NS(x)->c;
    for (i = 0; i < l; i++) {
      PUSH(x, a[i]);
      PUSH(x, b[i]);
      CALL(x, q, 1); DO(x, P_inner);
      a[i] = (B)POP(x);
    }
    POP(x);
  });
}

void P_fold(X* x, B* q) {
	OF1(x, {
		O* o = TS(x);
		B* a = (B*)o->v.i;
		I i;
		PUSH(x, a[0]);
		for (i = 1; i < o->c; i++) {
			PUSH(x, a[i]);
			CALL(x, q, 1); DO(x, P_inner);
		}
		DO(x, P_swap);
		POP(x);
	});
}

/* Input/output */
void P_print(X* x) {
  O* s = TO_R(x);
  I i;
  for (i = 0; i < s->c; i++) {
    PUSH(x, ((B*)s->v.i)[i]);
    EMIT(x);
  }
}

void P_read(X* x) {
  B* s;
  I c, i;
  OF1(x, {
    PUSH(x, 0);
    do {
      DO(x, KEY);
      DO(x, P_dup);
      DO(x, EMIT);
      if (TS(x)->v.i == 10) {
        POP(x);
        c = POP(x);
        s = (B*)Pmalloc(c);
        for (i = c - 1; i >= 0; i--) {
          s[i] = (B)POP(x); 
        } 
        PUSH(x, s);
				TS(x)->c = c;
        TS(x)->t *= I8*ARRAY*MANAGED;
        return;
      } else {
        DO(x, P_swap);
        TS(x)->v.i++;
      }
    } while(1);
  });
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
			case '$': UF1(x, { x->tr = POP(x); }); break;
			case 'q': x->err = ERR_EXIT; return; break;
			case '0': PUSH(x, 0); break;
			case '1': PUSH(x, 1); break;
			case '#': P_number(x); break;
      case 'k': DO(x, KEY); break;
      case 'e': DO(x, EMIT); break;
      case 'p': P_print(x); break;
      case 'r': P_read(x); break;
			case '+': P_add(x); break;
			case '-': P_sub(x); break;
			case '*': P_mul(x); break;
			case '/': P_div(x); break;
			case '%': P_mod(x); break;
			case '<': P_lt(x); break;
			case '=': P_eq(x); break;
			case '>': P_gt(x); break;
      case '&': DO(x, P_and); break;
      case '|': DO(x, P_or); break;
			case '!': P_not(x); break;
			case 'd': P_dup(x); break;
			case 's': P_swap(x); break;
      case 'o': P_over(x); break;
      /* TODO: Implement rot */
			case '\\': POP(x); break;
			case 'i': P_exec_i(x); break;
			case '[': 
				OF1(x, { 
				  PUSH(x, IP(x) + 1); 
				  TS(x)->t *= I8*ARRAY;
				  TS(x)->c = P_forward(x, '[', ']'); 
        });
				break;
			case ']': 
        if (RP(x) < r) { 
          IP(x) = RPOP(x); 
        } else { 
          if (RP(x) < STACK_SIZE) { 
            IP(x) = RPOP(x); 
          } 
          return; 
        } 
        break;
			case '?': P_ifthen(x, POP(x), (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
			case 'l': P_linrec(x, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
			case 'b': P_binrec(x, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
			case 't': P_times(x, POP(x), (B*)TO_R(x)->v.i); break;
			case 'w': P_while(x, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
      case 'm': DO1(x, P_map, (B*)TO_R(x)->v.i); break;
      case 'z': DO1(x, P_zip, (B*)TO_R(x)->v.i); break;
			case 'f': DO1(x, P_fold, (B*)TO_R(x)->v.i); break;
			case '\'': OF1(x, { PUSH(x, *++x->ip); }); break;
			/*
			case '"': 
				OF1(x, {
				  PUSH(x, x->ip + 1); 
				  TS(x)->t *= I8*ARRAY*STRING; 
				  TS(x)->c = P_forward(x, 0, '"'); 
        });
        break;
			*/
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
    memset(buf, 0, sizeof buf);
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
