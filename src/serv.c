/*
 *      Copyright (C) 2000 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "../lib/gnutls.h"
#include <port.h>

#define SA struct sockaddr
#define ERR(err,s) if(err==-1) {perror(s);return(1);}
#define MAX_BUF 100

int main()
{
    int err, listen_sd;
    int sd, ret;
    struct sockaddr_in sa_serv;
    struct sockaddr_in sa_cli;
    int client_len;
    char topbuf[512], *tmp;
    GNUTLS_STATE state;
    char buffer[MAX_BUF+1];
    int optval = 1;
    SRP_SERVER_CREDENTIALS cred;
    const SRP_AUTH_INFO *info;
 
    /* this is a password file (created with the included crypt utility) 
     * Read README.crypt prior to using SRP.
     */
    cred.password_file="tpasswd";
    cred.password_conf_file="tpasswd.conf";
    
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    ERR(listen_sd, "socket");

    memset(&sa_serv, '\0', sizeof(sa_serv));
    sa_serv.sin_family = AF_INET;
    sa_serv.sin_addr.s_addr = INADDR_ANY;
    sa_serv.sin_port = htons(PORT);	/* Server Port number */

    setsockopt( listen_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    err = bind(listen_sd, (SA *) & sa_serv, sizeof(sa_serv));
    ERR(err, "bind");
    err = listen(listen_sd, 1024);
    ERR(err, "listen");



    client_len = sizeof(sa_cli);
    for (;;) {
	gnutls_init(&state, GNUTLS_SERVER);
	gnutls_set_db_name(state, "/tmp/gdb");
	gnutls_set_cipher_priority( state, 4, GNUTLS_TWOFISH, GNUTLS_RIJNDAEL, GNUTLS_3DES, GNUTLS_ARCFOUR);
	gnutls_set_compression_priority( state, 2, GNUTLS_ZLIB, GNUTLS_NULL_COMPRESSION);
	gnutls_set_kx_priority( state, 2, GNUTLS_KX_SRP, GNUTLS_KX_ANON_DH);
	
	gnutls_set_kx_cred( state, GNUTLS_KX_ANON_DH, NULL);
	gnutls_set_kx_cred( state, GNUTLS_KX_SRP, &cred);
	
	gnutls_set_mac_priority( state, 2, GNUTLS_MAC_SHA, GNUTLS_MAC_MD5);
	sd = accept(listen_sd, (SA *) & sa_cli, &client_len);


	fprintf(stderr, "connection from %s, port %d\n",
		inet_ntop(AF_INET, &sa_cli.sin_addr, topbuf,
			  sizeof(topbuf)), ntohs(sa_cli.sin_port));



	ret = gnutls_handshake(sd, state);
	if (ret < 0) {
	    close(sd);
	    gnutls_deinit(&state);
	    fprintf(stderr, "Handshake has failed (%d)\n", ret);
	    gnutls_perror(ret);
	    continue;
	}
	fprintf(stderr, "Handshake was completed\n");

	/* print srp specific data */
	if ( gnutls_get_current_kx( state) == GNUTLS_KX_SRP) {
		info = gnutls_get_auth_info( state);
		if (info != NULL)
			printf("\nUser '%s' connected\n", info->username);
	}
	
	/* print state information */
	tmp = _gnutls_kx_get_name(gnutls_get_current_kx( state));
	printf("Key Exchange: %s\n", tmp); free(tmp);
	tmp = _gnutls_compression_get_name(gnutls_get_current_compression_method( state));
	printf("Compression: %s\n", tmp); free(tmp);
	tmp = _gnutls_cipher_get_name(gnutls_get_current_cipher( state));
	printf("Cipher: %s\n", tmp); free(tmp);
	tmp = _gnutls_mac_get_name(gnutls_get_current_mac_algorithm( state));
	printf("MAC: %s\n", tmp); free(tmp);

	fprintf(stderr, "Acting as echo server...\n");
/*	ret =
	    gnutls_write(sd, state, "hello client",
			sizeof("hello client"));
	if (ret < 0) {
	    close(sd);
	    gnutls_deinit(&state);
	    gnutls_perror(ret);
	    continue;
	}
*/
	for (;;) {
	    bzero( buffer, MAX_BUF+1);
	    ret = gnutls_read(sd, state, buffer, MAX_BUF);
	    if (gnutls_is_fatal_error(ret) == 1) {
		if (ret == GNUTLS_E_CLOSURE_ALERT_RECEIVED) {
		    fprintf(stderr,
			    "\nPeer has closed the GNUTLS connection\n");
		    break;
		} else {
		    fprintf(stderr, "\nReceived corrupted data(%d). Closing the connection.\n", ret);
		    break;
		}

	    }
	    gnutls_write(sd, state, buffer, strlen(buffer));
	}
	fprintf(stderr, "\n");
	gnutls_close(sd, state);
	close(sd);
	gnutls_deinit(&state);
    }
    close(listen_sd);
    return 0;

}
