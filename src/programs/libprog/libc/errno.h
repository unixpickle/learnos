#ifndef __ERRNO_H__
#define __ERRNO_H__

int * __errno();

#define errno (*__errno())

/* TODO: define errno's here. */
#define	EINVAL		22		/* Invalid argument */

#endif
