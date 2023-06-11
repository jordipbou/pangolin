#include<stdlib.h>
#include"unity.h"
#include"data.h"

void setUp() {
}

void tearDown() {
}

void test_O_I() {
  O* o = setOI(newO(), 7);

  TEST_ASSERT_EQUAL_INT(7, I(o));
  TEST_ASSERT_EQUAL_INT(0, C(o));
  TEST_ASSERT_EQUAL_INT(0, S(o));
  TEST_ASSERT_EQUAL_INT(INT, T(o));
  
  freeO(o);
}

void test_O_F() {
  O* o = setOF(newO(), 3.1415);

  TEST_ASSERT_EQUAL_INT(3.1415, F(o));
  TEST_ASSERT_EQUAL_INT(0, C(o));
  TEST_ASSERT_EQUAL_INT(0, S(o));
  TEST_ASSERT_EQUAL_INT(FLOAT, T(o));
  
  freeO(o);
}

void test_O_aB() {
  B* a = malloc(5);
  
  O* o = setOP(newO(), INT*I8*ARRAY, 5, 5, a);

  aB(o)[0] = 't';
  aB(o)[1] = 'e';
  aB(o)[2] = 's';
  aB(o)[3] = 't';
  aB(o)[4] = 0;

  TEST_ASSERT_EQUAL_INT(INT*I8*ARRAY, T(o));
  TEST_ASSERT_EQUAL_INT(5, C(o));
  TEST_ASSERT_EQUAL_INT(5, S(o));
  
  TEST_ASSERT_EQUAL_STRING("test", aB(o));

  free(a);
  freeO(o);
}

void test_O_MaB() {
  O* o = setOP(newO(), INT*I8*ARRAY*MANAGED, 5, 5, malloc(5));

  aB(o)[0] = 't';
  aB(o)[1] = 'e';
  aB(o)[2] = 's';
  aB(o)[3] = 't';
  aB(o)[4] = 0;

TEST_ASSERT_EQUAL_INT(INT*I8*ARRAY*MANAGED, T(o));
  TEST_ASSERT_EQUAL_INT(5, C(o));
  TEST_ASSERT_EQUAL_INT(5, S(o));
  
  TEST_ASSERT_EQUAL_STRING("test", aB(o));

  freeO(o);
  /* a must have been freed, test with Valgrind */
}

void test_LPUSH_LPOP() {
  I i;
  F f;
  O* o = setOP(newO(), OBJECT*ARRAY*MANAGED, 5, 0, malloc(5*sizeof(O)));
  B* a = malloc(10);
  I* b = malloc(5*sizeof(I));
  void* p;

  LPUSHI(o, 7);
  LPUSHF(o, 3.1415);
  LPUSHa(o, I8, 10, 0, a);
  LPUSHMa(o, I64, 5, 0, b);

  TEST_ASSERT_EQUAL_INT(4, S(o));
  TEST_ASSERT_EQUAL_INT(7, Iat(o, 0));
  TEST_ASSERT_EQUAL_INT(3.1415, Fat(o, 1));
  TEST_ASSERT_EQUAL_INT(a, Pat(o, 2));
  TEST_ASSERT_EQUAL_INT(b, Pat(o, 3));

  DROP(o);
  p = LPOPU(o);
  f = LPOPF(o);
  i = LPOPI(o);

  TEST_ASSERT_EQUAL_INT(a, p);
  TEST_ASSERT_EQUAL_INT(3.1415, f);
  TEST_ASSERT_EQUAL_INT(7, i);

  TEST_ASSERT_EQUAL_INT(0, S(o));

  free(a);
  freeO(o);
}

void test_Lreserve() {
  
}

int main() {
	UNITY_BEGIN();

  RUN_TEST(test_O_I);
  RUN_TEST(test_O_F);
  RUN_TEST(test_O_aB);
  RUN_TEST(test_O_MaB);
  RUN_TEST(test_LPUSH_LPOP);
  RUN_TEST(test_Lreserve);
  
	return UNITY_END();
}
