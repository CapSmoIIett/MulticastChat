// MulticastChat.cpp : Defines the entry point for the application.
//

#define DEBUG

#include "header.h"


using namespace std;

int main()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 255;
	}

	int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock <= 0)
	{
		return -1;
	}

	int broadcastEnable = 1;
	int ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		(const char*)&broadcastEnable, sizeof(broadcastEnable));
	if (ret < 0)
	{
		return -1;
	}

	int reuseEnable = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&reuseEnable, sizeof(reuseEnable));
	if (ret < 0)
	{
		return -1;
	}


	SOCKADDR_IN recv_addr;
	memset(&recv_addr, 0, sizeof(recv_addr));
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(1900);
	recv_addr.sin_addr.s_addr = INADDR_ANY;

	int len = sizeof(recv_addr);
	if (bind(sock, (SOCKADDR*)&recv_addr, sizeof(SOCKADDR_IN)) < 0)
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

		FD_ZERO(&fd);
		FD_SET(sock, &fd);

		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 10;

		auto r = select(sock + 1, &fd, nullptr, nullptr, &timeout);

		if (0 < r)
		{
			sockaddr_in addr;
			char buf[215];

			auto addr_len = sizeof(addr);
			recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
			std::cout << buf;
		}

		if (command.empty())
			continue;
		if (command[0] == "send")
		{
			if (command.size() > 1 && command[1] == "b")	// broadcast
			{
				sockaddr_in broadcastAddr; // Make an endpoint
				memset(&broadcastAddr, 0, sizeof broadcastAddr);

				char* request = "hello";

				broadcastAddr.sin_port = htons(1900); // Set port 1900
				broadcastAddr.sin_family = AF_INET;
				broadcastAddr.sin_addr = CalculateBroadcastAddr(GetLocalIp(), GetMask());// INADDR_ANY;
				//inet_pton(AF_INET, "239.255.255.250", &broadcastAddr.sin_addr); // Set the broadcast IP address

				// Send the broadcast request, ie "Any upnp devices out there?"
				//char* request = "M-SEARCH * HTTP/1.1\r\nHOST:239.255.255.250:1900\r\nMAN:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";
				auto nResult = sendto(sock, request, strlen(request), 0, (struct sockaddr*)&broadcastAddr, sizeof broadcastAddr);

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
				addr.sin_addr.s_addr = inet_addr("239.255.255.250");
				addr.sin_port = htons(1900);
			}
		}
		if (command[0] == "add")
		{
			ip_mreq mreq;
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");

			ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
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

		command.clear();
	}

	th.join();
	WSACleanup();

	std::cout << "Hello CMake." << endl;
	return 0;
}
