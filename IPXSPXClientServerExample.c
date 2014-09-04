#pragma hdrstop
#pragma argsused

#include <stdio.h>

#ifdef _WIN32
#include <tchar.h>
#else
  typedef char _TCHAR;
  #define _tmain main
#endif

 // Description:

//    This sample illustrates using IPX/SPXII client and

//    servers. The server is very simple and can only handle

//    one client connection at a time. Both client and server codes

//            bundled together in order to share common functions

//

//

// Command Line Parameters/Options:

//    IPXSPXClientServerExample -s -c -n IPX-Addr -e Socket -l IPX-Addr -p [d|s|p] -m -b bytes -r num

//    -s           Act as server

//    -c           Act as client

//    -n IPX-Addr  Servers IPX addr (AABBCCDD.AABBCCDDEEFF)

//    -e Socket    Socket endpoint server is listening on

//    -l IPX-Addr  Local IPX address

//    -p [d|s|p]   Protocol to use

//       d           Datagram   (IPX)

//       s           Stream     (SPXII)

//       p           Seq Packet (SPXII)

//    -m           Enumerate local IPX addresses

//    -b Bytes     Number of bytes to send

//    -r Num       How many sends to perform (client only)

//

//    To run the application as a server, the following command

//    line is an example:

//        IPXSPXClientServerExample -s -e 7171 -p s

//

//    To run the application to act as a client, the following

//    command line is an example:

//        IPXSPXClientServerExample -c -n AABBCCDD.AABBCCDDEEFF -e 7171 -p s

//

//    To enumerate the local IPX adapters, the following command

//    line is an example:

//        IPXSPXClientServerExample -m



#include <winsock2.h>

#include <stdio.h>

#include <wsipx.h>

#include <wsnwlink.h>



#define MAX_DATA_LEN 64000



// Global Variables, not a good approach!

BOOL   bServer = TRUE,     // client or server

bEnumerate = FALSE;                     // enumerate addresses



SOCKET sock = INVALID_SOCKET, newsock = INVALID_SOCKET;

char  *pszServerAddress,    // Server's IPX address string

*pszLocalAddress,                           // Local IPX address string

*pszServerEndpoint,                         // Server's endpoint (socket) string

chProtocol = 's';                                 // Protocol

DWORD  dwNumBytes=128,                      // Number of bytes to send

dwNumToSend=5;                                        // Number of times to send



// Function: CreateSocket

// Description:

//    Create a socket based upon the command line parameters. This

//    creates the main socket (i.e. the listening socket for the

//    server and the connecting socket for the client).

//    SPX sockets use either SOCK_STREAM or SOCK_SEQPACKET but must

//    be of the protocol NSPROTO_SPX or NSPROTO_SPXII.

//    IPX sockets must use SOCK_DGRAM and NSPROTO_IPX.

int CreateSocket()

{

            int  proto, sockettype;



            // Find out the socket type

            if (chProtocol == 'd')

                        sockettype = SOCK_DGRAM;

            else if (chProtocol == 's')

                        sockettype = SOCK_STREAM;

            else

                        sockettype = SOCK_SEQPACKET;



            // Get the protocol

            if (chProtocol == 'd')

                        proto = NSPROTO_IPX;

            else

                        proto = NSPROTO_SPX;



            sock = socket(AF_IPX, sockettype, proto);

            if (sock == INVALID_SOCKET)

            {

                        printf("socket() failed with error code %ld\n", WSAGetLastError());

                        return 1;

            }

            else

            {

                        printf("socket() looks fine!\n");

            }

            return 0;

}



// Function: usage

// Description: Print the usage information.

int usage(char *progname)

