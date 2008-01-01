/***************************************************************************
 *   bannergrab-ng - next gen banner grabbing tool                         *
 *   Copyright (C) 2007-2008 by Ian Ventura-Whiting (Fizz)                 *
 *   fizz@titania.co.uk                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 *   In addition, as a special exception, the copyright holders give       *
 *   permission to link the code of portions of this program with the      *
 *   OpenSSL library under certain conditions as described in each         *
 *   individual source file, and distribute linked combinations            *
 *   including the two.                                                    *
 *   You must obey the GNU General Public License in all respects          *
 *   for all of the code used other than OpenSSL.  If you modify           *
 *   file(s) with this exception, you may extend this exception to your    *
 *   version of the file(s), but you are not obligated to do so.  If you   *
 *   do not wish to do so, delete this exception statement from your       *
 *   version.  If you delete this exception statement from all source      *
 *   files in the program, then also delete it here.                       *
 ***************************************************************************/



// Includes...
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#if !defined(NOSSL)
#include <openssl/ssl.h>
#endif

// Defines...
#define false 0
#define true 1

#define MAX_LOADS 80
#define MAX_SIZE 513

// Show help
#define mode_help 0
// Show Version
#define mode_version 1
// Grab Banners
#define mode_grab 2
// Noisy Output, and show error text (default is to only show output
#define mode_verbose 3
// Don't send triggers, just connect and disconnect
#define mode_notrig 4
// Show supported triggers
#define mode_showtriggers 5

const char *program_banner = "    _                                                _\n"
                             "   | |__   __ _ _ __  _ __   ___ _ __ __ _ _ __ __ _| |__\n"
                             "   | '_ \\ / _` | '_ \\| '_ \\ / _ \\ '__/ _` | '__/ _` | '_ \\\n"
                             "   | |_) | (_| | | | | | | |  __/ | | (_| | | | (_| | |_) |\n"
                             "   |_.__/ \\__,_|_| |_|_| |_|\\___|_|  \\__, |_|  \\__,_|_.__/\n"
                             "                                     |___/\n"
                             "                         Version 3.3\n"
                             "              http://bannergrab.sourceforge.net\n"
                             "         Copyright (C) 2007-2008 Ian Ventura-Whiting\n\n";
const char *program_version = "bannergrab-ng version 3.3\nCopyright (C) 2007-2008 Ian Ventura-Whiting\n";


// Colour Console Output...
#if !defined(__WIN32__)
const char *RESET = "[0m";			// DEFAULT
const char *COL_RED = "[31m";		// RED
const char *COL_BLUE = "[34m";		// BLUE
const char *COL_GREEN = "[32m";	// GREEN
#else
const char *RESET = "";
const char *COL_RED = "";
const char *COL_BLUE = "";
const char *COL_GREEN = "";
#endif


struct triggerConfig
{
	int	size;						// Trigger length, can be set to 0 for auto.
	const char *trigger;			// Trigger
	struct triggerConfig *next;
};

// Specific service triggers...
struct serviceConfig
{
	int port;						// Usual Service Port
	const char *service;			// Service Name
	int tcp;						// TCP service (false == UDP)
	int show;						// Show the triggers in the output
	int connectBanner;				// Get a connection banner
	int ssl;						// Try SSL
	int timeout;					// Read timeout override
	struct triggerConfig *trigger;
	struct serviceConfig *next;
};

