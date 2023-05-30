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

int main() {
	UNITY_BEGIN();

	return UNITY_END();
}