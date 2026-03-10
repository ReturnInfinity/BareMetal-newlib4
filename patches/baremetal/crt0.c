#include <stdio.h> // fflush()
#include <time.h>

extern int main(int argc, char *argv[]);
extern void __libc_init_array(void);

extern char __bss_start;
extern char __bss_stop;

static void zero_bss(void);
static void _start_c(void);

/*
 * Entry point: BareMetal OS jumps here (not CALL), so RSP may be
 * 16-byte aligned.  The C compiler assumes _start was CALLed, which
 * would make RSP 8-mod-16 (return address pushed).  That mismatch
 * shifts every later frame by 8 bytes, and SSE instructions such as
 * MOVAPS in printf's float-formatting path will #GP on the mis-
 * aligned stack.
 *
 * Fix: use a naked stub that forces 16-byte alignment, then CALLs
 * into normal C code so the compiler's prologue works correctly.
 */
__attribute__((naked)) void _start(void)
{
	__asm__ volatile (
		"pushq %%rbp\n\t"        /* save rbp (callee-saved)     */
		"movq %%rsp, %%rbp\n\t"  /* remember original RSP       */
		"andq $-16, %%rsp\n\t"   /* ensure 16-byte alignment    */
		"call _start_c\n\t"      /* CALL so RSP is 8-mod-16 inside _start_c */
		"movq %%rbp, %%rsp\n\t"  /* restore original RSP        */
		"popq %%rbp\n\t"         /* restore rbp                 */
		"ret\n\t"                /* return to BareMetal OS       */
		::: "memory"
	);
}

static void _start_c(void)
{
	zero_bss();
	__libc_init_array();

	char *argv[2] = {".", 0};
	int retval = main(1, argv);

	fflush(stdout);

//	_exit(retval);
}

static void zero_bss(void)
{
	for (char *c = &__bss_start; c < &__bss_stop; c++)
		*c = 0;
}

void _init(void)
{
}

void _fini(void)
{
}

// localtime_r -- Convert time_t to struct tm in UTC.
// This replaces newlib's version which depends on tzset/malloc.
struct tm *localtime_r(const time_t *tim_p, struct tm *result)
{
	static const int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	long long t = (long long) *tim_p;
	int days, y, m, leap;

	result->tm_sec = t % 60;
	t /= 60;
	result->tm_min = t % 60;
	t /= 60;
	result->tm_hour = t % 24;
	days = t / 24;

	// Day of week: Jan 1 1970 was Thursday (4)
	result->tm_wday = (days + 4) % 7;

	// Year
	y = 1970;
	for (;;)
	{
		leap = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) ? 1 : 0;
		int yday = 365 + leap;
		if (days < yday)
			break;
		days -= yday;
		y++;
	}
	result->tm_year = y - 1900;
	result->tm_yday = days;

	// Month
	leap = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) ? 1 : 0;
	for (m = 0; m < 11; m++)
	{
		int dim = mdays[m] + (m == 1 ? leap : 0);
		if (days < dim)
			break;
		days -= dim;
	}
	result->tm_mon = m;
	result->tm_mday = days + 1;
	result->tm_isdst = 0;

	return result;
}

// localtime -- Convert time_t to struct tm using a static buffer.
struct tm *localtime(const time_t *tim_p)
{
	static struct tm tm_buf;
	return localtime_r(tim_p, &tm_buf);
}
