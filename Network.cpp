/* This file is part of Qenolaba.
   Copyright (C) 1999-2015 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   Qenolaba is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
 * Simple Network Support
 *
 * Install a listening socket; receive positions on incoming
 * connections
 */

#include "Network.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <QSocketNotifier>

Listener::Listener(const char* h, int p, struct sockaddr_in s,bool r)
{
    if (h==0)
	host[0]=0;
    else {
	int l = strlen(h);
	if (l>99) l=99;
	strncpy(host, h, l);
	host[l] = 0;
    }
    port = p;
    sin = s;
    reachable = r;
}

Network::Network(int port)
{
    struct sockaddr_in name;
    int i,j;

    fd = ::socket (PF_INET, SOCK_STREAM, 0);
    if (fd<0) return;

    for(i = 0; i<5;i++) {
	name.sin_family = AF_INET;
	name.sin_port = htons (port+i);
	name.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (fd, (struct sockaddr *) &name, sizeof (name)) >= 0)
	    break;
	//    printf("...Port %d in use\n", port+i);
    }
    mySin = name;
    //  printf("I'm using Port %d\n", port+i);
    if (i==5) {
	printf("Error in bind to port %d\n", port);
	close(fd);
	fd = -1;
	return;
    }
    for(j = 0; j<i;j++)
	addListener("127.0.0.1", port+j);

    if (::listen(fd,5)<0) {
	printf("Error in listen\n");
	close(fd);
	fd = -1;
	return;
    }

    sn = new QSocketNotifier( fd, QSocketNotifier::Read );
    QObject::connect( sn, SIGNAL(activated(int)),
		      this, SLOT(gotConnection()) );
}

Network::~Network()
{
    if (fd<0) return;
    close(fd);

    char tmp[50];
    int len = sprintf(tmp, "unreg %d", ntohs(mySin.sin_port));

    foreach (Listener* l, listeners) {
	if (l->reachable)
	    sendString( l->sin, tmp, len);
    }
    qDeleteAll(listeners);
    listeners.clear();

    delete sn;
}

void Network::gotConnection()
{
    static char tmp[1024];
    int len=0;
    struct sockaddr_in sin;
    socklen_t sz = sizeof (sin);

    //  printf("GotConnection: ");
    int s = accept(fd,(struct sockaddr *)&sin, &sz);
    if (s<0) {
	printf("Error in accept\n");
	return;
    }
    while(read(s, tmp+len, 1)==1) len++;
    close(s);
    tmp[len]=0; len++;
    //  printf("Got: '%s'\n",tmp);
    if (strncmp(tmp,"reg ",4)==0) {
	int port = atoi(tmp+4);
	sin.sin_port = htons( port );
	Listener *l = new Listener(0,0,sin);
	//    printf("Reg of 0x%x:%d\n",
	//	   ntohl(sin.sin_addr.s_addr ), ntohs(sin.sin_port));
	listeners.append(l);
	return;
    }

    if (strncmp(tmp,"unreg ",6)==0) {
	int port = atoi(tmp+6);
	sin.sin_port = htons( port );
	foreach (Listener* l, listeners) {
	    if (l->sin.sin_addr.s_addr == sin.sin_addr.s_addr &&
		l->sin.sin_port == sin.sin_port) {
		listeners.removeOne(l);
		delete l;
		//    printf("UnReg of 0x%x:%d\n",
		//	   ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port));
		return;
	    }
	}
	printf("Error: UnReg of 0x%x:%d. Not Found\n",
	       ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port));
	return;
    }

    if (strncmp(tmp,"pos ",4)==0) {
	emit gotPosition(tmp+4);
    }
}

void Network::addListener(const char* host, int port)
{
    struct hostent *hostinfo;
    struct sockaddr_in name;

    memset(&name, 0, sizeof(struct sockaddr_in));
    name.sin_family = AF_INET;
    name.sin_port = htons (port);
    hostinfo = gethostbyname (host);
    if (hostinfo == NULL) {
	printf ("Error in addListener: Unknown host %s.\n", host);
	return;
    }
    name.sin_addr = *(struct in_addr *) hostinfo->h_addr;

    Listener *l = new Listener(host,port,name);
    //  printf("Added Listener %s, 0x%x:%d\n",
    //	 host, ntohl(name.sin_addr.s_addr), ntohs(name.sin_port));
    listeners.append(l);

    char tmp[50];
    int len = sprintf(tmp, "reg %d", ntohs(mySin.sin_port));

    if (!sendString( name, tmp, len)) {
	listeners.removeOne(l);
	delete l;
    }
}

void Network::broadcast(const char* pos)
{
    char tmp[1024];
    int len = sprintf(tmp,"pos %s", pos);

    foreach (Listener* l, listeners) {
	if (l->reachable)
	    l->reachable = sendString(l->sin, tmp, len);
    }
}

bool Network::sendString(struct sockaddr_in sin, char* str, int len)
{
    int s = ::socket (PF_INET, SOCK_STREAM, 0);
    if (s<0) {
	printf("Error in sendString/socket ??\n");
	return false;
    }
    if (::connect (s, (struct sockaddr *)&sin, sizeof (sin)) <0) {
	printf("Error in sendString/connect to socket 0x%x:%d\n",
	       ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port) );
	return false;
    }
    write(s, str, len);
    close(s);
    //  printf("Send '%s' to 0x%x:%d\n", str,
    //	 ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port) );
    return true;
}

