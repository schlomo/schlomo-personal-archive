/* =======================================================================
 * Copyright 1996, Jan D. Wolter and Steven R. Weiss, All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names of the authors must not be used to endorse or
 *    promote products derived from this software without prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * =======================================================================
 */


/* EXTERNAL PASSWORD FILE AUTHENTICATION PROGRAM
 *
 * DESCRIPTION:
 *
 * This program reads a login name and a password in from standard input.
 * If they are valid it returns 0, otherwise it returns 1.
 *
 * INSTALLATION:
 *
 * This program is designed to be used with Apache to authenticate users out
 * of the password file.  To use it:
 *
 *  (1)  Compile Apache with the mod_auth_external.c version 2.1.1 or later.
 *
 *  (2)  Edit the config.h file this directory to set the configuration
 *       appropriate for your system.
 *
 *  (2)  Compile and install this program.  On systems with a shadow password
 *       file, it should be suid-root so it has the access needed to get the
 *       passwords.
 *
 *  (3)  Add to the Apache server configuration file a line that gives the
 *       full path to wherever you installed this program:
 *
 *	 AddExternalAuth pwauth /what/ever/path/pwauth
 *	 SetExternalAuthMethod pwauth pipe
 *
 *       It is possible to use this module with the "environment" method
 *       instead of the "pipe" method by compiling it with the ENV_METHOD
 *       flag defined, however this has security problems on some Unixes.
 *
 *  (4)  In the .htaccess file (or wherever you are setting up the
 *       authentication), replace the usual AuthUserFile stuff with:
 *
 *	 AuthExternal pwauth
 *
 * SECURITY CONSIDERATIONS:
 *
 * THE FUNDAMENTAL PROBLEM with this method of authenticating users is that
 * mod_auth_external uses what is called "Basic Authentication".  This is
 * still the widest used authentication method on the net, and the only method
 * supported by most browsers, but it is fundamentally flawed, in that it
 * involves sending your password in clear-text over the network with every
 * transaction you make.  Broadcasting your password through so many systems
 * so often is fundamentally risky.  If you aren't willing to take that
 * risk, don't do this, or use it only with https.  
 *
 * THE PWAUTH PROGRAM attempts to do several things to limit some of the
 * other problems with passwd-based authentication.
 *
 *  RECORD KEEPING:  On some unix systems, when you log in you will be told
 *  when you last logged in and if there have been failed login attempts since
 *  your last login.  pwauth has no way to report such information back to
 *  a user, but it does have the ability to update those databases (at least
 *  on some versions of unix), so that the next time you really log in, failed
 *  attempts to login through pwauth will be included in the record. 
 *  However, we don't support all types of logging for all unixes yet.
 *
 *  SPEED:  You don't want a program like this to be too fast about reporting
 *  failed logins, because if it is, attackers can zip through a dictionary
 *  of guesses quickly.  Traditionally, programs like su and login sleep a
 *  bit before giving you a chance to try another password after one has
 *  failed.  We need to be a bit tricker than that, because a single person
 *  could avoid the problem of a simple sleep just by doing lots of guesses
 *  in parallel, so all the processes are sleeping at once.  So we do a sleep,
 *  but we lock a file while we are doing it.  This means that after a
 *  failed login, there is a period of SLEEP_TIME (say 2) seconds when all
 *  authentications will be held up.  This serializes failed authentication
 *  requests.  Under normal usage, it shouldn't have any noticable effect,
 *  though you won't want to use it on systems that will get gigantic numbers
 *  of authentication requests.  In the event that someone tries barraging
 *  your system with password guesses, he will get responses back no faster
 *  than one every 2 seconds.  In the meantime, valid authentications could
 *  get very slow, and your system will accumulate very large numbers of
 *  processes blocked on the lock file.  Yuck.  But that's life.  So if you
 *  really expect to have problems with these kinds of attacks, use the
 *  maximum failures feature in the JFH_FAILLOG option instead.
 *
 *  DIRECT EXECUTION:  It is possible that users telnetted into your system
 *  might try to run this program directly.  It would tell them if the
 *  login/password combination they give is correct, but "login" and "su" do
 *  that too, so this isn't that big a deal.  Still, pwauth is probably easier
 *  to interface to than those are, so we do some things to discourage its
 *  use in such a way.  It checks to make sure that it is being run by either
 *  httpd or root, and won't work for anyone else.  You could also put the
 *  binary in a directory readable only by httpd.
 *
 *  NO CORE DUMPS:  On some version of unix, core dump files are publically
 *  readable.  It would be easy to extract passwords from such a file.  So
 *  pwauth disables core dumps.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>
#include "config.h"

/* Configuration is meant to be done in config.h */