// MS-SQL Trigger...
struct triggerConfig trig_mssql = {224, "\x10\x01\x00\xe0\x00\x00\x01\x00\xd8\x00\x00\x00\x01\x00\x00\x71\x00\x00\x00\x00\x00\x00\x00\x07\x6c\x04\x00\x00\x00\x00\x00\x00\xe0\x03\x00\x00\x00\x00\x00\x00\x09\x08\x00\x00\x56\x00\x0a\x00\x6a\x00\x0a\x00\x7e\x00\x00\x00\x7e\x00\x20\x00\xbe\x00\x09\x00\x00\x00\x00\x00\xd0\x00\x04\x00\xd8\x00\x00\x00\xd8\x00\x00\x00\x00\x0c\x29\xc6\x63\x42\x00\x00\x00\x00\xc8\x00\x00\x00\x42\x00\x61\x00\x6e\x00\x6e\x00\x65\x00\x72\x00\x47\x00\x72\x00\x61\x00\x62\x00\x42\x00\x61\x00\x6e\x00\x6e\x00\x65\x00\x72\x00\x47\x00\x72\x00\x61\x00\x62\x00\x4d\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74\x00\x20\x00\x44\x00\x61\x00\x74\x00\x61\x00\x20\x00\x41\x00\x63\x00\x63\x00\x65\x00\x73\x00\x73\x00\x20\x00\x43\x00\x6f\x00\x6d\x00\x70\x00\x6f\x00\x6e\x00\x65\x00\x6e\x00\x74\x00\x73\x00\x31\x00\x32\x00\x37\x00\x2e\x00\x30\x00\x2e\x00\x30\x00\x2e\x00\x31\x00\x4f\x00\x44\x00\x42\x00\x43\x00", 0};

// LDAP Trigger...
struct triggerConfig trig_ldap1 = {55, "\x30\x35\x02\x01\x02\x63\x30\x04\x00\x0a\x01\x00\x0a\x01\x00\x02\x01\x00\x02\x01\x00\x01\x01\x00\x87\x0b\x6f\x62\x6a\x65\x63\x74\x43\x6c\x61\x73\x73\x30\x10\x04\x0e\x6e\x61\x6d\x69\x6e\x67\x43\x6f\x6e\x74\x65\x78\x74\x73", 0};
struct triggerConfig trig_ldap  = {14, "\x30\x0c\x02\x01\x01\x60\x07\x02\x01\x03\x04\x00\x80\x00", &trig_ldap1};

// FW1 Admin Trigger...
struct triggerConfig trig_fw1admin = {0, "???\r\n?\r\n", 0};

// SNMP Trigger...
struct triggerConfig trig_snmp1 = {44, "\x30\x2a\x02\x01\x00\x04\x07\x70\x72\x69\x76\x61\x74\x65\xa0\x1c\x02\x04\xff\xff\xff\xfe\x02\x01\x00\x02\x01\x00\x30\x0e\x30\x0c\x06\x08\x2b\x06\x01\x02\x01\x01\x01\x00\x05\x00", 0};
struct triggerConfig trig_snmp  = {43, "\x30\x29\x02\x01\x00\x04\x06\x70\x75\x62\x6c\x69\x63\xa0\x1c\x02\x04\xff\xff\xff\xff\x02\x01\x00\x02\x01\x00\x30\x0e\x30\x0c\x06\x08\x2b\x06\x01\x02\x01\x01\x01\x00\x05\x00", &trig_snmp1};

// NetBIOS Name Service (NBNS)
struct triggerConfig trig_nbns  = {50, "\xa2\x48\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x20\x43\x4b\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x00\x00\x21\x00\x01", 0};

// NTP Trigger...
struct triggerConfig trig_ntp2 = {12, "\x16\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00", 0};
struct triggerConfig trig_ntp1 = {12, "\x16\x02\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00", &trig_ntp2};
struct triggerConfig trig_ntp  = {48, "\xe3\x00\x04\xfa\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xca\x9b\xa3\x35\x2d\x7f\x95\x0b", &trig_ntp1};

// NNTP Trigger...
struct triggerConfig trig_nntp2 = {0, "QUIT\r\n", 0};
struct triggerConfig trig_nntp1 = {0, "LIST NEWSGROUPS\r\n", &trig_nntp2};
struct triggerConfig trig_nntp  = {0, "HELP\r\n", &trig_nntp1};

// POP Trigger...
struct triggerConfig trig_pop = {0, "QUIT\r\n", 0};

// HTTP Trigger...
struct triggerConfig trig_http = {0, "OPTIONS / HTTP/1.0\r\n\r\n", 0};

