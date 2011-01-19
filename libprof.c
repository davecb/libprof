/*
 * libprof -- intercept calls to libc and report profiling information
 *
 *   Copyright (C) 2005 David Collier-Brown, Michael B.Allen
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "libprof.h"


/* Interposer variables, implemented as file statics. */
static void printProfile(int);
static double summarize(struct prof_t *p);
static char *getProgName();
static void sigHandler(int s, siginfo_t *reason, void *context);

/* Table of stats, imported from .i files. */
extern size_t ProfCount;
extern struct prof_t ProfStats[];
#define OVERHEAD 0
#define REST ProfCount+1

/* Misc. */
#define UNSORTED 0
#define SORTED 1

#ifdef __sun
extern int gettimeofday(struct timeval *, void *);
#endif
#ifdef __linux
extern int gettimeofday(struct timeval *, __timezone_ptr_t);
#define MAXLINE 1024
#endif

static double StartTime = 0.0;

#define RTLD_SETTING RTLD_NOW /* RTLD_LAZY for production, _NOW for dev't. */

#ifdef linux
#define RTLD_NEXT ((void *) -1l)
#endif

/*
 * profInit -- get start time and set upsignal handlers for
 *	SIGTERM and SIGUSR1. SIGTERM is the traditional ^C, 
 *	and terminates the program after printing the stats
 *	in soretd order.
 */
 void 
profInit() {
	struct sigaction newAct, oldAct;
	struct timeval t;

	/* Set up signal handler for SIGTERM */
	newAct.sa_sigaction = sigHandler;
	(void) sigemptyset(&newAct.sa_mask);
	/* newAct.sa_flags = SA_SIGINFO|SA_RESTART; */
	if (sigaction(SIGTERM, &newAct, &oldAct) == -1) {
		(void) fprintf(stderr, "libprof: sigaction(SIGTERM) failed\n");
		(void) _exit(-1);
	}
	if (sigaction(SIGUSR1, &newAct, &oldAct) == -1) {
		(void) fprintf(stderr, "libprof: sigaction(SIGTUSR1) failed\n");
		(void) _exit(-1);
	}

	/* And start data collection. */
	(void) gettimeofday(&t, NULL);
	StartTime = (double) (t.tv_sec + (t.tv_usec/1000000.0)); 
}

/*
 * profFini -- print out final results. 
 */
 void
profFini() {
	printProfile(SORTED);
}

/*
 * sigHandler -- print profiling information, and optionally exit.
 */
 /*ARGSUSED*/
 static void
sigHandler(int s, siginfo_t *reason, void *context) {

	if (s == SIGUSR1) {
		/* Don't sort until done collecting. */
	        printProfile(UNSORTED);
	}
	else {
		printProfile(SORTED);
	}
	/* Return zero, as we succeeded in profiling. */
	/* Odd, but that's what truss -c does, so... */
	(void) exit(0);
}
     

/*
 * compare -- compare two prof_t's, for qsort in printProfile
 */
