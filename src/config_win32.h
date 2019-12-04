/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.
**********/

#ifdef WIN32
#ifndef _CONFIG_WIN32_H
#define _CONFIG_WIN32_H

#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>   /* abs() */
#include <string.h>
#include <winsock2.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_IPv6
#ifdef MUSICA_IPV6
#include <winsock6.h>
#else
#ifdef WIN2K_IPV6
#include <ws2tcpip.h>
#include <tpipv6.h>
#else
#include <ws2ip6.h>
#include <ws2tcpip.h>
#endif
#endif
#endif

#ifndef MUSICA_IPV6
#include <ws2tcpip.h>
#endif

#include <mmreg.h>
//#include <msacm.h>
#include <mmsystem.h>
#include <windows.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>

typedef int		ttl_t;
typedef unsigned int	fd_t;

/*
 * the definitions below are valid for 32-bit architectures and will have to
 * be adjusted for 16- or 64-bit architectures
 */
//typedef u_char		uint8_t;
//typedef u_short		uint16_t;
//typedef u_long		uint32_t;
//typedef char		int8_t;
//typedef short		int16_t;
//typedef long		int32_t;
//typedef __int64		int64_t;
typedef unsigned long	in_addr_t;

#ifndef TRUE
#define FALSE	0
#define	TRUE	1
#endif /* TRUE */

#define USERNAMELEN	8

#define WORDS_SMALLENDIAN
#define NEED_INET_ATON
#define NEED_DRAND48
#define NEED_GETTIMEOFDAY

#ifdef NDEBUG
#define assert(x) if ((x) == 0) fprintf(stderr, "%s:%u: failed assertion\n", __FILE__, __LINE__)
#else
#include <assert.h>
#endif

#include <time.h>		/* For clock_t */

//#define inline
//#define __inline     

#define AUDIO_MICROPHONE	1
#define AUDIO_LINE_IN		2
#define AUDIO_CD            4
#define AUDIO_SPEAKER		0
#define AUDIO_HEADPHONE		1
#define AUDIO_LINE_OUT		4

#define IN_CLASSD(i)	(((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(i)	IN_CLASSD(i)

typedef char	*caddr_t;
typedef int	ssize_t;

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	256
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#define ECONNREFUSED	WSAECONNREFUSED
#define ENETUNREACH		WSAENETUNREACH
#define EHOSTUNREACH	WSAEHOSTUNREACH
#define EWOULDBLOCK		WSAEWOULDBLOCK

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT	WSAEAFNOSUPPORT
#endif

#define M_PI		3.14159265358979323846

#endif 
#endif
