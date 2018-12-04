
#include "osiSock.h"

class DAEDataUDP
{
private:
    std::string m_host;
	SOCKET m_sock_send;
	SOCKET m_sock_recv;
	struct sockaddr_in m_sa_read_send;
	struct sockaddr_in m_sa_read_recv;
	struct sockaddr_in m_sa_write_send;
	epicsMutex m_lock;

	void clearSocket(SOCKET fd, asynUser *pasynUser);
	
public:
	
    DAEDataUDP(const char* host);
	~DAEDataUDP();
    void readData(unsigned int start_address, uint32_t* data, size_t block_size, asynUser *pasynUser);
    void writeData(unsigned int start_address, const uint32_t* data, size_t block_size, bool verify, asynUser *pasynUser);
};
