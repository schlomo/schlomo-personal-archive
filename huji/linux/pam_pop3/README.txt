pam_pop3 version 10.09.2001 by Schlomo Schapiro (huji at schlomo dot schapiro dot org)

This module authenticates a user against a POP3 server. It supplies only the AUTH functions.

Recognized arguments:
debug		Print a lot of debugging info to syslog including the password
info		Print a single line for each try
use_first_pass	reuse an existing password and fail if there is no password
		(By default it will ask the user for a password if none is found)

hostname	The POP3 server to query (You HAVE to give this paramter !)
port		The POP3 port
pwprompt	Password prompt
timeout		Network Timeout (time in seconds to wait for a server response)
username	Username to use (mostly for pop3auth)
password	Password to use (        "          )


The last two are useful mostly for testing or maybe if all users use the same username.

Basically you can use this module to authenticate against everything that uses a simple USER
and PASS command structure by changing the *CMD and *STR defines at the top of the source. 
Please note that the program does not deal with multiline or multi-packet responses (e.g. the
response has to come in one recv() call.

I provide also a pop3auth program that allows you to check the functionality of the module without
installing it or as a stand-alone authentication program.

Installation:
-------------
Run make install in the source dir, it will create pam_pop3.so and copy it to /lib/security
Run make install-pop3auth in the source dir to make and install pop3auth into /usr/local/bin
Run make or make pop3auth to make the binaries without installing them. Note that they are not
stripped then.

Usage Examples:
---------------
put a line like this into a pam.d file:
auth    required        /lib/security/pam_pop3.so hostname=mail info pwprompt=Passwort: timeout=20

or call pop3auth like this
pop3auth hostname=mail username=YOURUSER password=YOURPASSWORD info timeout=20
Note: pop3auth cannot prompt for the password !
Note: Also pop3auth logs to the syslog under LOG_AUTH !

Version info is in the soure file.

I developed pam_pop3 on a standard Linux SuSE 7.2 i386 computer. If you have something different, 
you might (but probably not) have to change the includes. This is probably especially true
for non-Linux systems, though I didn't use anything Linux specific (AFAIK)
