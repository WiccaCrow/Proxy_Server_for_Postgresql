#include "config.hpp"

int     configValue(const char * str_conf, const char *msg, size_t length) {
    if (!std::strncmp(str_conf, msg, length))
        return (int)str_conf[length] - 48;
    return -1;
}

void    config(void) {
    std::string line;
 
    std::ifstream ofs_conf(CONFIG_TCPSERV);
    if (!ofs_conf.is_open()) {
        std::cerr << "Fatal: cannot open config file" << std::endl;
        exit(1);
    }

    int i_deep_logs = 0, i_output = 0, i = 1;

    for (; std::getline(ofs_conf, line) &&
                    i_deep_logs < 2 &&
                    i_output < 2; ++i) {
        if (line[0] == '#' || line.empty())
            continue ;
        if (line.length() == 12) {
            g_deep_logs = configValue(line.c_str(), "deep logs: ", 11);
            ++i_deep_logs;
        } else if (line.length() == 9) {
            g_output = configValue(line.c_str(), "output: ", 8);
            ++i_output;
        } else {
            g_deep_logs = g_output = -1;
            break ;
        }
        if (g_deep_logs < 0 || g_output < 0 || g_deep_logs > 3 || g_output > 1)
            break;
    }

    ofs_conf.close();
    
    if (i_deep_logs == 2 || i_output == 2 || g_deep_logs < 0 || g_output < 0 ||
        g_deep_logs > 3 || g_output > 1) {
        if (i_deep_logs == 2) 
            std::cerr << "Error: fatal: duplicated 'deep logs' : ";
        if (i_output == 2)
            std::cerr << "Error: fatal: duplicated 'output' : ";
        std::cerr << " wrong config line: " << i << std::endl;
        exit(1);
    }
}
