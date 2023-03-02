// MulticastChat.cpp : Defines the entry point for the application.
//

#define DEBUG

#include "header.h"

/*
 * https://www.ibm.com/docs/en/i/7.1?topic=designs-examples-using-multicasting-af-inet
 */

#define PORT1 2000
#define PORT2 1999

#define MULTICAST_IP "224.0.0.222"

using namespace std;

int main()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 255;
	}

	int br_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (br_sock <= 0)
	{
		return -1;
	}

	int mul_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mul_sock <= 0)
	{
		return -1;
	}



	int broadcastEnable = 1;
	int ret = setsockopt(br_sock , SOL_SOCKET, SO_BROADCAST,
		(const char*)&broadcastEnable, sizeof(broadcastEnable));
	if (ret < 0)
	{
		return -1;
	}

	int reuseEnable = 1;
	ret = setsockopt(mul_sock , SOL_SOCKET, SO_REUSEADDR,
		(const char*)&reuseEnable, sizeof(reuseEnable));
	if (ret < 0)
	{
		return -1;
	}

	/*int ttl = 30;
	setsockopt(mul_sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));
	if (ret < 0)
	{
		return -1;
	}


	/*
    int loopBack = 1;	// sending system does not receive a copy of the multicast datagrams it transmits
	ret = setsockopt(mul_sock , SOL_SOCKET, IP_MULTICAST_LOOP,
		(const char*)&loopBack, sizeof(loopBack));
	if (ret < 0)
	{
		return -1;
	}

	{
		struct sockaddr_in mul_addr;
		memset(&mul_addr, 0, sizeof(mul_addr));
		mul_addr.sin_family = AF_INET;
		mul_addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender

		int multicastIf = 1;	// which defines the local interface over which the multicast datagrams are sent.
		ret = setsockopt(mul_sock, SOL_SOCKET, IP_MULTICAST_IF,
			(const char*)&mul_addr, sizeof(mul_addr));
		if (ret < 0)
		{
			return -1;
		}
	}
	//*/



	SOCKADDR_IN br_addr;
	memset(&br_addr, 0, sizeof(br_addr));
	br_addr.sin_family = AF_INET;
	br_addr.sin_addr.s_addr = INADDR_ANY;
	br_addr.sin_port = htons(PORT1);

	int len = sizeof(br_addr);
	if (bind(br_sock , (SOCKADDR*)&br_addr, sizeof(SOCKADDR_IN)) < 0)
	{
		printf("ERROR binding in the server socket");
		return 0;
	}

	struct sockaddr_in mul_addr;
	memset(&mul_addr, 0, sizeof(mul_addr));
	mul_addr.sin_family = AF_INET;
	mul_addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
	mul_addr.sin_port = htons(PORT2);

	len = sizeof(mul_addr);
	if (bind(mul_sock , (SOCKADDR*)&mul_addr, sizeof(SOCKADDR_IN)) < 0)
	{
		printf("ERROR binding in the server socket");
		return 0;
	}



	std::vector<std::string> command;
	std::thread th([&command]()
		{
			while (true)
			{
				if (!command.empty())
					continue;

    				std::string msg;
				std::string word;

				msg.clear();

				std::cout << ">";
				std::getline(std::cin, msg);
				std::istringstream iSStream(msg);

				std::transform(msg.begin(), msg.end(), msg.begin(),
					[](unsigned char c) { return std::tolower(c); });

				while (iSStream >> word)
					command.push_back(word);
			}

		});


	while (true)
	{

		fd_set fd;
		fd_set fd2;

		FD_ZERO(&fd);
		FD_ZERO(&fd2);
		FD_SET(br_sock , &fd);
		FD_SET(mul_sock , &fd2);

		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 10;

		auto r = select(br_sock + 1, &fd, nullptr, nullptr, &timeout);
		auto r2 = select(mul_sock + 1, &fd2, nullptr, nullptr, &timeout);

		if (0 < r)
		{
			sockaddr_in addr;
			char buf[215];

			auto addr_len = sizeof(addr);
			recvfrom(br_sock , buf, sizeof(buf), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
			std::cout << buf;
		}

		if (0 < r2)
		{
			sockaddr_in addr;
			char buf[215];

			auto addr_len = sizeof(addr);
			recvfrom(mul_sock , buf, sizeof(buf), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
			std::cout << buf;
		}

		//cmd_mutex.lock();
		if (command.empty())
		{
            //cmd_mutex.unlock();
			continue;
		}

		if (command[0] == "send")
		{
			if (command.size() > 1 && command[1] == "b")	// broadcast
			{
				sockaddr_in broadcastAddr; // Make an endpoint
				memset(&broadcastAddr, 0, sizeof broadcastAddr);

				char* request = "hello";

				broadcastAddr.sin_port = htons(PORT1); // Set port 1900
				broadcastAddr.sin_family = AF_INET;
				broadcastAddr.sin_addr = CalculateBroadcastAddr(GetLocalIp(), GetMask());// INADDR_ANY;
				//inet_pton(AF_INET, "239.255.255.250", &broadcastAddr.sin_addr); // Set the broadcast IP address

				// Send the broadcast request, ie "Any upnp devices out there?"
				//char* request = "M-SEARCH * HTTP/1.1\r\nHOST:239.255.255.250:1900\r\nMAN:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";
				auto nResult = sendto(br_sock , request, strlen(request), 0, (struct sockaddr*)&broadcastAddr, sizeof broadcastAddr);

				if (nResult == SOCKET_ERROR)
				{
					//DBG(
					printf("ERROR: %d\n", WSAGetLastError());
					//)
					WSACleanup();
				}
			}
			if (command.size() > 1 && command[1] == "g")	// multicast
			{
				struct sockaddr_in addr;
				memset(&addr, 0, sizeof(addr));
				addr.sin_family = AF_INET;
				//addr.sin_addr.s_addr = inet_addr("239.255.255.250");
				addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);
				addr.sin_port = htons(PORT2);

				char* request = "hello";
				//char* request = "M-SEARCH * HTTP/1.1\r\nHOST:239.255.255.250:1900\r\nMAN:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";

				for (int i = 0; i < 1000; i++)
				{
					auto nResult = sendto(mul_sock, request, sizeof(request), 0, (struct sockaddr*)&addr, sizeof(addr));

					if (nResult == SOCKET_ERROR)
					{
						//DBG(
						printf("ERROR: %d\n", WSAGetLastError());
						//)
						WSACleanup();
					}
				}

			}
		}
		if (command[0] == "add")
		{
			ip_mreq mreq;
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);

			ret = setsockopt(mul_sock , IPPROTO_IP, IP_ADD_MEMBERSHIP,
				(const char*)&mreq, sizeof(mreq));
			if (ret < 0)
			{
				return -1;
			}

		}
		if (command[0] == "params")
		{
			std::cout << "IPv4 Address. . : " << GetLocalIp() << "\n";
			std::cout << "Subnet Mask . . : " << GetMask() << "\n";
			std::cout << "Broadcast . . . : " << CalculateBroadcast(GetLocalIp(), GetMask()) << "\n";

		}
		if (command[0] == "adaptparams")
		{
			_GetAdapterInfo();

		}
		if (command[0] == "quit" && command[0] == "q")
		{
			break;
		}

		std::cout << "\n";

        //cmd_mutex.unlock();
		command.clear();
		Sleep(10);
	}

	th.join();

	closesocket(br_sock);
	closesocket(mul_sock);

	WSACleanup();

	std::cout << "Hello CMake." << endl;
	return 0;
}
