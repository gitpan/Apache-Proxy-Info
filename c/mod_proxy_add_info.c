/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000 The Apache Software Foundation.  All rights
 * reserved.
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
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 * Portions of this software are based upon public domain software
 * originally written at the National Center for Supercomputing Applications,
 * University of Illinois, Urbana-Champaign.
 */

/* proxy_add_info module 
 *  
 *  This module is a modification of the proxy_add_forward module,
 *  adding more headers to proxied requests
 *  
 * *X-Forwarded-For 	=> IP of the original client (proxy_add_forward)
 *  X-Forwarded-Host 	=> Host: requested by original client
 *  X-HTTPS				=> On/Off : SSL connection or not
 *
 * Philippe M. Chiasson <gozer@hbesoftware.com>, May 2000
*/

/* proxy_add_forward module
 *
 * This module adds a 'X-Forwarded-For' header to outgoing
 * proxy requests like Squid does.
 *
 * You can then get the client ip back on the "proxied host" by
 * setting r->connection->remote_ip from this header.
 *
 * Ask Bjoern Hansen <ask@netcetera.dk>, October 1998

 * Changes:
 *
 * April 12 2000: Changed the license to the ASF 1.1 license.
 *
 * April 12 2000: Made it so that we append our IP to an existing
 *                "X-Forwarded-For" line instead of clobbering an 
 *                existing one. - <ahosey@systhug.com>
 *
 * June   8 1999: Added instructions on how to compile it into the
 *                frontend apache
 *
 * April 12 1999: Changed the sample code so it doesn't confuse the
 *                C compiler, ydkhr! Thanks to Mike Whitaker for 
 *                noticing. 
 *
 * March  1 1999: Added sample code on how to use the header with
 *                mod_perl
 * 

To use the module you have to compile it into the frontend part of
your server, I usually copy the module to apache-1.3/src/modules/extra/
and use APACI like:

  ./configure --prefix=/usr/local/apache \
     --activate-module=src/modules/extra/mod_proxy_add_forward.c \
     --enable-module=proxy_add_forward [... more apaci options ...]    

You should also be able to compile and use this module as a
dynamically loaded module (DSO).
 
TMTOWTDI, but I usually make the 'backend' part of the system
something like the following:

in startup.pl:

  sub My::ProxyRemoteAddr ($) {
    my $r = shift;

	# we'll only look at the X-Forwarded-For header if the requests
	# comes from our proxy at localhost
	return OK unless ($r->connection->remote_ip eq "127.0.0.1");

	if (my ($ip) = $r->header_in('X-Forwarded-For') =~ /([^,\s]+)$/) {
	  $r->connection->remote_ip($ip);
	}
	
	return OK;
  }

And in httpd.conf:

  PerlPostReadRequestHandler My::ProxyRemoteAddr
 
 */


#include "httpd.h"
#include "http_config.h"
#include "http_core.h"

module MODULE_VAR_EXPORT proxy_add_info_module;

static int proxy_add_header(const request_rec *, const char *, const char *);

static int add_info_header(request_rec *r)
{
    const char *oldvalue;

    if (r->proxyreq) {
		proxy_add_header(r,"X-Forwarded-For",r->connection->remote_ip);
		proxy_add_header(r,"X-Forwarded-Host",ap_table_get(r->headers_in,"Host"));
		proxy_add_header(r,"X-HTTPS",ap_table_get(r->subprocess_env,"HTTPS"));
		return OK;
    }
    return DECLINED;
}

#
static int proxy_add_header(const request_rec *r, const char *header_name, const char *header_value)
{
	const char *oldvalue;
	
	if (oldvalue = ap_table_get(r->headers_in, header_name)) {
	    ap_table_set(r->headers_in, header_name,
			 ap_pstrcat(r->pool, oldvalue, ", ", 
				    header_value, NULL));
	}
	else {
	    ap_table_set(r->headers_in, header_name,
			 header_value);
	}
return OK;
}

module MODULE_VAR_EXPORT proxy_add_info_module = {
    STANDARD_MODULE_STUFF,
    NULL,                       /* initializer */
    NULL,                       /* dir config creater */
    NULL,                       /* dir merger --- default is to override */
    NULL,                       /* server config */
    NULL,                       /* merge server configs */
    NULL,                       /* command table */
    NULL,                       /* handlers */
    NULL,                       /* filename translation */
    NULL,                       /* check_user_id */
    NULL,                       /* check auth */
    NULL,                       /* check access */
    NULL,                       /* type_checker */
    add_info_header,         	/* fixups */
    NULL,                       /* logger */
    NULL,                       /* header parser */
    NULL,                       /* child_init */
    NULL,                       /* child_exit */
    NULL                        /* post read-request */
};