// Finger Trigger...
struct triggerConfig trig_finger = {0, "root bin lp wheel spool adm mail postmaster news uucp snmp daemon\r\n", 0};

// SMTP Trigger...
struct triggerConfig trig_smtp5 = {0, "QUIT\r\n", 0};
struct triggerConfig trig_smtp4 = {0, "EXPN postmaster\r\n", &trig_smtp5};
struct triggerConfig trig_smtp3 = {0, "VRFY bannergrab123\r\n", &trig_smtp4};
struct triggerConfig trig_smtp2 = {0, "VRFY postmaster\r\n", &trig_smtp3};
struct triggerConfig trig_smtp1 = {0, "HELP\r\n", &trig_smtp2};
struct triggerConfig trig_smtp  = {0, "HELO bannergrab.com\r\n", &trig_smtp1};

// FTP Trigger...
struct triggerConfig trig_ftp3 = {0, "QUIT\n", 0};
struct triggerConfig trig_ftp2 = {0, "PASS banner@grab.com\n", &trig_ftp3};
struct triggerConfig trig_ftp1 = {0, "USER anonymous\n", &trig_ftp2};
struct triggerConfig trig_ftp  = {0, "HELP\n", &trig_ftp1};

// Echo Trigger...
struct triggerConfig trig_echo = {0, "Echo\r\n", 0};

// Null Trigger...
struct triggerConfig trig_null = {0, "", 0};


//                                Port  Description   tcp    shw t  shw c  ssl    tm triggers        next
struct serviceConfig service25 = {9100, "Printer",    true,  false, true,  false, 0, &trig_null,     0};
struct serviceConfig service24 = {3306, "MySQL",      true,  false, true,  false, 6, &trig_null,     &service25};
struct serviceConfig service23 = {1433, "MSSQL",      true,  false, false, false, 0, &trig_mssql,    &service24};
struct serviceConfig service22 = {902,  "VMWare",     true,  false, true,  false, 0, &trig_null,     &service23};
struct serviceConfig service21 = {636,  "LDAPS",      true,  false, false, true,  0, &trig_ldap,     &service22};
struct serviceConfig service20 = {631,  "IPP",        true,  false, false, false, 0, &trig_http,     &service21};
struct serviceConfig service19 = {587,  "Submission", true,  true,  true,  false, 0, &trig_smtp,     &service20};
struct serviceConfig service18 = {443,  "HTTPS",      true,  false, false, true,  0, &trig_http,     &service19};
struct serviceConfig service17 = {389,  "LDAP",       true,  false, false, false, 0, &trig_ldap,     &service18};
struct serviceConfig service16 = {256,  "FW1Admin",   true,  false, true,  false, 0, &trig_fw1admin, &service17};
struct serviceConfig service15 = {161,  "SNMP",       false, false, false, false, 2, &trig_snmp,     &service16};
struct serviceConfig service14 = {137,  "NetBIOS-NS", false, false, false, false, 2, &trig_nbns,     &service15};
struct serviceConfig service13 = {123,  "NTP",        false, false, false, false, 2, &trig_ntp,      &service14};
struct serviceConfig service12 = {119,  "NNTP",       true,  false, true,  false, 0, &trig_nntp,     &service13};
struct serviceConfig service11 = {110,  "POP3",       true,  false, true,  false, 0, &trig_pop,      &service12};
struct serviceConfig service10 = {109,  "POP2",       true,  false, true,  false, 0, &trig_pop,      &service11};
struct serviceConfig service9  = {80,   "HTTP",       true,  false, false, false, 0, &trig_http,     &service10};
struct serviceConfig service8  = {79,   "Finger",     true,  false, false, false, 0, &trig_finger,   &service9};
struct serviceConfig service7  = {25,   "SMTP",       true,  true,  true,  false, 0, &trig_smtp,     &service8};
struct serviceConfig service6  = {22,   "SSH",        true,  false, true,  false, 0, &trig_null,     &service7};
struct serviceConfig service5  = {21,   "FTP",        true,  true,  true,  false, 0, &trig_ftp,      &service6};
struct serviceConfig service4  = {19,   "Chargen",    true,  false, true,  false, 0, &trig_null,     &service5};
struct serviceConfig service3  = {17,   "QOTD",       true,  false, true,  false, 0, &trig_null,     &service4};
struct serviceConfig service2  = {13,   "Daytime",    true,  false, true,  false, 0, &trig_null,     &service3};
struct serviceConfig service1  = {9,    "Discard",    true,  false, false, false, 0, &trig_echo,     &service2};
struct serviceConfig service   = {7,    "Echo",       true,  true,  false, false, 0, &trig_echo,     &service1};

