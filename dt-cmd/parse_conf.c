#include "glb_var.h"
#include "cJSON.h"
#include "parse_conf.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>

bool parse_conf(struct cmd_conf *conf)
{
    char __conf[CONF_FILE_LEN];
    /* open */
    int __fp = open("cmd.conf", O_RDONLY);
    if (-1 == __fp) {
        printf("open cmd.conf failed:%s\n", strerror(errno));
        return false;
    }

    /* read */
    if (-1 == read(__fp, __conf, CONF_FILE_LEN)) {
        printf("read cmd.conf failed:%s\n", strerror(errno));
        return false;
    }

    /* parse conf */
    cJSON *root = cJSON_Parse(__conf);
    cJSON *item = NULL;
    if (!root) {
        printf("parse cmd.conf failed!");
        return false;
    }
    item = cJSON_GetObjectItem(root, "ip");
    if (item) {
        struct in_addr __ip;
        bzero(&__ip, sizeof(__ip));
        inet_pton(AF_INET, item->valuestring, (void*)&__ip);
        conf->ip = __ip.s_addr;
        strncpy(conf->ipstr, item->valuestring, sizeof(conf->ipstr));
    }
    item = cJSON_GetObjectItem(root, "port");
    if (item) {
        conf->port = item->valueint;
    }
    item = cJSON_GetObjectItem(root, "timeout");
    if (item) {
        conf->timeout= item->valueint;
    }
    item = cJSON_GetObjectItem(root, "kafkapath");
    if (item) {
        strncpy(conf->kfk_path, item->valuestring, PATH_LEN); 
    }

    return true;
}


