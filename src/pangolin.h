#ifndef PANGOLIN_H
#define PANGOLIN_H

#include<stdint.h>
#include<stdlib.h>

#ifndef Pmalloc
#define Pmalloc(c) (malloc(c))
#endif

#ifndef Prealloc
#define Prealloc(b, c) (realloc(b, c))
#endif

#ifndef Pfree
#define Pfree(p) (free(p))
#endif

/* DATA TYPES */

typedef char B;
typedef intptr_t I;
typedef double F;

enum {
  INT = 2,
  FLOAT = 3,
  MANAGED = 5,
  ARRAY = 7,
  I8 = 11,
  I16 = 13,
  I32 = 17,
  I64 = 19,
  F32 = 23,
  F64 = 29,
  OBJECT = 31,
  STRUCT = 37,
  RET_ADDR = 41
} TYPE;

typedef struct {
  I type;
  I capacity;
  I size;
  union {
    I integer;
    F real;
    void* pointer;
  } value;
} O;

#define T(o) ((o)->type)

#define IS(o, t) (T(o) % t == 0)
#define ARE(o1, t1, o2, t2) (IS(o1, t1) && IS(o2, t2))
#define BOTH(o1, o2, t) (ARE(o1, t, o2, t))

#define C(o) ((o)->capacity)
#define S(o) ((o)->size)
#define I(o) ((o)->value.integer)
#define F(o) ((o)->value.real)
#define P(o) ((o)->value.pointer)

#define PB(o) ((B*)P(o))
#define PI(o) ((I*)P(o))
#define PF(o) ((F*)P(o))
#define PO(o) ((O*)P(o))

#define Bat(o, i) (PB(o)[i])
#define Iat(o, i) (PI(o)[i])
#define Fat(o, i) (PF(o)[i])
#define Oat(o, i) (PO(o) + i)

O* setO(O* o, I t, I c, I s) { T(o) = t; C(o) = c; S(o) = s; return o; }
O* setOI(O* o, I t, I v) { I(setO(o, t, 0, 0)) = v; return o; }
O* setOF(O* o, I t, F v) { F(setO(o, t, 0, 0)) = v; return o; }
O* setOP(O* o, I t, I c, I s, void* p) { P(setO(o, t, c, s)) = p; return o; }

O* newO() { return Pmalloc(sizeof(O)); }
void freeO(O* o) { if (IS(o, MANAGED)) { Pfree(P(o)); } Pfree(o); }

O* newL(O* o, I n) {
  if (o == 0) {
    o = Pmalloc(sizeof(O));
    if (o == 0) return 0;
    P(o) = Pmalloc(n*sizeof(O));
    if (P(o) == 0) {
      Pfree(o);
      return 0;
    }
  } else {
    P(o) = Pmalloc(n*sizeof(O));
    if (P(o) == 0) return 0;
  }
  T(o) = INT*OBJECT*ARRAY;
  C(o) = n;
  S(o) = 0;
  return o;
}

#define STACK_SIZE 128
#define RSTACK_SIZE 256

typedef struct _X { 
	O* s;
  O* r;
	B* ip; 
	I tr; 
	I err;
  void (*k)(struct _X*);
  void (*e)(struct _X*);
  void (*ex[26])(struct _X*);
} X;

typedef void (*FUNC)(X*);

X* init() {
	X* x = malloc(sizeof(X)); if (x == 0) return 0;
  x->s = newL(0, STACK_SIZE); if (x->s == 0) { Pfree(x); return 0; }
  x->r = newL(0, RSTACK_SIZE); if (x->r == 0) { Pfree(x->s); Pfree(x); return 0; }
	x->err = x->tr = 0;
	x->ip = 0;
	return x;
}

#define KEY(x) (x->k(x))
#define EMIT(x) (x->e(x))

#define IP(x) (x->ip)
#define PEEK_TOKEN(x) (x->ip == 0 ? 0 : *(x->ip))
#define GET_TOKEN(x) (*(x->ip++))
#define IS_DIGIT(c) (c != 0 && c >= '0' && c <= '9')

#define TRACE(x) (x->tr)
#define ERROR(x) (x->err)

#define SP(x) (S(x->s))

