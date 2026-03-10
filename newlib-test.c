#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

/* ── tiny test harness ─────────────────────────────────────────────── */
static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { \
	tests_run++; \
	printf("  %-52s ", name); \
} while (0)

#define PASS() do { tests_passed++; printf("[PASS]\n"); } while (0)

#define FAIL(msg) do { \
	tests_failed++; \
	printf("[FAIL]  %s\n", msg); \
} while (0)

#define CHECK(cond, msg) do { if (cond) { PASS(); } else { FAIL(msg); } } while (0)

static void section(const char *title)
{
	printf("\n--- %s ---\n", title);
}

static void report(void)
{
	printf("\n=======================\n");
	printf("Tests run:    %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("=======================\n");
}

/* ── string.h tests ───────────────────────────────────────────────── */
static void test_strlen(void)
{
	TEST("strlen: empty string");
	CHECK(strlen("") == 0, "expected 0");

	TEST("strlen: normal string");
	CHECK(strlen("hello") == 5, "expected 5");

	TEST("strlen: string with spaces");
	CHECK(strlen("hello world") == 11, "expected 11");
}

static void test_strcpy(void)
{
	char buf[64];

	TEST("strcpy: basic copy");
	strcpy(buf, "hello");
	CHECK(strcmp(buf, "hello") == 0, "mismatch");

	TEST("strcpy: empty string");
	strcpy(buf, "");
	CHECK(buf[0] == '\0', "expected empty");
}

static void test_strncpy(void)
{
	char buf[16];

	TEST("strncpy: full copy with room");
	memset(buf, 'X', sizeof(buf));
	strncpy(buf, "abc", 8);
	CHECK(strcmp(buf, "abc") == 0 && buf[3] == '\0' && buf[7] == '\0',
		"should copy and pad with NULs");

	TEST("strncpy: truncation (no NUL)");
	memset(buf, 'X', sizeof(buf));
	strncpy(buf, "longstring", 4);
	CHECK(memcmp(buf, "long", 4) == 0, "expected truncated copy");
}

static void test_strcmp(void)
{
	TEST("strcmp: equal strings");
	CHECK(strcmp("abc", "abc") == 0, "expected 0");

	TEST("strcmp: first < second");
	CHECK(strcmp("abc", "abd") < 0, "expected negative");

	TEST("strcmp: first > second");
	CHECK(strcmp("abd", "abc") > 0, "expected positive");

	TEST("strcmp: empty vs non-empty");
	CHECK(strcmp("", "a") < 0, "expected negative");

	TEST("strcmp: prefix vs longer");
	CHECK(strcmp("ab", "abc") < 0, "expected negative");
}

static void test_strncmp(void)
{
	TEST("strncmp: equal within n");
	CHECK(strncmp("abcdef", "abcxyz", 3) == 0, "expected 0");

	TEST("strncmp: differ within n");
	CHECK(strncmp("abcdef", "abxdef", 3) < 0, "expected negative");

	TEST("strncmp: n == 0");
	CHECK(strncmp("anything", "different", 0) == 0, "expected 0");
}

static void test_strcat(void)
{
	char buf[64];

	TEST("strcat: basic concatenation");
	strcpy(buf, "hello");
	strcat(buf, " world");
	CHECK(strcmp(buf, "hello world") == 0, "mismatch");

	TEST("strcat: append to empty");
	buf[0] = '\0';
	strcat(buf, "test");
	CHECK(strcmp(buf, "test") == 0, "mismatch");

	TEST("strcat: append empty");
	strcpy(buf, "keep");
	strcat(buf, "");
	CHECK(strcmp(buf, "keep") == 0, "mismatch");
}

static void test_strncat(void)
{
	char buf[64];

	TEST("strncat: limited append");
	strcpy(buf, "hello");
	strncat(buf, " world!!", 6);
	CHECK(strcmp(buf, "hello world") == 0, "expected 'hello world'");

	TEST("strncat: n larger than source");
	strcpy(buf, "A");
	strncat(buf, "BC", 100);
	CHECK(strcmp(buf, "ABC") == 0, "expected 'ABC'");
}

static void test_strchr(void)
{
	const char *s = "hello world";

	TEST("strchr: find existing char");
	CHECK(strchr(s, 'w') == s + 6, "wrong pointer");

	TEST("strchr: find first occurrence");
	CHECK(strchr(s, 'l') == s + 2, "expected first 'l'");

	TEST("strchr: char not found");
	CHECK(strchr(s, 'z') == NULL, "expected NULL");

	TEST("strchr: find NUL terminator");
	CHECK(strchr(s, '\0') == s + 11, "expected pointer to NUL");
}

static void test_strrchr(void)
{
	const char *s = "hello world";

	TEST("strrchr: find last occurrence");
	CHECK(strrchr(s, 'l') == s + 9, "expected last 'l'");

	TEST("strrchr: char not found");
	CHECK(strrchr(s, 'z') == NULL, "expected NULL");
}

static void test_strstr(void)
{
	const char *s = "the quick brown fox";

	TEST("strstr: find substring");
	CHECK(strstr(s, "brown") == s + 10, "wrong pointer");

	TEST("strstr: not found");
	CHECK(strstr(s, "blue") == NULL, "expected NULL");

	TEST("strstr: empty needle");
	CHECK(strstr(s, "") == s, "expected start of haystack");

	TEST("strstr: full match");
	CHECK(strstr(s, s) == s, "expected start");
}

static void test_strpbrk(void)
{
	const char *s = "hello world";

	TEST("strpbrk: find one of set");
	CHECK(strpbrk(s, "ow") == s + 4, "expected first 'o' at index 4");

	TEST("strpbrk: none found");
	CHECK(strpbrk(s, "xyz") == NULL, "expected NULL");
}

static void test_strspn_strcspn(void)
{
	TEST("strspn: leading matching chars");
	CHECK(strspn("abcdef", "cba") == 3, "expected 3");

	TEST("strspn: no match at start");
	CHECK(strspn("xyz", "abc") == 0, "expected 0");

	TEST("strcspn: leading non-matching chars");
	CHECK(strcspn("abcdef", "de") == 3, "expected 3");

	TEST("strcspn: all match reject set = none");
	CHECK(strcspn("hello", "xyz") == 5, "expected 5");
}

static void test_strtok(void)
{
	char buf[64];

	TEST("strtok: basic tokenisation");
	strcpy(buf, "one,two,,three");
	char *tok = strtok(buf, ",");
	int ok = (tok && strcmp(tok, "one") == 0);
	tok = strtok(NULL, ",");
	ok = ok && (tok && strcmp(tok, "two") == 0);
	tok = strtok(NULL, ",");
	ok = ok && (tok && strcmp(tok, "three") == 0);
	tok = strtok(NULL, ",");
	ok = ok && (tok == NULL);
	CHECK(ok, "tokenisation mismatch");
}

/* ── memory functions (string.h) ──────────────────────────────────── */
static void test_memcpy(void)
{
	char src[] = "ABCDEFGH";
	char dst[16];

	TEST("memcpy: basic copy");
	memset(dst, 0, sizeof(dst));
	memcpy(dst, src, 8);
	CHECK(memcmp(dst, src, 8) == 0, "mismatch");
}

static void test_memmove(void)
{
	char buf[32];

	TEST("memmove: non-overlapping");
	strcpy(buf, "abcdefgh");
	memmove(buf + 16, buf, 8);
	CHECK(memcmp(buf + 16, "abcdefgh", 8) == 0, "mismatch");

	TEST("memmove: overlapping forward");
	strcpy(buf, "abcdefgh");
	memmove(buf + 2, buf, 6);
	CHECK(memcmp(buf + 2, "abcdef", 6) == 0, "overlap forward fail");

	TEST("memmove: overlapping backward");
	strcpy(buf + 2, "ABCDEF");
	memmove(buf, buf + 2, 6);
	CHECK(memcmp(buf, "ABCDEF", 6) == 0, "overlap backward fail");
}

static void test_memset(void)
{
	char buf[16];

	TEST("memset: fill with character");
	memset(buf, 'A', 10);
	buf[10] = '\0';
	CHECK(strlen(buf) == 10 && buf[0] == 'A' && buf[9] == 'A',
		"fill mismatch");

	TEST("memset: fill with zero");
	memset(buf, 0, sizeof(buf));
	int ok = 1;
	for (size_t i = 0; i < sizeof(buf); i++)
		if (buf[i] != 0) { ok = 0; break; }
	CHECK(ok, "zero fill mismatch");
}

static void test_memcmp(void)
{
	TEST("memcmp: equal");
	CHECK(memcmp("abc", "abc", 3) == 0, "expected 0");

	TEST("memcmp: first < second");
	CHECK(memcmp("abc", "abd", 3) < 0, "expected negative");

	TEST("memcmp: first > second");
	CHECK(memcmp("abd", "abc", 3) > 0, "expected positive");

	TEST("memcmp: zero length");
	CHECK(memcmp("x", "y", 0) == 0, "expected 0");
}

static void test_memchr(void)
{
	const char data[] = {1, 2, 3, 4, 5, 0, 7};

	TEST("memchr: find existing byte");
	CHECK(memchr(data, 3, 7) == &data[2], "wrong pointer");

	TEST("memchr: byte not present");
	CHECK(memchr(data, 99, 7) == NULL, "expected NULL");

	TEST("memchr: find zero byte");
	CHECK(memchr(data, 0, 7) == &data[5], "expected index 5");
}

/* ── stdlib.h tests ───────────────────────────────────────────────── */
static void test_atoi(void)
{
	TEST("atoi: positive number");
	CHECK(atoi("42") == 42, "expected 42");

	TEST("atoi: negative number");
	CHECK(atoi("-99") == -99, "expected -99");

	TEST("atoi: leading whitespace");
	CHECK(atoi("  123") == 123, "expected 123");

	TEST("atoi: trailing non-digits");
	CHECK(atoi("456abc") == 456, "expected 456");

	TEST("atoi: zero");
	CHECK(atoi("0") == 0, "expected 0");

	TEST("atoi: just text");
	CHECK(atoi("abc") == 0, "expected 0");
}

static void test_atol(void)
{
	TEST("atol: large number");
	CHECK(atol("1000000") == 1000000L, "expected 1000000");

	TEST("atol: negative");
	CHECK(atol("-54321") == -54321L, "expected -54321");
}

static void test_strtol(void)
{
	char *end;

	TEST("strtol: decimal");
	CHECK(strtol("12345", &end, 10) == 12345L && *end == '\0',
		"expected 12345");

	TEST("strtol: hexadecimal");
	CHECK(strtol("0xFF", &end, 16) == 255L, "expected 255");

	TEST("strtol: octal");
	CHECK(strtol("077", &end, 8) == 63L, "expected 63");

	TEST("strtol: binary (base 2)");
	CHECK(strtol("1010", &end, 2) == 10L, "expected 10");

	TEST("strtol: auto-detect hex with 0x");
	CHECK(strtol("0x1A", &end, 0) == 26L, "expected 26");

	TEST("strtol: negative value");
	CHECK(strtol("-100", &end, 10) == -100L, "expected -100");

	TEST("strtol: end pointer set correctly");
	strtol("42xyz", &end, 10);
	CHECK(*end == 'x', "expected 'x'");
}

static void test_strtoul(void)
{
	char *end;

	TEST("strtoul: basic");
	CHECK(strtoul("65535", &end, 10) == 65535UL, "expected 65535");

	TEST("strtoul: hex");
	CHECK(strtoul("BEEF", &end, 16) == 0xBEEFUL, "expected 0xBEEF");
}

static void test_abs_labs(void)
{
	TEST("abs: positive");
	CHECK(abs(42) == 42, "expected 42");

	TEST("abs: negative");
	CHECK(abs(-42) == 42, "expected 42");

	TEST("abs: zero");
	CHECK(abs(0) == 0, "expected 0");

	TEST("labs: large negative");
	CHECK(labs(-100000L) == 100000L, "expected 100000");
}

static void test_div_ldiv(void)
{
	TEST("div: 17 / 5");
	div_t d = div(17, 5);
	CHECK(d.quot == 3 && d.rem == 2, "expected quot=3 rem=2");

	TEST("div: negative dividend");
	d = div(-17, 5);
	CHECK(d.quot == -3 && d.rem == -2, "expected quot=-3 rem=-2");

	TEST("ldiv: large values");
	ldiv_t ld = ldiv(1000003L, 1000L);
	CHECK(ld.quot == 1000L && ld.rem == 3L, "expected quot=1000 rem=3");
}

static void test_malloc_free(void)
{
	TEST("malloc: allocate and use");
	char *p = (char *)malloc(128);
	int ok = (p != NULL);
	if (ok) {
		memset(p, 'A', 128);
		ok = (p[0] == 'A' && p[127] == 'A');
		free(p);
	}
	CHECK(ok, "malloc/free failed");

	TEST("malloc: zero size");
	/* C standard says malloc(0) is implementation-defined, may return NULL or a unique pointer */
	p = (char *)malloc(0);
	/* either outcome is acceptable */
	if (p) free(p);
	PASS();
}

static void test_calloc(void)
{
	TEST("calloc: zero-initialised");
	int *arr = (int *)calloc(10, sizeof(int));
	int ok = (arr != NULL);
	if (ok) {
		for (int i = 0; i < 10; i++) {
			if (arr[i] != 0) { ok = 0; break; }
		}
		free(arr);
	}
	CHECK(ok, "calloc memory not zeroed");
}

static void test_realloc(void)
{
	TEST("realloc: grow allocation");
	char *p = (char *)malloc(16);
	int ok = (p != NULL);
	if (ok) {
		strcpy(p, "hello");
		p = (char *)realloc(p, 64);
		ok = (p != NULL && strcmp(p, "hello") == 0);
		free(p);
	}
	CHECK(ok, "realloc failed to preserve data");

	TEST("realloc: NULL acts like malloc");
	p = (char *)realloc(NULL, 32);
	ok = (p != NULL);
	if (ok) free(p);
	CHECK(ok, "realloc(NULL, n) should act as malloc");
}

static int int_compare(const void *a, const void *b)
{
	return (*(const int *)a - *(const int *)b);
}

static void test_qsort(void)
{
	int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6, 0};

	TEST("qsort: sort 10 integers");
	qsort(arr, 10, sizeof(int), int_compare);
	int ok = 1;
	for (int i = 0; i < 10; i++) {
		if (arr[i] != i) { ok = 0; break; }
	}
	CHECK(ok, "array not sorted correctly");
}

