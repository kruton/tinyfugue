/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#include "tfconfig.h"
#include "port.h"
#include "tf.h"

#include <errno.h>

#ifdef NETINET_IN_H
# include NETINET_IN_H
#endif
#ifdef NETDB_H
# include NETDB_H
#endif

#include "wasm_platform.h"
#include "socket.h"
#include "pattern.h"
#include "search.h"
#include "tfio.h"
#include "tfselect.h"
#include "tty.h"
#include "util.h"
#include "expand.h"

#define WASM_STARTUP_SCRIPT_SIZE 8192

static int wasm_columns = 80;
static int wasm_lines = 24;

static int wasm_unsupported(void);

#ifdef __EMSCRIPTEN__
extern int tf_wasm_read_stdin(char *data, int len);
extern int tf_wasm_write_stdout(const char *data, int len);
extern int tf_wasm_schedule_wake(int delay_ms);
extern int tf_wasm_stdin_ready(void);
extern int tf_wasm_socket_open(int domain, int type, int protocol);
extern int tf_wasm_socket_connect(int fd);
extern int tf_wasm_socket_close(int fd);
extern int tf_wasm_socket_recv(int fd, char *buf, int len, int flags);
extern int tf_wasm_socket_send(int fd, const char *buf, int len, int flags);
extern int tf_wasm_socket_read_ready(int fd);
extern int tf_wasm_socket_write_ready(int fd);
extern int tf_wasm_resolve(const char *name, int name_len,
    const char *port, int port_len);
extern int tf_wasm_get_startup_script(char *buf, int len);
#else
static int tf_wasm_read_stdin(char *data, int len)
{
    (void)data;
    (void)len;
    return 0;
}

static int tf_wasm_write_stdout(const char *data, int len)
{
    (void)data;
    return len;
}

static int tf_wasm_schedule_wake(int delay_ms)
{
    (void)delay_ms;
    return 0;
}

static int tf_wasm_stdin_ready(void)
{
    return 0;
}

static int tf_wasm_socket_open(int domain, int type, int protocol)
{
    (void)domain;
    (void)type;
    (void)protocol;
    return wasm_unsupported();
}

static int tf_wasm_socket_connect(int fd)
{
    (void)fd;
    return wasm_unsupported();
}

static int tf_wasm_socket_close(int fd)
{
    (void)fd;
    return 0;
}

static int tf_wasm_socket_recv(int fd, char *buf, int len, int flags)
{
    (void)fd;
    (void)buf;
    (void)len;
    (void)flags;
    return wasm_unsupported();
}

static int tf_wasm_socket_send(int fd, const char *buf, int len, int flags)
{
    (void)fd;
    (void)buf;
    (void)len;
    (void)flags;
    return wasm_unsupported();
}

static int tf_wasm_socket_read_ready(int fd)
{
    (void)fd;
    return 0;
}

static int tf_wasm_socket_write_ready(int fd)
{
    (void)fd;
    return 0;
}

static int tf_wasm_resolve(const char *name, int name_len,
    const char *port, int port_len)
{
    (void)name;
    (void)name_len;
    (void)port;
    (void)port_len;
    return 0;
}

static int tf_wasm_get_startup_script(char *buf, int len)
{
    (void)buf;
    (void)len;
    return 0;
}
#endif

struct wasm_addrinfo {
    tf_addrinfo ai;
    struct sockaddr_in sin;
};

static int wasm_unsupported(void)
{
#ifdef ENOTSUP
    errno = ENOTSUP;
#else
    errno = ENOSYS;
#endif
    return -1;
}

