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

#ifndef _EVALSCHEME_H_
#define _EVALSCHEME_H_

#include "Move.h"


class EvalScheme
{
public:
    EvalScheme(QString);
    EvalScheme(EvalScheme&);
    ~EvalScheme() {}

    void setDefaults();

    static EvalScheme* create(QString);
    QString ascii();

    void setName(QString n) { _name = n; }
    void setRingValue(int ring, int value);
    void setRingDiff(int ring, int value);
    void setStoneValue(int stoneDiff, int value);
    void setMoveValue(int type, int value);
    void setInARowValue(int stones, int value);

    QString name() { return _name; }
    int ringValue(int r) { return (r>=0 && r<5) ? _ringValue[r] : 0; }
    int ringDiff(int r) { return (r>0 && r<5) ? _ringDiff[r] : 0; }
    int stoneValue(int s) { return (s>0 && s<6) ? _stoneValue[s] : 0; }
    int moveValue(int t) { return (t>=0 && t<Move::typeCount) ? _moveValue[t] : 0;}
    int inARowValue(int s) { return (s>=0 && s<InARowCounter::inARowCount) ? _inARowValue[s]:0; }

private:
    int _ringValue[5], _ringDiff[5];
    int _stoneValue[6], _moveValue[Move::none];
    int _inARowValue[InARowCounter::inARowCount];

    QString _name;
};

#endif // _EVALSCHEME_H_
