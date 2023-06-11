#ifndef P_DATA_H
#define P_DATA_H

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#ifndef Pmalloc
#define Pmalloc(c) (malloc(c))
#endif

#ifndef Prealloc
#define Prealloc(b, c) (realloc(b, c))
#endif

#ifndef Pfree
#define Pfree(p) (free(p))
#endif

#ifndef MIN_CAPACITY
#define MIN_CAPACITY 5
#endif

#ifndef GROWTH_FACTOR
#define GROWTH_FACTOR 1.5
#endif

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
  RETURN = 41
} TYPE;

typedef struct {
  I type;
  I capacity;
  I size;
  union {
    I integer;
    F floating_point;
    void* array;
  } value;
} O;

#define T(o) (o->type)
#define IS(o, t) (T(o) % t == 0)

#define C(o) (o->capacity)
#define S(o) (o->size)

#define I(o) (o->value.integer)
#define F(o) (o->value.floating_point)
#define P(o) (o->value.array)

#define aB(o) ((B*)P(o))
#define aI(o) ((I*)P(o))
#define aF(o) ((F*)P(o))
#define aO(o) ((O*)P(o))

#define Iat(o, n) ((aO(o)[n]).value.integer)
#define Fat(o, n) ((aO(o)[n]).value.floating_point)
#define Pat(o, n) ((aO(o)[n]).value.array)

O* setO(O* o, I t, I c, I s) {
  T(o) = t;
  C(o) = c;
  S(o) = s;
  return o;
}

O* setOI(O* o, I v) {
  I(setO(o, INT, 0, 0)) = v;
  return o;
}

O* setOF(O* o, F v) {
  F(setO(o, FLOAT, 0, 0)) = v;
  return o;
}

O* setOP(O* o, I t, I c, I s, void* p) {
  P(setO(o, t, c, s)) = p;
  return o;
}

O* newO() { 
  return Pmalloc(sizeof(O)); 
}

void freeO(O* o) {
  if (IS(o, MANAGED)) {
    Pfree(P(o));
  }
  Pfree(o);
}

#define OF(o, n) ((S(o) + n) > C(o))
#define UF(o, n) (S(o) >= n)

#define LPUSHI(o, v) (setOI(&aO(o)[S(o)++], v))
#define LPUSHF(o, v) (setOF(&aO(o)[S(o)++], v))
#define LPUSHa(o, t, c, s, p) (setOP(&aO(o)[S(o)++], INT*ARRAY*t, c, s, p))
#define LPUSHMa(o, t, c, s, p) (LPUSHa(o, t*MANAGED, c, s, p))

#define LPOPI(o) (Iat(o, --S(o)))
#define LPOPF(o) (Fat(o, --S(o)))
#define LPOPU(o) (Pat(o, --S(o)))

#define DROP(o) if (IS(o, MANAGED)) { Pfree(Pat(o, --S(o))); } else { S(o)--; }

I Lreserve(O* o, I n) {
  I sz;
  if (OF(o, n)) {
    sz = (I)(((F)(C(o) + n)) * GROWTH_FACTOR);
    void* a = Prealloc(P(o), sz*sizeof(O));
    if (a == 0) { return 0; }
    P(o) = a;
    C(o) = sz;
    return 1;
  } else {
    return 1;
  } 
}

#endif