#define TS(x) (Oat(x->s, SP(x) - 1))
#define NS(x)	(Oat(x->s, SP(x) - 2))
#define NNS(x) (Oat(x->s, SP(x) - 3))

#define RP(x) (S(x->r))

#define TR(x) (Oat(x->r, RP(x) - 1))

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
#define ERR_END_OF_CODE         -9

I handle(X* x, I err) {
  ERROR(x) = err;
	return 1;
}

#ifndef ERR
#define ERR(x, c, e, b) if (c) { if (handle(x, e)) { return; } else b } else b
#endif

#define OF(x, n, b) ERR(x, SP(x) + n >= STACK_SIZE, ERR_STACK_OVERFLOW, b)
#define UF(x, n, b) ERR(x, SP(x) - n < 0, ERR_STACK_UNDERFLOW, b)

#define R_OF(x, n, b) ERR(x, RP(x) + n >= RSTACK_SIZE, ERR_RSTACK_OVERFLOW, b)
#define R_UF(x, n, b) ERR(x, RP(x) - n < 0, ERR_RSTACK_UNDERFLOW, b)

#define DZ(x, b) ERR(x, I(TS(x)) == 0, ERR_DIVIDE_BY_ZERO, b)

#define DO(x, f) f; if (ERROR(x)) { return; }

#define PUSHI(x, t, v) setOI(Oat(x->s, S(x->s)++), t, v)
#define PUSHF(x, t, v) setOF(Oat(x->s, S(x->s)++), t, v)
#define PUSHP(x, t, c, sz, p) setOP(Oat(x->s, S(x->s)++), t, c, sz, p)

void DROP(X* x) { 
  if (IS(TS(x), MANAGED)) { /* Free recursively */ }
  SP(x)--;
}

I POPI(X* x) { I v = I(TS(x)); DROP(x); return v; }
F POPF(X* x) { F v = F(TS(x)); DROP(x); return v; }

#define R_PUSH(x, r) setOI(Oat(x->r, S(x->r)++), RET_ADDR, (I)r)

void R_DROP(X* x) {
  if (IS(TR(x), MANAGED)) { /* Free recursively */ }
  RP(x)--;
}

I R_POPI(X* x) { I v = I(TR(x)); R_DROP(x); return v; }

void RETURN(X* x, I r) {
  while (PEEK_TOKEN(x) == 0) {
    if (RP(x) == r || RP(x) == 0) { ERROR(x) = ERR_END_OF_CODE; return; }
    else if (IS(TR(x), RET_ADDR)) { IP(x) = (B*)R_POPI(x); return; }
    else { R_DROP(x); }
  }
}