// Default trigger...
struct triggerConfig trig_default = {0, "OPTIONS / HTTP/1.0\r\n\r\nHELP\r\n", 0};
struct serviceConfig defaultService = {0, "DEFAULT", true, false, true, false, 0, &trig_default, 0};


// Timeout Function
void timeoutHandler()
{
	exit(0);
}


// Create a TCP or UDP socket
int netConnect(const char *host, int port, int timeout, int verbose, int tcp)
{
	// Variables...
	int socketDescriptor;
	struct sockaddr_in localAddress;
	struct hostent *hostStruct;
	struct sockaddr_in serverAddress;
	int status;

	// Resolve Host Name
	hostStruct = gethostbyname(host);
	if (hostStruct == NULL)
	{
		if (verbose == true)
			printf("%sERROR: Could not resolve hostname %s.%s\n", COL_RED, host, RESET);
		return 0;
	}

	// Configure Server Address and Port
	serverAddress.sin_family = hostStruct->h_addrtype;
	memcpy((char *) &serverAddress.sin_addr.s_addr, hostStruct->h_addr_list[0], hostStruct->h_length);
	serverAddress.sin_port = htons(port);

	// Create Socket
	if (tcp == true)
		socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	else
		socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if(socketDescriptor < 0)
	{
		if (verbose == true)
			printf("%sERROR: Could not open a socket.%s\n", COL_RED, RESET);
		return 0;
	}

	// Configure Local Port
	localAddress.sin_family = AF_INET;
	localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddress.sin_port = htons(0);
	status = bind(socketDescriptor, (struct sockaddr *) &localAddress, sizeof(localAddress));
	if(status < 0)
	{
		if (verbose == true)
			printf("%sERROR: Could not bind to port.%s\n", COL_RED, RESET);
		return 0;
	}

	// Connect
	alarm(timeout);
	status = connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	alarm(0);
	if(status < 0)
	{
		if (verbose == true)
			printf("%sERROR: Could not open a connection to host %s on port %d.%s\n", COL_RED, host, port, RESET);
		return 0;
	}

	// Return
	return socketDescriptor;
}


// Print data
void printData(const unsigned char *buffer, int dataSize, int hexOutput)
{
	// Variables...
	int charPointer;
	int containsNonPrintable;
	int counter;
	char asciiOutput[17];

	// init...
	containsNonPrintable = false;

	// If HEX output is disabled...
	if (hexOutput == false)
		printf("%s", buffer);
	else
	{

		// Find any non-printable characters...
		charPointer = 0;
		while (charPointer < dataSize)
		{
			if ((buffer[charPointer] < 32) && (buffer[charPointer] > 13))
				containsNonPrintable = true;
			else if (buffer[charPointer] < 9)
				containsNonPrintable = true;
			else if ((buffer[charPointer] > 126) && (buffer[charPointer] < 160))
				containsNonPrintable = true;
			charPointer++;
		}

		// If all the characters are printable...
		if (containsNonPrintable == false)
			printf("%s", buffer);

		// If they are not all printabltrlene...
		else
		{

			// Init Variables...
			memset(asciiOutput, 0, 17);
			counter = 0;
			charPointer = 0;

			// Print to screen...
			while (charPointer < dataSize)
			{
				printf ("%02x ", buffer[charPointer]);
				if (buffer[charPointer] < 32)
					asciiOutput[counter] = '.';
				else if ((buffer[charPointer] > 126) && (buffer[charPointer] < 160))
					asciiOutput[counter] = '.';
				else
					asciiOutput[counter] = buffer[charPointer];

				// Increment or loop...
				if (counter < 15)
					counter++;
				else
				{
					printf("   %s\n", asciiOutput);
					memset(asciiOutput, 0, 17);
					counter = 0;
				}
				charPointer++;
			}

			// If more to go...
			if (counter != 0)
			{
				while (counter < 17)
				{
					printf("   ");
					counter++;
				}
				printf("%s\n", asciiOutput);
			}
		}
	}
}


