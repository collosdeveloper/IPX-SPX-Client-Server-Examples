IPX-SPX-Client-Server-Example
=============================
Setting : 
1. Build Project in C++ Builder XE7
2. Run in Windows XP or lower

IPX/SPX

The IPX protocol is known as the protocol most often used with computers featuring Novell NetWare client/server networking services. IPX provides connectionless communication between two processes; therefore, if a workstation transmits a data packet, there is no guarantee that the packet will be delivered to the destination. If an application needs guaranteed delivery of data and insists on using IPX, it can use a higher-level protocol over IPX, such as the Sequence Packet Exchange (SPX) and SPX II protocols, in which SPX packets are transmitted through IPX. Winsock provides applications with the capability to communicate through IPX on Windows 95, Windows 98, Windows Me, and Windows NT platforms but not on Windows CE.

 
The Addressing Scheme

In an IPX network, network segments are bridged using an IPX router. Every network segment is assigned a unique four-byte network number. As more network segments are bridged, IPX routers manage communication between different network segments using the unique network segment numbers. When a computer is attached to a network segment, it is identified using a unique six-byte node number, which is usually the network adapter's physical address. A node (which is a computer) is typically capable of having one or more processes forming communication over IPX. IPX uses socket numbers to distinguish communication for processes on a node.

To prepare a Winsock client or server application for IPX communication, you have to set up a SOCKADDR_IPX structure. The SOCKADDR_IPX structure is defined in the WSIPX.H header file, and your application must include this file after including WINSOCK2.H. The SOCKADDR_IPX structure is defined as:

typedef struct sockaddr_ipx

{

    short          sa_family;
    char           sa_netnum[4];
    char           sa_nodenum[6];

    unsigned short sa_socket;

} SOCKADDR_IPX, *PSOCKADDR_IPX, FAR *LPSOCKADDR_IPX;

The sa_family field should always be set to the AF_IPX value. The sa_netnum field is a four-byte number representing a network number of a network segment on an IPX network. The sa_nodenum field is a six-byte number representing a node number of a computer's physical address. The sa_socket field represents a socket or port used to distinguish IPX communication on a single node.

 
Creating a Socket

Creating a socket using IPX offers several possibilities. To open an IPX socket, call the socket function or the WSASocket() function with the address family AF_IPX, the socket type SOCK_DGRAM, and the protocol NSPROTO_IPX, as follows:

s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);

s = WSASocket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX, NULL, 0, WSA_FLAG_OVERLAPPED);

Note that the third parameter protocol must be specified and cannot be 0. This is important because this field can be used to set specific IPX packet types.

IPX provides unreliable connectionless communication using datagrams. If an application needs reliable communication using IPX, it can use higher-level protocols over IPX, such as SPX and SPX II. This can be accomplished by setting the type and protocol fields of the socket and WSASocket() calls to the socket type SOCK_SEQPACKET or SOCK_STREAM, and the protocol NSPROTO_SPX or NSPROTO_SPXII.

If SOCK_STREAM is specified, data is transmitted as a continuous stream of bytes with no message boundaries, similar to how sockets in TCP/IP behave. On the other hand, if SOCK_SEQPACKET is specified, data is transmitted with message boundaries. For example, if a sender transmits 2000 bytes, the receiver won't return until all 2000 bytes have arrived. SPX and SPX II accomplish this by setting an end-of-message bit in an SPX header. When SOCK_SEQPACKET is specified, this bit is respected, meaning Winsock recv() and WSARecv() calls won't complete until a packet is received with this bit set. If SOCK_STREAM is specified, the end-of-message bit isn't respected, and recv completes as soon as any data is received, regardless of the setting of the end-of-message bit. From the sender's perspective (using the SOCK_SEQPACKET type), sends smaller than a single packet are always sent with the end-of-message bit set. Sends larger than single packets are packetized with the end-of-message bit set on only the last packet of the send.

 
Binding a Socket

When an IPX application associates a local address with a socket using bind, you shouldn't specify a network number and a node address in a SOCKADDR_IPX structure. The bind() function populates these fields using the first IPX network interface available on the system. If a machine has multiple network interfaces (a multihomed machine), it isn't necessary to bind to a specific interface. Windows 95, Windows 98, Windows Me, and Windows NT platforms provide a virtual internal network in which every network interface can be reached regardless of the physical network it is attached to. We will describe internal network numbers in greater detail later in this chapter. After your application binds successfully to a local interface, you can retrieve local network number and node number information using the getsockname() function, as in the following code fragment:

SOCKET sdServer;

SOCKADDR_IPX IPXAddr;

int addrlen = sizeof(SOCKADDR_IPX);

if ((sdServer = socket (AF_IPX, SOCK_DGRAM, NSPROTO_IPX)) == INVALID_SOCKET)

{
    printf("socket failed with error %d\n", WSAGetLastError());

    return;
}

ZeroMemory(&IPXAddr, sizeof(SOCKADDR_IPX));

IPXAddr.sa_family = AF_IPX;

IPXAddr.sa_socket = htons(5150);