{

            printf("Usage: %s [-s|-c] -e Socket -n ServerAddr -l LocalAddr -p [d|s|p] -m -b bytes\n", progname );

            printf("\t -s           Act as server (default)\n");

            printf("\t -c           Act as client\n");

            printf("\t -e Socket    Server's socket (port)\n" );

            printf("\t -n Addr      Server's IPX Address\n" );

            printf("\t -l Addr      Local IPX Address\n" );

            printf("\t -p d|s|p     Protocol type\n");

            printf("\t    d         datagram (IPX)\n");

            printf("\t    s         stream   (SPXII)\n");

            printf("\t    p         sequenced packet (SPXII)\n");

            printf("\t -m           Enumerate Local Addresses\n");

            printf("\t -b int       Number of bytes to send\n");

            printf("\t -r num       Number of repitions to send\n");

            printf("\n");

            printf("Example run as server: %s -s -e 7171 -p s\n", progname);

            printf("Example run as client: %s -c -n AABBCCDD.AABBCCDDEEFF -e 7171 -p s\n", progname);

            printf("Example to enumerate the local IPX adapters: %s -m\n", progname);

            return 0;

 }



 // Function: PrintIpxAddress

 // Description: This function prints out an IPX address in human readable form.

void PrintIpxAddress(char *lpsNetnum, char *lpsNodenum)

{

            int i;



            // Print the network number first

            for (i=0; i < 4 ;i++)

            {

                        printf("%02X", (UCHAR)lpsNetnum[i]);

            }

            printf(".");



            // Print the node number

            for (i=0; i < 6 ;i++)

            {

                        printf("%02X", (UCHAR) lpsNodenum[i]);

            }

            printf("\n");

}



// Function: BtoH

// Description: BtoH () returns the equivalent binary value for an individual

//    character specified in the ascii format.

UCHAR BtoH(char ch)

{

            if ((ch >= '0') && (ch <= '9'))

            {

                        return(ch - '0');

            }

            if ((ch >= 'A') && (ch <= 'F'))

            {

                        return(ch - 'A' + 0xA);

            }

            if ((ch >= 'a') && (ch <= 'f'))

            {

                        return(ch - 'a' + 0xA);

            }



            // Illegal values in the IPX address will not be accepted

            printf("Illegal characters in IPX address!\n");

            return -1;

 }



// Function: AtoH

// Description: AtoH () coverts the IPX address specified in the string

//    (ascii) format to the binary (hexadecimal) format.

void AtoH(char *szDest, char *szSource, int iCount)

{

            while (iCount--)

            {

                        *szDest++ = (BtoH(*szSource++) << 4) + BtoH(*szSource++);

            }

}



// Function: FillIpxAddress

// Description:

//    FillIpxAddress() fills a structure of type SOCKADDR_IPX

//    with relevant address-family, network number, node number

//    and socket (endpoint) parameters

void FillIpxAddress(SOCKADDR_IPX *psa, LPSTR lpsAddress, LPSTR lpsEndpoint)

{

            LPSTR pszPoint;



            ZeroMemory(psa, sizeof(SOCKADDR_IPX));

            psa->sa_family = AF_IPX;



            // Check if an address is specified

            if (lpsAddress != NULL)

            {

                        // Get the offset for node number/network number separator

                        pszPoint = strchr(lpsAddress, '.');



                        if (pszPoint == NULL)

                        {

                                    printf("IPX address does not have a separator\n");

                                    return;

                        }



                        // convert the address in the  string format to binary format

                        AtoH((CHAR *)psa->sa_netnum,  lpsAddress, 4);

                        AtoH((CHAR *)psa->sa_nodenum, pszPoint + 1, 6);

            }

            if (lpsEndpoint != NULL)

            {

                        psa->sa_socket = (USHORT)atoi(lpsEndpoint);

            }

}



// Function: BindSocket

// Description:

//    BindSocket() binds the global socket descriptor 'sock' to

//    the specified address. If an endpoint is specified it uses

//    that or it binds to a system  assigned port.

int BindSocket(SOCKADDR_IPX *psa, LPSTR lpsAddress, LPSTR lpsEndpoint)

