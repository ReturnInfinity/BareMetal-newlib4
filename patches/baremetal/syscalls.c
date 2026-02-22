// ============================================================================
// BareMetal -- a 64-bit OS written in Assembly for x86-64 systems
// Copyright (C) 2008-2016 Return Infinity -- see LICENSE.TXT
//
// Syscalls glue for Newlib
// ============================================================================


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <errno.h>

unsigned char inportbyte(unsigned int port);
void outportbyte(unsigned int port,unsigned char value);

// --- Process Control ---

// exit -- Exit a program without cleaning up files
void _exit(int val)
{

}

// execve -- Transfer control to a new process
// Minimal implementation
int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}

// environ - A pointer to a list of environment variables and their values
// Minimal implementation
char *__env[1] = { 0 };
char **environ = __env;

// getpid -- Process-ID
// Return 1 by default
int _getpid(void)
{
	return 1;
}

// fork -- Create a new process
// Minimal implementation
int _fork(void)
{
	errno = ENOTSUP; // EAGAIN?
	return -1;
}

// kill -- Send a signal
int _kill(int pid, int sig)
{
	if(pid == 1)
		_exit(sig);

	errno = EINVAL;
	return -1;
}

// wait -- Wait for a child process
// Minimal implementation
int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

// --- I/O ---

// isatty - Query whether output stream is a terminal
int _isatty(int fd)
{
	if (fd == 0 || fd == 1 || fd == 2)
		return 1;
	else
		return 0;
}

// close - Close a file
// Minimal implementation
int _close(int file)
{
	return -1;
}

// link - Establish a new name for an existing file
// Minimal implementation
int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

// lseek - Set position in a file
// Minimal implementation
int _lseek(int file, int ptr, int dir)
{
	return 0;
}

// open - Open a file
// Minimal implementation
int _open(const char *name, int flags, ...)
{
	return -1;
}

// read - Read from a file
int _read(int file, char *ptr, int len)
{
	if (file == 0) // STDIN
	{
		asm volatile ("call *0x00100010" : "=c"(len) : "c"(len), "D"(ptr));
		ptr[len] = '\n'; // BareMetal does not add a newline after keyboard input ...
		ptr[len+1] = 0; // ... but C expects it.
		len+=1;
	}
	return len;
}

// write - Write to a file
int _write(int file, char *ptr, int len)
{
	if (file == 1 || file == 2) // STDOUT = 1, STDERR = 2
	{
		asm volatile ("call *0x00100018" : : "S"(ptr), "c"(len)); // Make sure source register (RSI) has the string address (str)
	}
	else
	{
		// File!
		return -1;
	}
	return len;
}

// fstat - Status of an open file.
// Minimal implementation
int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

// stat - Status of a file
// Minimal implementation
int _stat(const char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

// unlink - Remove a file's directory entry
int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

// --- Memory ---

/* _end is set in the linker command file */
//extern caddr_t _end;

//#define PAGE_SIZE 2097152ULL
//#define PAGE_MASK 0xFFFFFFFFFFE00000ULL
//#define HEAP_ADDR (((unsigned long long)&_end + PAGE_SIZE) & PAGE_MASK)

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
// sbrk - Increase program data space

caddr_t _sbrk(int incr)
{
//	asm volatile ("xchg %bx, %bx"); // Debug
	extern char __bss_stop; /* Defined by the linker */
	static char *heap_end;
	char *prev_heap_end;
//	write (2, "sbrk\n", 5);
	if (heap_end == 0)
	{
//		write (2, "sbrk end\n", 9);
		heap_end = &__bss_stop;
	}
	prev_heap_end = heap_end;
//	if (heap_end + incr > stack_ptr) {
//		write (2, "Heap and stack collision\n", 25);
//		abort ();
//	}
	heap_end += incr;
	return (caddr_t) prev_heap_end;
}


// --- Other ---

// Read a BCD value from the CMOS RTC
static int cmos_read_bcd(unsigned char reg)
{
	unsigned char bcd;
	outportbyte(0x70, reg);
	bcd = inportbyte(0x71);
	return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

// Convert broken-down UTC time to seconds since Unix epoch.
// This avoids calling mktime() which depends on malloc/timezone.
static long long epoch_from_utc(int year, int mon, int mday, int hour, int min, int sec)
{
	// Days in each month (non-leap year)
	static const int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	long long days = 0;
	int y, m;

	// Sum days for complete years since 1970
	for (y = 1970; y < year; y++)
	{
		days += 365;
		if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
			days += 1;
	}

	// Sum days for complete months in the current year
	for (m = 0; m < mon - 1; m++)
	{
		days += mdays[m];
		if (m == 1 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
			days += 1;
	}

	// Add remaining days in the current month
	days += mday - 1;

	return days * 86400LL + hour * 3600LL + min * 60LL + sec;
}

// gettimeofday --
int _gettimeofday(struct timeval *p, void *z)
{
	int year, mon, mday, hour, min, sec;

	year = 2000 + cmos_read_bcd(0x09);
	mon  = cmos_read_bcd(0x08);
	mday = cmos_read_bcd(0x07);
	hour = cmos_read_bcd(0x04);
	min  = cmos_read_bcd(0x02);
	sec  = cmos_read_bcd(0x00);

	p->tv_sec = (long) epoch_from_utc(year, mon, mday, hour, min, sec);
	p->tv_usec = 0;

	return 0;
}

// times - Timing information for current process.
clock_t _times(struct tms *buf){
	// get current process time
	unsigned long long proc_time = 1;

	/*
	 * Process time is assumed to be the CPU time charged for
	 * the execution of user instructions of the calling process.
	 * This is not necessary accurate since CPU time may also be
	 * charged for execution by the system on behalf of the calling
	 * process (i.e. when a syscall is executed); the ability to
	 * differentiate user and system time should be added in future
	 * development.
	 */
	buf->tms_utime = proc_time;
	buf->tms_stime = 0;
	buf->tms_cutime = 0;
	buf->tms_cutime = 0;

	return proc_time;
}

unsigned char inportbyte(unsigned int port)
{
	// read a byte from a port
	unsigned char ret;
	asm volatile ("inb %%dx,%%al":"=a"(ret):"d"(port));
	return ret;
}

void outportbyte(unsigned int port,unsigned char value)
{
	// write a byte to a port
	asm volatile ("outb %%al,%%dx": :"d"(port),"a"(value));
}

// EOF