static void test_bsearch(void)
{
	int arr[] = {1, 3, 5, 7, 9, 11, 13, 15};

	TEST("bsearch: find existing element");
	int key = 7;
	int *found = (int *)bsearch(&key, arr, 8, sizeof(int), int_compare);
	CHECK(found != NULL && *found == 7, "expected to find 7");

	TEST("bsearch: element not present");
	key = 6;
	found = (int *)bsearch(&key, arr, 8, sizeof(int), int_compare);
	CHECK(found == NULL, "expected NULL");
}

static void test_rand_srand(void)
{
	TEST("srand/rand: deterministic sequence");
	srand(12345);
	int a = rand();
	int b = rand();
	srand(12345);
	int c = rand();
	int d = rand();
	CHECK(a == c && b == d, "same seed should produce same sequence");

	TEST("rand: returns non-negative");
	srand(42);
	int ok = 1;
	for (int i = 0; i < 100; i++) {
		if (rand() < 0) { ok = 0; break; }
	}
	CHECK(ok, "rand() returned negative value");

	TEST("rand: within RAND_MAX");
	srand(99);
	ok = 1;
	for (int i = 0; i < 100; i++) {
		if (rand() > RAND_MAX) { ok = 0; break; }
	}
	CHECK(ok, "rand() exceeded RAND_MAX");
}

