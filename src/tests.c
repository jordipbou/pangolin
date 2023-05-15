#include "unity.h"
#include "pangolin.h"

void setUp() {}

void tearDown() {}

void test_INT() {
	char s[255];
	X* x = init();
	P p;

	PUSHC(x, INT, 7);
	PUSHC(x, INT, 11);

	s[0] = 0;
	TEST_ASSERT_EQUAL_STRING("7 11 ", dump_stack(s, x, 0));

	p = POP(x);

	TEST_ASSERT_EQUAL_INT(p.t, INT);
	TEST_ASSERT_EQUAL_INT(p.v.c, 11);

	s[0] = 0;
	TEST_ASSERT_EQUAL_STRING("7 ", dump_stack(s, x, 0));

	p = POP(x);

	TEST_ASSERT_EQUAL_INT(p.t, INT);
	TEST_ASSERT_EQUAL_INT(p.v.c, 7);

	s[0] = 0; TEST_ASSERT_EQUAL_STRING("", dump_stack(s, x, 0));

	free(x);
}

void test_STR() {
	char s[255];
	X* x = init();

	PUSHS(x, "test string", 11);

	s[0] = 0; TEST_ASSERT_EQUAL_STRING("\"test string\" ", dump_stack(s, x, 0));

	free(x);
}

void test_FLOAT() {
	char s[255];
	X* x = init();

	PUSHC(x, INT, 7);
	PUSHF(x, FLOAT, 3.1415);

	s[0] = 0; TEST_ASSERT_EQUAL_STRING("7 3.141500 ", dump_stack(s, x, 0));

	free(x);
}

void test_parse_token() {
	char s[255];
	X* x = init();

	x->b = "test string";
	x->bl = 11;
	x->in = 0;

	parse_token(x);
	parse_token(x);
	parse_token(x);

	s[0] = 0; TEST_ASSERT_EQUAL_STRING("\"test\" \"string\" \"\" ", dump_stack(s, x, 0));

	free(x);
}

void test_parse_until() {
	char s[255];
	X* x = init();

	x->b = "  s\" string literal\"  ";
	x->bl = 21;
	x->in = 0;
	
	parse_token(x);
	PUSHC(x, CHAR, '"');
	parse_until(x);

	s[0] = 0; TEST_ASSERT_EQUAL_STRING("\"s\"\" \"string literal\" ", dump_stack(s, x, 0));

	free(x);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_INT);
	RUN_TEST(test_STR);
	RUN_TEST(test_FLOAT);

	RUN_TEST(test_parse_token);
	RUN_TEST(test_parse_until);

	return UNITY_END();
}

