/* include/config.h.  Generated automatically by configure.  */
/* include/config.h.in.  Generated automatically from configure.in by autoheader.  */
/*
	Compiler/Machine-Specific Configuration
*/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you support file names longer than 14 characters.  */
#define HAVE_LONG_FILE_NAMES 1

/* Define if you have a working `mmap' system call.  */
#define HAVE_MMAP 1

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if major, minor, and makedev are declared in <mkdev.h>.  */
/* #undef MAJOR_IN_MKDEV */

/* Define if major, minor, and makedev are declared in <sysmacros.h>.  */
/* #undef MAJOR_IN_SYSMACROS */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if the X Window System is missing or not being used.  */
/* #undef X_DISPLAY_MISSING */

/* "Proper" package name */
#define PROGRAM "QuakeForge: UQuake"

/* Define this to the Quake version you support */
#define QUAKE_VERSION "1.09"

/* Define this to the QSG standard version you support */
#define QSG_VERSION "1.0"

/* Define if you want to use QF-style defaults instead of Id-style */
/* #undef NEWSTYLE */

/* Define this to the location of the global config file */
#define FS_GLOBALCFG "/etc/uquake.conf"

/* Define this to the shared game directory root */
#define FS_SHAREPATH "."

/* Define this to the unshared game directory root */
#define FS_USERPATH "."

/* Define this to the base game for the engine to load */
#define BASEGAME "id1"

/* Define this if you want to use Intel assembly optimizations */
#define USE_INTEL_ASM 1

/* Define if you have the XFree86 DGA extension */
#define HAVE_DGA 1

/* Define if you have the XFree86 VIDMODE extension */
#define HAVE_VIDMODE 1

/* Define this if you have GLX */
#define HAVE_GLX 1

/* Define this if you have GL_COLOR_INDEX8_EXT in GL/gl.h */
#define HAVE_GL_COLOR_INDEX8_EXT 1

/* Define this if you are using a version of Mesa with X mode change support */
/* #undef HAVE_XMESA */

/* Define this if C symbols are prefixed with an underscore */
/* #undef HAVE_SYM_PREFIX_UNDERSCORE */

/* Define this if your system has socklen_t */
#define HAVE_SOCKLEN_T 1

/* Define this if your system has size_t */
#define HAVE_SIZE_T 1

/* Define this if you have ss_len member in struct sockaddr_storage (BSD) */
/* #undef HAVE_SS_LEN */

/* Define this if you have sin6_len member in struct sockaddr_in6 (BSD) */
/* #undef HAVE_SIN6_LEN */

/* Define this if you have sa_len member in struct sockaddr (BSD) */
/* #undef HAVE_SA_LEN */

/* Define if you have the dlopen function.  */
#define HAVE_DLOPEN 1

/* Define this to something sane if you don't have stricmp */
#define stricmp strcasecmp

/* Define if you have the _ftime function.  */
/* #undef HAVE__FTIME */

/* Define if you have the _snprintf function.  */
/* #undef HAVE__SNPRINTF */

/* Define if you have the _vsnprintf function.  */
/* #undef HAVE__VSNPRINTF */

/* Define if you have the connect function.  */
#define HAVE_CONNECT 1

/* Define if you have the dlopen function.  */
#define HAVE_DLOPEN 1

/* Define if you have the fcntl function.  */
#define HAVE_FCNTL 1

/* Define if you have the ftime function.  */
#define HAVE_FTIME 1

/* Define if you have the getaddrinfo function.  */
#define HAVE_GETADDRINFO 1

/* Define if you have the gethostbyname function.  */
#define HAVE_GETHOSTBYNAME 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the getnameinfo function.  */
#define HAVE_GETNAMEINFO 1

/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the getwd function.  */
#define HAVE_GETWD 1

/* Define if you have the mkdir function.  */
#define HAVE_MKDIR 1

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the stat function.  */
#define HAVE_STAT 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strsep function.  */
#define HAVE_STRSEP 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <arpa/inet.h> header file.  */
#define HAVE_ARPA_INET_H 1

/* Define if you have the <asm/io.h> header file.  */
#define HAVE_ASM_IO_H 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the <dsound.h> header file.  */
/* #undef HAVE_DSOUND_H */

/* Define if you have the <errno.h> header file.  */
#define HAVE_ERRNO_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <fnmatch.h> header file.  */
#define HAVE_FNMATCH_H 1

/* Define if you have the <initguid.h> header file.  */
/* #undef HAVE_INITGUID_H */

/* Define if you have the <linux/soundcard.h> header file.  */
#define HAVE_LINUX_SOUNDCARD_H 1

/* Define if you have the <machine/soundcard.h> header file.  */
/* #undef HAVE_MACHINE_SOUNDCARD_H */

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <mgraph.h> header file.  */
/* #undef HAVE_MGRAPH_H */

/* Define if you have the <mme/mme_public.h> header file.  */
/* #undef HAVE_MME_MME_PUBLIC_H */

/* Define if you have the <mme/mmsystem.h> header file.  */
/* #undef HAVE_MME_MMSYSTEM_H */

/* Define if you have the <mmsystem.h> header file.  */
/* #undef HAVE_MMSYSTEM_H */

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netdb.h> header file.  */
#define HAVE_NETDB_H 1

/* Define if you have the <netinet/in.h> header file.  */
#define HAVE_NETINET_IN_H 1

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1

/* Define if you have the <sys/asoundlib.h> header file.  */
/* #undef HAVE_SYS_ASOUNDLIB_H */

/* Define if you have the <sys/audioio.h> header file.  */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define if you have the <sys/dir.h> header file.  */
#define HAVE_SYS_DIR_H 1

/* Define if you have the <sys/filio.h> header file.  */
/* #undef HAVE_SYS_FILIO_H */

/* Define if you have the <sys/io.h> header file.  */
#define HAVE_SYS_IO_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/mman.h> header file.  */
#define HAVE_SYS_MMAN_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/socket.h> header file.  */
#define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/soundcard.h> header file.  */
#define HAVE_SYS_SOUNDCARD_H 1

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/timeb.h> header file.  */
#define HAVE_SYS_TIMEB_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <windows.h> header file.  */
/* #undef HAVE_WINDOWS_H */

/* Define if you have the db library (-ldb).  */
/* #undef HAVE_LIBDB */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Name of package */
#define PACKAGE "uquake"

/* Version number of package */
#define VERSION "0.1.99pre2"

/* Define if struct ioc_read_toc_single_entry has field entry. */
/* #undef HAVE_STRUCT_IOC_READ_TOC_SINGLE_ENTRY_ENTRY */

#endif // _CONFIG_H_
