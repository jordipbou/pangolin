#include"unity.h"
#include"pangolin.h"
#include<string.h>
#include<stdlib.h>

B buf[255];
X* x;

void setUp() {
	memset(buf, 0, sizeof buf);
	x = init();
	x->sp = 0;
	x->rp = 0;
}

void tearDown() {
	free(x);
}

void test_PUSH() {
	PUSH(x, 7);

	TEST_ASSERT_EQUAL_INT(1, x->sp);
	TEST_ASSERT_EQUAL_INT(7, TS(x).v.i);

	PUSH(x, 11);

	TEST_ASSERT_EQUAL_INT(2, x->sp);
	TEST_ASSERT_EQUAL_INT(11, TS(x).v.i);
	TEST_ASSERT_EQUAL_INT(7, NS(x).v.i);
}

void test_PUSHF() {
	PUSHF(x, 3.1415);

	TEST_ASSERT_EQUAL_INT(1, x->sp);
	TEST_ASSERT_EQUAL_INT(3.1415, TS(x).v.f);
}

void test_DROP() {
}

void test_POP() {
}

void test_POPF() {
}

void test_PUSHR() {
}

void test_POPR() {
}

void test_DUMP_STACK() {
	PUSH(x, 7);
	PUSH(x, 11);

	dump_stack(buf, x, 0);
	TEST_ASSERT_EQUAL_STRING("7 11 ", buf);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x, 1);
	TEST_ASSERT_EQUAL_STRING("7\n11\n", buf);
}

void test_DUMP_RSTACK() {
	x->ip = "11+]t";

	dump_rstack(buf, x);
	TEST_ASSERT_EQUAL_STRING("11+]", buf);
}

void test_INNER_0() {
	x->ip = "00";
	inner(x);
	dump_stack(buf, x, 0);
	TEST_ASSERT_EQUAL_STRING("0 0 ", buf);
}

void test_INNER_1() {
	x->ip = "11";
	inner(x);
	dump_stack(buf, x, 0);
	TEST_ASSERT_EQUAL_STRING("1 1 ", buf);
}

void test_INNER_add() {
	x->ip = "111++";
	inner(x);
	dump_stack(buf, x, 0);
	TEST_ASSERT_EQUAL_STRING("3 ", buf);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_PUSH);
	RUN_TEST(test_PUSHF);
	RUN_TEST(test_DROP);
	RUN_TEST(test_POP);
	RUN_TEST(test_POPF);
	RUN_TEST(test_PUSHR);
	RUN_TEST(test_POPR);

	RUN_TEST(test_DUMP_STACK);
	RUN_TEST(test_DUMP_RSTACK);

	RUN_TEST(test_INNER_0);
	RUN_TEST(test_INNER_1);
	RUN_TEST(test_INNER_add);

	return UNITY_END();
}