/*
O* TO_R(X* x) {
  O* s, * d;
  UF1(x, { 
    ROF1(x, {
      RP(x)++;
      s = TS(x);
      d = TR(x);
      T(d) = T(s);
      C(d) = C(s);
      I(t) = I(s);
      SP(x)--;
      return d;
    })
  })
}

I POP(X* x) { 
  O* o;
  I i;
  UF1(x, {
    o = TS(x);
    i = I(o);
    DROP(x);
    return i;
  })
}

#define POPF(x) UF1(x, { LPOPF(x->s); })

#define RPUSH(x, v) ROF1(x, { LPUSHI(x->r, v); T(TR(x)) *= RETURN; })

B* RPOP(X* x, I r) {
  O* o;
  while (RP(x) > r) {
    o = TR(x);
    if (o->t % RETURN == 0) {
      return LPOPI(x->r);
    } else {
      DROP(x->r);
    }
  }
  return 0;
}

#define CALL(x, d) if (IP(x) != 0 && *(IP(x) + 1) != 0 && *(IP(x) + 1) != ']') { RPUSH(x, IP(x)); } IP(x) = d
*/
/* External representation */
/*
#define DUMP_CODE(op) for (i = 0; *(op + i) != 0 && *(op + i) != 10 && t > 0; i++) { n++; *s++ = *(op + i); if (*(op + i) == '[') t++; else if (*(op + i) == ']') t--; }

I dump_o(B* s, O* o) {
	if (T(o) % ARRAY == 0) {
		I i, n = 0, t;
		if (T(o) % I8 == 0) {
			I r = 1; 
			for (i = 0; i < C(o); i++) {
				if (aB(o)[i] < 31 || aB(o)[i] > 126) { r = 0; }
			}
			if (r) {
				return sprintf(s, "[%.*s]", (unsigned int)C(o), aB(o);
			} else {
				s += t = sprintf(s, "["); n += t;
				for (i = 0; i < C(o); i++) {
					s += t = sprintf(s, "%d ", aB(o)[i]); n += t;
				}
				s += t = sprintf(s, "]"); n += t;
				return n;
			}
		} else if (T(o) % I64 == 0) {
			s += t = sprintf(s, "["); n += t;
			for (i = 0; i < C(o); i++) {
				s += t = sprintf(s, "%ld ", aI(o)[i]); n += t;
			}
			s += t = sprintf(s, "]"); n += t;
			return n;
		}
	}
  else if (T(o) % RETURN == 0) { 
    I i, t = 1, n = 0; 
    if (I(o) != 0) {
      DUMP_CODE(aB(o)); 
      return n; 
    }
  }
	else if (T(o) % INT == 0) { return sprintf(s, "%ld", I(o)); } 
	else if (T(o) % FLOAT == 0) { return sprintf(s, "%g", F(o)); }
}

I dump_stack(B* s, X* x, I nl) {
	I i, t, n = 0;
	for (i = 0; i < SP(x); i++) { 
		if (nl) { s += t = sprintf(s, "[%c] ", T(PK(x, i)) % MANAGED == 0 ? 'M' : ' '); n += t; }
		if (nl) { s += t = sprintf(s, "%08X ", (unsigned int)I(PK(x, i))); n += t; }
		s += t = dump_o(s, PK(x, i)); 
		*s++ = nl ? '\n' : ' '; 
		n += t + 1; 
	}
	return n;
}

#define SEPARATOR	*s++ = ' '; *s++ = ':'; *s++ = ' '; n += 3;

I dump_rstack(B* s, X* x) {
  I i, j, t = 1, n = 0;
  if (IP(x)) {
    DUMP_CODE(IP(x));
  }
  for (j = RDEPTH(x); j > 0; j--) {
    SEPARATOR;
    if (T(RPK(x, j - 1)) % RETURN != 0) {
      *s++ = '('; n++;
    }
    s += t = dump_o(s, RPK(x, j - 1));
    n += t;
    if (T(RPK(x, j - 1)) % RETURN != 0) {
      *s++ = ')'; n++;
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
	n += t;

	return t;
}
void P_add(X* x) { UF2(x, { I(NS(x)) += I(TS(x)); SP(x)--; }); }
void P_sub(X* x) { UF2(x, { I(NS(x)) -= I(TS(x)); SP(x)--; }); }
void P_mul(X* x) { UF2(x, { I(NS(x)) *= I(TS(x)); SP(x)--; }); }
void P_div(X* x) { UF2(x, { DZ(x, { I(NS(x)) /= I(TS(x)); SP(x)--; }); }); }
void P_mod(X* x) { UF2(x, { I(NS(x)) %= I(TS(x)); SP(x)--; }); }
void P_lt(X* x) { UF2(x, { I(NS(x)) = I(NS(x)) < I(TS(x)); SP(x)--; }); }
void P_eq(X* x) { UF2(x, { I(NS(x)) = I(NS(x)) == I(TS(x)); SP(x)--; }); }
void P_gt(X* x) { UF2(x, { I(NS(x)) = I(NS(x)) > I(TS(x)); SP(x)--; }); }
void P_and(X* x) { UF2(x, { I(NS(x)) = I(NS(x)->v.i) & I(TS(x)); SP(x)--; }); }
void P_or(X* x) { UF2(x, { I(NS(x)) = I(NS(x)) | I(TS(x)); SP(x)--; }); }
void P_not(X* x) { UF1(x, { I(TS(x)) = !I(TS(x)); }); }
void P_dup(X* x) { 
  UF1(x, {
    OF1(x, {
	    if (T(TS(x)) % ARRAY == 0) {
	      I sz;
        void* a;
	      if (T(TS(x)) % I8 == 0) sz = 1;
	      else if (T{TS(x)) % I16 == 0) sz = 2;
	      else if (T(TS(x)) % I32 == 0) sz = 4;
	      else if (T(TS(x)) % I64 == 0) sz = 8;
	      a = Pmalloc(C(TS(x)) * sz);
	      PUSH(x, a);
	      T(TS(x)) = T(NS(x)) % MANAGED == 0 ? T(NS(x)) : T(NS(x)) * MANAGED;
	      C(TS(x)) = C(NS(x));
	      memcpy(aB(TS(x)), aB(NS(x), C(NS(x)) * sz);
	    } else {
	      I i = I(TS(x)); PUSH(x, i); T(TS(x)) = T(NS(x)); C(TS(x)) = C(NS(x));
	    }
    });
  });
}

void P_swap(X* x) { 
  UF2(x, {
	  I t = I(TS(x)); I(TS(x)) = I(NS(x)); I(NS(x)) = t; 
	  t = T(TS(x)); T(TS(x)) = T(NS(x)); T(NS(x)) = t;
	  t = C(TS(x)); C(TS(x)) = C(NS(x)); C(NS(x)) = t;
  });
}
void P_rot(X* x) { 
	UF3(x, {
		I t = I(TS(x)); I(TS(x)) = I(NNS(x)); I(NNS(x)) = I(NS(x)); I(NS(x)) = t;
		t = T(TS(x)); T(TS(x)) = T(NNS(x)); T(NNS(x)) = T(NS(x)); T(NS(x)) = t;
		t = C(TS(x)); C(TS(x)) = C(NNS(x)); C(NNS(x)) = C(NS(x)); C(NS(x)) = t;
	});
}

void P_over(X* x) {
  UF2(x, {
		if (T(NS(x)) % ARRAY == 0) {
			DO(x, P_swap);
			DO(x, P_dup);
			DO(x, P_rot);
			DO(x, P_rot);
		} else if (T(NS(x)) % INT == 0) {
			I i = I(NS(x));
	    PUSH(x, i);
			T(TS(x)) = T(NNS(x));
		} else if (T(NS(x)) % FLOAT == 0) {
			F f = F(NS(x));
			PUSHF(x, f);
			T(TS(x)) = T(NNS(x));
		}
  })
}

void P_exec_i(X* x) { B* q = aB(TO_R(x)); CALL(x, q - 1); }

void P_ifthen(X* x, I c, B* t, B* e) { CALL(x, (c ? t : e) - 1); }
void P_times(X* x, I t, B* q) { 
	for(;t > 0; t--) { 
		CALL(x, q); DO(x, P_inner);
	} 
}
void P_while(X* x, B* c, B* q) { 
	do { 
		CALL(x, c); DO(x, P_inner);
		if (!POP(x)) { return; } 
		CALL(x, q); DO(x, P_inner);
	} while(1); 
}

void P_linrec(X* x, B* i, B* t, B* r1, B* r2) {
	CALL(x, i); DO(x, P_inner);
	if (POP(x)) { CALL(x, t); DO(x, P_inner); }
	else {
		CALL(x, r1); DO(x, P_inner);
		P_linrec(x, i, t, r1, r2);
		CALL(x, r2); DO(x, P_inner);
	}
}

void P_binrec_r(X* x, B* c, B* t, B* r1, B* r2) {
	CALL(x, c); DO(x, P_inner);
	if (POP(x)) { CALL(x, t); DO(x, P_inner); } 
	else {
		CALL(x, r1); DO(x, P_inner);
		DO4(x, P_binrec_r, c, t, r1, r2);
		DO(x, P_swap);
		DO4(x, P_binrec_r, c, t, r1, r2);
		CALL(x, r2); DO(x, P_inner);
	}
}

void P_binrec(X* x) {
  OF4(x, {
    I r = RP(x);
    B* r2 = aB(TO_R(x));
    B* r1 = aB(TO_R(x));
    B* t = aB(TO_R(x));
    B* c = aB(TO_R(x));
    DO4(x, P_binrec_r, c, t, r1, r2);
  });
}

void P_count(X* x, I a, I b, I s) {
  OF1(x, {
    I c = (b - a)/s + 1;
    I i;
    I j = 0; 
    I* p = Pmalloc(c * sizeof(I));
    if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
    PUSHM(x, p);
    T(TS(x)) *= I64;
    C(TS(x)) = c;
    for (i = a; i <= b; i += s) {
      p[j++] = i;
    }
  });
}

void P_shape(X* x) {
	UF2(x, {
		I s = POP(x);
		I n = POP(x);
		I* a = Pmalloc(s*sizeof(I));
		I i;
		if (a == 0) { ERROR(x) = ERR_ALLOCATION; return; }
		for (i = 0; i < s; i++) {
			a[i] = n;
		}
		PUSHM(x, a);
		T(TS(x)) *= I64;
		C(TS(x)) = s;
	});
}

void P_map(X* x) {
  OF2(x, {
    B* q = aB(TO_R(x));
    I i;
    I l = C(TS(x));
		if (T(TS(x)) % I8 == 0) {
	    B* b = aB(TS(x));
	    for (i = 0; i < l; i++) {
	      PUSH(x, b[i]);
	      CALL(x, q); DO(x, P_inner);
	      b[i] = (B)POP(x);
	    }
		} else if (T(TS(x)) % I64 == 0) {
	    I* b = aI(TS(x));
	    for (i = 0; i < l; i++) {
	      PUSH(x, b[i]);
	      CALL(x, q); DO(x, P_inner);
	      b[i] = POP(x);
	    }
		}
  });
}

void P_zip(X* x, B* q) {
  OF2(x, {
    I i;
    I l = NS(x)->c;
		if (T(TS(x)) % I8 == 0 && T(NS(x)) % I8 == 0) {
			B* a = aB(NS(x));
    	B* b = aB(TS(x));
    	for (i = 0; i < l; i++) {
    	  PUSH(x, a[i]);
    	  PUSH(x, b[i]);
    	  CALL(x, q); DO(x, P_inner);
    	  a[i] = (B)POP(x);
    	}
    	POP(x);
		} else if (T(TS(x)) % I64 == 0 && T(NS(x)) % I64 == 0) {
			I* a = aI(NS(x));
    	I* b = aI(TS(x));
    	for (i = 0; i < l; i++) {
    	  PUSH(x, a[i]);
    	  PUSH(x, b[i]);
    	  CALL(x, q); DO(x, P_inner);
    	  a[i] = POP(x);
    	}
    	POP(x);
		}
  });
}

void P_left_fold(X* x, I v) {
	OF2(x, {
		B* q = aB(TO_R(x));
		O* o = TO_R(x);
		I i;
		if (T(o) % I8 == 0) {
			B* a = aB(o);
			if (!v) PUSH(x, a[0]);
			for (i = v ? 0 : 1; i < o->c; i++) {
				PUSH(x, a[i]);
				CALL(x, q); DO(x, P_inner);
			}
		} else if (T(o) % I64 == 0) {
			I* a = aI(o);
			if (!v) PUSH(x, a[0]);
			for (i = v ? 0 : 1; i < o->c; i++) {
				PUSH(x, a[i]);
				CALL(x, q); DO(x, P_inner);
			}
		}
	});
}

void P_right_fold(X* x, I v) {
	OF2(x, {
		B* q = aB(TO_R(x));
		O* o = TO_R(x);
		I i;
		if (T(o) % I8 == 0) {
			B* a = aB(o);
			if (!v) PUSH(x, a[o->c - 1]);
			for (i = o->c - (v ? 1 : 2); i >= 0; i--) {
				PUSH(x, a[i]);
				CALL(x, q); DO(x, P_inner);
			}
		} else if (T(o) % I64 == 0) {
			I* a = aI(o);
			if (!v) PUSH(x, a[o->c - 1]);
			for (i = C(o) - (v ? 1 : 2); i >= 0; i--) {
				PUSH(x, a[i]);
				CALL(x, q); DO(x, P_inner);
			}
		}
	});
}

void P_filter(X* x) {
	UF2(x, {
		O* f = TO_R(x);
		O* a = TO_R(x);
		I c = 0;
		I i;
		I j;
		for (i = 0; i < f->c; i++) {
			c += (aI(f))[i];
		}
		if (T(a) % I8 == 0) {
			B* r;
			if ((r = Pmalloc(c)) == 0) { ERROR(x) = ERR_ALLOCATION; return; }
			PUSHM(x, r);
			T(TS(x)) *= I8*ARRAY;
			C(TS(x)) = c;
			j = 0;
			for (i = 0; i < a->c; i++) {
				if (((I*)f->v.i)[i] == 1) {
					r[j++] = ((B*)a->v.i)[i];
				}
			}
		} else if (T(a) % I64 == 0) {
			I* r;
			if ((r = Pmalloc(c*sizeof(I))) == 0) { ERROR(x) = ERR_ALLOCATION; return; }
			PUSHM(x, r);
			T(TS(x)) *= I64*ARRAY;
			C(TS(x)) = c;
			j = 0;
			for (i = 0; i < a->c; i++) {
				if (((I*)f->v.i)[i] == 1) {
					r[j++] = ((I*)a->v.i)[i];
				}
			}
		}
	});
}

void P_append(X* x) {
  UF2(x, {
    if (NS(x)->t % ARRAY == 0) {
      if (TS(x)->t % ARRAY == 0) {
        O* b = TO_R(x);
        O* a = TO_R(x);
        I* p = Pmalloc((a->c + b->c) * sizeof(I));
        if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
        PUSHM(x, p);
        TS(x)->t *= I64*ARRAY;
        TS(x)->c = a->c + b->c;
        memcpy((I*)TS(x)->v.i, (I*)a->v.i, a->c * sizeof(I));
        memcpy(((I*)TS(x)->v.i) + a->c, (I*)b->v.i, b->c * sizeof(I));
      } else {
        I b = POP(x);
        O* o = TO_R(x);
        I* p = Pmalloc((o->c + 1) * sizeof(I));
        if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
        PUSHM(x, p);
        TS(x)->t *= I64*ARRAY;
        TS(x)->c = o->c + 1;
        ((I*)TS(x)->v.i)[o->c] = b;
        memcpy((I*)TS(x)->v.i, (I*)o->v.i, o->c * sizeof(I));
      }
    } else {
      if (TS(x)->t % ARRAY == 0) {
        O* o = TO_R(x);
        I a = POP(x);
        I* p = Pmalloc((o->c + 1)*sizeof(I));
        if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
        PUSHM(x, p);
        TS(x)->t *= I64*ARRAY;
        TS(x)->c = o->c + 1;
        p[0] = a;
        memcpy(p + 1, (I*)o->v.i, o->c * sizeof(I));
      } else {
        I b = POP(x);
        I a = POP(x);
        I* p = Pmalloc(2*sizeof(I));
        if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
        PUSHM(x, p);
        TS(x)->t *= I64*ARRAY;
        TS(x)->c = 2;
        p[0] = a;
        p[1] = b;
      }
    }
  });
}

void P_drop_from(X* x) {
  UF2(x, {
    I n = POP(x);
    O* a = TO_R(x);
    if (n >= a->c) {
      PUSH(x, 0);
      TS(x)->t *= I64*ARRAY;
      TS(x)->c = 0;
    } else {
      I* p = Pmalloc((a->c - n) * sizeof(I));
      if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
      PUSHM(x, p);
      TS(x)->t *= I64*ARRAY;
      TS(x)->c = a->c - n;
      memcpy(p, ((I*)a->v.i) + n, (a->c - n) * sizeof(I));
    }
  });
}

void P_take_from(X* x) {
  UF2(x, {
    I n = POP(x);
    if (n >= TS(x)->c) return;
    if (n == 1 && TS(x)->c >= 1) {
      I i = ((I*)TS(x)->v.i)[0];
      POP(x);
      PUSH(x, i);
      return;
    }
    O* o = TO_R(x);
    I* p = Pmalloc(n * sizeof(I));
    if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
    PUSHM(x, p);
    TS(x)->t *= I64*ARRAY;
    TS(x)->c = n;
    memcpy(p, (I*)o->v.i, n * sizeof(I));
  });
}

void P_drop_last(X* x) {
  UF2(x, {
    I n = POP(x);
    if (n >= TS(x)->c) {
      POP(x);
      PUSH(x, 0);
      TS(x)->t *= I64*ARRAY;
      TS(x)->c = 0;  
    } else {
      O* a = TO_R(x);      
      I* p = Pmalloc((a->c - n) * sizeof(I));
      if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
      PUSHM(x, p);
      TS(x)->t *= I64*ARRAY;
      TS(x)->c = a->c - n;
      memcpy(p, (I*)a->v.i, (a->c - n) * sizeof(I));
    }
  });
}

void P_take_last(X* x) {
  UF2(x, {
    I n = POP(x);
    if (n == 1 && TS(x)->c >= 1) {
      I i = ((I*)TS(x)->v.i)[TS(x)->c - 1];
      POP(x);
      PUSH(x, i);
      return;
    }
    if (n >= TS(x)->c) return;
    O* a = TO_R(x);
    I* p = Pmalloc(n * sizeof(I));
    if (p == 0) { ERROR(x) = ERR_ALLOCATION; return; }
    PUSHM(x, p);
    TS(x)->t *= I64*ARRAY;
    TS(x)->c = n;
    memcpy(p, ((I*)a->v.i) + (a->c - n), n * sizeof(I));
  });
}

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

void P_print_object(X* x) {
	B buf[255];
	I sz;
	I i;
	memset(buf, 0, sizeof buf);
	UF1(x, {
		sz = dump_o(buf, TS(x));
		for (i = 0; i < sz; i++) {
			PUSH(x, buf[i]);
			EMIT(x);
		}
		POP(x);
	});
}

void P_number(X* x) {
	char *end, *end2;
	F f;
	I i;
  f = strtod(x->ip, &end);
  if (f == 0 && end == x->ip) {
	} else {
    i = strtol(x->ip, &end2, 0);
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

#define DOT(v1, v2) if (*(IP(x) + 1) == '.') { IP(x)++; v1; } else { v2; }

#define VAR2(v2, v1, v0) if (*(IP(x) + 1) == ':') { IP(x)++; v2; } else if (*(IP(x) + 1) == '.') { IP(x)++; v1; } else { v0; }

void P_inner(X* x) {
	B buf[255];
	B op;
	I r = RP(x);
  while (IP(x) && x->err == ERR_OK) {
		if (x->tr) {
			memset(buf, 0, sizeof buf);
			dump(buf, x);
			printf("%s\n", buf);
		}
		switch (*IP(x)) {
			case 'q': x->err = ERR_EXIT; return; break;
      case 'k': DO(x, KEY); break;
      case 'e': DO(x, EMIT); break;
			case '_': DO(x, P_print_object); break;
      case 'p': DO(x, P_print); break;
      case 'r': DO(x, P_read); break;
			case '+': DO(x, P_add); break;
			case '-': DO(x, P_sub); break;
			case '*': DO(x, P_mul); break;
			case '/': DO(x, P_div); break;
			case '%': DO(x, P_mod); break;
			case '<': DO(x, P_lt); break;
			case '=': DO(x, P_eq); break;
			case '>': DO(x, P_gt); break;
      case '&': DO(x, P_and); break;
      case '|': DO(x, P_or); break;
			case '!': DO(x, P_not); break;
			case '\\': POP(x); break;
			case 'd': DO(x, P_dup); break;
			case 's': DO(x, P_swap); break;
      case '@': DO(x, P_rot); break;
      case 'o': DO(x, P_over); break;
			case 'i': DO(x, P_exec_i); break;
			case '?': P_ifthen(x, POP(x), (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
			case 't': P_times(x, POP(x), (B*)TO_R(x)->v.i); break;
			case 'w': P_while(x, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
			case 'l': P_linrec(x, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i, (B*)TO_R(x)->v.i); break;
      case 'b': DO(x, P_binrec); break;
      case '#': VAR2(DO3(x, P_count, POP(x), POP(x), POP(x)), DO3(x, P_count, POP(x), POP(x), 1), DO3(x, P_count, 0, POP(x) - 1, 1)); break;
			case '$': DO(x, P_shape); break;
      case 'm': DO(x, P_map); break;
      case 'z': DO1(x, P_zip, (B*)TO_R(x)->v.i); break;
			case '{': DOT(DO1(x, P_left_fold, 1), DO1(x, P_left_fold, 0)); break;
			case '}': DOT(DO1(x, P_right_fold, 1), DO1(x, P_right_fold, 0));
			case 'f': DO(x, P_filter); break;
      case ';': DO(x, P_append); break;
      case '(': DOT(DO(x, P_take_last), DO(x, P_drop_from)); break;
      case ')': DOT(DO(x, P_drop_last), DO(x, P_take_from)); break;
			case '\'': OF1(x, { PUSH(x, *++x->ip); }); break;
      case '0': case '1': case '2': case'3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        DO(x, P_number);
        break;
			case '[': 
				OF1(x, { 
				  PUSH(x, IP(x) + 1); 
				  TS(x)->t *= I8*ARRAY;
				  TS(x)->c = P_forward(x, '[', ']'); 
        });
				break;
      case 0:
			case ']':
        IP(x) = 0;
        break;
      case '`':
        IP(x)++;
        switch (*IP(x)) {
          case 't': x->tr = 1 - x->tr; break;
          case 'r': PUSH(x, RP(x)); break;
        }
      break;
			default:
				op = *x->ip;
				if (op >= 'A' && op <= 'Z') {
					DO(x, x->ext[op - 'A']);
				}
		}
    if (IP(x) == 0) {
      IP(x) = RPOP(x, r);
      if (IP(x) == 0) { 
        if (x->tr) {
			    memset(buf, 0, sizeof buf);
			    dump(buf, x);
			    printf("%s\n", buf);
		    }
        return; 
      }
    }
    IP(x)++;
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
*/
/* Virtual Machine */

