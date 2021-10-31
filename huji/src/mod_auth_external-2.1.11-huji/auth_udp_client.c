/* AUTH_UDP_CLIENT.C		V0.1
/ This is a wrapper for Yehavi's udp_client. It takes a string in addition
/ that can be eiterh staff, student or all and designates which type of user 
/ is OK.
*/
#include <stdio.h>
#include <string.h>
#include "udp_client.c"



int checkpasshuji(username,password,access)
char *username;
char *password;
char *access;
{

	int faculty, title, status, mintitle, maxtitle;


/* Set limits for title, default to all */
	if (strcmp(access,"student")==0) {
		mintitle=0;
		maxtitle=100;
	}
	else if (strcmp(access,"staff")==0) {
		mintitle=-1;
		maxtitle=1;
	}
	else {
		mintitle=-1;
		maxtitle=100;
	}

#ifdef DEBUG

	printf("Checking %s %s %s %d %d\n",username,password,access,mintitle,maxtitle);
	
#endif

	status = verify_user(username, password, &faculty, &title);

#ifdef DEBUG
	printf("Status: %d\n", status);
	printf("Faculty=%d, title=%d\n", faculty, title);
#endif

/* If not asked for all, then make sure that either students or staff pass */
	if (!(title>mintitle && title<maxtitle)) {
		status = 0;
	}
		
#ifdef DEBUG
	printf("Status regarding %s : %d\n",access, status);
#endif
		
			

	return status;
}

#ifdef MKMAIN
int main(cc, xx)
char	*xx[];
{
	return checkpasshuji(xx[1],xx[2],xx[3]);
}
#else
#endif	/* MAIN */