static int compare(const void *this, const void *that) {
	double d;

	d = (((struct prof_t *)that)->time - 
		((struct prof_t *)this)->time);
	if (d > 0.00000001) {
		return 1;
	}
	else if (d < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

/*
 * summarize -- sum up a table of struct prof_t *s 
 */
 static double 
summarize(struct prof_t *p) {
	double sum = 0.0;

	while (p->name != NULL) {
		sum += p->time;
		p++;
	}
	return sum;
}
	
/*
 * printProfile -- print the results
  */
 static void
printProfile(int sort) {
	struct prof_t	*p;
	struct timeval t;
	double	totalTime, programTime;
	double	overhead = 0.0;
	static void *myprintf = NULL;

	/* Initialize. */
	(void) gettimeofday(&t, NULL);
	totalTime = (double) (t.tv_sec + (t.tv_usec/1000000.0)) - StartTime; 
	myprintf = dlsym(RTLD_NEXT, "fprintf");
	assert(myprintf != NULL);
#define MYFPRINTF ((int (*)(FILE *, char *, ...))myprintf)

	/* Print the header. */
	(void) MYFPRINTF(stderr,"\nResource usage profile for "
		"process %ld, %s\n", getpid(), getProgName());
	(void) MYFPRINTF(stderr, 
		"%-30s %15s %-5s %11s %-10s\n",
		 "Name", "Duration", "Pct.", "Calls", "Sec/Call");

	(void) MYFPRINTF(stderr,
		"%-30s %15s %5s %11s %10s\n",
		"----", "------------", "-----", "-----", "----------");
	/* The time for the whole program. */
	(void) MYFPRINTF(stderr, 
		"%-30.30s %15.8f %4.1f %11ld %.8f\n",
		"Entire program",
		totalTime,
		(double) 100.0,
		(long) 1,
		totalTime);

	/* Add overhead at ~0.00002628 seconds per interface */
	/* for version using hrtime... */
#ifdef USING_HRTIME
	for (p=&ProfStats[0]; p->name != NULL; p++) {
		if (p->calls > 0) {
			overhead += 0.00002628;
		}
	}
	ProfStats[OVERHEAD].calls = 1;
	ProfStats[OVERHEAD].time = overhead * 1.0E9;
#else
	overhead = 0.00002628;
	ProfStats[OVERHEAD].calls = 1;
	ProfStats[OVERHEAD].time = overhead;
#endif

	/* print time for the non-library parts of the program. */
	programTime = totalTime - summarize(&ProfStats[0]);
	(void) MYFPRINTF(stderr, 
		"%-30.30s %15.8f %5.2f %11ld %.8f\n",
		"Non-library time",
		programTime,
		(double) programTime * 100.0 / totalTime,
		(long) 1,
		programTime);
	(void) MYFPRINTF(stderr,"%-30s\n","----");

	/* Sort the table, which makes further collection invalid. */
	if (sort == SORTED) {
		qsort(ProfStats, ProfCount, sizeof(struct prof_t), compare);
	}

	/* Print it out. */
	for (p=&ProfStats[0]; p->name != NULL; p++) {
		if (p->calls > 0) {
			(void) MYFPRINTF(stderr, 
				"%-30.30s %15.8f %5.2f %11ld %.8f\n",
				p->name,
				p->time, /* Time, seconds */
				(p->time * 100.0) / totalTime,
				p->calls, 
				p->time / p->calls);
		}
	}
}



/*
 * getProgName -- get the name from a struct which contains it.
 */
#if defined(__sun)
 char *
getProgName() {
        int     fdp;
        static struct psinfo pbuff;

	if ((fdp = open("/proc/self/psinfo", O_RDONLY, 0)) < 0) {
		return NULL;
	}	
	if (read(fdp, (void *)&pbuff, sizeof(pbuff)) != (int)sizeof(pbuff)) {
		(void) close(fdp);
		return NULL;
	}
	(void) close(fdp);
	return pbuff.pr_fname;
}


#elif defined(__linux)

static char *skipPrefix(char *);

 char *
getProgName() {
	int     fdp;
        static char line[MAXLINE];

	if ((fdp = open("/proc/self/cmdline", 0 /* O_RDONLY */,0)) < 0) {
		return NULL;
	}	
	if (read(fdp, (void *)&line, sizeof(line)) <= 0) {
		(void) close(fdp);
		return NULL;
	}
	(void) close(fdp);
	return skipPrefix(&line[0]);
}

/*
 * skipPrefix -- get to the basename part of a path
 */
 static char *
skipPrefix(char *p) {
	char	*q;

	if ((q = strrchr(p, '/')) != NULL) {
		return q+1;
	}
	else {
		return p;	
	}
}

#ifdef GETHRTIME
/*
 * gethrtime -- hi-res timer for Linux, courtesy of 
 *	Sternberg. Returns time in nanoseconds, aka 
 *	cycles/1GHz
 */
unsigned long gethrtime(void) {
	static unsigned long cpuSpeed = 0;
	unsigned long result;

	if (cpuSpeed == 0) {
		cpuSpeed = GetCpuSpeed();
	}

	asm("rdtsc":"=A" (result));
	return (unsigned long) (result/cpuSpeed);
}

/*
 * GetCpuSpeed -- get cpu speed, courtesy of Michael B Allen.
 * Returns CPU clock in khz, from playstation linux port.
 */
static unsigned int GetCpuSpeed(void) {
	unsigned long long int tscstart, tscstop;
	unsigned int start, stop;

	tscstart = rdtsc();
	start = GetTimer();
	usec_sleep(50000);
	stop = GetTimer();
	tscstop = rdtsc();

	return((tscstop-tscstart)/((stop-start)/1000.0));
}
#endif /* GETHRTIME */

#endif /* linux */