void P_parseLiteral(X* x) {
  OF(x, 1, {
    I n = 0;
    while (IS_DIGIT(PEEK_TOKEN(x))) { n = 10*n + (GET_TOKEN(x) - '0'); }
    if (PEEK_TOKEN(x) == '.') {
      F d = (F)n;
      F mult = 0.1;
      GET_TOKEN(x);
      while (IS_DIGIT(PEEK_TOKEN(x))) { 
        d = d + mult*((F)(GET_TOKEN(x) - '0')); 
        mult /= 10;
      }
      PUSHF(x, FLOAT, d);
    } else {
      PUSHI(x, INT, n); 
    }
  });
}

void P_parseQuotation(X* x) {
  OF(x, 1, {
    B* s;
    B c;
    I t = 1;
    I l = 0;
    GET_TOKEN(x);
    s = IP(x);
    while (t > 0) {
      l++;
      c = GET_TOKEN(x); 
      if (c == '[') t++;
      if (c == ']') t--;
    } 
    PUSHP(x, INT*I8*ARRAY, l, l, s);
  });
}

void P_add(X* x) {
  UF(x, 2, {
    I b = POPI(x);
    I a = POPI(x);
    PUSHI(x, INT, a + b);
  });
}

void P_sub(X* x) {
  UF(x, 2, {
    I b = POPI(x);
    I a = POPI(x);
    PUSHI(x, INT, a - b);
  });
}

