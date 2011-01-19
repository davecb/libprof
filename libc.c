#include "libprof.h"

/* Includes for intercepted functions. */
#include <math.h>	/* For floor(). */
#include <dirent.h>	/* For getdents(). */
#include <fcntl.h>	/* For creat(). */
#include <fmtmsg.h>	/* For fmtmsg(). */
#include <ftw.h>	/* For ftw(). */
#include <glob.h>	/* For globfree(). */
#include <iconv.h>	/* For iconv(). */
#include <limits.h>	/* For times(). */
#include <nl_types.h>	/* For catclose(). */
#include <pthread.h>	/* For sigwait(). */
#include <poll.h>	/* For poll(). */
#include <regex.h>	/* For regcomp(). */
#include <search.h>	/* For hsearch(). */
#include <setjmp.h>	/* For setjmp(). */
#include <signal.h>	/* For sigaction(). */
#include <stdarg.h>	/* For varargs functions. */
#include <stdio.h>	/* For getrctl(). */
#include <stdlib.h>	/* For sigwait(). */
#include <string.h>	/* For sigwait(). */
#include <stropts.h>	/* For getmsg(). */
#include <sys/ipc.h>	/* For semctl(). */
#include <sys/mman.h>	/* For sigaltstack(). */
#include <sys/mount.h>	/* For umount(). */
#include <sys/msg.h>	/* For msgctl(). */
#include <sys/resource.h>	/* For getrlimit(). */
#include <sys/sem.h>	/* For semctl(). */
#include <sys/shm.h>	/* For shmctl(). */
#include <sys/stat.h>	/* For chmod(). */
#include <sys/statvfs.h>	/* For fstatvfs(). */
#include <sys/swap.h>	/* For swapctl(). */
#include <sys/time.h>	/* For adjtime(). */
#include <sys/timeb.h>	/* For ftime(). */
#include <sys/times.h>	/* For times(). */
#include <sys/types.h>	/* For chmod(). */
#include <sys/uio.h>	/* For write(). */
#include <sys/utsname.h> /* For uname(). */
#include <sys/wait.h>	/* For wait(). */
#include <termios.h>	/* For cfgetispeed(). */
#include <time.h>	/* For time(). */
#include <ucontext.h>	/* For getcontext(). */
#include <ulimit.h>	/* For ulimit(). */
#include <unistd.h>	/* For access(). */
#include <unistd.h>	/* For resolvepath(). */
#include <ustat.h>	/* For ustat(). */
#include <utime.h>	/* For utime(). */
#include <utmpx.h>	/* For getutxent(). */
#include <wait.h>	/* For waitid(). */
#include <wchar.h>	/* For btowc(). */
#include <wctype.h>	/* For towctrans(). */
#include <wordexp.h>	/* For wordexp(). */

#ifdef __sun /* Solaris-only includes */
#include <getwidth.h>   /* For getwidth(). */
#include <ieeefp.h>	/* For fpclass(). */
#include <mon.h>	/* For monitor(). */
#include <rctl.h>	/* For getrctl(). */
#include <synch.h>	/* For sigwait(). */
#include <sys/acl.h>	/* For acl(). */
#include <sys/exacct.h>	/* For getacct(). */
#include <sys/fsid.h>	/* For sysfs(). */
#include <sys/fstyp.h>	/* For sysfs(). */
#include <sys/mnttab.h>	/* For getmntent(). */
#include <sys/processor.h> /* For getcpuid(). */
#include <sys/systeminfo.h>	/* For sysinfo(). */
#include <sys/task.h>	/* For getprojid(). */
#include <sys/uadmin.h>	/* For uadmin(). */
#include <sys/vfstab.h> /* For getvfsany(). */
#endif /* __sun */

#include "libc.i"

