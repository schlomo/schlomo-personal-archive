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
 *              permission.
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


/* PWAUTH configuration file 
 *
 * Note - the default settings in this file are the way I use it.  I
 * guarantee you, they won't work for you.  You must change them.
 *
 */


/* There are lots of different kinds of password databases.  Define one of
 * the following:
 *
 *  - SHADOW_NONE: standard Unix systems which have the encrypted passwords
 *    in the passwd file.  Actually, since you don't need to be root to access
 *    these, you might do better using Lou Langholtz's mod_auth_system.c, which
 *    is available from the contributed modules directory at apache.org.
 *
 *  - SHADOW_BSD: This is the shadow password system in BSDI, NetBSD and
 *    FreeBSD .  It uses exactly the same calls as SHADOW_NONE, but getpwnam()
 *    only returns the encrypted password if you are root.  This has not been
 *    tested.
 *
 *  - SHADOW_SUN:  This is the shadow password system in Solaris and IRIX 5.3
 *    systems.  It uses getspnam() to fetch passwords and crypt() to  encrypt
 *    them.  At least some Linux shadow passwd systems, including Redhat 6.x,
 *    are compatible with this.
 *
 *  - SHADOW_JFH:  This the old version of Julianne F. Haugh's public-domain
 *    shadow package.  It uses getspnam() to fetch passwords and pw_encrpyt()
 *    to encrypt them.  The JFH shadow code is available at
 *        ftp://gumby.dsd.trw.com/pub/security/shadow-3.2.2.tar.Z
 *    Newer versions seem to be compatible with SHADOW_SUN.
 *
 *  - SHADOW_MDW:  This is Grex's variation on the JFH shadow file system,
 *    which uses Marcus D. Watt's interface to the password encryption.  If you
 *    ain't Grex, you ain't got it.  This has been tested through the roof.
 *
 *  - SHADOW_AIX:  This is the AIX shadow password system.  It uses getuserpw()
 *    to fetch passwords and (apparantly) crypt() to encrypt them.  This has
 *    not been tested.
 *
 *  - SHADOW_HPUX:  This is the HP-UX shadow password system.  It uses
 *    getprpwnam() to fetch passwords and either crypt() or bigcrypt() to
 *    encrypt them (I'm not sure which is right).  This has not been tested
 *    and probably doesn't work.
 *
 *  - PAM: Talk to the authentication system through PAM - the plug-in
 *    authentication module interface.  This exists on Solaris 7 and
 *    on some versions of Linux, including Redhat 6.x.  You'll need to create
 *    /etc/pam.d/pwauth or edit /etc/pam.config to include entries for
 *    pwauth.  If you are using PAM to authenticate out of something you don't
 *    need to be root to access, then you might use instead Ingo Lutkebohle's
 *    mod_auth_pam.c module.
 *
 *  - PAM_SOLARIS_26:  Solaris 2.6 PAM has some header file declarations and
 *    function behaviors that don't agree with either their own documentation
 *    or with any other implementation.  Bugs, in short.  This option uses
 *    PAM with work-arounds for the Solaris 2.6 bugs.
 */

/* #define SHADOW_NONE		/**/
/* #define SHADOW_BSD		/**/
#define SHADOW_SUN		/**/
/* #define SHADOW_JFH		/**/
/* #define SHADOW_MDW		/**/
/* #define SHADOW_AIX		/**/
/* #define PAM			/**/
/* #define PAM_SOLARIS_26	/**/


/* There is also support for one failure logging system (the database that
 * informs you that "there have been 3426 unsuccessful attempts to log into
 * your account since your last login" and which may disable accounts with
 * too many failed logins).
 *
 * Very few Unix systems seem to have faillog files installed, so most users
 * will not want this option.
 *
 * No faillog option should be used with PAM.  This kind of logging is handled
 * at a lower level with PAM.
 *
 *  - FAILLOG_JFH:  This is associated with the JFH shadow system.  Some Linux
 *    systems may have this, but most don't seem to.  Failures are logged in
 *    the /var/adm/faillog file, and if any user accumulates too many failed
 *    logins, future logins are denied.  The faillog.h header file is part of
 *    the JFH shadow file package.  Note that you can use this even if you
 *    don't use the JFH shadow package (though you'll need the faillog.h header
 *    file).  Just create the file.  You'll need some kind of tool like the
 *    faillog command in the JFH package to administer the file though.
 */

/* #define FAILLOG_JFH		/**/


/* If UNIX_LASTLOG is defined, the program will update the lastlog entry so
 * that there is a record of the user having logged in.  This is important on
 * systems where you expire unused accounts and some users may only log in
 * via the web.
 */

#define UNIX_LASTLOG		/**/

/* SERVER_UID is the uid number of the account that your httpd is configured
 * to run as by the "User" directive in your httpd.conf file.  This is the
 * "nobody" account on many systems, though it probably shouldn't be.
 */

#define SERVER_UID 99		/* user "nobody" */


/* If MIN_UNIX_UID is defined to an integer, logins with uid numbers less than
 * that value will be rejected, even if the password is correct.  If you are
 * using PAM to authenticate against anything other than the local unix
 * password file, then leave this undefined (if you define it, only login
 * names which are in the local unix passwd file and have uid's greater the
 * given value will be accepted).
 */

#define MIN_UNIX_UID 500	/**/


/* On failed authentications, pwauth will sleep for SLEEP_TIME seconds, using
 * a lock on the file whose full path is given by SLEEP_LOCK to prevent any
 * other instances of pwauth from running during that time.  This is meant
 * to slow down password guessing programs, but could be a performance
 * problem on extremely heavily used systems.  To disable this, don't define
 * SLEEP_LOCK.  SLEEP_TIME defaults to 2 seconds if not defined.
 */

#define SLEEP_LOCK "/var/run/pwauth.lock"

/* If ENV_METHOD is defined, pwauth expects mod_auth_external to be configured
 *     SetAuthExternalMethod environment
 * instead of
 *     SetAuthExternalMethod pipe
 * This is insecure on some versions of Unixes, but might be a bit faster.
 */

/* #define ENV_METHOD		/**/

/* If /usr/include/paths.h exists define this.  Obviously I need to autoconfig
 * this.
 */

#define PATHS_H 		/**/
