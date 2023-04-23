#include <ParserCli.hpp>

Parser     formatCli;

void
parse(std::string &str) {
    std::map<char, Parser>::const_iterator it = validReqCli.find(str[0]);

    if (it == validReqCli.end()) {
        others(str);
        return ;
    }
    formatCli = it->second;
    (*formatCli)(str);
}

void
logCommand(std::string str) {
    auto it = str.cbegin();
    for(unsigned char c = *it; it != str.cend(); ++it, c = *it) {
        if (!isprint(c)) {
            for (; !isprint(c) && it != str.cend(); ++it, c = *it) ;
            if (it == str.cend() && *it != ';')
                *g_ofs_log << " " << g_color;
            if (it == str.cend())
                break ;
        }
        *g_ofs_log << (unsigned char)c ;
    }
    *g_ofs_log << ::std::endl;
}

void
Close(std::string &str) {
    int len = convRawBytesToInt32({str[1], str[2], str[3], str[4]});
    logCommand(str.substr(4, len - 4));
}

void
Query(std::string &str) {
    int len = convRawBytesToInt32({str[1], str[2], str[3], str[4]});
    logCommand(str.substr(5, len - 5));
}

void
CopyData(std::string &str) {
    int len = convRawBytesToInt32({str[1], str[2], str[3], str[4]});
    logCommand(str.substr(4, len - 4));
}

void
CopyFail(std::string &str) {
    int len = convRawBytesToInt32({str[1], str[2], str[3], str[4]});
    logCommand(str.substr(4, len - 4));
}

void
CopyDone(std::string &str) {
    (void)str;
}

void
others(std::string &str) {
    str = "";
}

static std::map<char, Parser>
initRequestsMapCli(void) {
    std::map<char, Parser> tmp;

    tmp.insert(std::make_pair('Q', &Query));
    // tmp.insert(std::make_pair('C', &Close));
    // tmp.insert(std::make_pair('d', &CopyData));
    // tmp.insert(std::make_pair('f', &CopyFail));
    
    return tmp;
}

const std::map<char, Parser> validReqCli = initRequestsMapCli();

