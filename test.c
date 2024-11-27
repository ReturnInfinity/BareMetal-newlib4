#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	struct tm *local;
	time_t t;
	int i=0;

	printf("NewLib Test Application\n=======================\n");

	// Test time
	t = time(NULL);
	local = localtime(&t);
	printf("Local time and date: %s\n", asctime(local));

	printf("%s %d\n", "printf-example:", 1234);

	return 0;
}
