#include<stdlib.h>
#include"unity.h"
#include"data.h"

void setUp() {
}

void tearDown() {
}

void test_L_alloc() {
	O* o = L_alloc(10);

	TEST_ASSERT_NOT_NULL(o);
	TEST_ASSERT_EQUAL_INT(INT*OBJECT*ARRAY, TYPE(o));
	TEST_ASSERT_EQUAL_INT(10, SZ(o));
	TEST_ASSERT_EQUAL_INT(0, START(o));
	TEST_ASSERT_EQUAL_INT(0, END(o));

	L_free(o);
}

void test_L_push() {
	I i;
	I r;
	O* o = L_alloc(10);

	r = L_push_I(o, 7);

	TEST_ASSERT_EQUAL_INT(1, r);
	TEST_ASSERT_EQUAL_INT(1, LEN(o));
	TEST_ASSERT_EQUAL_INT(0, START(o));
	TEST_ASSERT_EQUAL_INT(1, END(o));
	TEST_ASSERT_EQUAL_INT(7, I(idxO(o, 0)));

	while (LEN(o) < SZ(o)) {
		r = L_push_I(o, 7);
		TEST_ASSERT_EQUAL_INT(1, r);
	}

	TEST_ASSERT_EQUAL_INT(10, LEN(o));
	TEST_ASSERT_EQUAL_INT(0, START(o));
	TEST_ASSERT_EQUAL_INT(10, END(o));

	for (i = 0; i < 10; i++) {
		TEST_ASSERT_EQUAL_INT(7, I(idxO(o, i)));
	}

	r = L_push_I(o, 7);

	TEST_ASSERT_EQUAL_INT(0, r);

	L_free(o);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_L_alloc);
	RUN_TEST(test_L_push);

	return UNITY_END();
}