if (bind(sdServer, (PSOCKADDR) &IPXAddr, sizeof(SOCKADDR_IPX))  == SOCKET_ERROR)

{
    printf("bind failed with error %d\n", WSAGetLastError());

    return;
}

if (getsockname(sdServer, (PSOCKADDR) &IPXAddr, &addrlen) == SOCKET_ERROR)

{
    printf("getsockname failed with error %d", WSAGetLastError());

    return;
}

// Print out SOCKADDR_IPX information returned from getsockname()

Network Number vs. Internal Network Number

A network number (known as an external network number) identifies network segments in IPX and is used for routing IPX packets between network segments. Windows 95, Windows 98, Windows Me, and Windows NT platforms and so on also feature an internal network number that is used for internal routing purposes and to uniquely identify the computer on an inter-network (several networks bridged together). The internal network number is also known as a virtual network number, the internal network number identifies another (virtual) segment on the inter-network. Thus, if you configure an internal network number for a computer running Windows 95, Windows 98, Windows Me, or Windows NT platforms, a NetWare server or an IPX router will add an extra hop in its route to that computer.

The internal virtual network serves a special purpose in the case of a multihomed computer. When applications bind to a local network interface, they shouldn't specify local interface information but instead should set the sa_netnum and sa_nodenum fields of a SOCKADDR_IPX structure to 0. This is because IPX is able to route packets from any external network to any of the local network interfaces using the internal virtual network. For example, even if your application explicitly binds to the network interface on Network A, and a packet comes in on Network B, the internal network number will cause the packet to be routed internally so that your application receives it.

 
Setting IPX Packet Types Through Winsock

Winsock allows your application to specify IPX packet types when you create a socket using the NSPROTO_IPX protocol specification. The packet type field in an IPX packet indicates the type of service offered or requested by the IPX packet. In Novell, the following IPX packet types are defined:

    01h Routing Information Protocol (RIP) Packet
    04h Service Advertising Protocol (SAP) Packet
    05h Sequenced Packet Exchange (SPX) Packet
    11h NetWare Core Protocol (NCP) Packet
    14h Propagated Packet for Novell NetBIOS

To modify the IPX packet type, simply specify NSPROTO_IPX + n as the protocol parameter of the socket API, with n representing the packet type number. For example, to open an IPX socket that sets the packet type to 04h (SAP Packet), use the following socket call:

s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX + 0x04);

Name Resolution

As you can probably tell, addressing IPX in Winsock is sort of ugly because you must supply multi-byte network and node numbers to form an address. IPX provides applications with the ability to locate services by using user-friendly names to retrieve network number, node number, and port number in an IPX network through the SAP protocol. Winsock 2 provides a protocol-independent method for name registration using the WSASetService() API function. Through the SAP protocol, IPX server applications use WSASetService() to register under a user-friendly name the network number, node number, and port number they are listening on. Winsock 2 also provides a protocol-independent method of name resolution through the following API functions: WSALookupServiceBegin(), WSALookupServiceNext(), and WSALookupServiceEnd().

It is possible to perform your own name-service registration and lookups by opening an IPX socket and specifying an SAP packet type. After opening the socket, you can begin broadcasting SAP packets to the IPX network to register and locate services on the network. This requires that you understand the SAP protocol in great detail and that you deal with the programming details of decoding an IPX SAP packet.

 
IPX Client-server Program Example

The following program example is an IPX client and server application that demonstrates how to transmit datagrams over IPX or transmit reliable data communication over SPX. Create a new empty Win32 console mode application and add the project/solution name.

See Code!

If the IPX/SPX protocol was installed on your Windows machine, it should be visible as NWLink IPX/SPX/NetBIOS Compatible Transport Protocol shown below seen in the Local Area Connection Properties page.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol005.png)


For program example testing purpose, if the IPX/SPX protocol not installed on your testing machine, follow the following steps (shown for Windows XP Pro SP2 machine).

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol006.png)

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol008.png)

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol009.png)


The following screenshots show some of the IPX/SPX compatible protocol settings.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol010.png)

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol011.png)


Next, we install the client component for IPX/SPX.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol012.png)

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol013.png)


After completing the IPX/SPX components installation, we just disable and then enable the Local Area Connection to make the new settings effective instead of restarting the machine.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol014.png)

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol015.png)

Testing the IPX/SPX Client-server program

Now, let test our IPX/SPX program example demonstrating the client-server communication.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol016.png)


Firstly, we run the program as a server using the following arguments.
IPXSPXClientServerExample -s -e 7171 -p s

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol017.png)


Then run the program as client using the server’s IPX address, 12345678.000000000001 and server’s listening socket, 7171. The command is as follows:
IPXSPXClientServerExample -c -n 12345678.000000000001 -e 7171 -p s

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol018.png)

The previous server program screenshot is shown below.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol019.png)

The following screenshot shows the local IPX addresses enumeration output sample.

![alt tag](http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4_files/winsock2otherprotocol020.png)


The IPX/SPX protocol owned by Novell used in its NetWare OS.

http://frolov-lib.ru/books/bsp/v08/ch2_4.htm



