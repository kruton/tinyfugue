/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#ifndef SOCKET_H
#define SOCKET_H

#include "tfconfig.h"
#include <stddef.h>

/* socktime ids */
#define SOCK_RECV	0
#define SOCK_SEND	1

/* /connect flags */
#define CONN_AUTOLOGIN	0x01
#define CONN_QUIETLOGIN	0x02
#define CONN_SSL	0x04
#define CONN_BG		0x08
#define CONN_FG		0x10

struct World;	/* declares struct World */
#if HAVE_GETADDRINFO
struct addrinfo;
typedef struct addrinfo tf_addrinfo;
#else
struct tfaddrinfo {
    int     ai_flags;     /* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
    int     ai_family;    /* PF_xxx */
    int     ai_socktype;  /* SOCK_xxx */
    int     ai_protocol;  /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
    size_t  ai_addrlen;   /* length of ai_addr */
    char   *ai_canonname; /* canonical name for nodename */
    struct sockaddr *ai_addr;     /* binary address */
    struct tfaddrinfo *ai_next;   /* next structure in linked list */
};
typedef struct tfaddrinfo tf_addrinfo;
#endif
struct sockaddr;

typedef int (*tf_getaddrinfo_func_t)(const char *name, const char *port,
                    const tf_addrinfo *hints, tf_addrinfo **res);
typedef void (*tf_freeaddrinfo_func_t)(tf_addrinfo *res);
typedef int (*tf_socket_open_func_t)(int domain, int type, int protocol);
typedef int (*tf_socket_connect_func_t)(int fd, const struct sockaddr *addr,
                    int addrlen);
typedef int (*tf_socket_bind_func_t)(int fd, const struct sockaddr *addr,
                    int addrlen);
typedef int (*tf_socket_close_func_t)(int fd);
typedef int (*tf_socket_setsockopt_func_t)(int fd, int level, int optname,
                    const void *optval, int optlen);
typedef int (*tf_socket_getsockopt_func_t)(int fd, int level, int optname,
                    void *optval, int *optlen);
typedef int (*tf_socket_fcntl_func_t)(int fd, int cmd, int arg);
typedef int (*tf_socket_recv_func_t)(int fd, char *buf, int len, int flags);
typedef int (*tf_socket_send_func_t)(int fd, const char *buf, int len,
                    int flags);

extern String *incoming_text;
extern int quit_flag;
extern struct Sock *xsock;

extern int     tf_getaddrinfo(const char *name, const char *port,
                    const tf_addrinfo *hints, tf_addrinfo **res);
extern void    tf_freeaddrinfo(tf_addrinfo *res);
extern void    tf_set_addrinfo_funcs(tf_getaddrinfo_func_t getaddrinfo_func,
                    tf_freeaddrinfo_func_t freeaddrinfo_func);
extern int     tf_socket_open(int domain, int type, int protocol);
extern int     tf_socket_connect(int fd, const struct sockaddr *addr,
                    int addrlen);
extern int     tf_socket_bind(int fd, const struct sockaddr *addr,
                    int addrlen);
extern int     tf_socket_close(int fd);
extern int     tf_socket_setsockopt(int fd, int level, int optname,
                    const void *optval, int optlen);
extern int     tf_socket_getsockopt(int fd, int level, int optname,
                    void *optval, int *optlen);
extern int     tf_socket_fcntl(int fd, int cmd, int arg);
extern int     tf_socket_recv(int fd, char *buf, int len, int flags);
extern int     tf_socket_send(int fd, const char *buf, int len, int flags);
extern void    tf_set_socket_ops(tf_socket_open_func_t open_func,
                    tf_socket_connect_func_t connect_func,
                    tf_socket_bind_func_t bind_func,
                    tf_socket_close_func_t close_func,
                    tf_socket_setsockopt_func_t setsockopt_func,
                    tf_socket_getsockopt_func_t getsockopt_func,
                    tf_socket_fcntl_func_t fcntl_func);
extern void    tf_set_socket_io_funcs(tf_socket_recv_func_t recv_func,
                    tf_socket_send_func_t send_func);
extern void    main_loop(void);
extern void    tf_tick(void);
extern long    tf_next_deadline_ms(void);
extern void    init_sock(void);
extern int     sockecho(void);
extern int     is_active(int fd);
extern void    readers_clear(int fd);
extern void    readers_set(int fd);
extern struct timeval *socktime(const char *name, int dir);
extern int     tog_bg(Var *var);
extern int     tog_keepalive(Var *var);
extern int     openworld(const char *name, const char *port, int flags);
extern void    world_output(struct World *world, conString *line);
extern int     send_line(const char *s, unsigned int len, int eol_flag);
extern conString *fgprompt(void);
extern int     tog_lp(Var *var);
extern void    transmit_window_size(void);
extern int     local_echo(int flag);
extern int     handle_send_function(conString *string, const char *world,
                     const char *flags);
#if ENABLE_ATCP
extern int     handle_atcp_function(conString *string, const char *world);
#endif
#if ENABLE_GMCP
extern int     handle_gmcp_function(conString *string, const char *world);
#endif
#if ENABLE_OPTION102
extern int     handle_option102_function(conString *string, const char *world);
#endif
extern int     handle_fake_recv_function(conString *string, const char *world,
		    const char *flags);
extern int     is_connected(const char *worldname);
extern int     is_open(const char *worldname);
extern int     nactive(const char *worldname);
extern int     world_hook(const char *fmt, const char *name);

extern struct World *xworld(void);
extern int	     xsock_is_fg(void);
extern int	     have_active_socks(void);
extern void          xsock_alert_id(void);
extern const char   *fgname(void);
extern const char   *world_info(const char *worldname, const char *fieldname);
extern struct World *named_or_current_world(const char *name);
extern int           xsock_is_ssl(void);

#endif /* SOCKET_H */
