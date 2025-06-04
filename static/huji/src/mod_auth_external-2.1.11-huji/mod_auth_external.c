/* ====================================================================
 * Copyright (c) 1995 The Apache Group.  All rights reserved.
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
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * IT'S CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */

/********

Mod_Auth_External version 2.1.11

Add to server configuration file (normally httpd.conf):
-------------------------------------------------------

	AddExternalAuth <keyword> <path-to-authenticator>
	AddExternalGroup <keyword> <path-to-group-checker>

	SetExternalAuthMethod <keyword> <method>
	SetExternalGroupMethod <keyword> <method>

 The AddExternalAuth and AddExternalGroup directives specify the full pathnames
 of the external programs to be used to check user and group authentication
 for the given external authentication keywords.

 The SetExternalAuthMethod and SetExternalGroupMethod directives specify how
 the data is to be passed to the external authentication programs. There are
 currently three possible methods:

	environment     use environment variables. (default)
	pipe            write newline-terminated strings to stdin.
	checkpassword   write null-terminated strings to filedescriptor 3.
	function        call an internal function instead of external program.

 For "environment", "pipe" and "checkpassword", the option specifies a 
 program to run.  We do not start a shell to run the command, so full path
 must be given and no special shell characters (wildcards, I/O redirection,
 etc.) can be used.  If you need these, write an sh-script wrapper for
 your authentication program.

 Checkpassword is a standard protocol sometimes used for mail systems.  See
 http://cr.yp.to/checkpwd.html .  Various different authenticators using this
 protocol are available on the net.  The checkpassword protocol does not
 support group checking, but we do it anyway.

 For the "function" method to work, you must #define _HARDCODE_ and code your
 function into this module.  The <path-to-authenticator> argument is treated
 as a control argument to that function, normally of the form:
        AddExternalAuth <keyword> <type>:<config file>

Usage in auth config files:
---------------------------

	AuthExternal <keyword>
	AuthExternal afs
	GroupExternal <keyword>
	GroupExternal unix

	AuthExternalAuthoritative on|off
	AuthExternalGroupsAtOnce on|off

 Obviously, the keywords given must match those in the config files.

 The AuthExternalAuthoritative controls whether or not there is fall-through
 to other authentication methods.  If it is off, and the user's login and
 password are not accepted by the external authentication program, then that
 same login and password can be checked by other authentication methods
 (e.g., basic authentication with htpasswd files or dbm files).  If it is
 "on" there is no fall through.  It defaults to "on".  This is a change
 from earlier versions of mod_auth_external which were never authoritative.

 Originally when an external group authenticator was used, and more than one
 allowed group was listed, then the external authenticator would be run once
 to check each group.  Now all groups are sent as a list.   The old behavior
 can be restored by setting "AuthExternalGroupsAtOnce off".  It defaults to
 "on".

 If mod_auth_external is the only authentication method configured in a
 directory then it doesn't much matter if it is authoritative or not.  However,
 if you are using the GroupExternal and "require group ..." directives, then
 you should do "AuthExternalAuthoritative on".  Otherwise access failures get
 a server error page instead of an authentication denied page because there
 is no authoritative group authenticator.

Implementation of external authentication programs:
---------------------------------------------------

 How the external authentication program gets its arguments depends on
 the method used.  In the "environment" method, the arguments are passed
 in environment variables.  AuthExternals are passed the user id and the clear-
 text password in the USER and PASS environment variables.  GroupExternals are
 passed the userid and group name in the USER and GROUP environment variables.

 In the "pipe" method, the external authentication program reads its inputs
 from standard input.  AuthExternals will find the user id on the first line
 and the clear-text password on the second.  GroupExternals will find the
 user id on the first line and the group name on the second.

 In all cases, the external authentication should return 0 if the given
 password is correct for the given user (or if the given user is in the
 given group), and a non-zero error code if not.

 Whichever method is used, the environment variable IP will be set to the
 client's ip-address, and the environment variable HOST will be set to the
 client's hostname.  These may be useful for logging.  Note that if you have
 configured "HostnameLookups Off" then HOST will usually not be set.  If
 you really want hostnames, either turn on HostnameLookups or do your own
 gethostbyaddr() calls from the authenticator when HOST is not defined.

 The external authenticator will also be passed a copy of the httpd's
 PATH environment variable.  However, this is just whatever PATH was in
 effect when it was launched, and may differ if the server was launched
 automatically during a reboot or manually by an admin.  Probably your
 program should set its own PATH if it needs one.

 If the authenticator is a script rather than a compiled program, it normally
 has to start with a "#!/bin/sh" or "#!/usr/bin/perl" type directive.  Scripts
 without such directives may get interpreted by the shell, or may just not
 work, depending on your installation.

Security considerations:
------------------------

 The "environment" method of passing arguments is insecure on many versions
 of Unix.  Some versions of Unix (including SunOS and IRIX) allow any user
 to run "ps -e" commands to inspect the environment variables of any running
 process.  This would allow any user of the system to see logins and clear-
 text passwords in the USER and PASS variables of authenticator processes.
 Some versions of Unix (including Linux) only allow users to see environment
 variables on their own processes, but even this could be a problem if
 untrusted users can put CGI programs on your server, since those CGI programs
 may run as the same user as the authenticator processes.  The "pipe" method
 should be used to avoid these problems.

 The authenticator program should be written with great care.  It should not
 make any assumptions about the maximum length of logins and passwords.
 It should take steps to avoid dumping core (on some systems core files are
 publically readable, and may contain part of your password database).  Etc.
 You get points for paranoia.

Performance considerations:
---------------------------

 Using an external authenticator is always slower than hardcoding the
 authenticator into the module.

Comments/questions/etc. to janc@cyberspace.org

********/