{

            int ret, len;



            // Fill the givenSOCKADDR_IPX structure

            FillIpxAddress(psa, lpsAddress, lpsEndpoint);



            ret = bind(sock, (SOCKADDR *) psa, sizeof (SOCKADDR_IPX));

            if (ret == SOCKET_ERROR)

            {

                        printf("bind() failed with error code %ld\n", WSAGetLastError());

                        return 1;

            }

            else

                        printf("bind() is OK...\n");



            // Print the address we are bound to. If a particular interface is not

            // mentioned in the BindSocket() call, this may print the address as

            // 00000000.0000000000000000. This is because of the fact that an

            // interface is picked only when the actual connection establishment

            // occurs, in case of connection oriented socket

            len = sizeof(SOCKADDR_IPX);

            getsockname(sock, (SOCKADDR *)psa, &len);

            printf("Bound to Local Address: ");

            PrintIpxAddress((char *) psa->sa_netnum, (char *) psa->sa_nodenum);

            return 0;

}



// Function: EnumerateAdapters

// Description:EnumerateAdapters () will enumerate the available IPX adapters

//    and print the addresses.

void EnumerateAdapters()

{

            SOCKADDR_IPX     sa_ipx;

            IPX_ADDRESS_DATA ipx_data;

            int        ret, cb, nAdapters, i=0;



            // Create a local socket

            chProtocol = 'd';

            if(CreateSocket() == 0)

                        printf("CreateSocket() is OK...\n");

            else

            {

                        printf("CreateSocket() failed with error code %ld\n", WSAGetLastError());

                        return;

            }



            // Bind to a local address and endpoint

            if(BindSocket(&sa_ipx, NULL, NULL) == 0)

                        printf("BindSocket() failed!...\n");

            else

                        printf("BindSocket() is OK!\n");



            // Call getsockopt() see the total number of adapters

            cb = sizeof(nAdapters);

            ret = getsockopt(sock, NSPROTO_IPX, IPX_MAX_ADAPTER_NUM, (CHAR *) &nAdapters, &cb);

            if (ret == SOCKET_ERROR)

            {

                        printf("getsockopt(IPX_MAX_ADAPTER_NUM) failed with error code %ld\n", WSAGetLastError());

                        return;

            }

            else

                        printf("getsockopt(IPX_MAX_ADAPTER_NUM) is OK...\n");



            printf("Total number of adapters: %d\n", nAdapters);

            // Get the address of each adapter

            for (i=0; i < nAdapters ;i++)

            {

                        memset (&ipx_data, 0, sizeof(ipx_data));

                        ipx_data.adapternum = i;

                        cb = sizeof(ipx_data);



                        ret = getsockopt(sock, NSPROTO_IPX, IPX_ADDRESS, (CHAR *) &ipx_data, &cb);

                        if (ret == SOCKET_ERROR)

                        {

                                    printf("getsockopt(IPX_ADDRESS) failed with error code %ld\n", WSAGetLastError());

                                    return;

        }

                        else

                                    printf("getsockopt(IPX_ADDRESS) #%d is OK...\n", i);



                        // Print each address

                        PrintIpxAddress((char *) ipx_data.netnum, (char *) ipx_data.nodenum);

            }

}



// SendData() is generic routine to send some data over a

// connection-oriented IPX socket.

int SendData(SOCKET s, char *pchBuffer)

{

            int ret;



            ret = send(s, pchBuffer, strlen(pchBuffer), 0);

            if (ret == SOCKET_ERROR)

            {

                        printf("send() failed with error code %ld\n", WSAGetLastError());

                        return -1;

            }

            else

                        printf("send() is OK...\n");



            return ret;

}



// ReceiveData() is generic rotuine to receive some data over a

// connection-oriented IPX socket.

int ReceiveData(SOCKET s, char *pchBuffer)

{

            int ret,iTotal = 0,iLeft=dwNumBytes;



            while (iLeft > 0)

            {

                        ret = recv(s, &pchBuffer[iTotal], iLeft, 0);

                        if (ret == SOCKET_ERROR)

                        {

                                    // This error just redundant...

                                    if (WSAGetLastError() == WSAEWOULDBLOCK)

                                    {

                                                printf("recv() failed with error code %ld\n", WSAGetLastError());

                                                break;

                                    }

                                    if (WSAEDISCON == WSAGetLastError())

                                                printf("Connection closed by peer...\n");

                                    else

                                                printf("recv() failed with error code %ld\n", WSAGetLastError());

                                    return -1;

                        }

                        if (ret == 0)

                        {

                                    printf("recv() is OK...\n");

                                    break;

                        }

                        iTotal += ret;

                        iLeft  -= ret;

            }

            return iTotal;

}



