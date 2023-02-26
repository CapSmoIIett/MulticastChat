
#include "header.h"

std::string GetLocalIp()
{
    char host[255];
    int hostname = 0;

    char* buffer = nullptr;
    struct hostent* host_entry = nullptr;

    hostname = gethostname(host, sizeof(host));
    if (hostname == -1)
    {
        return "";
    }

    host_entry = gethostbyname(host);
    if (host_entry == nullptr)
    {
        return "";
    }

    buffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    if (buffer == nullptr)
    {
        return "";
    }

    return std::string(buffer);
}

std::string GetMask()
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapter = NULL;
    ULONG ulOutBufLen = 0;
    u_char p[6];

    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
    {
        free(pAdapterInfo);
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
    }
    GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    pAdapter = pAdapterInfo;

    auto IP = GetLocalIp();

    while (pAdapter) 
    {
        if (IP == pAdapter->IpAddressList.IpAddress.String)
            return std::string(pAdapter->IpAddressList.IpMask.String);

        pAdapter = pAdapter->Next;
    }

    return "0.0.0.0";
}

std::string CalculateBroadcast(std::string ip, std::string mask)
{
    auto n_ip = inet_addr(ip.c_str());
    auto n_mask = inet_addr(mask.c_str());

    unsigned int bits = n_mask ^ 0xffffffff;
    in_addr addr;
    
    addr.s_addr = n_ip | bits;
    
    return inet_ntoa(addr);
}

in_addr CalculateBroadcastAddr(std::string ip, std::string mask)
{
    auto n_ip = inet_addr(ip.c_str());
    auto n_mask = inet_addr(mask.c_str());

    unsigned int bits = n_mask ^ 0xffffffff;
    in_addr addr;

    addr.s_addr = n_ip | bits;
    
    return addr;
}


void _GetAdapterInfo ()
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapter = NULL;
    ULONG ulOutBufLen = 0;
    u_char p[6];

    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
    {
        free(pAdapterInfo);
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
    }
    GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    pAdapter = pAdapterInfo;

    while (pAdapter) 
    {
        printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
        printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
        memcpy(p, pAdapter->Address, 6);
        //printf("\tAdapter Addr: \tX:X:X:X:X:X\n",
         //   p[0], p[1], p[2], p[3], p[4], p[5]);
        printf("\tIP Addr: \t%s\n", pAdapter->IpAddressList.IpAddress.String);
        printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);
        printf("\tIP Gateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
        if (pAdapter->DhcpEnabled) {
            printf("\tDHCP Enable: Yes\n");
            printf("\tLease Obtained: %ld\n", pAdapter->LeaseObtained);
        }
        else {
            printf("\tDHCP Enable: No\n");
        }
        if (pAdapter->HaveWins) {
            printf("\tHave Wins: Yes\n");
            printf("\t\tPrimary Wins Server: \t%s\n", pAdapter->PrimaryWinsServer.IpAddress.String);
            printf("\t\tSecondary Wins Server: \t%s\n", pAdapter->SecondaryWinsServer.IpAddress.String);
        }
        else {
            printf("\tHave Wins: No\n");
        }
        printf("\n\n");

        pAdapter = pAdapter->Next;
    }

    return;
}