/* Uncomment if you want to use a HARDCODE'd check (default off) */
#define _HARDCODE_ 

#ifdef _HARDCODE_
  /* Uncomment if you want to use your own Hardcode (default off) */
  /*             MUST HAVE _HARDCODE_ defined above!                */
#include "auth_udp_client.c"
#endif 


/*
 * Original External authentication module by nneul@umr.edu
 * Versions through 2.0.1 by <nneul@umr.edu & allison@nas.nasa.gov>
 * Versions after 2.1.0 by janc@cyberspace.org
 */


#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include <sys/wait.h>
#include <signal.h>

#if !defined(APACHE_RELEASE) || (APACHE_RELEASE < 1030100)
#error This module requires Apache 1.3.1 or later.
#endif

/* Names of environment variables used to pass data to authenticator */
#define ENV_USER	"USER"
#define ENV_PASS	"PASS"
#define ENV_GROUP	"GROUP"
#define ENV_URI		"URI"
#define ENV_HOST	"HOST"
#define ENV_IP		"IP"
/* Define this if you want cookies passed to the script */
#define ENV_COOKIE	"COOKIE"

/* Maximum number of arguments passed to an authenticator */
#define MAX_ARG 32

/*
 * Structure for the module itself
 */

module external_auth_module;

/*
 *  Data types for per-dir and server configuration
 */
typedef struct
{
    char *auth_extname;
    char *group_extname;
    int  external_authoritative;
    int  external_groupsatonce;
} extauth_dir_config_rec;

typedef struct 
{
    table *auth_extpath;
    table *auth_extmethod;

    table *group_extpath;
    table *group_extmethod;
} extauth_server_config_rec;


/*
 * Creators for per-dir and server configurations
 */

static void *create_extauth_dir_config(pool *p, char *d)
{
    extauth_dir_config_rec *r= ap_pcalloc(p, sizeof(extauth_dir_config_rec));

    r->external_authoritative= 1;	/* strong by default */
    r->external_groupsatonce= 1;	/* default to on */
    return r;
}


static void *create_extauth_server_config( pool *p, server_rec *s)
{
    extauth_server_config_rec *scr;

    scr= (extauth_server_config_rec *) ap_palloc(p,
    	sizeof(extauth_server_config_rec) );

    scr->auth_extmethod= ap_make_table(p, 4);
    scr->auth_extpath= ap_make_table(p, 4);
    scr->group_extmethod= ap_make_table(p, 4);
    scr->group_extpath= ap_make_table(p, 4);

    return (void *)scr;
}


