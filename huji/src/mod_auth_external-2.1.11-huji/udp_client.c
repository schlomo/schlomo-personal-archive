/* UDP_CLIENT.C		V2.1
 | Query the WWW password server.
 | This module looks for the user and password, and if found returns also the
 | faculty and title of the user. If both are zero then it is a staff member
 | and not students.
 |
 | Entry point: verify_user(ID, password, &faculty, &title)
 | where:  ID, password - strings to check.
 |         &faculty, &ttile - pointers to integers.
 | returns: 0 - found, faculty and title has valid data.
 |          1 - Not found.
 */

/* Password server parameters */
#ifdef VMS
#include <time.h>
#include "multinet_root:[multinet.include.sys]types.h"
#include "multinet_root:[multinet.include.sys]socket.h"
#include "multinet_root:[multinet.include.netinet]in.h"
#include "multinet_root:[multinet.include.sys]time.h"
#include <stdio.h>
#include "multinet_root:[multinet.include]netdb.h"
#else
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#endif

#define	PASSWORD_SERVER_ADDRESS	"control.huji.ac.il"
#define	UDP_READ_TIMEOUT	10

static int	s = 0;	/* File descriptor to access the socket */
static struct	sockaddr_in	sin;	/* Place here the address to connect to */

#ifdef MAIN
main(cc, vv)
char	*vv[];
{
	char	username[128], password[128];
	int	faculty, title, status;


	strcpy(username, vv[1]);
	strcpy(password, vv[2]);

	status = verify_user(username, password, &faculty, &title);

#ifdef DEBUG
	printf("Status: %d\n", status);
	printf("Faculty=%d, title=%d\n", faculty, title);
#endif
	return status;
}
#endif	/* MAIN */


/*
 | Search for username and password, and if found return the title and faculty.
 | returns: 1 - Found.
 |          0 - Not found.
 */
verify_user(username, password, faculty, title)
char	*username, *password;
int	*faculty, *title;
{
	char	buffer[256];

	sprintf(buffer, "%s %s", username, password);
	bzero(password, 8);	/* We don't know the original buffer size... */

	if(open_udp_connection() == 0) return 0;
	if(write_udp_line(buffer) <= 0) return 0;
	bzero(buffer, sizeof(buffer) - 1);
	if(read_udp_line(buffer, sizeof(buffer)) <= 0) return 0;
	close_udp_connection();

/* check the status */
	sscanf(buffer, "%*s %d %d", faculty, title);
	bzero(&buffer[1], sizeof(buffer) - 2);	/* Erase any traces fori pasword
		but leave the first char as we need it in the next statment */
	if(*buffer == '1') {	/* Found */
		return(1);
	} else	return(0);
}

/*
 | Opens a TCP connection to the server (later we'll add support to more than
 | one server).
 | Return 1 on success, zero otherwise.
 */
int
open_udp_connection()
{
	struct hostent	*hp;	/* get the server's parameters here */

/* Translate the server's name to address */
	if((hp = gethostbyname(PASSWORD_SERVER_ADDRESS)) == NULL) {
#ifdef DEBUG
		perror("Gethostbybname");
#endif
		return 0;
	}

/* Get a local socket */
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
#ifdef DEBUG
		perror("Socket");
#endif
		return 0;	/* Signal error to caller */
	}

/* Insert the parameters of the remote host to connect to */
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
	sin.sin_port = htons(8887);

	return 1;	/* All ok */
}

/*
 | Write data to TCP channel. Return the number of characters written or 0
 | in case of error.
 */
int
write_udp_line(buffer)
char	*buffer;
{
	int	size;

	size = strlen(buffer);
#ifdef DEBUG
	printf("Sending to server: '%s'\n", buffer);
#endif
	if(sendto(s, buffer, strlen(buffer), 0, &sin, sizeof(sin)) != size) {	/* Some error in write */
#ifdef DEBUG
		perror("Write");
#endif
		return -1;
	}
	return size;	/* All OK */
}


/*
 | Read one line from TCP channel. In the future we'll add timeout.
 | Zero-delimit the line and return the number of characters read.
 */
int
read_udp_line(buffer, BufferSize)
char	*buffer;		/* Where to return the read data */
int	BufferSize;		/* Size of that buffer */
{
	int	size,		/* Size of data we read so far */
		status,		/* Status of read - actually how many chars it read */
	TableSize = getdtablesize();	/* For Select */
	struct timeval	TimeoutStruct;	/* For Select's timeout */
	fd_set	ReadFds;

/* Setup Select arguments and call select to see whether we have something to
   read.
*/
	TimeoutStruct.tv_sec = UDP_READ_TIMEOUT;	/* Set the timeout */
	TimeoutStruct.tv_usec = 0;
	FD_ZERO(&ReadFds);
	FD_SET(s, &ReadFds);

	status = select(TableSize, &ReadFds, NULL, NULL, &TimeoutStruct);
	if(status <= 0) {	/* Some error or timeout */
#ifdef DEBUG
		perror("Select");
#endif
		return -1;
	}

/* Select is ok - there is some input */
	size = sizeof(sin);
	if((status = recvfrom(s, buffer, BufferSize, 0, &sin, &size)) <= 0) {	/* Some error */
#ifdef DEBUG
		perror("Recvfrom");
#endif
		return -1;
	}
	buffer[status] = '\0';	/* Delimit the line */
#ifdef DEBUG
printf("Read: '%s'\n", buffer);
#endif
	return status;
}


/*
 | Close the TCP connection.
 */
int
close_udp_connection()
{
	close(s);
	s = 0;
}
