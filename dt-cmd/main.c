#include "glb_var.h"
#include "server.h"
#include "cmd.h"
#include "parse_conf.h"

struct cmd_conf g_conf;

int main(int argc, char** argv)
{
    /*每次仅执行一个命令，完成后退出*/
    parse_conf(&g_conf);   /*解析配置文件，主要包括本机的IP，监听的端口 */
    if (parse_cmd(argc, argv)) {  /* 解析参数，解析成对应的命令，通过本地的文件上传到kafka */
      run_server(g_conf.ip, g_conf.port); /* 运行服务，等待storm把结果返回*/
    }
    return 0;
}
