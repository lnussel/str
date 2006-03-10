/*
 * print addresses and content of variables ordered by address
 *
 * in the function you want to examine e.g variables foo and bar add
 * the following macros:
 *
 * STACK_S
 *   VAR(foo)
 *   VAR(bar)
 * STACK_E
 *
 * the macros EBP and FRAMESTART can be used to print the addresses
 * of the ebp register (i386 only) resp the start of the current
 * frame (uses gcc __builtin_frame_address)
 *
 * Anywhere in the code you may add CALLGDB to set a breakpoint for
 * gdb
 *
 * -- by lnussel@suse.de
 */

/* TODO: deal with unaligned variables better */

#ifndef _STACKDUMP_H
#define _STACKDUMP_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

static void print_addrs(char* name, void* start, size_t size, ...) __attribute__((noinline));

struct var_s
{
    void* addr;
    char* name;
    size_t size;
};

static int compare(const void* a, const void* b)
{
    return (((const struct var_s*)a)->addr < ((const struct var_s*)b)->addr);
}

static void print_addrs(char* name, void* addr, size_t size, ...)
{
    va_list ap;
    void* p = NULL;
    unsigned i;
    struct var_s stack[32];
    unsigned maxvar = sizeof(stack)/sizeof(stack[0]);
    unsigned increment = sizeof(void*);
    unsigned unaligned = 0;

    va_start(ap, size);
    for(i=0; i < maxvar  && name; ++i )
    {
	stack[i].name = name;
	stack[i].addr = addr;
	stack[i].size = size;

	name = va_arg(ap, char*);
	addr = va_arg(ap, void*);
	size = va_arg(ap, size_t);
    }
    va_end(ap);

    if(i < maxvar)
	stack[i].name = NULL;

    qsort(stack, i, sizeof(stack[0]), compare);

    for(i=0; i < maxvar && stack[i].name; ++i )
    {
	if(((uintptr_t)stack[i].addr)%increment)
	{
	    ++unaligned;
	    printf("%s unaligned at %p\n", stack[i].name, stack[i].addr);
	    continue;
	}
	if(i)
	{
	    p = (void*)((uintptr_t)stack[i-unaligned-1].addr-increment);
	    for(; p > stack[i].addr; p=(void*)((uintptr_t)p-increment))
	    {
		printf("%p%c %0*lx",
			p,
			((uintptr_t)stack[i].addr % increment)?'*':' ',
			(unsigned)(sizeof(void*)*2),
			*(unsigned long*)p);
		if(p < (void*)((uintptr_t)stack[i].addr + stack[i].size))
		    puts(" .");
		else
		    puts("");

		if(p > (void*)((uintptr_t)stack[i].addr +10*increment))
		{
		    unsigned skip = (uintptr_t)p-((uintptr_t)stack[i].addr+2*increment);
		    printf("%*c  [%d bytes skipped]\n", (unsigned)(sizeof(void*)*2)+2, ' ', skip);
		    p=(void*)((uintptr_t)p-skip);
		}
	    }
	    unaligned = 0;
	}
	printf("%p%c %0*lx %s\n",
		stack[i].addr,
		((uintptr_t)stack[i].addr%increment)?'*':' ',
		(unsigned)(sizeof(void*)*2),
		*(unsigned long*)stack[i].addr,
		stack[i].name);
    }
}

#ifdef __i386__
#  define STACK_S \
    { \
	void* ebp = NULL; \
	__asm__ __volatile__ ("mov %%ebp, %0" : "=m" (ebp)); \
	print_addrs(
#else
#  define STACK_S \
    { \
	print_addrs(
#endif

#define STACK_E \
	    (void*)NULL); \
    }

#if defined(__i386__)
#  define EBP \
	"%ebp", ebp, sizeof(ebp), \
	"%ebp+4", (uintptr_t)ebp+4, sizeof(ebp),
#else
#  define EBP
#endif

#define FRAMESTART \
    "<- frame address", __builtin_frame_address(0), sizeof(void*),

#define VAR(x) #x, &x, sizeof(x),

#define CALLGDB \
        __asm__ __volatile__ (".byte 0xCC\n");

#endif

/* vim: sw=4
 * */