/*
 * Handler for a AddExternalAuth server config line - add a external auth
 * type to the server configuration
 */

static const char *add_extauth(cmd_parms *cmd, void *dummy, char *keyword,
				char *path)
{
    extauth_server_config_rec *sc_rec;

    sc_rec= ap_get_module_config( cmd->server->module_config,
	&external_auth_module);

    ap_table_set( sc_rec->auth_extpath, keyword, path );

    ap_table_set( sc_rec->auth_extmethod, keyword, "environment" );

    return NULL;
}

/*
 * Handler for a AddExternalGroup server config line - add a external group
 * type to the server configuration
 */

static const char *add_extgroup(cmd_parms *cmd, void *dummy, char *keyword,
				char *path)
{
    extauth_server_config_rec *sc_rec;

    sc_rec= ap_get_module_config( cmd->server->module_config,
	&external_auth_module);

    ap_table_set( sc_rec->group_extpath, keyword, path );

    ap_table_set( sc_rec->group_extmethod, keyword, "environment" );

    return NULL;
}

/*
 * Handler for a SetExternalAuthMethod server config line - change an external
 * auth method in the server configuration
 */

static const char *set_extauth_method(cmd_parms *cmd, void *dummy,
					char *keyword, char *method)
{
	extauth_server_config_rec *sc_rec;

	sc_rec= ap_get_module_config( cmd->server->module_config,
	    &external_auth_module);

	ap_table_set( sc_rec->auth_extmethod, keyword, method );
	
	return NULL;
}

/*
 * Handler for a SetExternalGroupMethod server config line - change an external
 * group method in the server configuration
 */

static const char *set_extgroup_method(cmd_parms *cmd, void *dummy,
					char *keyword, char *method)
{
	extauth_server_config_rec *sc_rec;

	sc_rec= ap_get_module_config( cmd->server->module_config,
	    &external_auth_module);

	ap_table_set( sc_rec->group_extmethod, keyword, method );
	
	return NULL;
}


/*
 * Commands that this module can handle
 */

command_rec extauth_cmds[]= {

	{ "AuthExternal", ap_set_string_slot, 
	(void*)XtOffsetOf(extauth_dir_config_rec,auth_extname),
	OR_AUTHCFG, TAKE1, "a keyword indicating which authenticator to use" },

	{ "AddExternalAuth", add_extauth, NULL, RSRC_CONF, TAKE2,
	"a keyword followed by a path to the authenticator program" },

        { "SetExternalAuthMethod", set_extauth_method, NULL, RSRC_CONF, TAKE2,
        "a keyword followed by the method by which the data is passed" },

	{ "GroupExternal", ap_set_string_slot, 
	(void*)XtOffsetOf(extauth_dir_config_rec,group_extname),
	OR_AUTHCFG, TAKE1, "a keyword indicating which group checker to use" },

	{ "AddExternalGroup", add_extgroup, NULL, RSRC_CONF, TAKE2,
	"a keyword followed by a path to the group check program" },

        { "SetExternalGroupMethod", set_extgroup_method, NULL, RSRC_CONF, TAKE2,
        "a keyword followed by the method by which the data is passed" },

        { "AuthExternalAuthoritative", ap_set_flag_slot,
          (void*)XtOffsetOf(extauth_dir_config_rec, external_authoritative),
          OR_AUTHCFG, FLAG,
          "Set to 'off' to allow access control to be passed along to lower "
          "modules if the UserID is not known to this module" },

        { "AuthExternalGroupsAtOnce", ap_set_flag_slot,
          (void*)XtOffsetOf(extauth_dir_config_rec, external_groupsatonce),
          OR_AUTHCFG, FLAG,
          "Set to 'off' if group authenticator cannot handle multiple group "
	  "names in one invocation" },

	{ NULL }
};


/* Lookup a header in the request - this routine lifted from mod_rewrite.c
 */

