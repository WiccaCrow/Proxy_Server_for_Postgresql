#pragma once

class Request
{
private:
    bool _formed;
public:
    Request();
    ~Request();

    bool    formed(void) const ;
    void    formed(bool formed);

};
