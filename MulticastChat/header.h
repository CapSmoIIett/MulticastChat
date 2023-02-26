// MulticastChat.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <sstream>

#include <vector>
#include <string>

#include <algorithm>

#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
#include <IPTypes.h>
#include <Iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")

#ifdef  DEBUG	 
#define DBG(exp) exp
#else
#define DBG(exp) 
#endif

std::string GetLocalIp();
std::string GetMask();

std::string CalculateBroadcast(std::string ip, std::string mask);
in_addr CalculateBroadcastAddr(std::string ip, std::string mask);

void _GetAdapterInfo();

