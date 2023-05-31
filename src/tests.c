#include"unity.h"
#define ERR(x, c, e, b)   if (c) { if (handle(x, e)) { } else b } else b
#include"pangolin.h"
#include<string.h>
#include<stdlib.h>

B buf[255];
X* x;

void setUp() {
	memset(buf, 0, sizeof buf);
	x = init();
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

void test_to_r() {
  
}

int main() {
	UNITY_BEGIN();

  RUN_TEST(test_data_stack);
  RUN_TEST(test_data_stack_2);
  
  RUN_TEST(test_return_stack);
  RUN_TEST(test_return_stack_2);

  RUN_TEST(test_to_r);
  
	return UNITY_END();
}