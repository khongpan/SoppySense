#ifndef NETWORK_H
#define NETWORK_H

class Network {
public:
    Network();
    ~Network();
    void Setup();
    void PowerOn();
    void Connect();
    void HttpGet();
    void Maintain();
    void Disconnect();
    void PowerOff();
    
private:
};

extern Network Net;


#endif


