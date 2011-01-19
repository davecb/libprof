/*
 * testProgram -- see if the profiler works
 */
#include <stdio.h>
#include <stdlib.h>
#include <fmtmsg.h>

#include <mba/suba.h>

 int
main(int argc, char *argv[]) {
	unsigned char mem[0x7FFFF];
	struct allocator *al = suba_init(mem, 0x7FFFF, 1, 0);
	int	i, 
		j = 100000;

#ifdef DEBUG
	profInit();
#endif
	/* sleep(30); */
	if (argc > 1) {
		j = atoi(argv[1]);	
	}
	for (i=0; i < j; i++) {
		void *p = allocator_alloc(al, 4, 0);
		(void) a64l("hello");
	}
	(void) addseverity(42, "the world");
#ifdef DEBUG
	profFini();
#endif
	return 0;
}
