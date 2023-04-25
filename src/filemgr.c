#include "filemgr.h"


/* Return the path to the directory the current executable
   resides in, as a dynamically allocated string.
   If an error occurs, returns NULL with errno set.
*/
char *exe_dir(void)
{
    size_t  size = 512, i, n;
    char   *path, *temp;

    while (1) {
        ssize_t  used;

        path = malloc(size);
        if (!path) {
            errno = ENOMEM;
            return NULL;
        }

        used = readlink("/proc/self/exe", path, size);

        if (used == -1) {
            const int saved_errno = errno;
            free(path);
            errno = saved_errno;
            return NULL;
        } else
        if (used < 1) {
            free(path);
            errno = EIO;
            return NULL;
        }

        if ((size_t)used >= size) {
            free(path);
            size = (size | 2047) + 2049;
            continue;
        }

        size = (size_t)used;
        break;
    }

    /* Find final slash. */
    n = 0;
    for (i = 0; i < size; i++)
        if (path[i] == '/')
            n = i;

    /* Optimize allocated size,
       ensuring there is room for
       a final slash and a
       string-terminating '\0', */
    temp = path;
    path = realloc(temp, n + 2);
    if (!path) {
        free(temp);
        errno = ENOMEM;
        return NULL;
    }

    /* and properly trim and terminate the path string. */
    path[n+0] = '/';
    path[n+1] = '\0';

    return path;
}