// Read data
int readData(int socketDescriptor, int timeout, int earlyTerminate, int hexOutput)
{
	// Variables...
	fd_set connectionRead;
	struct timeval timeoutStruct;
	unsigned char buffer[MAX_SIZE];
	int done;
	int resultSize;
	int totalRead;

	// Init...
	done = false;
	totalRead = 0;
	resultSize = 0;
	timeoutStruct.tv_usec = 0;
	timeoutStruct.tv_sec = timeout;
	if (earlyTerminate == true)
		earlyTerminate = 1;
	else
		earlyTerminate = MAX_LOADS;

	while ((done == false) && (earlyTerminate > 0))
	{
		earlyTerminate--;

		// Set recieve timeout
		FD_ZERO(&connectionRead);
		FD_SET(socketDescriptor, &connectionRead);
	
		// Timeout for socket...
		select(socketDescriptor +1, &connectionRead, 0, 0, &timeoutStruct);
		if (FD_ISSET(socketDescriptor, &connectionRead))
		{
			// Recieve Data
			memset(buffer ,0 , sizeof(buffer));
			resultSize = recv(socketDescriptor, buffer, sizeof(buffer) -1, 0);
			if (resultSize > 0)
			{
				printData(buffer, resultSize, hexOutput);
				memset(buffer, 0, sizeof(buffer));
			}
			else
			{
				resultSize = 0;
				done = true;
			}
		}
		else
			done = true;
		totalRead += resultSize;
	}

	return totalRead;
}


// Print a list of triggers...
void printTriggers()
{
	// Variables...
	struct serviceConfig *servicePointer = 0;
	int loop;

	// Start info...
	printf("%sThe available BannerGrab triggers are:%s\n", COL_BLUE, RESET);
	printf("  %s>%s DEFAULT    (Port: N/A)\n", COL_GREEN, RESET);

	// Loop through services...
	servicePointer = &service;
	while (servicePointer->next != 0)
	{
		// Service name
		printf("  %s>%s %s ", COL_GREEN, RESET, servicePointer->service);

		// Print extra space (if needed)...
		for (loop = strlen(servicePointer->service); loop < 10; loop++)
			printf(" ");

		// Print port no...
		if (servicePointer->tcp == true)
			printf("(TCP Port: %d)\n", servicePointer->port);
		else
			printf("(UDP Port: %d)\n", servicePointer->port);
		servicePointer = servicePointer->next;
	}

	// Print an extra newline...
	printf("\n");
}


