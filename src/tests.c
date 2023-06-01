#include<stdlib.h>
#include"unity.h"

#define ERR(x, c, e, b)   if (c) { if (handle(x, e)) { } else b } else b

int allocated;

#define Palloc(n) (allocated++, malloc(n))
#define Pfree(p) (allocated--, free(p))

#include"pangolin.h"

B buf[255];
X* x;

void setUp() {
	memset(buf, 0, sizeof buf);
	x = init();
  SP(x) = 0;
  RP(x) = 0;
  allocated = 0;
}

void tearDown() {
	free(x);
}

void test_data_stack() {
  I i, j;
  TEST_ASSERT_EQUAL_INT(0, DEPTH(x));

  for (i = 0; DEPTH(x) < MAX_DEPTH(x); i++) {
    PUSH(x, i);
    TEST_ASSERT_EQUAL_INT(i + 1, DEPTH(x));
    TEST_ASSERT_EQUAL_INT(i, TS(x)->v.i);
    TEST_ASSERT_EQUAL_INT(ERR_OK, ERROR(x));
  }

  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x), DEPTH(x));
  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x) - 1, TS(x)->v.i);

  PUSH(x, -1);
   
  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x), DEPTH(x));
  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x) - 1, TS(x)->v.i);
  
  TEST_ASSERT_EQUAL_INT(ERR_STACK_OVERFLOW, ERROR(x));

  ERROR(x) = ERR_OK;

  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x), DEPTH(x));
  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x) - 1, TS(x)->v.i);
  
  for (i = MAX_DEPTH(x) - 1; DEPTH(x) > 0; i--) {
    j = POP(x);
    TEST_ASSERT_EQUAL_INT(i, j);
    TEST_ASSERT_EQUAL_INT(ERR_OK, ERROR(x)); 
  }

  j = POP(x);
  TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, ERROR(x));
}

void test_data_stack_2() {
  while (!ERROR(x)) {
    PUSH(x, 7);
  }

  TEST_ASSERT_EQUAL_INT(ERR_STACK_OVERFLOW, ERROR(x));
  TEST_ASSERT_EQUAL_INT(MAX_DEPTH(x), DEPTH(x));

  ERROR(x) = ERR_OK;

  while (!ERROR(x)) {
    POP(x);
  }

  TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, ERROR(x));
  TEST_ASSERT_EQUAL_INT(0, DEPTH(x));
}

void test_return_stack() {
  I i, j;
  TEST_ASSERT_EQUAL_INT(0, RDEPTH(x));

  for (i = 0; RDEPTH(x) < RMAX_DEPTH(x); i++) {
    RPUSH(x, i);
    TEST_ASSERT_EQUAL_INT(i + 1, RDEPTH(x));
    TEST_ASSERT_EQUAL_INT(i, TR(x)->v.i);
    TEST_ASSERT_EQUAL_INT(0, ERROR(x));
  }

  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x), RDEPTH(x));
  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x) - 1, TR(x)->v.i);
  
  RPUSH(x, -1);
   
  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x), RDEPTH(x));
  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x) - 1, TR(x)->v.i);
  
  TEST_ASSERT_EQUAL_INT(ERR_RSTACK_OVERFLOW, ERROR(x));

  ERROR(x) = ERR_OK;

  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x), RDEPTH(x));
  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x) - 1, TR(x)->v.i);
  
  for (i = RMAX_DEPTH(x) - 1; RDEPTH(x) > 0; i--) {
    j = (I)RPOP(x);
    TEST_ASSERT_EQUAL_INT(i, j);
    TEST_ASSERT_EQUAL_INT(0, ERROR(x)); 
  }

  j = (I)RPOP(x);
  TEST_ASSERT_EQUAL_INT(ERR_RSTACK_UNDERFLOW, ERROR(x));
}