void P_lt(X* x) {
  UF(x, 2, {
    I b = POPI(x);
    I a = POPI(x);
    PUSHI(x, INT, a < b);
  });
}

void P_swap(X* x) {
  UF(x, 2, {
    O o;
    O* b = TS(x);
    O* a = NS(x);
    setOP(&o, T(b), C(b), S(b), P(b));
    setOP(b, T(a), C(a), S(a), P(a));
    setOP(a, T(&o), C(&o), S(&o), P(&o));
  });
}

void P_dup(X* x) {
  UF(x, 1, {
    clone(TS(x)); 
  });
}

void P_over(X* x) {
  
}

void P_drop(X* x) {
  
}

void P_times(X* x) {
  
}

void R_bin_rec(X* x, B* i, B* t, B* r1, B* r2) {
  
}

void P_bin_rec(X* x) {
  
}

void P_inner(X* x) {
  I r = RP(x); 
  do {
    B op = PEEK_TOKEN(x);
    switch (op) {
    case 0: RETURN(x, r); break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      P_parseLiteral(x);
      break;
    case '[':
      P_parseQuotation(x);
      break;
    default:
      op = GET_TOKEN(x);
      switch (op) {
        case '+': P_add(x); break;
        case '-': P_sub(x); break;
        case '<': P_lt(x); break;
        case 's': P_swap(x); break;
        case 'd': P_dup(x); break;
        case 'o': P_over(x); break;
        case '\\': P_drop(x); break;
        case 't': P_times(x); break;
        case 'b': P_bin_rec(x); break;
      }
    }
    if (ERROR(x)) return;
  } while(1);
}

void P_repl(X* x) {
  
}

#endif