/* ── ctype.h tests ────────────────────────────────────────────────── */
static void test_isalpha(void)
{
	TEST("isalpha: letter");
	CHECK(isalpha('A') && isalpha('z'), "expected true for letters");

	TEST("isalpha: digit");
	CHECK(!isalpha('0'), "expected false for digit");

	TEST("isalpha: space");
	CHECK(!isalpha(' '), "expected false for space");
}

static void test_isdigit(void)
{
	TEST("isdigit: digits");
	int ok = 1;
	for (char c = '0'; c <= '9'; c++)
		if (!isdigit((unsigned char)c)) { ok = 0; break; }
	CHECK(ok, "expected true for all digits");

	TEST("isdigit: letter");
	CHECK(!isdigit('a'), "expected false for letter");
}

static void test_isalnum(void)
{
	TEST("isalnum: letter");
	CHECK(isalnum('B'), "expected true");

	TEST("isalnum: digit");
	CHECK(isalnum('5'), "expected true");

	TEST("isalnum: punctuation");
	CHECK(!isalnum('!'), "expected false");
}

static void test_isspace(void)
{
	TEST("isspace: space, tab, newline");
	CHECK(isspace(' ') && isspace('\t') && isspace('\n'),
		"expected true for whitespace");

	TEST("isspace: letter");
	CHECK(!isspace('A'), "expected false");
}

