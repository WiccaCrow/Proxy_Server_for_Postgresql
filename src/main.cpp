#include "main.hpp"

int     main(int ac, char **av) {

    if (checkArgumens(ac, av))
        return 1;
    
    config();
    openLogFile();
    g_server.start();
    closeLogFile();

    return 0;
}

int     checkArgumens(int ac, char **av) {

    if (ac == 1) {
        std::cout << "Default settings for PostgreSQL:\n    host    127.0.0.1 \n    port    5432" << std::endl;
        return 0;
    }
    
    if ( (av[1][0] != 'h' && av[1][0] != 'p') ||
         (ac > 2  && (!(av[1][0] == 'h' && av[2][0] == 'p') && 
                      !(av[2][0] == 'h' && av[1][0] == 'p'))) ||
         (ac > 3)) {
        if (ac > 3)
            std::cerr << "Error: too much arguments.  " << std::endl;
        else
            std::cerr << "Error: wrong arguments.  " << std::endl;
        std::cerr << "Example: ./TCPserv h127.0.0.1 p5432" << std::endl
                  << "         ./TCPserv h127.0.0.1 " << std::endl
                  << "         ./TCPserv p5432" << std::endl
                  << "         ./TCPserv " << std::endl
                  << "Default: h127.0.0.1 p5432" << std::endl;
        return 1;
    }

    if (check_host_or_port(av[1]))
        return 1;

    int check_av2 = 0;
    if (ac > 2) {
        check_av2 = check_host_or_port(av[2]);
        if (!check_av2) {
            std::cout << "Settings: " << std::endl
                    << "    host: " << g_db_host << std::endl
                    << "    port: " << g_db_port << std::endl;
        }
    }
    return check_av2;
}

int     check_host_or_port(char* h_or_p) {
    if (h_or_p[0] == 'h') {
        return (checkHost(h_or_p));
    }
    return (checkPort(h_or_p + 1));
}

int     checkPort(std::string port) {
    if (!port.empty() 
        && port.length() < 6
        && (port.find_first_not_of("0123456789") == port.npos) 
        && std::stoi(port) < 65536) {
            g_db_port = port;
            return 0;
        }
    std::cerr << "Error: incorrect port format " << std::endl
              << "p[port], where port is a number less than 65536." << std::endl;
    return 1;
}

int     checkHost(std::string host) {

    if (host.length() > 16 || host.length() < 8 ||
        !std::isdigit(host[1])) {
        std::cerr << "Error: incorrect host format" << std::endl;
        return 1;
    }

    for (size_t pos_next = 1, pos = 1, point_counter = 1; point_counter < 5 && pos_next < host.length(); ++point_counter) {
        pos_next = host.find_first_of( '.', pos);

        if (   (pos_next == std::string::npos && point_counter != 4)
            || (pos_next - pos) == 0
            || (pos_next != std::string::npos && (pos_next - pos) > 3)
            || (pos_next != std::string::npos && point_counter > 3) 
            || (pos_next == std::string::npos && point_counter == 4 && 
                            (host.length() - pos > 3 || host.length() - pos == 0))
            || (std::stoi(host.c_str() + pos) > 255)) {
            std::cerr << "Error: incorrect host format" << std::endl;
            return 1;
        }

        for (; pos != pos_next && pos != host.length(); ++pos) {
            if (!std::isdigit(host[pos])) {
                std::cerr << "Error: incorrect host format" << std::endl;
                return 1;
            }
        }

        ++pos;
    }
    g_db_host = host;
    return 0;
}

std::string
        nameLogFile(void) {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::string str = LOG_PATH "/logs_" +
    std::to_string(now->tm_year + 1900) + "." +
    std::to_string(now->tm_mon + 1) + "." +
    std::to_string(now->tm_mday) + "_" +
    std::to_string(now->tm_hour) + ":" +
    std::to_string(now->tm_min) + ":" +
    std::to_string(now->tm_sec);
    return str;
}

void    openLogFile(void) {
    if (g_output == 1) {
        g_ofs_log = &stdcout;
        return ;
    }
    g_color = g_color_end = "";
    g_ofs_log = &ofs_log;
    std::string logFile = nameLogFile();
    ofs_log.open(logFile.c_str(), std::ofstream::out);
    if (ofs_log.is_open())
        return ;
    std::cerr << "Fatal: cannot open logs file" << std::endl;
    exit(1);
}

void    closeLogFile(void) {
    if (g_output == 0) {
        ofs_log.close();
        return ;
    }
}

