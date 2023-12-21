#include <errno.h>
#include <fcntl.h>
#include <taputil.h>
#include <unistd.h>

static int add_fdflags(int fd, int add_flags) {
    int fd_flags;

    fd_flags = fcntl(fd, F_GETFL);
    if (fd_flags == -1) {
        return errno;
    }

    /* Add flags already set */
    if ((fd_flags & add_flags) == add_flags) {
        return 0;
    }

    fd_flags |= add_flags;
    if (fcntl(F_SETFL, fd_flags) == -1) {
        return errno;
    }
    return 0;
}

int tap_pipe_setup(int fds[2]) {
    int err;

    err = 0;
    if (pipe(fds) == -1) {
        err = errno;
        goto failed;
    }

    err = add_fdflags(fds[TAP_PIPE_RX], O_RDONLY);
    if (err != 0) {
        goto failed;
    }

    err = add_fdflags(fds[TAP_PIPE_TX], O_WRONLY);
    if (err != 0) {
        goto failed;
    }

    return 0;

failed:
    close(fds[TAP_PIPE_RX]);
    close(fds[TAP_PIPE_TX]);
    return err;
}