// SendDatagram() is generic routine to send a datagram to a specifid host.

int SendDatagram(SOCKET s, char *pchBuffer, SOCKADDR_IPX *psa)

{

            int ret;



            ret = sendto(s, pchBuffer, strlen (pchBuffer), 0,(SOCKADDR *)psa, sizeof(SOCKADDR_IPX));



            if (ret == SOCKET_ERROR)

            {

                        printf("sendto() failed with error code %ld\n", WSAGetLastError());

                        return -1;

            }

            else

                        printf("sendto() is OK...\n");

            return ret;

}



// ReceiveDatagram() is generic routine to receive a datagram from a specified host.

int ReceiveDatagram(SOCKET s, char *pchBuffer, SOCKADDR_IPX *psa, int *pcb)

{

            int ret;



            ret = recvfrom(s, pchBuffer, MAX_DATA_LEN, 0, (SOCKADDR *) psa, pcb);

            if (ret == SOCKET_ERROR)

            {

                        printf("recvfrom() failed with error code %ld\n", WSAGetLastError());

                        return -1;

            }

            else

                        printf("recvfrom() is OK...\n");



            return ret;

}



// Server () performs the connection-less/connection-oriented server related tasks

void Server()

{

            SOCKADDR_IPX  sa_ipx,  // Server address

                        sa_ipx_client;                        // Client address

            char     chBuffer[MAX_DATA_LEN]; // Data buffer

            int        ret,cb;



            if(CreateSocket() == 0)

                        printf("CreateSocket() is OK...\n");

            else

            {

                        printf("CreateSocket() failed with error code %ld\n", WSAGetLastError());

                        return;

            }



            // Bind to a local address and endpoint

            if(BindSocket(&sa_ipx, pszLocalAddress, pszServerEndpoint) == 0)

                        printf("BindSocket() is OK!\n");

            else

                        printf("BindSocket() failed!\n");



            // Check the Specified protocol. Call listen(), accept() if a connection

            // oriented protocol is specified

            if (chProtocol != 'd')

            {

                        ret = listen(sock, 5);

                        if (ret == SOCKET_ERROR)

                        {

                                    printf("listen() failed with error code %ld\n", WSAGetLastError());

                        }

                        else

                        {

                                    printf("listen() looks fine!\n");

                                    printf("Waiting for a Connection...\n");

                        }



                        // Wait for a connection

                        while (1)

                        {

                                    cb = sizeof(sa_ipx_client);

                                    newsock = accept(sock, (SOCKADDR *) &sa_ipx_client, &cb);



                                    if (newsock == INVALID_SOCKET)

                                    {

                                                printf("accept() failed: %d\n", WSAGetLastError());

                                                return;

                                    }

                                    else

                                                printf("accept() is OK...\n");



                                    // Print the address of connected client

                                    printf("Connected to Client Address: ");

                                    PrintIpxAddress(sa_ipx_client.sa_netnum, sa_ipx_client.sa_nodenum);



                                    while (1)

                                    {

                                                // Receive data on newly created socket

                                                ret = ReceiveData(newsock, chBuffer);

                                                if (ret == 0)

                                                            break;

                                                else if (ret == -1)

                                                            return;



                                                // Print the contents of received data

                                                chBuffer[ret] = '\0';

                                                printf("%d bytes of data received: %s\n", ret, chBuffer);



                                                // Send data on newly created socket

                                                // printf("Sending data...\n");

                                                ret = SendData(newsock, chBuffer);

                                                if (ret == 0)

                                                            break;

                                                else if (ret == -1)

                                                            return;

                                                printf("%d bytes of data sent\n", ret);

                                    }

                                    closesocket(newsock);

                        }

            }

            else  // Server will recv and send datagrams

            {

                        // Receive a datagram on the bound socket

                        while (1)

                        {

                                    cb = sizeof (sa_ipx_client);

                                    ret = ReceiveDatagram(sock, chBuffer, &sa_ipx_client, &cb);

                                    if (ret == -1)

                                                return;



                                    // Print the contents of received datagram and the senders address

                                    printf("Message Received from Client Address - ");

                                    PrintIpxAddress( sa_ipx_client.sa_netnum, sa_ipx_client.sa_nodenum );

                                    chBuffer[ret] = '\0';



                                    printf("%d bytes of data received->%s\n", ret, chBuffer);

                                    // Echo the datagram on the bound socket to the client

                                    ret = SendDatagram(sock, chBuffer, &sa_ipx_client);

                                    if (ret == -1)

                                                return;

                                    printf("%d bytes of data sent\n", ret);

                        }

                        closesocket(newsock);

            }

}



