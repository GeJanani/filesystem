#ifndef	MANDOC_CONFIG_H
#define	MANDOC_CONFIG_H

#if defined(__linux__) || defined(__MINT__)
# define _GNU_SOURCE /* strptime(), getsubopt() */
#endif

#include <sys/types.h>
#include <stdio.h>

#define VERSION ""
#define HAVE_STRNLEN
#define HAVE_STRPTIME
#define HAVE_STRSEP

#if !defined(__BEGIN_DECLS)
#  ifdef __cplusplus
#  define	__BEGIN_DECLS		extern "C" {
#  else
#  define	__BEGIN_DECLS
#  endif
#endif
#if !defined(__END_DECLS)
#  ifdef __cplusplus
#  define	__END_DECLS		}
#  else
#  define	__END_DECLS
#  endif
#endif

#ifndef HAVE_FGETLN
extern	char	 *fgetln(FILE *, size_t *);
#endif
#ifndef HAVE_GETSUBOPT
extern	int	  getsubopt(char **, char * const *, char **);
extern	char	 *suboptarg;
#endif
#ifndef HAVE_STRCASESTR
extern	char	 *strcasestr(const char *, const char *);
#endif
#ifndef HAVE_STRLCAT
extern	size_t	  strlcat(char *, const char *, size_t);
#endif
#ifndef HAVE_STRLCPY
extern	size_t	  strlcpy(char *, const char *, size_t);
#endif
#ifndef HAVE_STRNLEN
extern	size_t	  strnlen(const char *, size_t);
#endif
#ifndef HAVE_STRSEP
extern	char	 *strsep(char **, const char *);
#endif

#endif /* MANDOC_CONFIG_H */
