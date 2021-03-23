#ifndef __STATUS_H
#define __STATUS_H

#define EIO              5      /* I/O error */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ENOMEM          12      /* Out of memory */
#define EINVAL          22      /* Invalid argument */

#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */

#define ENOSPC          28		/* No space left on device */

#define ENOMEDIUM       123     /* No medium found */
#define EMEDIUMTYPE     124     /* Wrong medium type */
#endif
