/* This file is part of Qenolaba.
   Copyright (C) 1997-2015 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

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
 * Handle rendering of a Board onto a Qt widget,
 * shows moves and manages input of moves
 */

#ifndef _BOARDWIDGET_H_
#define _BOARDWIDGET_H_

#include "Move.h"
#include "Board.h"
#include "Piece.h"

#include <QWidget>
#include <QPixmap>

class BoardWidget : public BallWidget
{
    Q_OBJECT
public:
    BoardWidget(Board&, QWidget *parent = 0);
    ~BoardWidget();

    void createPos(int , int , int , Ball*);
    void initBalls();
    void updateBalls();

    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent( QMouseEvent* pEvent );
    virtual void mouseReleaseEvent( QMouseEvent* pEvent );
    virtual void mouseMoveEvent( QMouseEvent* pEvent );

    void renderBalls(bool r);

    void choseMove(MoveList*);

    /* Show a move with highlighting
     *  step 0: state before move
     *       1: state after move
     *       2: remove all marks (for blinking)
     *       3: highlight marks (before move)
     *       4: highlight marks (after move)
     * Always call with step 0 before actual playing the move !!
     */
    void showMove(const Move& m, int step, bool updateGUI = true);

    /* Only highlight start
     * Step 0: original state
     *      1: highlight start
     * Always call with step 0 before actual playing the move !!
     */
    void showStart(const Move& m, int step, bool updateGUI = true);


    /* enable/disable Edit Mode:
     * On disabling & valid position it's actually copied to Board
     */
    bool setEditMode(bool);
    //	int validState() { return board->validState(); }
    //	bool isValid() { return validState()==Board::valid; }

    /* copy actual board position */
    void updatePosition(bool updateGUI = false);
    void clearPosition();

    int getColor1Count()      { return color1Count; }
    int getColor2Count()      { return color2Count; }

public slots:
    void configure();

signals:
    void moveChoosen(Move&);
    void rightButtonPressed(int,const QPoint&);
    void updateSpy(QString);
    void edited(int);
protected:
    virtual QSize sizeHint() const;

private:
    int positionOf(int x, int y);
    bool isValidStart(int pos, bool);
    bool isValidEnd(int pos);
    void drawBoard();

    QPixmap boardPM;
    Board& board;
    int actValue;

    bool editMode, renderMode;
    int editColor;

    /* copied position */
    int field[Board::AllFields];
    int color1Count, color2Count, color;
    bool boardOK;

    /* for getting user Move */
    MoveList *pList;
    Move actMove;
    bool gettingMove, mbDown, startValid, startShown;
    int startPos, actPos, oldPos, shownDirection;
    int startField, startField2, actField, oldField, startType;
    QColor *boardColor, *redColor, *yellowColor, *redHColor, *yellowHColor;
    QCursor *arrowAll, *arrow[7];

    Ball *n1, *n2, *h1, *h2, *d1, *d2; //, *e;
};

/* Test BoardWidget */

class TestGame: public QObject
{
    Q_OBJECT
public:
    TestGame();

    Board b;
    MoveList l;
    BoardWidget w;

public slots:
    void draw(Move& m);
};


#endif /* _BOARDWIDGET_H_ */

