/* This file is part of Qenolaba.
   Copyright (C) 2000-2015 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

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

/* Evaluation Schemes are used for evalution of a board position */

#include "EvalScheme.h"

#include <QStringList>

// Default Values
static int defaultRingValue[] = { 45, 35, 25, 10, 0 };
static int defaultRingDiff[]  = {  0, 10, 10,  8, 5 };
static int defaultStoneValue[]= { 0,-800,-1800,-3000,-4400,-6000 };
static int defaultMoveValue[Move::typeCount] = { 40,30,30, 15,14,13,
						 5,5,5, 2,2,2, 1 };
static int defaultInARowValue[InARowCounter::inARowCount]= { 2, 5, 4, 3 };


/**
 * Constructor: Set Default values
 */
EvalScheme::EvalScheme(QString n)
{
    _name = n;
    setDefaults();
}


/**
 * Copy Constructor
 */
EvalScheme::EvalScheme(EvalScheme& s)
{
    _name = s._name;

    for(int i=0;i<6;i++)
	_stoneValue[i] = s._stoneValue[i];

    for(int i=0;i<Move::typeCount;i++)
	_moveValue[i] = s._moveValue[i];

    for(int i=0;i<InARowCounter::inARowCount;i++)
	_inARowValue[i] = s._inARowValue[i];

    for(int i=0;i<5;i++)
	_ringValue[i] = s._ringValue[i];

    for(int i=0;i<5;i++)
	_ringDiff[i] = s._ringDiff[i];
}


void EvalScheme::setDefaults()
{
    for(int i=0;i<6;i++)
	_stoneValue[i] = defaultStoneValue[i];

    for(int i=0;i<Move::typeCount;i++)
	_moveValue[i] = defaultMoveValue[i];

    for(int i=0;i<InARowCounter::inARowCount;i++)
	_inARowValue[i] = defaultInARowValue[i];

    for(int i=0;i<5;i++)
	_ringValue[i] = defaultRingValue[i];

    for(int i=0;i<5;i++)
	_ringDiff[i] = defaultRingDiff[i];
}



void EvalScheme::setRingValue(int ring, int value)
{
    if (ring >=0 && ring <5)
	_ringValue[ring] = value;
}

void EvalScheme::setRingDiff(int ring, int value)
{
    if (ring >=1 && ring <5)
	_ringDiff[ring] = value;
}

void EvalScheme::setStoneValue(int stoneDiff, int value)
{
    if (stoneDiff>0 && stoneDiff<6)
	_stoneValue[stoneDiff] = value;
}

void EvalScheme::setMoveValue(int type, int value)
{
    if (type>=0 && type<Move::typeCount)
	_moveValue[type] = value;
}

void EvalScheme::setInARowValue(int stones, int value)
{
    if (stones>=0 && stones<InARowCounter::inARowCount)
	_inARowValue[stones] = value;
}

/**
 * Create a EvalScheme out of a String of format
 *
 *  <SchemeName>=<v1>,<v2>,<v3>,<v4>,... (34 values)
 *
 */

EvalScheme* EvalScheme::create(QString scheme)
{
    int pos = scheme.indexOf('=');
    if (pos<0) return 0;

    EvalScheme* evalScheme = new EvalScheme( scheme.left(pos) );
    evalScheme->setDefaults();

    QStringList list = scheme.right(pos+1).split( QChar(',') );

    int i=0;
    while(i<list.count()) {
	if (i<5)
	    evalScheme->_stoneValue[i+1] = list[i].toInt();
	else if (i<10)
	    evalScheme->_ringValue[i-5] = list[i].toInt();
	else if (i<15)
	    evalScheme->_ringDiff[i-10] = list[i].toInt();
	else if (i<15+Move::typeCount)
	    evalScheme->_moveValue[i-15] = list[i].toInt();
	else if (i<15+Move::typeCount+InARowCounter::inARowCount)
	    evalScheme->_inARowValue[i-15-Move::typeCount] = list[i].toInt();
	i++;
    }

    return evalScheme;
}

QString EvalScheme::ascii()
{
    QString res;
    int i;

    res.sprintf("%s=%d", _name.toAscii().constData(), _stoneValue[1]);
    for(i=1;i<6;i++)
	res += QString(",%1").arg( _stoneValue[i] );
    for(i=0;i<Move::typeCount;i++)
	res += QString(",%1").arg( _moveValue[i] );
    for(i=0;i<InARowCounter::inARowCount;i++)
	res += QString(",%1").arg( _inARowValue[i] );
    for(i=0;i<5;i++)
	res += QString(",%1").arg( _ringValue[i] );
    for(i=0;i<5;i++)
	res += QString(",%1").arg( _ringDiff[i] );

    return res;
}
