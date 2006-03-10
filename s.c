/* demo of C string pitfalls
 *
 * -- by lnussel@suse.de
 * */

#include "stackdump.h"

#define x(x) do { if(help) printf("%2d: ", n); puts(#x); if(!help) { x; } } while(0)

int main(int argc, char* argv[])
{
    char a[8] = {0};
    char b[8] = {0};
    const char XXX[] = "XXXXXXXXXX";
    unsigned n = 0, help = 0;

    memset(a, '~', sizeof(a)-1);
    memset(b, '\0', sizeof(b)-1);

    if(argc < 2 || (n = atoi(argv[1])) < 1 || n > 14)
	    help = n = 1;

    if(!help)
    {
	STACK_S
	    FRAMESTART
	    VAR(a)
	    VAR(b)
	STACK_E
    }

    do
    {
	switch(n)
	{
	    case 1:
		x(strncat(b,"XXXXX", sizeof(b)-1));
		// WRONG: writes \0 into a (... if a and b are 8byte aligned)
		x(strncat(b,"YYYYY", sizeof(b)-strlen(b)));
		break;
	    case 2:
		// correct
		x(strncat(b,"XXXXX", sizeof(b)-1));
		x(strncat(b,"YYYYY", sizeof(b)-strlen(b)-1));
		break;
	    case 3:
		// WRONG: writes \0 into a
		x(strncat(b,XXX, sizeof(b)));
		break;
	    case 4:
		// correct
		x(strncat(b,XXX, sizeof(b)-1));
		break;
	    case 5:
		// WRONG: not terminated
		x(strncpy(b,XXX, sizeof(b)));
		break;
	    case 6:
		x(strncpy(b,XXX, sizeof(b)-1));
		// WRONG: writes \0 into a
		x(b[sizeof(b)] = '\0');
		break;
	    case 7:
		// correct
		x(strncpy(b,XXX, sizeof(b)-1));
		x(b[sizeof(b)-1] = '\0');
		break;
	    case 8:
		// WRONG: writes \0 into a
		x(sscanf(XXX, "%8s", b));
		break;
	    case 9:
		// correct
		x(sscanf(XXX, "%7s", b));
		break;
	    case 10:
		// WRONG: writes \0 into a
		x(sscanf(XXX, "%8[^*]", b));
		break;
	    case 11:
		// correct
		x(sscanf(XXX, "%7[^*]", b));
		break;
	    case 12:
		// correct
		x(snprintf(b, sizeof(b), "%s", XXX));
		break;
	    case 13:
		// WRONG: writes \0 into a
		x(sprintf(b, "%.8s", XXX));
		break;
	    case 14:
		// correct
		x(sprintf(b, "%.7s", XXX));
		break;
	}
	++n;
    } while (help && n <= 14);

    if(!help)
    {
	STACK_S
	    FRAMESTART
	    VAR(a)
	    VAR(b)
	STACK_E

	if(*a != '~' || b[sizeof(b)-1] != '\0') puts("*** BROKEN ***");
    }

    return 0;
}

/* vim: sw=4
 * */