#if defined(ENV_COOKIE)
static char *lookup_header(const request_rec *r, const char *name)
{
    array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    hdrs_arr= ap_table_elts(r->headers_in);
    hdrs= (table_entry *)hdrs_arr->elts;
    for (i= 0; i < hdrs_arr->nelts; ++i)
    {
	if (hdrs[i].key != NULL && strcasecmp(hdrs[i].key, name) == 0)
	    return hdrs[i].val;
    }
    return NULL;
}
#endif


/* Run an external authentication program using either the "pipe",
 * "checkpasswd" or "environment" method to pass the arguments.
 */

int exec_external(const char *extpath, const char *extmethod,
		const request_rec *r, const char *dataname, const char *data)
{
    int pipe_to_auth[2];
    int pid, status;
    conn_rec *c= r->connection;
    const char *remote_host;
    int usecheck= extmethod && !strcasecmp(extmethod, "checkpassword");
    int usepipe= usecheck || (extmethod && !strcasecmp(extmethod, "pipe"));

    if (usepipe && pipe(pipe_to_auth))

	/* pipe() failed - weird */
	return -3;

    if ( (pid= fork()) < 0 )
    {
	/* fork() failed - weird */
	if (usepipe)
	{
	    close(pipe_to_auth[0]);
	    close(pipe_to_auth[1]);
	}
	return -4;
    }
    else if (pid == 0)
    {
	/* We are the child process */

	char *child_env[10];
	char *child_arg[MAX_ARG+2];
	const char *t;
	char *cookie;
	int i= 0;

	/* Put PATH, hostname, ip address, uri, etc into environment */
	child_env[i++]= ap_pstrcat(r->pool, "PATH=", getenv("PATH"), NULL);

	remote_host= ap_get_remote_host(c, r->per_dir_config, REMOTE_HOST);
	if (remote_host != NULL)
	    child_env[i++]= ap_pstrcat(r->pool, ENV_HOST"=", remote_host, NULL);

	if (c->remote_ip)
	    child_env[i++]= ap_pstrcat(r->pool, ENV_IP"=", c->remote_ip, NULL);

	if (r->uri)
	    child_env[i++]= ap_pstrcat(r->pool, ENV_URI"=", r->uri, NULL);

#ifdef ENV_COOKIE
	if ((cookie= lookup_header(r,"Cookie")) != NULL)
	    child_env[i++]= ap_pstrcat(r->pool, ENV_COOKIE"=", cookie, NULL);
#endif

	/* Direct stdout and stderr to log file */
	ap_error_log2stderr(r->server);
	dup2(2,1);

	/* Close any open file descriptors and such in the pool. This won't 
	 * close the pipe because it isn't in the pool.  It will close
	 * anything that might be on fd 3, which is important to do before
	 * we try to attach the pipe to that in the checkpassword case.  */
	ap_cleanup_for_exec();

	if (usepipe)
	{
	    /* Connect stdin to pipe */
	    dup2(pipe_to_auth[0], usecheck ? 3 : 0);
	    close(pipe_to_auth[0]);
	    close(pipe_to_auth[1]);
	}
	else
	{
	    /* Put user name and password/group into environment */
	    child_env[i++]= ap_pstrcat(r->pool, ENV_USER"=", c->user, NULL);
	    child_env[i++]= ap_pstrcat(r->pool, dataname, "=", data, NULL);
	}

	/* End of environment */
	child_env[i]= NULL;

	/* Construct argument array */
	for (t= extpath, i=0; t[0] && (i <= MAX_ARG + 1);
	     child_arg[i++]= ap_getword_white(r->pool, &t)) {}
	child_arg[i]= NULL;

	/* Overwrite ourselves with the authenticator program */
	execve(child_arg[0], child_arg, child_env);

	/* If execve failed: */ exit(-1);
    }
    else
    {
	/* We are the parent process */

	if (usepipe)
	{
	    close(pipe_to_auth[0]);
	  
	    /* Send the user */
	    write(pipe_to_auth[1], c->user, strlen(c->user));
	    write(pipe_to_auth[1], usecheck ? "\0" : "\n", 1);
	  
	    /* Send the password */
	    write(pipe_to_auth[1], data, strlen(data));
	    write(pipe_to_auth[1], usecheck ? "\0" : "\n", 1);

	    /* Send dummy timestamp for checkpassword */
	    if (usecheck) write(pipe_to_auth[1], "0", 2);

	    /* Close output */
	    close(pipe_to_auth[1]);
	}
      
	/* Await a response */
	waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -2;
    }
}


