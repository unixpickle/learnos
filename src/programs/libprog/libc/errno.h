#ifndef __ERRNO_H__
#define __ERRNO_H__

int * __errno();

#define errno (*__errno())

#define	EPERM		1		/* Operation not permitted */
#define	EDEADLK		11		/* Resource deadlock avoided */
#define	ENOMEM		12		/* Cannot allocate memory */
#define	EINVAL		22		/* Invalid argument */
#define EAGAIN          35              /* Resource temporarily unavailable */
#define ENOTSUP         45              /* Operation not supported */
#define ETIMEDOUT       60              /* Operation timed out */

#endif