// Client () performs the connection-less/connection-oriented client related tasks

void Client()

{

            SOCKADDR_IPX  sa_ipx,  // client address

                        sa_ipx_server;                       // server address

            char     chBuffer[MAX_DATA_LEN]; // data buffer

            int        ret,cb;

            DWORD i;



            if(CreateSocket() == 0)

                        printf("CreateSocket() is OK...\n");

            else

            {

                        printf("CreateSocket() failed with error code %ld\n", WSAGetLastError());

                        return;

            }



            // Bind to a local address and endpoint

            if(BindSocket(&sa_ipx, pszLocalAddress, NULL) == 0)

                        printf("BindSocket() is OK!\n");

            else

                        printf("BindSocket() failed!\n");



            if (pszServerEndpoint == NULL)

            {

                        printf("Server Endpoint must be specified....Exiting\n");

                        return;

            }



            // Fill the sa_ipx_server address with server address and endpoint

            if (pszServerAddress != NULL)

            {

                        FillIpxAddress ( &sa_ipx_server, pszServerAddress, pszServerEndpoint);

            }

            else

            {

                        printf("Server Address must be specified....Exiting\n");

                        return;

            }



            // Check the Specified protocol. Call connect() if a connection oriented

            // protocol is specified

            if (chProtocol != 'd')

            {

                        printf("Connecting to Server: ");

                        PrintIpxAddress ( sa_ipx_server.sa_netnum, sa_ipx_server.sa_nodenum );



                        // Connect to the server

                        ret = connect(sock, (SOCKADDR *) &sa_ipx_server, sizeof sa_ipx_server);

                        if (ret == SOCKET_ERROR)

                        {

                                    printf("connect() failed with error code %ld\n", WSAGetLastError());

                        }

                        else

                                    printf("connect() is OK...\n");



                        printf("Connected to Server Address: ");

                        PrintIpxAddress( sa_ipx_server.sa_netnum, sa_ipx_server.sa_nodenum );



                        // Send data to the specified server

                        memset(chBuffer, '$', dwNumBytes);

                        chBuffer[dwNumBytes] = 0;



                        for (i=0; i < dwNumToSend ;i++)

                        {

                                    ret = SendData(sock, chBuffer);

                                    if (ret == 0)

                                                return;

                                    else if (ret == -1)

                                                return;

                                    printf("%d bytes of data sent\n", ret);



                                    // Receive data from the server

                                    ret = ReceiveData(sock, chBuffer);

                                    if (ret == 0)

                                                return;

                                    else if (ret == -1)

                                                return;

                                    // Print the contents of received data

                                    chBuffer[ret] = '\0';

                                    printf("%d bytes of data received: %s\n", ret, chBuffer);

                        }

            }

            else

            {

                        // Send a datagram to the specified server

                        memset(chBuffer, '$', dwNumBytes);

                        chBuffer[dwNumBytes] = 0;



         for (i=0; i < dwNumToSend ;i++)

                         {

                                     ret = SendDatagram(sock, chBuffer, &sa_ipx_server);

                                     if (ret == -1)

                                                 return;

                                     printf("%d bytes of data sent...\n", ret);



                                     // Receive a datagram from the server

                                     cb = sizeof(sa_ipx_server);

                                     ret = ReceiveDatagram(sock, chBuffer, &sa_ipx_server, &cb);

                                     if (ret == -1)

                                                 return;



                                     // Print the contents of received data

                                     chBuffer[ret] = '\0';

                                     printf("%d bytes of data received: %s\n", ret, chBuffer);

                         }

            }

}