/*
 * Authenticate a user
 */

static int extauth_basic_user(request_rec *r)
{
    extauth_dir_config_rec *dir_config_rec=
	(extauth_dir_config_rec *)ap_get_module_config(r->per_dir_config,
	&external_auth_module);

    extauth_server_config_rec *server_config_rec=
	(extauth_server_config_rec *)ap_get_module_config(r->server->module_config,
	&external_auth_module);

    const char *sent_pw;
    int res, code= 1;
    const char *extname;
    const char *extpath, *extmethod;

    conn_rec *c= r->connection;

    /* Get the password, exit if can't get */
    if ((res= ap_get_basic_auth_pw(r, &sent_pw)))
	    return res;

    /* Extract which external was chosen */
    extname= dir_config_rec->auth_extname;

    /* Check if we are supposed to handle this authentication */
    if ( !extname )
	return DECLINED;

    /* Get the path associated with that external */
    if (!(extpath= ap_table_get(server_config_rec->auth_extpath, extname)))
    {
	errno= 0;
	ap_log_reason(ap_psprintf(r->pool, "invalid AuthExternal keyword (%s)",
				  extname), r->filename, r);
	ap_note_basic_auth_failure(r);
	return AUTH_REQUIRED;
    }

    /* Get the Method requested */
    extmethod= ap_table_get(server_config_rec->auth_extmethod, extname);

    if ( extmethod && !strcasecmp(extmethod, "function") )
#ifdef _HARDCODE_ 
    {
	char *check_type;		/* Pointer to HARDCODE type check  */
	char *config_file;		/* Pointer to HARDCODE config file */
	int standard_auth= 0;

	/* Parse a copy of extpath into type and filename */
	check_type= ap_pstrdup(r->pool, extpath);
	config_file= strchr(check_type, ':');
	
	if (config_file == NULL) {        /* No colon! httpd.conf is wrong */
	    errno= 0;
	    ap_log_reason(ap_psprintf(r->pool,
	    			      "Problem parsing config file: "
	    			      "(%s) directive. You must have a ':' "
	    			      "seperator!\n", check_type),
			  r->filename, r);
	    return SERVER_ERROR;
	}
	*config_file= '\0';		    /* Mark end of type */
	config_file++;                      /* Start of filename */

	/* This is where you make your function call.       */
	/* Here is an example of what one looks like.       */
	/*                                                  */
	/* if (strcmp(check_type,"RADIUS")==0)              */
	/*   code= radcheck(c->user,sent_pw,config_file);  */
	/*                                                  */
	if (strcmp(check_type,"HUJI")==0)
		code= ! checkpasshuji(c->user,sent_pw,config_file);
	/* Replace 'RADIUS' with whatever you are using as  */
	/* the <type> in:                                   */
	/* AddExternalAuth <keyword> <type>:<config file>   */
	/*                                                  */
	/* Replace radcheck with whatever the name of your  */
	/* function is.                                     */
	/* Note: If you dont use a config_file you must at  */
	/* least include the ':'..for example..at a minimum */
	/* you must have 'RADIUS:' in the httpd.conf for your */
	/* function to be called.                           */

	else
	{
	    errno= 0;
	    ap_log_reason(ap_psprintf(r->pool,"Problem parsing config file: "
	    		              "(%s) directive (%s) config_file\n",
	    		              check_type, config_file),
			  r->filename, r);
	    return SERVER_ERROR;
	}
    }
#else
	  /* If _HARDCODE_ is not defined than no mater what the user says */
	  /* about wanting to use a function we fail!!                     */
	  code= -4;
#endif
    else
    	code= exec_external(extpath, extmethod, r, ENV_PASS, sent_pw);

    if (!code) return OK;
    
    errno= 0;

    if (!dir_config_rec->external_authoritative)
    	return DECLINED;

    ap_log_reason(ap_psprintf(r->pool,"AuthExtern %s [%s]: Failed (%d) for "
    			      "user %s", extname, extpath, code, c->user),
		  r->filename, r);
    ap_note_basic_auth_failure(r);
    return AUTH_REQUIRED;
}


