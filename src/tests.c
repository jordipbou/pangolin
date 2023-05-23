#include"unity.h"
#include"pangolin.h"
#include<string.h>
#include<stdlib.h>

void setUp() {}

void tearDown() {}

void test_PUSH_DROP() {
	X* x = init();

	B* b = malloc(5);
	strcpy(b, "test");

	PUSHI(x, NUMBER, 7);
	PUSHF(x, 3.14);
	PUSHU(x, STRING, 4, "test");
	PUSHM(x, STRING, 4, b);

	TEST_ASSERT_EQUAL_INT(4, SP(x));

	DROP(x);
	DROP(x);
	DROP(x);
	DROP(x);

	TEST_ASSERT_EQUAL_INT(0, SP(x));

	free(x);
}

void test_EXT() {
	B buf[255];
	O o;
	o.t = INT*NUMBER;
	o.c = 0;
	o.v.i = 7;

	memset(buf, 0, sizeof buf);
	ext(buf, &o); 
	TEST_ASSERT_EQUAL_STRING("7", buf);

	o.t = FLOAT*NUMBER;
	o.c = 0;
	o.v.f = 3.1415;

	memset(buf, 0, sizeof buf);
	ext(buf, &o); 
	TEST_ASSERT_EQUAL_STRING("3.1415", buf);

	o.t = INT*CHAR;
	o.c = 0;
	o.v.i = (I)'r';

	memset(buf, 0, sizeof buf);
	ext(buf, &o);
	TEST_ASSERT_EQUAL_STRING("'r'", buf);

	o.t = UREF*STRING;
	o.c = 4;
	o.v.r = "test";

	memset(buf, 0, sizeof buf);
	ext(buf, &o);
	TEST_ASSERT_EQUAL_STRING("\"test\"", buf);
}

void test_DUMP_STACK() {
	B buf[255];
	X* x = init();

	PUSHI(x, NUMBER, 7);
	PUSHI(x, CHAR, 'r');
	PUSHF(x, 3.1415);
	PUSHU(x, STRING, 4, "test");

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("7 'r' 3.1415 \"test\" ", buf);
	
	free(x);
}

void test_DUMP_RSTACK() {
	B buf[255];
	X* x = init();

	PUSHR(x, "11+d*");

	memset(buf, 0, sizeof buf);
	dump_rstack(buf, x);
	TEST_ASSERT_EQUAL_STRING("11+d*", buf);
	
	free(x);
}

void test_DUMP() {
	B buf[255];
	X* x = init();

	PUSHI(x, NUMBER, 7);
	PUSHI(x, CHAR, 'r');
	PUSHF(x, 3.1415);
	PUSHU(x, STRING, 4, "test");

	PUSHR(x, "11+d*");

	x->tib = "test input";

	memset(buf, 0, sizeof buf);
	dump(buf, x);
	TEST_ASSERT_EQUAL_STRING("7 'r' 3.1415 \"test\" : 11+d* << test input", buf);
	
	free(x);
}

/* Arithmetics: ADD */

void test_ADD_INT_INT() {
	B buf[255]; 
	X* x = init();

	PUSHI(x, NUMBER, 7);
	PUSHI(x, NUMBER, 11);

	ADD(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("18 ", buf);
	
	free(x);
}

void test_ADD_INT_FLOAT() {
	B buf[255]; 
	X* x = init();

	PUSHI(x, NUMBER, 7);
	PUSHF(x, 3.1415);

	ADD(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("10.1415 ", buf);
	
	free(x);
}

void test_ADD_FLOAT_INT() {
	B buf[255]; 
	X* x = init();

	PUSHF(x, 3.1415);
	PUSHI(x, NUMBER, 7);

	ADD(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("10.1415 ", buf);
	
	free(x);
}

void test_ADD_FLOAT_FLOAT() {
	B buf[255]; 
	X* x = init();

	PUSHF(x, 3.1415);
	PUSHF(x, 2.7182);

	ADD(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("5.8597 ", buf);
	
	free(x);
}

/* Arithmetics: SUB */

void test_SUB_INT_INT() {
	B buf[255]; 
	X* x = init();

	PUSHI(x, NUMBER, 7);
	PUSHI(x, NUMBER, 11);

	SUB(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("-4 ", buf);
	
	free(x);
}

void test_SUB_INT_FLOAT() {
	B buf[255]; 
	X* x = init();

	PUSHI(x, NUMBER, 7);
	PUSHF(x, 3.1415);

	SUB(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("3.8585 ", buf);
	
	free(x);
}

void test_SUB_FLOAT_INT() {
	B buf[255]; 
	X* x = init();

	PUSHF(x, 3.1415);
	PUSHI(x, NUMBER, 7);

	SUB(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("-3.8585 ", buf);
	
	free(x);
}

void test_SUB_FLOAT_FLOAT() {
	B buf[255]; 
	X* x = init();

	PUSHF(x, 3.1415);
	PUSHF(x, 2.7182);

	SUB(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("0.4233 ", buf);
	
	free(x);
}

/* Stack operations */

void test_DUP() {
	B buf[255];
	X* x = init();

	PUSHI(x, NUMBER, 7);

	DUP(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("7 7 ", buf);

	PUSHI(x, CHAR, 'r');

	DUP(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("7 7 'r' 'r' ", buf);

	PUSHF(x, 3.1415);

	DUP(x);

	memset(buf, 0, sizeof buf);
	dump_stack(buf, x);
	TEST_ASSERT_EQUAL_STRING("7 7 'r' 'r' 3.1415 3.1415 ", buf);
	
	free(x);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_PUSH_DROP);
	RUN_TEST(test_EXT);
	RUN_TEST(test_DUMP_STACK);
	RUN_TEST(test_DUMP_RSTACK);
	RUN_TEST(test_DUMP);

	RUN_TEST(test_ADD_INT_INT);
	RUN_TEST(test_ADD_INT_FLOAT);
	RUN_TEST(test_ADD_FLOAT_INT);
	RUN_TEST(test_ADD_FLOAT_FLOAT);

	RUN_TEST(test_SUB_INT_INT);
	RUN_TEST(test_SUB_INT_FLOAT);
	RUN_TEST(test_SUB_FLOAT_INT);
	RUN_TEST(test_SUB_FLOAT_FLOAT);

	RUN_TEST(test_DUP);

	return UNITY_END();
}

