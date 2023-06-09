#include<stdio.h>
#include<string.h>

#include"pangolin.h"

#ifdef _WIN32
  #include <conio.h>
#else
	#include <unistd.h>
	#include <termios.h>
#endif

/*
 Source code for getch is taken from:
 Crossline readline (https://github.com/jcwangxp/Crossline).
 It's a fantastic readline cross-platform replacement, but only getch was
 needed and there's no need to include everything else.
*/
#ifdef _WIN32
int _getch (void) {	fflush (stdout); return _getch(); }
#else
int _getch ()
{
	char ch = 0;
	struct termios old_term, cur_term;
	fflush (stdout);
	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
	cur_term = old_term;
	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); /* echoing off, canonical off, no signal chars */
	cur_term.c_cc[VMIN] = 1;
	cur_term.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } /* signal will interrupt */
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
	return ch;
}
#endif

void key(X* x) { OF(x, 1, { PUSHI(x, _getch()); }); }
void emit(X* x) { UF(x, 1, { printf("%c", (B)POPI(x)); }); }

int main(int argc, char* argv[]) {
	FILE *fptr;
	B buf[255];
	X* x = init();

	x->k = &key;
	x->e = &emit;

	x->tr = 0;
  x->lw = 20;

	if (argc == 2 || argc == 3) {
		fptr = fopen(argv[1], "r");
		while (fgets(buf, 255, fptr)) {
			EVAL(x, buf);
			if (ERROR(x) != ERR_OK && ERROR(x) != ERR_EXIT) {
					printf("ERROR: %ld\n", x->err);
					return;
			}
			x->err = 0;
		}
	}

	if (argc == 1 || argc == 3) {
		P_repl(x);
	}

  /* x->ip = "3.1415d11d[Hello world!]dso"; */
	/*x->ip = "1 1 34[so+]ts\\";*/
	/* x->ip = "36[d2<][][1-d1-][+]b"; */
  /* x->ip = "[Hello world!]p10e"; */
  /* x->ip = "3.1415_"; */
  /* P_inner(x); */
  /*
  memset(buf, 0, sizeof(buf));
  dump_S(buf, x, 1);
  printf("%s\n", buf);
*/
}