#ifdef UNIX_LASTLOG
#define NEED_UID
#include <utmp.h>
#include <lastlog.h>
#ifndef UT_LINESIZE
#define UT_LINESIZE 8
#endif
#ifndef UT_HOSTSIZE
#define UT_HOSTSIZE 16
#endif
#if !defined(LASTLOG) && defined(PATHS_H)
#include <paths.h>
#ifdef _PATH_LASTLOG
#define LASTLOG _PATH_LASTLOG
#endif
#endif
#ifndef LASTLOG
#define LASTLOG "/usr/adm/lastlog"
#endif
#endif /*UNIX_LASTLONG*/


#ifdef FAILLOG_JFH
#include "faillog.h"
#ifndef NEED_UID
#define NEED_UID
#endif
#endif


#ifdef MIN_UNIX_UID
#if MIN_UNIX_UID <= 0
#undef MIN_UNIX_UID
#else
#ifndef NEED_UID
#define NEED_UID
#endif
#endif
#endif


#ifdef NEED_UID
int hisuid;
#endif

#ifdef SHADOW_BSD
/* BSDI shadow password system requires no special coding - good job */
#define SHADOW_NONE
#endif /* SHADOW_BSD */

#ifndef SLEEP_TIME
#define SLEEP_TIME 2
#endif

#ifndef BFSZ
#define BFSZ 1024
#endif

#ifdef PAM_SOLARIS_26
#define PAM
#endif /* PAM_SOLARIS_26 */

#ifdef PAM
#ifdef NEED_UID
#include <pwd.h>
#endif
#include <security/pam_appl.h>
#endif

#ifdef SHADOW_SUN
#ifdef NEED_UID
#include <pwd.h>
#endif
#include <shadow.h>
struct spwd *getspnam();
char *crypt();
#endif /* SHADOW_SUN */

#ifdef SHADOW_JFH
#ifdef NEED_UID
#include <pwd.h>
#endif
#include <shadow.h>	  /* this may be hidden in /usr/local/include */
struct spwd *getspnam();
char *pw_encrypt();
#endif /* SHADOW_JFH */

#ifdef SHADOW_MDW
#ifdef NEED_UID
#include <pwd.h>
#endif
#include <shadow.h>	  /* is -I/usr/local/include on gcc command? */
char *kg_pwhash(char *clear, char *user, char *result, int resultlen);
char *pw_encrypt();
#endif /* SHADOW_MDW */

#ifdef SHADOW_AIX
#ifdef NEED_UID
#include <sys/types.h>
#include <pwd.h>
#endif
#include <userpw.h>
#endif /* SHADOW_AIX */

#ifdef SHADOW_HPUX
#include <sys/types.h>
#include <hpsecurity.h>
#include <prot.h>
#endif /* SHADOW_HPUX */

#ifdef SHADOW_NONE
#include <unistd.h>
#include <pwd.h>
#endif


#ifdef PAM
/* ========================= PAM Authentication ======================== */
/*         Based on version by Karyl Stein (xenon313@arbornet.org)       */
/*            and on parts of mod_auth_pam.c by Ingo Lutkebohnle         */

/* Application data structure passed to PAM_conv: */

struct ad_user {
	char *login;
	char *passwd;
	};

/* The pam_unix.so library in Solaris 2.6 fails to pass along appdata_ptr
 * when it calls the conversation function.  So we use a global variable.  */

#ifdef PAM_SOLARIS_26
struct ad_user user_info;
#endif /* PAM_SOLARIS_26 */

/* PAM_CONF - PAM Conversation Function.  Called by PAM to get login/password.
 * Note that "appdata_ptr" is really a "struct ad_user *" structure.
 */

#ifdef PAM_SOLARIS_26
int PAM_conv (int num_msg, struct pam_message **msg,
     struct pam_response **resp, void *appdata_ptr)
{
struct ad_user *user= &user_info;
#else
int PAM_conv (int num_msg, const struct pam_message **msg,
     struct pam_response **resp, void *appdata_ptr)
{
struct ad_user *user= (struct ad_user *)appdata_ptr;
#endif /* PAM_SOLARIS_26 */
struct pam_response *response;
int i;