int main(int argc, char *argv[])
{
	// Variables...
	// Program options...
	int port = 80;					// The port to test
	int tcp = true;					// TCP? (false == UDP)
	int host = 0;					// The host to test
	int mode = mode_grab;			// The program mode

	int timeout = 5;				// Connection timeout
	int readTimeout = 3;			// Read Timeout
	int trigger = 0;
	int verbose = false;
	int hexOutput = true;

	// Throwaway Variables...
	int loop;
	int tempInt;
	int earlyTerminate;
	unsigned char buffer[MAX_SIZE];

	// Handles, Pointers etc...
	int socketDescriptor = 0;
	struct serviceConfig *servicePointer = 0;
	struct triggerConfig *triggerPointer = 0;
	int readTotal = 0;

#if !defined(NOSSL)
	// SSL Variables...
	int trySSL = true;				// Try SSL?
	int cipherStatus = 0;
	int tempInt2 = 0;
	SSL *ssl = NULL;
	SSL_CTX *ctx;
	BIO *cipherConnectionBio = 0;
	BIO *stdoutBIO = NULL;
#endif

	// Get program parameters...
	if (argc < 2)
		mode = mode_help;
	else if (argc == 2)
	{
		// Show Triggers...
		if (strcmp("--show-triggers", argv[1]) == 0)
			mode = mode_showtriggers;

		// Version
		else if (strcmp("--version", argv[1]) == 0)
			mode = mode_version;

		else
			mode = mode_help;
	}
	else
	{
		for (loop = 1; loop < argc; loop++)
		{
			// Host...
			if (loop + 2 == argc)
				host = loop;

			// Port...
			else if (loop + 1 == argc)
				port = atoi(argv[loop]);

			// Verbose...
			else if (strcmp("--verbose", argv[loop]) == 0)
				verbose = true;

			// No Triggers...
			else if (strcmp("--no-triggers", argv[loop]) == 0)
				mode = mode_notrig;

			// UDP...
			else if (strcmp("--udp", argv[loop]) == 0)
				tcp = false;

			// Specify a trigger...
			else if (strncmp("--trigger=", argv[loop], 10) == 0)
				trigger = loop;

#if !defined(NOSSL)
			// No SSL...
			else if (strcmp("--no-ssl", argv[loop]) == 0)
				trySSL = false;
#endif

			// No HEX...
			else if (strcmp("--no-hex", argv[loop]) == 0)
				hexOutput = false;

			// Connection Timeout...
			else if (strncmp("--conn-time=", argv[loop], 12) == 0)
				timeout = atoi(argv[loop] + 12);

			// Read Timeout...
			else if (strncmp("--read-time=", argv[loop], 12) == 0)
				readTimeout = atoi(argv[loop] + 12);
			else if ((strcmp("-t", argv[loop]) == 0) && (loop + 3 < argc))
			{
				loop++;
				timeout = atoi(argv[loop]);
				readTimeout = atoi(argv[loop]);
			}

			// If all else fails...
			else
				mode = mode_help;
		}
	}

	// Do the stuff...
	switch (mode)
	{
		case mode_version:
			printf("%s%s%s", COL_BLUE, program_version, RESET);
			break;

		case mode_help:
			printf("%s%s%s", COL_BLUE, program_banner, RESET);
			printf("BannerGrab  performs  connection,  trigger  and  basic service\n");
			printf("information collection. There are basic banner grabbing modes,\n");
			printf("the  first  mode (the  default  one)  sends  triggers  to  the\n");
			printf("services and performs basic information collection. The second\n");
			printf("mode (--no-triggers), only connects to the service and returns\n");
			printf("the connection banner.\n\n");
			printf("%sCommand:%s\n", COL_BLUE, RESET);
			printf("  %s%s [Options] host port%s\n\n", COL_GREEN, argv[0], RESET);
			printf("%sOptions:%s\n", COL_BLUE, RESET);
			printf("  %s--udp%s                Connect  to  a  port  using  UDP.   The\n", COL_GREEN, RESET);
			printf("                       default is to use TCP.\n");
#if !defined(NOSSL)
			printf("  %s--no-triggers%s        Collect only the connection banner,  no\n", COL_GREEN, RESET);
			printf("                       triggers and no SSL.\n");
			printf("  %s--trigger=<trigger>%s  Specify  the  trigger  to use.  Specify\n", COL_GREEN, RESET);
			printf("                       DEFAULT to use the default trigger.\n");
			printf("  %s--no-ssl%s             Prevent SSL connection creation.\n", COL_GREEN, RESET);
#else
			printf("  %s--no-triggers%s        Collect only the connection banner,  no\n", COL_GREEN, RESET);
			printf("                       triggers.\n");
			printf("  %s--trigger=<trigger>%s  Specify  the  trigger  to use.  Specify\n", COL_GREEN, RESET);
			printf("                       DEFAULT to use the default trigger.\n");
#endif
			printf("  %s--no-hex%s             Output     containing     non-printable\n", COL_GREEN, RESET);
			printf("                       characters are  converted to  hex. This\n");
			printf("                       option prevents the conversion.\n");
			printf("  %s--conn-time=<secs>%s   Connection timeout (default is 5s).\n", COL_GREEN, RESET);
			printf("  %s--read-time=<secs>%s   Read timeout (default is 3s).\n", COL_GREEN, RESET);
			printf("  %s--verbose%s            Show additional program details such as\n", COL_GREEN, RESET);
			printf("                       any errors.\n");
			printf("  %s--show-triggers%s      Show the supported triggers.\n", COL_GREEN, RESET);
			printf("  %s--version%s            Show the program version.\n", COL_GREEN, RESET);
			printf("  %s--help%s               Display the  help text  you are reading\n", COL_GREEN, RESET);
			printf("                       now.\n\n");
			printf("%sExample:%s\n", COL_BLUE, RESET);
			printf("  %s%s 127.0.0.1 80%s\n\n", COL_GREEN, argv[0], RESET);
			break;


		// Show Triggers...
		case mode_showtriggers:
			printf("%s%s%s", COL_BLUE, program_banner, RESET);
			printTriggers();
			break;

		// No triggers, just grab the connection response...
		case mode_notrig:
			if (verbose == true)
				printf("%s%s%s", COL_BLUE, program_banner, RESET);

			// Capture the timeout...
			signal(14, timeoutHandler);

			// Connect to port...
			socketDescriptor = netConnect(argv[host], port, timeout, verbose, tcp);
			if (socketDescriptor != 0)
			{
				if (port == 19)
					earlyTerminate = true;
				else
					earlyTerminate = false;

				// Read the data
				readData(socketDescriptor, readTimeout, earlyTerminate, hexOutput);

				// Disconnect from host
				close(socketDescriptor);
			}
			break;


		// Send triggers and grab the banners...
		case mode_grab:
			if (verbose == true)
				printf("%s%s%s", COL_BLUE, program_banner, RESET);

			// Capture the timeout...
			signal(14, timeoutHandler);

			// Get a trigger to use with the port...
			// If no trigger is specified...
			if (trigger == 0)
			{
				servicePointer = &service;
				while ((servicePointer->next != 0) && (servicePointer->port != port))
					servicePointer = servicePointer->next;
				if (servicePointer->port != port)
					servicePointer = &defaultService;
			}
			else
			{
				// If default trigger is selected...
				if (strcasecmp("DEFAULT", argv[trigger] + 10) == 0)
					servicePointer = &defaultService;

				// Search for trigger...
				else
				{
					servicePointer = &service;
					while ((servicePointer->next != 0) && !((strcasecmp(servicePointer->service, argv[trigger] + 10) == 0) && (servicePointer->tcp == tcp)))
						servicePointer = servicePointer->next;
					if (!((strcasecmp(servicePointer->service, argv[trigger] + 10) == 0) && (servicePointer->tcp == tcp)))
					{
						printf("%sERROR: Could not find the specified trigger!%s\n\n", COL_RED, RESET);
						printTriggers();
						exit(0);
					}
				}
			}

			// Terminate early for chargen...
			if (port == 19)
				earlyTerminate = true;
			else
				earlyTerminate = false;

			// If a read timeout override is configured...
			if ((servicePointer->timeout != 0) && (readTimeout == 3))
				readTimeout = servicePointer->timeout;

			// If no SSL connection...
#if !defined(NOSSL)
			if (servicePointer->ssl == false)
#else
			if (1 == 1)
#endif
			{

				// Connect to port...
				socketDescriptor = netConnect(argv[host], port, timeout, verbose, tcp);
				if (socketDescriptor != 0)
				{

					// Read the connection banner data
					if (servicePointer->connectBanner == true)
						readTotal = readData(socketDescriptor, readTimeout, earlyTerminate, hexOutput);

					// Loop through all triggers...
					triggerPointer = servicePointer->trigger;
					while (triggerPointer != 0)
					{
						// Send trigger...
						if (triggerPointer->size == 0)
							tempInt = send(socketDescriptor, triggerPointer->trigger, strlen(triggerPointer->trigger), 0);
						else
							tempInt = send(socketDescriptor, triggerPointer->trigger, triggerPointer->size, 0);
						if (servicePointer->show == true)
							printf("%s", triggerPointer->trigger);

						// Read the data
						readTotal += readData(socketDescriptor, readTimeout, earlyTerminate, hexOutput);
						triggerPointer = triggerPointer->next;
					}

					// Disconnect from host
					close(socketDescriptor);
				}
			}

#if !defined(NOSSL)
			// If using the default connection and nothing has been returned,
			// try SSL. Or maybe SSL is supposed to be attempted straight
			// away
			if ((servicePointer->ssl == true) || ((readTotal == 0) && (trySSL == true) && (servicePointer == &defaultService)))
			{

				// Connect to port...
				socketDescriptor = netConnect(argv[host], port, timeout, verbose, tcp);
				if (socketDescriptor != 0)
				{

					// Setup Context Object...
					SSL_library_init();
					ctx = SSL_CTX_new(SSLv23_client_method());
					if (ctx != NULL)
					{

						// Create SSL object...
						ssl = SSL_new(ctx);
						if (ssl != NULL)
						{

							// Connect socket and BIO
							cipherConnectionBio = BIO_new_socket(socketDescriptor, BIO_NOCLOSE);
							if (cipherConnectionBio != NULL)
							{

								// Connect BIO
								SSL_set_bio(ssl, cipherConnectionBio, cipherConnectionBio);

								// Connect SSL over socket
								alarm(timeout);
								cipherStatus = SSL_connect(ssl);
								alarm(0);
								if (cipherStatus == 1)
								{

									// Stdout BIO
									stdoutBIO = BIO_new(BIO_s_file());
									BIO_set_fp(stdoutBIO, stdout, BIO_NOCLOSE);

									// Read the connection banner data
									memset(buffer ,0 , sizeof(buffer));
									if (servicePointer->connectBanner == true)
									{
										alarm(timeout);
										readTotal = SSL_read(ssl, buffer, sizeof(buffer -1));
										alarm(0);
										if (readTotal > 0)
											printData(buffer, readTotal, hexOutput);
									}

									// Loop through all triggers...
									triggerPointer = servicePointer->trigger;
									while (triggerPointer != 0)
									{

										// Send trigger...
										if (triggerPointer->size == 0)
											SSL_write(ssl, triggerPointer->trigger, strlen(triggerPointer->trigger));
										else
											SSL_write(ssl, triggerPointer->trigger, triggerPointer->size);
										if (servicePointer->show == true)
											printf("%s", triggerPointer->trigger);

										// Read the data
										readTotal = 1;
										while (readTotal != 0)
										{
											memset(buffer ,0 , sizeof(buffer));
											readTotal = SSL_read(ssl, buffer, sizeof(buffer) - 1);
											if (readTotal > 0)
												printData(buffer, readTotal, hexOutput);
										}

										triggerPointer = triggerPointer->next;
									}

									// SSL Service Information...
									printf("\n\n---- 8< -------------------------------------\n");
									printf("SSL SERVICE INFORMATION\n\n");

									// SSL Default Cipher...
									printf("Prefered Cipher:\n");
									printf("  Name: %s\n", SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)));
									printf("  Bits: %d\n", SSL_CIPHER_get_bits(SSL_get_current_cipher(ssl), &tempInt2));
									printf("  Protocol: %s\n\n", SSL_CIPHER_get_version(SSL_get_current_cipher(ssl)));

									// SSL Certificate...
									X509_print(stdoutBIO, SSL_get_peer_certificate(ssl));

									// Free BIO
									BIO_free(stdoutBIO);

									// Disconnect SSL over socket
									SSL_shutdown(ssl);
								}
							}

							// Free SSL object
							SSL_free(ssl);
						}

						// Free CTX Object
						SSL_CTX_free(ctx);
					}
					// Disconnect from host
					close(socketDescriptor);
				}
			}
#endif
			break;
	}
	return 0;
}

