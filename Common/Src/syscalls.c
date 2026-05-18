/* Minimal syscalls for newlib-nano to satisfy linker (stubs)
 * Provides: _write_r, _read_r, _close_r, _lseek_r, _fstat_r, _isatty_r
 * These are minimal stubs: adjust to route output to UART/ITM if desired.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <reent.h>
#include <stddef.h>

int _close_r(struct _reent *r, int fd)
{
  (void)r; (void)fd;
  r->_errno = EBADF;
  return -1;
}

off_t _lseek_r(struct _reent *r, int fd, off_t offset, int whence)
{
  (void)r; (void)fd; (void)offset; (void)whence;
  r->_errno = EBADF;
  return (off_t)-1;
}

ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t count)
{
  (void)r; (void)fd; (void)buf; (void)count;
  r->_errno = ENOSYS;
  return -1;
}

ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t count)
{
  /* Minimal: pretend all bytes written. To actually output, route to UART or ITM here. */
  (void)r; (void)fd; (void)buf;
  return (ssize_t)count;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
  (void)r; (void)fd;
  st->st_mode = S_IFCHR; /* character device */
  return 0;
}

int _isatty_r(struct _reent *r, int fd)
{
  (void)r; (void)fd;
  return 1;
}