// Function: CheckProtocol

// Description: CheckProtocol() checks if a valid protocol is specified on the command line.

BOOL CheckProtocol(char chProtocol)

{

            if (('d' != chProtocol) && ('s' != chProtocol) && ('p' != chProtocol))

            {

                        return FALSE;

            }

            return TRUE;

}



// Function: ValidateArgs

// Description:

//    Parses the command line arguments and sets some global

//    variables to determine the behavior of the application.

void ValidateArgs(int argc, char **argv)

{

            int   i;



            for (i = 1; i < argc; i++)

            {

                        if ( (argv[i][0] == '-') || (argv[i][0] == '/')  )

                        {

                                    switch (tolower(argv[i][1]) )

                                    {

                                    case '?':

                                                usage(argv[0]);

                                                break;

                                    case 's':   // act as server

                                                bServer = TRUE;

                                                break;

                                    case 'c':   // act as client

                                                bServer = FALSE;

                                                break;

                                    case 'e':   // server endpoint (port)

                                                if(i+1 < argc)

                                                {

                                                            pszServerEndpoint = argv[i+1];

                                                            ++i;

                                                }

                                                break;

                                    case 'n':   // server address

                                                if(i+1 < argc)

                                                {

                                                            pszServerAddress = argv[i+1];

                                                            ++i;

                                                }

                                                break;

                                    case 'l':   // local address

                                                if(i+1 < argc)

                                                {

                                                            pszLocalAddress = argv[i+1];

                                                            ++i;

                                                }

                                                break;

                                    case 'm':   // enumerate local addresses

                                                bEnumerate = TRUE;

                                                break;

                                    case 'p':   // protocol

                                                if(i+1 < argc)

                                                {

                                                            chProtocol = tolower(argv[i+1][0]);

                                                            if(FALSE == CheckProtocol(chProtocol))

                                                            {

                                                                        printf("Unknown protocol specified!\n\n");

                                                                        usage(argv[0]);

                                                            }

                                                            ++i;

                                                }

                                                break;

                                    case 'b':   // number of bytes to send

                                                if(i+1 < argc)

                                                {

                                                            dwNumBytes = atoi(argv[i+1]);

                                                            ++i;

                                                }

                                                break;

                                    case 'r':   // number of repititions

                                                if(i+1 < argc)

                                                {

                                                            dwNumToSend = atoi(argv[i+1]);

                                                            ++i;

                                                }

                                                break;

                                    default:

                                                usage(argv[0]);

                                                break;

                                    }

                        }

            }

}



// Function: main

// Description:

//    The main function which loads Winsock, parses the command line,

//    and initiates either the client or server routine depending

//    on the parameters passed.

int main(int argc, char **argv)

{

			WSADATA    wsd;



			// Check whether there are arguments supplied

			if(argc == 1)

			{

						usage(argv[0]);

						return 1;

			}

			else

			{

						printf("There are argument(s) supplied...\n");

						ValidateArgs(argc, argv);

			}



	 if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)

	 {

		printf("WSAStartup() failed with error code %ld\n", GetLastError());

		return -1;

	 }

			 else

						 printf("WSAStartup() is OK!\n");



			 // Check to see if the role of the application is

			 // just to enumerate the local adapters

			 if (bEnumerate)

						 EnumerateAdapters();

			 else

			 {

						 // Act as server

						 if (bServer)

									 Server();

						 // Act as client

						 else

									 Client();

			 }



			 // Need to do the shutdown() for SOCK_STREAM/TCP?



			 // Close the bound socket

			 if(closesocket(sock) == 0)

						 printf("closesocket(sock) is OK!\n");

			 else

						 printf("closesocket(sock) failed with error code %ld\n",WSAGetLastError());



			 // Do the WSA* cleanup

			 if(WSACleanup() == 0)

						 printf("WSACleanup() is OK!\n");

			 else

						 printf("WSACleanup() failed with error code %ld\n",WSAGetLastError());

			 return 0;

 }
