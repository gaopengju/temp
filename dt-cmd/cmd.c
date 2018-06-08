#include "glb_var.h"
#include "parse_conf.h"
#include "cmd.h"
#include "cJSON.h"
#include <string.h>

extern struct cmd_conf g_conf;
/**
 * 写到kafka中的格式为：
 {
   "ip":"1.2.3.4",
   "port": 65533,
   "type":"cmd",
   "opttype":"get|set|add|del",
   "object":"conf|...",
   "item":"mirrors|inDebug...",
   "value":value
 }
 */
void usage()
{
    /* 输出该工具的用法 */
    printf("usage: xxx get|set|add|del object <item> <value>\n");
    printf("object list:\n");
    printf("\tconf\n");
    printf("item: all conf key\n");
    printf("eg: \n\tget conf");
    printf("\tadd conf mirrors 1.2.3.4\n");
    printf("\tadd conf inDebug true\n");
}
bool put_kafka(char *msg)
{
    char __file[512];
    (void)memset(__file, 0, sizeof(__file));
    strcat(__file, g_conf.kfk_path);
    strcat(__file, "cmd");
    FILE *__fp = fopen(__file, "w");
    if (NULL == __fp) {
        printf("open kafka file[%s] failed err: %s\n", __file, strerror(errno));
        return false;
    }
    fprintf(__fp, msg);
    fclose(__fp);
    return true;
}
bool parse_cmd(int argc, char** argv)
{
    /* 解析命令，写到kafka文件中 */
    for (int i=0; i<argc; i++) {
        printf(" arg[%d]:%s", i, argv[i]);
    }
    if (argc < 3) {
        usage();
        return false;
    }
    char opt[8];
    char obj[32];
    char item[32];
    char value[128];
    strncpy(opt, argv[1], sizeof(opt)); 
    strncpy(obj, argv[2], sizeof(obj)); 
    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        printf("create json failed!");
        goto FAILED;
    }

    cJSON_AddStringToObject(root, "ip", g_conf.ipstr);
    cJSON_AddNumberToObject(root, "port", g_conf.port);
    cJSON_AddStringToObject(root, "type", "cmd");
    cJSON_AddStringToObject(root, "opttype", opt);
    cJSON_AddStringToObject(root, "object", obj);
    if (argc > 3) {
        strncpy(item, argv[3], sizeof(item)); 
        cJSON_AddStringToObject(root, "item", item);
    } else {
        cJSON_AddNullToObject(root, "item");
    }
    if(argc > 4) {
        strncpy(value, argv[4], sizeof(value)); 
        cJSON_AddStringToObject(root, "value", value);
    } else {
        cJSON_AddNullToObject(root, "value");
    }
    if(argc > 5) {
        usage();
        goto FAILED;
    }
    put_kafka(cJSON_Print(root));
    cJSON_Delete(root);
    return true;
FAILED:
    cJSON_Delete(root);
    return false;
}
