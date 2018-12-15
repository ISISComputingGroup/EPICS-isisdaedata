#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <cstddef>
#include <cctype>
#include <stdint.h>

#include <osiUnistd.h>
#include <osiSock.h>
#include <epicsMutex.h>
#include <epicsGuard.h>

//#include <winsock2.h> // needs to be before windows.h
#ifdef _WIN32
#include <WS2tcpip.h>

//#include <windows.h>
//#include <tchar.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#endif /* _WIN32 */
#include <sys/stat.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <math.h>
//#include <eh.h>
#include "asynPortDriver.h"
//#include <psapi.h>

#include "daedataUDP.h"

#define MAX_BLOCK_SIZE 256

// these structrues need to be packed tightly

#ifdef _MSC_VER
#pragma pack(push,2)
#endif /* _MSC_VER */

struct read_send
{
	int32_t start_addr;
	int16_t block_size; // up to MAX_BLOCK_SIZE
	read_send(int32_t a, int16_t b) : start_addr(htonl(a)), block_size(htons(b)) { }
};

struct read_recv
{
	int32_t start_addr;
	int16_t block_size; // up to MAX_BLOCK_SIZE
	uint32_t data[MAX_BLOCK_SIZE];
	void flip_endian()
	{
		start_addr = ntohl(start_addr);
		block_size = ntohs(block_size);
		for(int i=0; i<MAX_BLOCK_SIZE; ++i)
		{
			data[i] = ntohl(data[i]);
		}
	}
};

struct write_send
{
	int32_t start_addr;
	int16_t block_size; // up to MAX_BLOCK_SIZE
	uint32_t data[MAX_BLOCK_SIZE];
	write_send(int32_t a, int16_t b, const uint32_t* d) : start_addr(htonl(a)), block_size(htons(b)) { for(int i=0; i<b; ++i) { data[i] = htonl(d[i]); } }
	int byteSize() { return 4 + 2 + 4 * ntohs(block_size); } 
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

// end of packing
static const char* socket_errmsg()
{
	static char error_message[2048];
	epicsSocketConvertErrnoToString(error_message, sizeof(error_message));
	return error_message;
}
static const std::string FUNCNAME = "DAEDataUDP";

	
  DAEDataUDP::DAEDataUDP(const char* host, bool simulate) : m_host(host), m_simulate(simulate), m_sock_send(-1), m_sock_recv(-1)
	{
		if ( (aToIPAddr(host, 10000, &m_sa_read_send) < 0) ||
			 (aToIPAddr("0.0.0.0", 10000, &m_sa_read_recv) < 0) ||
		     (aToIPAddr(host, 10002, &m_sa_write_send) < 0) )
		{
			throw std::runtime_error(std::string(FUNCNAME) + ": Bad IP address : " + host);
		}
	    m_sock_recv = epicsSocketCreate(PF_INET, SOCK_DGRAM, 0);
		if (m_sock_recv == INVALID_SOCKET)
		{
				throw std::runtime_error(std::string(FUNCNAME) + ": Can't create recv socket: " + socket_errmsg());
		}
		if (bind(m_sock_recv, (struct sockaddr *) &m_sa_read_recv, sizeof(m_sa_read_recv)) < 0)
		{
			std::string error_msg = socket_errmsg();  // make copy before calling epicsSocketDestroy
			epicsSocketDestroy(m_sock_recv);
			throw std::runtime_error(std::string(FUNCNAME) + ": bind failed: " + error_msg);
		}
	    m_sock_send = epicsSocketCreate(PF_INET, SOCK_DGRAM, 0);
		if (m_sock_send == INVALID_SOCKET)
		{
			epicsSocketDestroy(m_sock_recv);
			throw std::runtime_error(std::string(FUNCNAME) + ": Can't create send socket: " + socket_errmsg());
		}
	}
	
	DAEDataUDP::~DAEDataUDP()
	{
		if (INVALID_SOCKET != m_sock_recv)
		{
			epicsSocketDestroy(m_sock_recv);
		}
		if (INVALID_SOCKET != m_sock_send)
		{
			epicsSocketDestroy(m_sock_send);
		}
	}
	

