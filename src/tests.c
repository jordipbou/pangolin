#include"unity.h"
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
  /* needs: push/pop/depth/max_depth/error */
  TEST_ASSERT_EQUAL_INT(0, DEPTH(x));

  for (i = 0; DEPTH(x) < MAX_DEPTH(x); i++) {
    PUSH(x, i);
    TEST_ASSERT_EQUAL_INT(i, DEPTH(x));
    TEST_ASSERT_EQUAL_INT(0, ERROR(x));
  }

  PUSH(x, -1);
   
  TEST_ASSERT_EQUAL_INT(ERR_STACK_OVERFLOW, ERROR(x));

  ERROR(x) = ERR_OK;

  for (i = MAX_DEPTH(x) - 1; DEPTH(x) > 0; i--) {
    j = pop(x);
    TEST_ASSERT_EQUAL_INT(i, j);
    TEST_ASSERT_EQUAL_INT(0, ERROR(x)); 
  }

  j = pop(x);
  TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, ERROR(x));
}

int main() {
	UNITY_BEGIN();

  RUN_TEST(test_data_stack);
  
	return UNITY_END();
}