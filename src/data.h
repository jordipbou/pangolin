#ifndef P_DATA_H
#define P_DATA_H

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#define Pmalloc(c) (malloc(c))
#define Pfree(p) (free(p))

typedef char B;
typedef intptr_t I;
typedef double F;

enum { ANY = 1, INT = 2, FLOAT = 3, MANAGED = 5, ARRAY = 7, I8 = 11, I16 = 13, I32 = 17, I64 = 19, F32 = 23, F64 = 29, OBJECT = 31, RETURN = 37 } T;

typedef struct _O { I t; I c; I s; I e; union { I i; F f; B* ab; I* ai; F* af; struct _O* ao; } v; } O;

#define IS(o, t) (o->t % t == 0)

#define TYPE(o) (o->t)
#define AS(o, t) (TYPE(o) = (IS(o, t) ? TYPE(o) : (TYPE(o)*t)))

#define START(o) (o->s)
#define END(o) (o->e)
#define LEN(o) (END(o) - START(o))
#define SZ(o) (o->c)
#define FREE(o) (SZ(o) - LEN(o))

#define I(o) (o->v.i)
#define F(o) (o->v.f)
#define aI8(o) (o->v.ab)
#define aI(o) (o->v.ai)
#define aF(o) (o->v.af)
#define aO(o) (o->v.ao)
#define idxO(o, idx) (&aO(o)[idx])

O* L_alloc(I sz) {
	O* o = Pmalloc(sizeof(O));
	if (o == 0) return 0;
	o->v.i = (I)Pmalloc(sz * sizeof(O));
	if (aO(o) == 0) {
	  free(o);
		return 0;
	}
	TYPE(o) = INT*OBJECT*ARRAY;
	START(o) = 0;
	END(o) = 0;
	SZ(o) = sz;
}

/* Returns 1 if no error and 0 if there's not enough space to push another element */
I L_push_I(O* o, I v) {
	O* item;
	if (END(o) == SZ(o)) return 0; 
	item = idxO(o, END(o)++);
	TYPE(item) = INT;
	SZ(item) = 0;
	I(item) = v;

	return 1;
}

O* L_reserve(O* o, I n) {
  if ((END(o) + n) <= SZ(o)) { 
    return o;
  } else {
		if (FREE(o) >= n) {
      I i; 
      for (i = 0; i < LEN(o); i++) {
        aO(o)[i] = aO(o)[i + START(o)];
      }
			END(o) = END(o) - START(o);
			START(o) = 0;
		} else {
			I i;
      I sz = (2*SZ(o)) < n ? n : SZ(o);
			O* n = Pmalloc((2*sz) * sizeof(O));
			if (n == 0) return 0;
			SZ(n) = 2*sz;
			TYPE(n) = TYPE(o);
			memcpy(aO(n), aO(o) + START(o), SZ(o) * sizeof(O));
			Pfree(o);
			o = n;
		}
	}
}

void L_free(O* o) {
  if (o->v.i != 0) {
	  Pfree((O*)o->v.i);
  }
  if (o != 0) {
	  Pfree(o);
  }
}

#endif