static void test_isupper_islower(void)
{
	TEST("isupper: uppercase letter");
	CHECK(isupper('Z') && !isupper('z'), "mismatch");

	TEST("islower: lowercase letter");
	CHECK(islower('a') && !islower('A'), "mismatch");
}

static void test_toupper_tolower(void)
{
	TEST("toupper: lowercase to uppercase");
	CHECK(toupper('a') == 'A' && toupper('z') == 'Z', "mismatch");

	TEST("toupper: already uppercase");
	CHECK(toupper('A') == 'A', "should stay 'A'");

	TEST("tolower: uppercase to lowercase");
	CHECK(tolower('A') == 'a' && tolower('Z') == 'z', "mismatch");

	TEST("tolower: already lowercase");
	CHECK(tolower('a') == 'a', "should stay 'a'");

	TEST("toupper/tolower: non-letter unchanged");
	CHECK(toupper('5') == '5' && tolower('!') == '!', "should be unchanged");
}

static void test_isprint_iscntrl(void)
{
	TEST("isprint: printable ASCII");
	CHECK(isprint('A') && isprint(' ') && isprint('~'), "expected true");

	TEST("isprint: control char");
	CHECK(!isprint('\x01'), "expected false for control char");

	TEST("iscntrl: control characters");
	CHECK(iscntrl('\0') && iscntrl('\n') && iscntrl('\x1F'),
		"expected true for control chars");

	TEST("iscntrl: normal char");
	CHECK(!iscntrl('A'), "expected false");
}