void test_return_stack_2() {
  while (!ERROR(x)) {
    RPUSH(x, 7);
  }

  TEST_ASSERT_EQUAL_INT(ERR_RSTACK_OVERFLOW, ERROR(x));
  TEST_ASSERT_EQUAL_INT(RMAX_DEPTH(x), RDEPTH(x));

  ERROR(x) = ERR_OK;

  while (!ERROR(x)) {
    RPOP(x);
  }

  TEST_ASSERT_EQUAL_INT(ERR_RSTACK_UNDERFLOW, ERROR(x));
  TEST_ASSERT_EQUAL_INT(0, RDEPTH(x));
}

void test_pop_and_free() {
  I i;
  void* a = Palloc(32);
  TEST_ASSERT_EQUAL_INT(1, allocated);
  PUSHM(x, a);
  PUSH(x, 7);
  i = POP(x);
  TEST_ASSERT_EQUAL_INT(7, i);
  i = POP(x);
  TEST_ASSERT_EQUAL_INT(0, i);
  TEST_ASSERT_EQUAL_INT(0, allocated);
}

void test_to_r() {
  I i;
  void* a = Palloc(32);
  O* o;
  PUSHM(x, a);
  o = TO_R(x);
  TEST_ASSERT_EQUAL_INT(a, (void*)o->v.i);
  TEST_ASSERT_EQUAL_INT(0, DEPTH(x));
  TEST_ASSERT_EQUAL_INT(1, RDEPTH(x));
  RPOP(x);
  TEST_ASSERT_EQUAL_INT(0, RDEPTH(x));
  TEST_ASSERT_EQUAL_INT(0, allocated);
}

void test_rpop() {
  B* s1 = "[11+]";
  B* s2 = "[01-]";
  B* i;
  RPUSH(x, s1);
  PUSH(x, 7);
  TO_R(x);
  PUSH(x, 11);
  TO_R(x);
  RPUSH(x, s2);
  PUSH(x, 13);
  TO_R(x);
  TEST_ASSERT_EQUAL_INT(5, RDEPTH(x));
  i = RPOP(x);
  TEST_ASSERT_EQUAL_PTR(s2, i);
  TEST_ASSERT_EQUAL_INT(3, RDEPTH(x));
  i = RPOP(x);
  TEST_ASSERT_EQUAL_PTR(s1, i);
  TEST_ASSERT_EQUAL_INT(0, RDEPTH(x));
}

void test_dump_o() {
  PUSH(x, 7);
  dump_o(buf, TS(x));
  TEST_ASSERT_EQUAL_STRING("7", buf);
  
  PUSHF(x, 3.1415);
  memset(buf, 0, sizeof buf);
  dump_o(buf, TS(x));
  TEST_ASSERT_EQUAL_STRING("3.1415", buf);

  RPUSH(x, "[11+]");
  memset(buf, 0, sizeof buf);
  dump_o(buf, TR(x));
  TEST_ASSERT_EQUAL_STRING("[11+]", buf);

  RPUSH(x, "11+[01-]i]sd");
  memset(buf, 0, sizeof buf);
  dump_o(buf, TR(x));
  TEST_ASSERT_EQUAL_STRING("11+[01-]i]", buf); 
}

void test_dump_rstack() {
  IP(x) = "11+]";
  PUSH(x, 7);
  TO_R(x);
  RPUSH(x, "ds");
  memset(buf, 0, sizeof buf);
  dump_rstack(buf, x);
  TEST_ASSERT_EQUAL_STRING("11+] : ds", buf);
}

int main() {
	UNITY_BEGIN();

  RUN_TEST(test_data_stack);
  RUN_TEST(test_data_stack_2);
  
  RUN_TEST(test_return_stack);
  RUN_TEST(test_return_stack_2);

  RUN_TEST(test_pop_and_free);
  
  RUN_TEST(test_to_r);
  RUN_TEST(test_rpop);

  RUN_TEST(test_dump_o);
  RUN_TEST(test_dump_rstack);
  
	return UNITY_END();
}
