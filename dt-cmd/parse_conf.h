#ifndef _PARSE_H
#define _PARSE_H
struct cmd_conf {
    uint32_t ip;
    uint16_t port;
    uint16_t timeout;
    char kfk_path[PATH_LEN];
    char ipstr[16];
};
bool parse_conf(struct cmd_conf *conf);


#endif