static int wasm_getaddrinfo(const char *name, const char *port,
    const tf_addrinfo *hints, tf_addrinfo **res)
{
    struct wasm_addrinfo *addr;
    int portnum = 0;

    (void)hints;
    if (res)
        *res = NULL;
    if (!name || !res) {
#ifdef EAI_NONAME
        return EAI_NONAME;
#else
        return -1;
#endif
    }
    if (port && is_digit(*port))
        portnum = atoi(port);
    tf_wasm_resolve(name, strlen(name), port ? port : "", port ? strlen(port) : 0);
    addr = (struct wasm_addrinfo *)MALLOC(sizeof(*addr));
    if (!addr) {
#ifdef EAI_MEMORY
        return EAI_MEMORY;
#else
        return -1;
#endif
    }
    memset(addr, 0, sizeof(*addr));
    addr->sin.sin_family = AF_INET;
    addr->sin.sin_port = htons(portnum);
    addr->sin.sin_addr.s_addr = htonl(0x7f000001);
    addr->ai.ai_family = AF_INET;
    addr->ai.ai_socktype = SOCK_STREAM;
    addr->ai.ai_protocol = IPPROTO_TCP;
    addr->ai.ai_addrlen = sizeof(addr->sin);
    addr->ai.ai_addr = (struct sockaddr *)&addr->sin;
    addr->ai.ai_next = NULL;
    *res = &addr->ai;
    return 0;
}

static void wasm_freeaddrinfo(tf_addrinfo *res)
{
    FREE(res);
}

static int wasm_socket_open(int domain, int type, int protocol)
{
    int fd = tf_wasm_socket_open(domain, type, protocol);
    if (fd < 0)
        errno = EACCES;
    return fd;
}

static int wasm_socket_connect(int fd, const struct sockaddr *addr,
    int addrlen)
{
    int result;

    (void)addr;
    (void)addrlen;
    result = tf_wasm_socket_connect(fd);
    if (result < 0)
        errno = EACCES;
    return result;
}

static int wasm_socket_bind(int fd, const struct sockaddr *addr, int addrlen)
{
    (void)fd;
    (void)addr;
    (void)addrlen;
    return wasm_unsupported();
}

static int wasm_socket_close(int fd)
{
    return tf_wasm_socket_close(fd);
}

static int wasm_socket_setsockopt(int fd, int level, int optname,
    const void *optval, int optlen)
{
    (void)fd;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    return wasm_unsupported();
}

static int wasm_socket_getsockopt(int fd, int level, int optname,
    void *optval, int *optlen)
{
    (void)fd;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    return wasm_unsupported();
}

static int wasm_socket_fcntl(int fd, int cmd, int arg)
{
    (void)fd;
    (void)cmd;
    (void)arg;
    return wasm_unsupported();
}

static int wasm_socket_recv(int fd, char *buf, int len, int flags)
{
    return tf_wasm_socket_recv(fd, buf, len, flags);
}

static int wasm_socket_send(int fd, const char *buf, int len, int flags)
{
    return tf_wasm_socket_send(fd, buf, len, flags);
}

static int wasm_read_stdin(char *data, int len)
{
    return tf_wasm_read_stdin(data, len);
}

static int wasm_write_stdout(const char *data, int len)
{
    return tf_wasm_write_stdout(data, len);
}

static FILE *wasm_popen(const char *command, const char *mode)
{
    (void)command;
    (void)mode;
    wasm_unsupported();
    return NULL;
}

static int wasm_pclose(FILE *file)
{
    (void)file;
    return 0;
}

static int wasm_select(int nfds, fd_set *readers, fd_set *writers,
    fd_set *excepts, struct timeval *timeout)
{
    int delay_ms = -1;
    int count = 0;
    int fd;
    fd_set ready_readers;
    fd_set ready_writers;

    (void)excepts;
    FD_ZERO(&ready_readers);
    FD_ZERO(&ready_writers);
    for (fd = 0; fd < nfds; fd++) {
        if (readers && fd == 0 && FD_ISSET(fd, readers) &&
                tf_wasm_stdin_ready()) {
            FD_SET(fd, &ready_readers);
            count++;
            continue;
        }
        if (readers && FD_ISSET(fd, readers) && tf_wasm_socket_read_ready(fd)) {
            FD_SET(fd, &ready_readers);
            count++;
        }
        if (writers && FD_ISSET(fd, writers) && tf_wasm_socket_write_ready(fd)) {
            FD_SET(fd, &ready_writers);
            count++;
        }
    }
    if (readers)
        *readers = ready_readers;
    if (writers)
        *writers = ready_writers;
    if (timeout) {
        delay_ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
        if (delay_ms < 0)
            delay_ms = 0;
    }
    tf_wasm_schedule_wake(delay_ms);
    return count;
}