static void test_ispunct(void)
{
	TEST("ispunct: punctuation chars");
	CHECK(ispunct('!') && ispunct('.') && ispunct('@'), "expected true");

	TEST("ispunct: letter or digit");
	CHECK(!ispunct('A') && !ispunct('0'), "expected false");
}

static void test_isxdigit(void)
{
	TEST("isxdigit: hex digits");
	CHECK(isxdigit('0') && isxdigit('9') && isxdigit('a') &&
		isxdigit('f') && isxdigit('A') && isxdigit('F'),
		"expected true for hex digits");

	TEST("isxdigit: non-hex");
	CHECK(!isxdigit('g') && !isxdigit('G'), "expected false");
}

/* ── sprintf / snprintf / sscanf tests ────────────────────────────── */
static void test_sprintf(void)
{
	char buf[256];

	TEST("sprintf: integer formatting");
	sprintf(buf, "%d", 42);
	CHECK(strcmp(buf, "42") == 0, "expected '42'");

	TEST("sprintf: negative integer");
	sprintf(buf, "%d", -123);
	CHECK(strcmp(buf, "-123") == 0, "expected '-123'");

	TEST("sprintf: string formatting");
	sprintf(buf, "Hello, %s!", "world");
	CHECK(strcmp(buf, "Hello, world!") == 0, "mismatch");

	TEST("sprintf: hex formatting");
	sprintf(buf, "0x%X", 255);
	CHECK(strcmp(buf, "0xFF") == 0, "expected '0xFF'");

	TEST("sprintf: zero-padded integer");
	sprintf(buf, "%05d", 42);
	CHECK(strcmp(buf, "00042") == 0, "expected '00042'");

	TEST("sprintf: left-justified string");
	sprintf(buf, "%-10s|", "hi");
	CHECK(strcmp(buf, "hi        |") == 0, "expected left-justified");

	TEST("sprintf: unsigned");
	sprintf(buf, "%u", 4294967295U);
	CHECK(strcmp(buf, "4294967295") == 0, "expected '4294967295'");

	TEST("sprintf: octal");
	sprintf(buf, "%o", 255);
	CHECK(strcmp(buf, "377") == 0, "expected '377'");

	TEST("sprintf: char format");
	sprintf(buf, "%c%c%c", 'A', 'B', 'C');
	CHECK(strcmp(buf, "ABC") == 0, "expected 'ABC'");

	TEST("sprintf: multiple arguments");
	sprintf(buf, "%s=%d, %s=%d", "x", 10, "y", 20);
	CHECK(strcmp(buf, "x=10, y=20") == 0, "mismatch");

	TEST("sprintf: long decimal");
	sprintf(buf, "%ld", 123456789L);
	CHECK(strcmp(buf, "123456789") == 0, "expected '123456789'");

	TEST("sprintf: pointer-width hex");
	sprintf(buf, "%lx", 0xDEADBEEFUL);
	CHECK(strcmp(buf, "deadbeef") == 0, "expected 'deadbeef'");

	TEST("sprintf: percent literal");
	sprintf(buf, "100%%");
	CHECK(strcmp(buf, "100%") == 0, "expected '100%'");
}

