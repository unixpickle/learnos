#ifndef __ERRNO_H__
#define __ERRNO_H__

int * __errno();

#define errno (*__errno())

/* TODO: define errno's here. */
#define	EPERM		1		/* Operation not permitted */
#define	EDEADLK		11		/* Resource deadlock avoided */
#define	ENOMEM		12		/* Cannot allocate memory */
#define	EINVAL		22		/* Invalid argument */

#endif