static int extauth_check_auth(request_rec *r) 
{
    extauth_dir_config_rec *dir_config_rec=
	(extauth_dir_config_rec *)ap_get_module_config(r->per_dir_config,
	&external_auth_module);

    extauth_server_config_rec *server_config_rec=
	(extauth_server_config_rec *)ap_get_module_config(r->server->module_config,
	&external_auth_module);

    conn_rec *c= r->connection;

    int m= r->method_number;
    int code;
    char *extname;
    const char *extpath, *extmethod;

    const array_header *reqs_arr= ap_requires(r);
    require_line *reqs= reqs_arr ? (require_line *)reqs_arr->elts : NULL;

    int x;
    const char *t;
    char *w;

    /* Extract which external was chosen */
    extname= dir_config_rec->group_extname;

    /* Check if we are supposed to handle this authentication */
    if ( !extname )
	return DECLINED;

    if (!reqs_arr)
    	return DECLINED;

    for(x=0; x < reqs_arr->nelts; x++) 
    {
	if (!(reqs[x].method_mask & (1 << m))) continue;

	t= reqs[x].requirement;
	w= ap_getword_white(r->pool, &t);

	if(!strcmp(w,"valid-user"))
	    return OK;

	if(!strcmp(w,"user")) 
	{
	    while(t[0]) 
	    {
		w= ap_getword_conf(r->pool, &t);
		if(!strcmp(c->user,w))
		    return OK;
	    }
	}
	else if( !strcmp(w,"group") ) 
	{
	    if (t[0])
	    {
		/* Get the path and method associated with that external */
		if (!(extpath= ap_table_get(
			server_config_rec->group_extpath, extname)) ||
		    !(extmethod= ap_table_get(
		    	server_config_rec->group_extmethod, extname)))
		{
		    errno= 0;
		    ap_log_reason(ap_psprintf(r->pool,
			"invalid GroupExternal keyword (%s)", extname),
			r->filename, r);
		    ap_note_basic_auth_failure(r);
		    return AUTH_REQUIRED;
		}

		if (dir_config_rec->external_groupsatonce)
		{
		    /* Pass rest of require line to authenticator */
		    code= exec_external(extpath, extmethod, r, ENV_GROUP, t);
		    if (!code) return OK;
		}
		else
		{
		    /* Call authenticator once for each group name on line */
		    do {
		        w= ap_getword_white(r->pool, &t);
			code= exec_external(extpath,
				extmethod, r, ENV_GROUP, w);
			if (!code) return OK;
		    } while(t[0]);
		}
	    }
	}
	else if (dir_config_rec->external_authoritative)
	{
	    /* If we aren't authoritative, any require directives could be
	     * valid even if we don't grok it.  However if we are, we can
	     * warn the user that they did something wrong.
	     */
	    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
	    	"access to %s failed, reason: unknown require directive:"
	    	"\"%s\"", r->uri, reqs[x].requirement);
	}
    }
    
    if (!dir_config_rec->external_authoritative)
	return DECLINED;

    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
    	"access to %s failed, reason: user %s not allowed access",
    	r->uri, c->user);

    ap_note_basic_auth_failure(r);
    return AUTH_REQUIRED;
}

module external_auth_module= {
    STANDARD_MODULE_STUFF,
    NULL,			 /* initializer */
    create_extauth_dir_config,	 /* dir config creater */
    NULL,			 /* dir merger --- default is to override */
    create_extauth_server_config,/* server config */
    NULL,			 /* merge server config */
    extauth_cmds,		 /* command table */
    NULL,			 /* handlers */
    NULL,			 /* filename translation */
    extauth_basic_user,		 /* check_user_id */
    extauth_check_auth,		 /* check auth */
    NULL,			 /* check access */
    NULL,			 /* type_checker */
    NULL,			 /* fixups */
    NULL			 /* logger */
};