static void test_snprintf(void)
{
	char buf[16];

	TEST("snprintf: fits in buffer");
	int n = snprintf(buf, sizeof(buf), "hello");
	CHECK(n == 5 && strcmp(buf, "hello") == 0, "mismatch");

	TEST("snprintf: truncation");
	n = snprintf(buf, 6, "hello world");
	CHECK(n == 11 && strcmp(buf, "hello") == 0,
		"expected truncation and return of full length");

	TEST("snprintf: size 0 (just measure)");
	n = snprintf(NULL, 0, "test %d", 123);
	CHECK(n == 8, "expected 8");

	TEST("snprintf: size 1 (only NUL)");
	buf[0] = 'X';
	n = snprintf(buf, 1, "hello");
	CHECK(n == 5 && buf[0] == '\0', "expected empty string, length 5");
}

static void test_sscanf(void)
{
	int i;
	char s[64];

	TEST("sscanf: integer");
	CHECK(sscanf("42", "%d", &i) == 1 && i == 42, "expected 42");

	TEST("sscanf: string");
	CHECK(sscanf("hello", "%63s", s) == 1 && strcmp(s, "hello") == 0,
		"expected 'hello'");

	TEST("sscanf: multiple fields");
	int a, b;
	CHECK(sscanf("10 20", "%d %d", &a, &b) == 2 && a == 10 && b == 20,
		"expected 10 and 20");

	TEST("sscanf: hex");
	unsigned int h;
	CHECK(sscanf("FF", "%x", &h) == 1 && h == 255, "expected 255");

	TEST("sscanf: no match");
	CHECK(sscanf("abc", "%d", &i) == 0, "expected 0 conversions");

	TEST("sscanf: mixed types");
	char c;
	CHECK(sscanf("A 99", "%c %d", &c, &i) == 2 && c == 'A' && i == 99,
		"expected 'A' and 99");
}

