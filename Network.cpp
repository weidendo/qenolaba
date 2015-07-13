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
#include "Board.h"      // for broadcast(Board*)

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <QSocketNotifier>

Listener::Listener(const char* h, struct sockaddr_in s, bool r)
{
    setHost(h);
    sin = s;
    reachable = r;
}

void Listener::setHost(const char* h)
{
    if (h==0)
	host[0]=0;
    else {
	int l = strlen(h);
	if (l>99) l=99;
	strncpy(host, h, l);
	host[l] = 0;
    }
}

int Listener::port()
{
    return ntohs(sin.sin_port);
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
	//    qDebug("...Port %d in use\n", port+i);
    }
    mySin = name;
    //  qDebug("I'm using Port %d\n", port+i);
    if (i==5) {
	qDebug("Error in bind to port %d\n", port);
	close(fd);
	fd = -1;
	return;
    }

    if (::listen(fd,5)<0) {
	qDebug("Error in listen\n");
	close(fd);
	fd = -1;
	return;
    }

    sn = new QSocketNotifier( fd, QSocketNotifier::Read );
    QObject::connect( sn, SIGNAL(activated(int)),
		      this, SLOT(gotConnection()) );
    sentPos = 0;

    for(j = 0; j<i;j++)
	addListener("127.0.0.1", port+j);
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

Listener* Network::listenerMatch(struct sockaddr_in sin)
{
    foreach (Listener* l, listeners)
	if (l->sin.sin_addr.s_addr == sin.sin_addr.s_addr &&
	    l->sin.sin_port == sin.sin_port) return l;
    return 0;
}

void Network::gotConnection()
{
    static char tmp[1024];
    int len=0;
    struct sockaddr_in sin;
    socklen_t sz = sizeof (sin);

    if (fd<0) {
	qDebug("Error: gotConnection without valid network?");
	return;
    }

    //  qDebug("GotConnection: ");
    int s = accept(fd,(struct sockaddr *)&sin, &sz);
    if (s<0) {
	qDebug("Error in accept\n");
	return;
    }
    while(read(s, tmp+len, 1)==1) len++;
    close(s);
    tmp[len]=0; len++;
    //  qDebug("Got: '%s'\n",tmp);
    if (strncmp(tmp,"reg ",4)==0) {
	int port = atoi(tmp+4);
	sin.sin_port = htons( port );
	Listener* l = listenerMatch(sin);
	if (l)
	    l->reachable = true;
	else {
	    l = new Listener(0,sin);
	    listeners.append(l);
	}
	//    qDebug("Reg of 0x%x:%d\n",
	//	   ntohl(sin.sin_addr.s_addr ), ntohs(sin.sin_port));

	if (sentPos)
	    l->reachable = sendString(l->sin, sentPos, sentLen);

	return;
    }

    if (strncmp(tmp,"unreg ",6)==0) {
	int port = atoi(tmp+6);
	sin.sin_port = htons( port );
	Listener* l = listenerMatch(sin);
	if (l) {
	    listeners.removeOne(l);
	    delete l;
	    //    qDebug("UnReg of 0x%x:%d\n",
	    //	   ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port));
	}
	else
	    qDebug("Error: UnReg of 0x%x:%d. Not Found\n",
		   ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port));
	return;
    }

    if (strncmp(tmp,"pos ",4)==0) {
	sentPos = 0;
	emit gotPosition(tmp+4);
    }
}

void Network::addListener(const char* host, int port)
{
    struct hostent *hostinfo;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    hostinfo = gethostbyname (host);
    if (hostinfo == NULL) {
	qDebug ("Error in addListener: Unknown host '%s'.\n", host);
	return;
    }
    sin.sin_addr = *(struct in_addr *) hostinfo->h_addr;

    Listener* l = listenerMatch(sin);
    if (l) {
	l->reachable = true;
	l->setHost(host);
    }
    else {
	l = new Listener(host, sin);
	listeners.append(l);
	//  qDebug("Added Listener %s, 0x%x:%d\n",
	//	 host, ntohl(name.sin_addr.s_addr), ntohs(name.sin_port));
    }

    char tmp[50];
    int len = sprintf(tmp, "reg %d", ntohs(mySin.sin_port));

    if (!sendString( sin, tmp, len)) {
	listeners.removeOne(l);
	delete l;
    }
}

void Network::addListener(const char *a)
{
    QString host = QString("127.0.0.1");
    int port = defaultPort;
    QString addr(a);
    int sep = addr.indexOf(':');
    if (sep>0)
	host = addr.mid(0,sep);
    if (sep>=0)
	port = addr.mid(sep+1).toInt();
    else
	host = addr;
    addListener(host.toUtf8().constData(), port);
}

void Network::broadcast(const char* pos)
{
    static char tmp[1024];
    int len = sprintf(tmp,"pos %s", pos);

    foreach (Listener* l, listeners) {
	if (l->reachable)
	    l->reachable = sendString(l->sin, tmp, len);
    }

    sentPos = tmp;
    sentLen = len;
}

void Network::broadcast(Board *b)
{
    broadcast(qPrintable(b->getState()));
}

bool Network::sendString(struct sockaddr_in sin, const char* str, int len)
{
    int s = ::socket (PF_INET, SOCK_STREAM, 0);
    if (s<0) {
	qDebug("Error in sendString/socket ??\n");
	return false;
    }
    if (::connect (s, (struct sockaddr *)&sin, sizeof (sin)) <0) {
	qDebug("Error in sendString/connect to socket 0x%x:%d\n",
	       ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port) );
	return false;
    }
    while(len>0) {
	int written = write(s, str, len);
	if (written <= 0) {
	    qDebug("sendString: Error in write\n");
	    break;
	}
	str += written;
	len -= written;
    }
    close(s);
    //  qDebug("Send '%s' to 0x%x:%d\n", str,
    //	 ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port) );
    return true;
}

