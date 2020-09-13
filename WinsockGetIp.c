// WinsockGetIp.c:  Simple winsock program to get your IP and other associated information with your network adapters

#ifdef _WIN32
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0600
	#endif

	#include <WinSock2.h>
	#include <WS2tcpip.h>
	#include <iphlpapi.h>
	#include <IPTypes.h>
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "iphlpapi.lib")

#endif

#include <stdio.h>
#include <stdlib.h>

int main()
{
#ifdef _WIN32
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize winsock.\n");
		return 1;
	}
#endif

	printf("Ready to use socket API.\n");
		
	DWORD asize = 20000;
	PIP_ADAPTER_ADDRESSES adapters;

	do
	{
		adapters = (PIP_ADAPTER_ADDRESSES)malloc(asize);

		if (adapters == NULL)
		{
			fprintf(stderr, "Couldn't allocate %ld bytes for adapters.\n", asize);
			WSACleanup();
			return EXIT_FAILURE;
		}

		int r = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS, NULL, adapters, &asize);

		if (r == ERROR_BUFFER_OVERFLOW)
		{
			fprintf(stderr, "GetAdaptersAddresses wants %ld bytes.\n", asize);
			free(adapters);
		}
		else if (r == ERROR_SUCCESS)
			break;
		else
		{
			fprintf(stderr, "Error from GetAdaptersAddresses: %d\n", r);
			free(adapters);
			WSACleanup();
			return EXIT_FAILURE;
		}

	} while (!adapters);

	PIP_ADAPTER_ADDRESSES adapter = adapters;
	while (adapter != NULL)
	{
		printf("\n%ls\n", adapter->FriendlyName);

		PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;

		switch(adapter->OperStatus)
		{
		case  IfOperStatusUp:
			while (address != NULL)
			{
				printf("\t%s", address->Address.lpSockaddr->sa_family == AF_INET ? "IPv4:" : "IPv6:");

				char ap[100];

				getnameinfo(address->Address.lpSockaddr, address->Address.iSockaddrLength, ap, sizeof(ap), NULL, NULL, NI_NUMERICHOST);

				printf("\t\t%s\n", ap);

				address = address->Next;
			}

			// display dhcp status
			printf("\t%s", (adapter->Flags & IP_ADAPTER_DHCP_ENABLED) == IP_ADAPTER_DHCP_ENABLED ? "DHCP:\t\t" : "DHCP:\t\tNo");

			// ipv4 & ipv6 dhcp
			if ((adapter->Flags & IP_ADAPTER_DHCP_ENABLED) == IP_ADAPTER_DHCP_ENABLED)
			{
				char dhcpv4ap[100];
				char dhcpv6ap[100];
								
				if ((adapter->Flags & IP_ADAPTER_IPV4_ENABLED) == IP_ADAPTER_IPV4_ENABLED)
				{
					getnameinfo(adapter->Dhcpv4Server.lpSockaddr, adapter->Dhcpv4Server.iSockaddrLength, dhcpv4ap, sizeof(dhcpv4ap), NULL, NULL, NI_NUMERICHOST);
					printf("v4:%s\n", dhcpv4ap);
				}

				// need to test  Not sure if this works properly
				if ((adapter->Flags & IP_ADAPTER_IPV6_MANAGE_ADDRESS_CONFIG) == IP_ADAPTER_IPV6_MANAGE_ADDRESS_CONFIG)
				{
					getnameinfo(adapter->Dhcpv6Server.lpSockaddr, adapter->Dhcpv6Server.iSockaddrLength, dhcpv6ap, sizeof(dhcpv6ap), NULL, NULL, NI_NUMERICHOST);
					printf("v6:%s\n", dhcpv6ap);
				}
			}
			else
				printf("\n");
		
			// gateway addresses
			char gateway[100];
			PIP_ADAPTER_GATEWAY_ADDRESS gateways = adapter->FirstGatewayAddress;
			int gatewayCounter = 1;

			while(gateways != NULL)
			{
				getnameinfo(gateways->Address.lpSockaddr, gateways->Address.iSockaddrLength, gateway, sizeof(gateway), NULL, NULL, NI_NUMERICHOST);
				printf("\tGateway %d:\t%s\n", gatewayCounter, gateway);
				
				gateways = gateways->Next;
				gatewayCounter++;
			}

			//dns servers
			char dns[100];
			PIP_ADAPTER_DNS_SERVER_ADDRESS dnsServers= adapter->FirstDnsServerAddress;
			int dnsCounter = 1;

			while (dnsServers != NULL)
			{
				getnameinfo(dnsServers->Address.lpSockaddr, dnsServers->Address.iSockaddrLength, dns, sizeof(dns), NULL, NULL, NI_NUMERICHOST);
				printf("\tDNS %d:\t\t%s\n", dnsCounter, dns);

				dnsServers = dnsServers->Next;
				dnsCounter++;
			}

			break;

		case IfOperStatusDown:
			printf("\tStatus: Not operational\n");
			break;
		
		default:
			printf("\tStatus: Not operational but with extra conditions\n");
			break;
		}

		adapter = adapter->Next;
	}

	free(adapters);

#ifdef _WIN32
	WSACleanup();
#endif

	return EXIT_SUCCESS;
}