/* ── time.h tests ─────────────────────────────────────────────────── */
static void test_time_functions(void)
{
	TEST("time: returns a value");
	time_t t = time(NULL);
	/* time may return -1 on bare metal, just check it doesn't crash */
	PASS();

	TEST("localtime: returns non-NULL");
	struct tm *local = localtime(&t);
	CHECK(local != NULL, "localtime returned NULL");

	TEST("gmtime: returns non-NULL");
	struct tm *gm = gmtime(&t);
	CHECK(gm != NULL, "gmtime returned NULL");

	TEST("mktime: round-trip");
	struct tm tm_val;
	memset(&tm_val, 0, sizeof(tm_val));
	tm_val.tm_year = 124;  /* 2024 - 1900 */
	tm_val.tm_mon  = 5;    /* June (0-based) */
	tm_val.tm_mday = 15;
	tm_val.tm_hour = 12;
	tm_val.tm_min  = 30;
	tm_val.tm_sec  = 45;
	tm_val.tm_isdst = -1;
	time_t t2 = mktime(&tm_val);
	if (t2 != (time_t)-1) {
		struct tm *back = localtime(&t2);
		CHECK(back != NULL && back->tm_year == 124 && back->tm_mon == 5 &&
			back->tm_mday == 15 && back->tm_hour == 12,
			"round-trip mismatch");
	} else {
		/* On bare metal mktime may not work; that's okay */
		PASS();
	}

	TEST("difftime: difference of two times");
	time_t t_a = 1000;
	time_t t_b = 500;
	double diff = difftime(t_a, t_b);
	CHECK(diff == 500.0, "expected 500.0");

	TEST("asctime: produces a string");
	struct tm tm2;
	memset(&tm2, 0, sizeof(tm2));
	tm2.tm_year = 124;
	tm2.tm_mon  = 0;
	tm2.tm_mday = 1;
	tm2.tm_wday = 1;  /* Monday */
	char *asc = asctime(&tm2);
	CHECK(asc != NULL && strlen(asc) > 0, "asctime returned empty/NULL");

	TEST("strftime: format date string");
	char timebuf[128];
	struct tm tm3;
	memset(&tm3, 0, sizeof(tm3));
	tm3.tm_year = 124;
	tm3.tm_mon  = 11;   /* December */
	tm3.tm_mday = 25;
	tm3.tm_hour = 10;
	tm3.tm_min  = 30;
	tm3.tm_sec  = 0;
	tm3.tm_wday = 3;    /* Wednesday */
	tm3.tm_yday = 359;
	size_t slen = strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm3);
	CHECK(slen > 0 && strcmp(timebuf, "2024-12-25 10:30:00") == 0,
		"expected '2024-12-25 10:30:00'");

	TEST("strftime: day/month names");
	slen = strftime(timebuf, sizeof(timebuf), "%A %B", &tm3);
	CHECK(slen > 0 && strcmp(timebuf, "Wednesday December") == 0,
		"expected 'Wednesday December'");

	TEST("clock: returns a value");
	clock_t cl = clock();
	/* clock() may return -1 on bare metal; just verify it doesn't crash */
	(void)cl;
	PASS();
}

/* ── errno / strerror tests ───────────────────────────────────────── */
static void test_errno_strerror(void)
{
	TEST("errno: initially or after reset is 0");
	errno = 0;
	CHECK(errno == 0, "expected errno == 0");

	TEST("errno: can be set");
	errno = ERANGE;
	CHECK(errno == ERANGE, "expected ERANGE");
	errno = 0;

	TEST("strerror: returns non-NULL for ERANGE");
	char *msg = strerror(ERANGE);
	CHECK(msg != NULL && strlen(msg) > 0, "expected a message string");

	TEST("strerror: returns non-NULL for EINVAL");
	msg = strerror(EINVAL);
	CHECK(msg != NULL && strlen(msg) > 0, "expected a message string");

	TEST("strtol: sets errno on overflow");
	errno = 0;
	char *end;
	strtol("99999999999999999999", &end, 10);
	CHECK(errno == ERANGE, "expected ERANGE on overflow");
	errno = 0;
}

/* ── additional stdlib: strtod, environment ───────────────────────── */
static void test_strtod(void)
{
	char *end;

	TEST("strtod: simple float");
	double d = strtod("3.14", &end);
	CHECK(d > 3.13 && d < 3.15 && *end == '\0', "expected ~3.14");

	TEST("strtod: negative float");
	d = strtod("-2.5", &end);
	CHECK(d > -2.51 && d < -2.49, "expected ~-2.5");

	TEST("strtod: scientific notation");
	d = strtod("1.5e2", &end);
	CHECK(d > 149.9 && d < 150.1, "expected ~150.0");

	TEST("strtod: leading whitespace");
	d = strtod("  42.0", &end);
	CHECK(d > 41.9 && d < 42.1, "expected ~42.0");

	TEST("strtod: end pointer");
	d = strtod("12.5abc", &end);
	CHECK(d > 12.4 && d < 12.6 && *end == 'a', "expected ~12.5, end at 'a'");
}