    void DAEDataUDP::clearSocket(SOCKET fd, asynUser *pasynUser)
	{	
		int n = 1; 
		char buffer[2048];
		fd_set reply_fds;
		struct timeval no_delay;
		struct sockaddr_in reply_sa;
		socklen_t reply_sa_len;
		while (n > 0)
		{
			no_delay.tv_sec = no_delay.tv_usec = 0;
			FD_ZERO(&reply_fds);
			FD_SET(fd, &reply_fds);
			reply_sa_len = sizeof(reply_sa);
			if ( select((int)fd + 1, &reply_fds, NULL, NULL, &no_delay) > 0 ) // nfds parameter is ignored on Windows, so cast to avoid warning
			{
				n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&reply_sa, &reply_sa_len);
				asynPrint(pasynUser, ASYN_TRACE_FLOW, "discarded %d bytes from %s port %hu ", n, inet_ntoa(reply_sa.sin_addr), ntohs(reply_sa.sin_port));
			}
			else
			{
			    n = 0;
			}
		}
	}

    void DAEDataUDP::readData(unsigned int start_address, uint32_t* data, size_t block_size, asynUser *pasynUser)
	{
		for(int i=0; i<block_size; ++i)
		{
		    readDataImpl(start_address + 4 * i, data + i, 1, pasynUser);
		}
	}
	
    void DAEDataUDP::readDataImpl(unsigned int start_address, uint32_t* data, size_t block_size, asynUser *pasynUser)
	{
		epicsGuard<epicsMutex> _lock(m_lock);
		std::ostringstream error_message;
		if (block_size <= 0 || block_size > MAX_BLOCK_SIZE)
		{
			error_message << FUNCNAME << ": Block size error " << block_size;
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		if (m_simulate)
		{
		    for(int i=0; i<block_size; ++i)
		    {
		        data[i] = start_address + 4 * i;
		    }
			return;
		}
		read_send rs(start_address, block_size);
		read_recv rr;
		clearSocket(m_sock_recv, pasynUser);
		int stat = sendto(m_sock_send, (char*)&rs, sizeof(rs), 0, (struct sockaddr *) &m_sa_read_send, sizeof(m_sa_read_send));
		if (stat < 0)
		{
			error_message << FUNCNAME << ": cannot send: " << socket_errmsg();
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		else if (stat != sizeof(rs))
		{
			error_message << FUNCNAME << ": send size error: " << stat << " != " << sizeof(rs);
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		fd_set reply_fds;
		struct timeval wait_time;
		FD_ZERO(&reply_fds);
		FD_SET(m_sock_recv, &reply_fds);
		wait_time.tv_sec = 5;
		wait_time.tv_usec = 0;
		struct sockaddr_in reply_sa;
		socklen_t reply_sa_len = sizeof(reply_sa);
		stat = select((int)m_sock_recv + 1, &reply_fds, NULL, NULL, &wait_time); // nfds parameter is ignored on Windows, so cast to avoid warning 
		if (stat == 0) 
		{
			error_message << FUNCNAME << ": select timeout reading address 0x" << std::hex << start_address;
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		} 
		else if (stat < 0)
		{
			error_message << FUNCNAME << ": cannot select: " << socket_errmsg();
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		stat = recvfrom(m_sock_recv, (char*)&rr, sizeof(rr), 0, (struct sockaddr *)&reply_sa, &reply_sa_len);
		if (stat < 0)
		{
			error_message << FUNCNAME << ": cannot recvfrom: " << socket_errmsg();
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		else if (stat != 6+4*block_size)
		{
			error_message << FUNCNAME << ": recvfrom incorrect size: " << stat << " != " << 6+4*block_size << " for address 0x" << std::hex << start_address;
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
//		if (reply_sa.sin_port != htons(10002) reply from wrong port
		if (reply_sa.sin_addr.s_addr != m_sa_read_send.sin_addr.s_addr)
		{
			error_message << FUNCNAME << ": reply from wrong host " << inet_ntoa(reply_sa.sin_addr);
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		if (rr.start_addr != rs.start_addr)
		{
			error_message << FUNCNAME << ": Mismatch in returned memory start addresss";
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		if (rr.block_size != rs.block_size)
		{
			error_message << FUNCNAME << ": Mismatch in returned block size";
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		rr.flip_endian();
		for(int i=0; i<block_size; ++i)
		{
		    data[i] = rr.data[i];
		}
	}

    void DAEDataUDP::writeData(unsigned int start_address, const uint32_t* data, size_t block_size, bool verify, asynUser *pasynUser)
	{
		epicsGuard<epicsMutex> _lock(m_lock);
		std::ostringstream error_message;
		if (block_size <= 0 || block_size > MAX_BLOCK_SIZE)
		{
			error_message << FUNCNAME << ": Block size error";
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		if (m_simulate)
		{
			return;
		}
		write_send ws(start_address, block_size, data);
		int stat = sendto(m_sock_send, (char*)&ws, ws.byteSize(), 0, (struct sockaddr *) &m_sa_write_send, sizeof(m_sa_write_send));
		if (stat < 0)
		{
			error_message << FUNCNAME << ": cannot sendto: " << socket_errmsg();
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		else if (stat != ws.byteSize())
		{
			error_message << FUNCNAME << ": sendto size error ";
			asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
			throw std::runtime_error(error_message.str());
		}
		if (verify)
		{
			uint32_t* data_rb = new uint32_t[block_size];
			readData(start_address, data_rb, block_size, pasynUser);
			for(int i=0; i<block_size; ++i)
			{
				if (data[i] != data_rb[i])
				{
					error_message << FUNCNAME << std::hex << "Verify failed for address 0x" << start_address + 4*i << ": 0x" << data[i] << " != 0x" << data_rb[i] << std::dec;
					asynPrint(pasynUser, ASYN_TRACE_ERROR, error_message.str().c_str());
					throw std::runtime_error(error_message.str());
				}
			}
			delete[] data_rb;
		}
	}


