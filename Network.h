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

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <sys/types.h>
#include <netinet/in.h>

#include <QObject>
#include <QList>

class Listener {
public:
    Listener(const char*,int,struct sockaddr_in,bool=true);

    char host[100];
    int port;
    struct sockaddr_in sin;
    bool reachable;
};


class QSocketNotifier;

class Network: public QObject
{
    Q_OBJECT
    
public:
    enum { defaultPort = 23412 };

    /* install listening TCP socket on port */
    Network(int port = defaultPort);
    ~Network();

    bool isOK() { return (fd>=0); }
    void addListener(const char* host, int port);
    void broadcast(const char* pos);

signals:
    void gotPosition(const char* pos);

private slots:
    void gotConnection();

private:
    bool sendString(struct sockaddr_in sin, char* str, int len);

    QList<Listener*> listeners;
    struct sockaddr_in mySin;
    int fd, myPort;
    QSocketNotifier *sn;
};

#endif // _NETWORK_H_