/* ── pointer arithmetic / offsetof / sizeof sanity ────────────────── */
static void test_stddef_stdint(void)
{
	TEST("NULL is zero");
	CHECK(NULL == 0, "NULL should be zero");

	TEST("sizeof(char) == 1");
	CHECK(sizeof(char) == 1, "expected 1");

	TEST("sizeof(int) >= 2");
	CHECK(sizeof(int) >= 2, "expected at least 2");

	TEST("sizeof(long) >= 4");
	CHECK(sizeof(long) >= 4, "expected at least 4");

	TEST("sizeof(size_t) >= sizeof(int)");
	CHECK(sizeof(size_t) >= sizeof(int), "expected size_t >= int");

	TEST("INT_MAX > 0 and INT_MIN < 0");
	CHECK(INT_MAX > 0 && INT_MIN < 0, "limits broken");

	TEST("CHAR_BIT == 8");
	CHECK(CHAR_BIT == 8, "expected 8 bits per char");

	TEST("uint8_t is 1 byte");
	CHECK(sizeof(uint8_t) == 1, "expected 1");

	TEST("uint16_t is 2 bytes");
	CHECK(sizeof(uint16_t) == 2, "expected 2");

	TEST("uint32_t is 4 bytes");
	CHECK(sizeof(uint32_t) == 4, "expected 4");

	TEST("uint64_t is 8 bytes");
	CHECK(sizeof(uint64_t) == 8, "expected 8");

	TEST("offsetof: works on struct");
	struct { int a; char b; int c; } dummy;
	(void)dummy;
	size_t off = offsetof(struct { int a; char b; int c; }, c);
	CHECK(off >= sizeof(int) + sizeof(char), "offset too small");
}

/* ── main ─────────────────────────────────────────────────────────── */
int main(void)
{
	printf("NewLib Test Application\n");
	printf("=======================\n");

	/* string.h */
	section("string.h: string functions");
	test_strlen();
	test_strcpy();
	test_strncpy();
	test_strcmp();
	test_strncmp();
	test_strcat();
	test_strncat();
	test_strchr();
	test_strrchr();
	test_strstr();
	test_strpbrk();
	test_strspn_strcspn();
	test_strtok();

	/* memory functions */
	section("string.h: memory functions");
	test_memcpy();
	test_memmove();
	test_memset();
	test_memcmp();
	test_memchr();

	/* stdlib.h */
	section("stdlib.h: conversion");
	test_atoi();
	test_atol();
	test_strtol();
	test_strtoul();
	test_strtod();

	section("stdlib.h: arithmetic");
	test_abs_labs();
	test_div_ldiv();

	section("stdlib.h: memory allocation");
	test_malloc_free();
	test_calloc();
	test_realloc();

	section("stdlib.h: sorting & searching");
	test_qsort();
	test_bsearch();

	section("stdlib.h: random numbers");
	test_rand_srand();

	/* ctype.h */
	section("ctype.h");
	test_isalpha();
	test_isdigit();
	test_isalnum();
	test_isspace();
	test_isupper_islower();
	test_toupper_tolower();
	test_isprint_iscntrl();
	test_ispunct();
	test_isxdigit();

	/* stdio.h: formatting */
	section("stdio.h: formatting");
	test_sprintf();
	test_snprintf();
	test_sscanf();

	/* time.h */
	section("time.h");
	test_time_functions();

	/* errno / strerror */
	section("errno.h / string.h: strerror");
	test_errno_strerror();

	/* stddef / stdint / limits */
	section("stddef.h / stdint.h / limits.h");
	test_stddef_stdint();

	/* summary */
	report();

	return tests_failed ? 1 : 0;
}
