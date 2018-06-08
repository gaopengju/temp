#include "glb_var.h"
#include "ev.h"
#include "parse_conf.h"
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

/**
 * 
 */
extern struct cmd_conf g_conf;
typedef void (*io_callback)(EV_P_ ev_io *w, int revents);

typedef struct _conn_t {
    int fd;
    ev_io read;
    ev_io write;
    struct _conn_t * next;
}SOCK_CONN;

static SOCK_CONN* s_conn;

bool init_conn()
{
    s_conn = (SOCK_CONN*)malloc(SOCK_MAX * sizeof(SOCK_CONN));
    if (!s_conn) {
        return false;
    }
    (void)memset(s_conn, 0, SOCK_MAX * sizeof(SOCK_CONN));
    for (int i=0; i<SOCK_MAX; i++) {
        s_conn[i].next = &s_conn[i+1];
    }
    s_conn[SOCK_MAX-1].next = NULL;
    return true;
}

static SOCK_CONN * get_conn()
{
    if (s_conn) {
        SOCK_CONN *__conn = s_conn;
        s_conn = s_conn->next;
        return __conn;
    } else {
        printf("get conn failed");
        return NULL;
    }
}
static void free_conn(SOCK_CONN* __conn) 
{
    if (s_conn) {
        __conn->next = s_conn;
    }
    s_conn = __conn;
}

/*-----------------conn 分割线 -----------*/

struct ev_loop *loop;

static void cb_timeout(EV_P_ ev_timer *w, int revents)
{
    printf("time out!!");
    exit(0);
}

static void cb_read(EV_P_ ev_io *w, int revents)
{
    /* 读取数据，并输出 */
    printf("do read data\n");
    char __buf[10240];
    int buf_len = 10240;
    int __fd = ((SOCK_CONN*)(w->data))->fd;
    struct msghdr msg;
    struct iovec io;
    io.iov_base = __buf;
    io.iov_len = buf_len;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    size_t recv_size = recvmsg(__fd, &msg, 0);
    char * ret_msg = msg.msg_iov[0].iov_base;
    ret_msg[recv_size] = '\0';
    printf("get result:\n%s\n", ret_msg);
    exit(0);
}
static void cb_write(EV_P_ ev_io *w, int revents)
{
    /* 写数据，并输出 */

    printf("do write data");
}
static void cb_accept(EV_P_ ev_io *w, int revents)
{
    /* 接受一个新的连接，加入到ev循环 */
    SOCK_CONN *__conn;
    int __fd;
    if (revents & EV_ERROR) {
        printf("accept E_ERROR");
        return;
    }

    __conn = get_conn();
    if (!__conn) {
        printf("get conn failed!");
        return;
    }

    __fd = accept(w->fd, NULL, NULL);
    if (-1 == __fd) {
        printf("accept err:%s\n", strerror(errno));
        free_conn(__conn);
        return;
    }

    __conn->fd = __fd;
    __conn->read.data = __conn;
    __conn->write.data = __conn;

    ev_io_init(&__conn->read, cb_read, __fd, EV_READ);
    ev_io_init(&__conn->write, cb_write, __fd, EV_WRITE);
    ev_io_start(EV_A_ &__conn->read);
}


bool init_server()
{
    struct sockaddr_in __sip;
    SOCK_CONN *__conn = NULL;
    int __fd;

    __conn = get_conn();
    if(!__conn) {
        printf("get conn failed");
        return false;
    }

    __conn->read.data = __conn;
    __conn->write.data = __conn;

    (void)memset(&__sip, 0, sizeof(struct sockaddr_in));
    __sip.sin_family = AF_INET;
    __sip.sin_port = htons(g_conf.port);
    __sip.sin_addr.s_addr = g_conf.ip;

    __fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (-1 == __fd) {
        printf("create socket failed:%s\n", strerror(errno));
        goto FREE;
    }
    __conn->fd = __fd;

    /* 设置socket属性 */
    int __flag = 1;
    setsockopt(__fd, SOL_SOCKET, SO_REUSEADDR, &__flag, sizeof(__flag));
    setsockopt(__fd, IPPROTO_TCP, TCP_NODELAY, &__flag, sizeof(__flag));

    if (bind(__fd, (const struct sockaddr *)&__sip, sizeof(struct sockaddr_in)) == -1) {
        printf(" bind failed, err:%s\n", strerror(errno));
        goto CLOSEFD;
    }

    if (listen(__fd, 512) == -1) {
        printf("listen failed:%s\n", strerror(errno));
        goto CLOSEFD;
    }
    /* 加入到事件循环 */
    ev_io_init(&__conn->read, cb_accept, __fd, EV_READ);
    ev_io_start(EV_A_ &__conn->read);

    return true;

CLOSEFD:
    close(__fd);
FREE:
    free_conn(__conn);
    return false;
}

void run_server()
{
  /**
   * 这里会开启一个服务，监听storm把结果回送到该端口
   * 超时后返回
   */
    signal(SIGPIPE, SIG_IGN);

    loop = EV_DEFAULT;
    ev_timer timeout_watcher;

    /* 开启监听事件 */
    if(false == init_conn()) {
        printf("init conn failed\n");
        return;
    }

    if (false == init_server()) {
        printf("init server failed\n");
        return;
    }
    /* 增加超时处理 */
    ev_timer_init(&timeout_watcher, cb_timeout, g_conf.timeout, 0);
    ev_timer_start(loop, &timeout_watcher);

    /* 开启ev */
    ev_run(EV_A_ 0);
}