    /* Sanity checking */
    if (msg == NULL || resp == NULL || user == NULL)
    	return PAM_CONV_ERR;

    response= (struct pam_response *)
    	malloc(num_msg * sizeof(struct pam_response));

    for (i= 0; i < num_msg; i++)
    {
	response[i].resp_retcode= 0;
	response[i].resp= NULL;

	switch (msg[i]->msg_style)
	{
	case PAM_PROMPT_ECHO_ON:
	    /* Store the login as the response */
	    /* This likely never gets called, since login was on pam_start() */
	    response[i].resp= appdata_ptr ? (char *)strdup(user->login) : NULL;
	    break;

	case PAM_PROMPT_ECHO_OFF:
	    /* Store the password as the response */
	    response[i].resp= appdata_ptr ? (char *)strdup(user->passwd) : NULL;
	    break;

	case PAM_TEXT_INFO:
	case PAM_ERROR_MSG:
	    /* Shouldn't happen since we have PAM_SILENT set. If it happens
	     * anyway, ignore it. */
	    break;

	default:
	    /* Something strange... */
	    if (response != NULL) free(response);
	    return PAM_CONV_ERR;
	}
    }
    /* On success, return the response structure */
    *resp= response;
    return PAM_SUCCESS;
}


/* CHECK_AUTH - Return true if the given login has the given password.
 * (This version for systems using PAM.)
 */

int check_auth(char *login, char *passwd)
{
#ifndef PAM_SOLARIS_26
struct ad_user user_info= {login, passwd};
#endif /* PAM_SOLARIS_26 */
struct pam_conv conv= { PAM_conv, (void *)&user_info };
pam_handle_t *pamh= NULL;
int retval;

#ifdef NEED_UID
    struct passwd *pwd;

    if ((pwd= getpwnam(login)) == NULL) return(0);
    hisuid= pwd->pw_uid;
#endif
#ifdef MIN_UNIX_UID
    if (hisuid < MIN_UNIX_UID) return(0);
#endif

#ifdef PAM_SOLARIS_26
    user_info.login= login;
    user_info.passwd= passwd;
#endif /* PAM_SOLARIS_26 */

    retval= pam_start("pwauth", login, &conv, &pamh);

    if (retval == PAM_SUCCESS)
	retval= pam_authenticate(pamh, PAM_SILENT);

    if (retval == PAM_SUCCESS)
	retval= pam_acct_mgmt(pamh, 0);		/* permitted access? */

    if (pam_end(pamh,retval) != PAM_SUCCESS)	/* close PAM */
    {
	pamh= NULL;
	return(1);
    }

    return (retval == PAM_SUCCESS ? 1 : 0);	/* indicate success */
}
#endif  /* PAM */


#if defined(SHADOW_JFH) || defined(SHADOW_SUN)
/* ===================== JFH and SUN Authentication ===================== */


/* CHECK_AUTH - Return true if the given login has the given password.
 * (This version for systems with getspnam() call.)
 */

int check_auth(char *login, char *passwd)
{
char *cpass;
struct spwd *spwd= getspnam(login);
#ifdef NEED_UID
struct passwd *pwd;
#endif
	if (spwd == NULL) return(0);
#ifdef NEED_UID
	if ((pwd= getpwnam(login)) == NULL) return(0);
	hisuid= pwd->pw_uid;
#endif
#ifdef MIN_UNIX_UID
	if (hisuid < MIN_UNIX_UID) return(0);
#endif
#ifdef SHADOW_JFH
	cpass= pw_encrypt(passwd, spwd->sp_pwdp);
#else
	cpass= crypt(passwd, spwd->sp_pwdp);
#endif
	return(!strcmp(cpass, spwd->sp_pwdp));
}
#endif /* SHADOW_JFH || SHADOW_SUN */


#ifdef SHADOW_MDW
/* ===================== MDW Authentication ===================== */


/* CHECK_AUTH - Return true if the given login has the given password.
 * (This version for systems with kg_pwhash() call.)
 */

int check_auth(char *login, char *passwd)
{
char *cpass;
char bf[40];
struct spwd *spwd= getspnam(login);
#ifdef NEED_UID
struct passwd *pwd;
#endif
	if (spwd == NULL) return(0);
#ifdef NEED_UID
	if ((pwd= getpwnam(login)) == NULL) return(0);
	hisuid= pwd->pw_uid;
#endif
#ifdef MIN_UNIX_UID
	if (hisuid < MIN_UNIX_UID) return(0);
#endif
	if (spwd->sp_pwdp[0] != '%')
		cpass= pw_encrypt(passwd, spwd->sp_pwdp);
	else if ((cpass= kg_pwhash(passwd, login, bf, 40)) == NULL)
		return(0);

	return(!strcmp(cpass, spwd->sp_pwdp));
}
#endif /* SHADOW_MDW */


#ifdef SHADOW_NONE
/* ===================== NONE Authentication ===================== */


/* CHECK_AUTH - Return true if the given login has the given password.
 * (This version for systems with only a getpwnam() call.)
 */

int check_auth(char *login, char *passwd)
{
char *cpass;
struct passwd *pwd= getpwnam(login);
	if (pwd == NULL) return(0);
#ifdef NEED_UID
	hisuid= pwd->pw_uid;
#endif
#ifdef MIN_UNIX_UID
	if (hisuid < MIN_UNIX_UID) return(0);
#endif
	cpass= crypt(passwd, pwd->pw_passwd);
	return(!strcmp(cpass, pwd->pw_passwd));
}
#endif /* SHADOW_NONE */


#ifdef SHADOW_AIX
/* ===================== AIX Authentication ===================== */


/* CHECK_AUTH - Return true if the given login has the given password.
 * (This version for systems with a getuserpw() call.)
 */

int check_auth(char *login, char *passwd)
{
char *cpass;
struct userpw *upwd= getuserpw(login);
#ifdef NEED_UID
struct passwd *pwd;
#endif
	if (upwd == NULL) return(0);
#ifdef NEED_UID
	if ((pwd= getpwnam(login)) == NULL) return(0);
	hisuid= pwd->pw_uid;
#endif
#ifdef MIN_UNIX_UID
	if (hisuid < MIN_UNIX_UID) return(0);
#endif
	cpass= crypt(passwd, upwd->upw_passwd);
	return(!strcmp(cpass, upwd->upw_passwd));
}
#endif /* SHADOW_AIX */

#ifdef SHADOW_HPUX
/* ===================== HPUX Authentication ===================== */


/* CHECK_AUTH - Return true if the given login has the given password.
 * (This version for systems with a getprpwnam() call.)
 */

int check_auth(char *login, char *passwd)
{
char *cpass;
struct pr_passwd *pwd= getprpwnam(login);
	if (pwd == NULL) return(0);
#ifdef NEED_UID
	hisuid= pwd->ufld.fd_uid;
#endif
#ifdef MIN_UNIX_UID
	if (hisuid < MIN_UNIX_UID) return(0);
#endif
	/* Should this be a call to bigcrypt() instead? */
	cpass= crypt(passwd, pwd->ufld.fd_encrypt);
	return(!strcmp(cpass, pwd->ufld.fd_encrypt));
}
#endif /* SHADOW_HPUX */


/* LASTLOG - update the system's lastlog */

#ifdef UNIX_LASTLOG
void lastlog()
{
struct lastlog ll;
int fd;
char *hostname= getenv("HOST");
char *hostaddr= getenv("IP");

	ll.ll_time= time(0L);
	strncpy(ll.ll_line,"http",UT_LINESIZE);

	if (hostname != NULL && strlen(hostname) <= UT_HOSTSIZE)
		strncpy(ll.ll_host,hostname,UT_HOSTSIZE);
	else if (hostaddr != NULL)
		strncpy(ll.ll_host,hostaddr,UT_HOSTSIZE);
	else
		ll.ll_host[0]= '\0';

	if ((fd= open(LASTLOG,O_WRONLY)) < 0) return;

	lseek(fd, (long)(hisuid * sizeof(struct lastlog)), 0);
	write(fd, &ll, sizeof(struct lastlog));
	close(fd);
}
#endif /*UNIX_LASTLOG*/


/* CHECK_FAILS - Check if the account is disable due to the maximum number of
 * failed logins being exceeded.  Returns true if the account is OK to log
 * into or if the faillog information doesn't exist.  Unlike the login
 * program, we don't reset the failed count upon successful login, because
 * we have no way to report failure counts.
 */

check_fails()
{
int result= 1;
#ifdef FAILLOG_JFH
struct faillog flog;
int flfd

    if ((flfd= open(FAILFILE,O_RDONLY)) > 0)
    {
	lseek(flfd, hisuid * sizeof(struct faillog), 0);
	result= (read(flfd, &flog, sizeof(struct faillog))
	         != sizeof(struct faillog)) || 
	        flog.fail_max == 0 || flog.fail_cur < flog.fail_max;
    }
    close(flfd);
#endif
    return(result);
}


/* LOG_FAILURE - Do whatever we need to do to log a failed login attempt.
 */

log_failure()
{
#ifdef FAILLOG_JFH
int flfd;
struct faillog flog;

    /* Log the failure in /var/adm/faillog - JFH style */
    if ((flfd= open(FAILFILE,O_RDWR)) > 0)
    {
	/* Read the user's record (indexed by uid) */
	lseek(flfd, hisuid * sizeof(struct faillog), 0);
	if (read(flfd,&flog,sizeof(struct faillog)) != sizeof(struct faillog))
	{
	    faillog.fail_cnt= 0;
	    faillog.fail_max= 0;
	    faillog.fail_time= 0L;
	    faillog.failline[0]= '\0';
	}

	/* Increment the count (checking for overflow) */
	if (flog.fail_cnt + 1 > 0)
		flog.fail_cnt++;
	faillog.fail_time= time(0L);
	strcpy(faillog.fail_line,"http");

	/* Write it back out */
	lseek(flfd, hisuid * sizeof(struct faillog), 0);
	write(flfd, &flog, sizeof(struct faillog));
	close(flfd);
    }
#endif /* FAILLOG_JFH */
}


/* SNOOZE - Do a serialized sleep of the given number of seconds.  This means,
 * wait till no other pwauth processes are in their sleeps, and then sleep
 * for the number of seconds given.  Note that a snooze(0) may lead to some
 * sleep time, if other pwauth processes are in sleeps.
 */

snooze(int seconds)
{
#ifdef SLEEP_LOCK
int slfd;
#endif

	/* Lock the sleep-lock file to serialize our sleeps */
#ifdef SLEEP_LOCK
	if ((slfd= open(SLEEP_LOCK,O_CREAT|O_RDONLY,0644)) >= 0)
		flock(slfd,LOCK_EX);
#endif

	sleep(seconds);

	/* Release sleep-lock file */
#ifdef SLEEP_LOCK
	/*flock(slfd,LOCK_UN);*/
	close(slfd);
#endif
}


main(int argc, char **argv)
{
#ifdef ENV_METHOD
char *login, *passwd;
#else
char login[BFSZ+1], passwd[BFSZ+1];
char *c, *strchr();
#endif
int uid;
int passwd_ok;
struct rlimit rlim;

	/* Don't dump core (could contain part of shadow file) */
	rlim.rlim_cur = rlim.rlim_max = 0;
	(void)setrlimit(RLIMIT_CORE, &rlim);

	/* Check that we were invoked by httpd */
	uid= getuid();
	if (uid != SERVER_UID && uid != 0)
		exit(2);

	/* Get the arguments (login and password) */
#ifdef ENV_METHOD
	if ((login= getenv("USER")) == NULL ||
	    (passwd= getenv("PASS")) == NULL)
		exit(3);
#else
	if (fgets(login, BFSZ, stdin) == NULL ||
	    fgets(passwd, BFSZ, stdin) == NULL)
		exit(3);

	if ((c= strchr(login,'\n')) != NULL) *c= '\0';
	if ((c= strchr(passwd,'\n')) != NULL) *c= '\0';
#endif

	/* Check validity of login/passwd */
	passwd_ok= check_auth(login,passwd);
	bzero(passwd,strlen(passwd));	/* Erase plain-text from our memory */
	if (passwd_ok && check_fails())
	{
		/* Good login */
#ifdef UNIX_LASTLOG
		lastlog();
#endif
		snooze(0);
		exit(0);
	}
	else
	{
		/* Bad login */
		snooze(SLEEP_TIME);
		log_failure();
		exit(1);
	}
}