static int wasm_tty_isatty(int fd)
{
    (void)fd;
    return 0;
}

static void wasm_tty_mode_noop(void)
{
}

static int wasm_get_window_size(int *new_columns, int *new_lines)
{
    if (new_columns)
        *new_columns = wasm_columns;
    if (new_lines)
        *new_lines = wasm_lines;
    return 1;
}

int tf_wasm_resize(int new_columns, int new_lines)
{
    if (new_columns <= 0 || new_lines <= 0)
        return 0;
    wasm_columns = new_columns;
    wasm_lines = new_lines;
    return set_window_size(new_columns, new_lines);
}

void run_wasm_startup_script(void)
{
    char script[WASM_STARTUP_SCRIPT_SIZE];
    int len;
    String *cmd;

    len = tf_wasm_get_startup_script(script, sizeof(script));
    if (len <= 0)
        return;
    if (len >= (int)sizeof(script))
        len = sizeof(script) - 1;
    script[len] = '\0';
    (cmd = Stringnew(script, len, 0))->links++;
    macro_run(CS(cmd), 0, NULL, 0, SUB_NEWLINE, "\bWASM");
    Stringfree(cmd);
}

int tf_wasm_smoke(void)
{
    struct timeval tv;
    const char msg[] = "tinyfugue wasm smoke\n";
    const char socket_msg[] = "relay outbound";
    tf_addrinfo *addr = NULL;
    char socket_buf[32];
    fd_set readers;
    int fd;
    int ready;
    int received;

    wasm_write_stdout(msg, sizeof(msg) - 1);
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    wasm_select(0, NULL, NULL, NULL, &tv);
    if (wasm_getaddrinfo("mud.example", "4000", NULL, &addr) != 0 || !addr)
        return 7;
    fd = wasm_socket_open(0, 0, 0);
    if (fd < 0)
        return 1;
    if (wasm_socket_connect(fd, NULL, 0) < 0)
        return 2;
    if (wasm_socket_send(fd, socket_msg, sizeof(socket_msg) - 1, 0) !=
            (int)sizeof(socket_msg) - 1)
        return 3;
    FD_ZERO(&readers);
    FD_SET(fd, &readers);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    ready = wasm_select(fd + 1, &readers, NULL, NULL, &tv);
    if (ready != 1 || !FD_ISSET(fd, &readers))
        return 4;
    received = wasm_socket_recv(fd, socket_buf, sizeof(socket_buf), 0);
    if (received != 13)
        return 5;
    if (wasm_socket_close(fd) != 0)
        return 6;
    wasm_freeaddrinfo(addr);
    return 0;
}

void init_wasm_platform(void)
{
    tf_set_addrinfo_funcs(wasm_getaddrinfo, wasm_freeaddrinfo);
    tf_set_socket_ops(wasm_socket_open, wasm_socket_connect,
        wasm_socket_bind, wasm_socket_close, wasm_socket_setsockopt,
        wasm_socket_getsockopt, wasm_socket_fcntl);
    tf_set_socket_io_funcs(wasm_socket_recv, wasm_socket_send);
    tf_set_read_stdin_func(wasm_read_stdin);
    tf_set_write_stdout_func(wasm_write_stdout);
    tf_set_popen_funcs(wasm_popen, wasm_pclose);
    tf_set_select_func(wasm_select);
    tf_set_tty_isatty_func(wasm_tty_isatty);
    tf_set_tty_mode_funcs(wasm_tty_mode_noop, wasm_tty_mode_noop);
    tf_set_window_size_func(wasm_get_window_size);
}
