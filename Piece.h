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
 * Rendering of animated game pieces
 * with custom colors and ripple texture, supporting blending/rotate
 */

#ifndef _PIECE_H_
#define _PIECE_H_

#include <QPixmap>
#include <QColor>
#include <QWidget>
#include <QList>

/* textures for pieces */
#define TEX_FLAT   0
#define TEX_RIPPLE 1

class Piece {

public:
    static void setSpecials(double z, double f, double l)
    { zoom = z, flip=f, limit=l; }

    Piece(const QColor& c, double a = 0.0, int t=TEX_RIPPLE );
    ~Piece();

    QPixmap* pixmap();

    double angle() { return an; }
    QColor pieceColor() { return bColor; }

    static int w() { return sizeX; }
    static int h() { return sizeY; }
    static void setSize(int x,int y);
    static void setLight(int x=5, int y=3, int z=10,
			 const QColor& c = QColor(200,230,255) );
    static void setTexture(double c=13., double d=.2);

private:
    void render();
    static void invalidate();

    //static QImage back;
    static int sizeX, sizeY;
    static double lightX, lightY, lightZ;
    static QColor lightColor;
    static double rippleCount, rippleDepth;

    QPixmap pm;
    QColor bColor;
    double an, sina, cosa;
    int tex;

    Piece *next;
    static Piece* first;

    static double zoom, flip, limit;
};


class PieceAnimation {
public:
    PieceAnimation(int s, Piece*, Piece*);
    ~PieceAnimation();

    int steps;
    QList<Piece*> pieces;
};

#define ANIMATION_STOPPED 0
#define ANIMATION_FORWARD 1
#define ANIMATION_BACK    2
#define ANIMATION_LOOP    3
#define ANIMATION_CYCLE   4

class PiecePosition {
public:
    PiecePosition(int xp,int yp, Piece* d);

    int x, y, actStep, actDir, actType;
    Piece* def;
    PieceAnimation* actAnimation;
};

#define MAX_POSITION  130
#define MAX_ANIMATION  20

class PieceWidget : public QWidget
{
    Q_OBJECT

public:
    PieceWidget(int _freq, int bFr, QWidget *parent = 0);
    ~PieceWidget();

    void createBlending(int, int, Piece* , Piece* );
    void createPiecePosition(int, int x, int y, Piece*);

    void startAnimation(int pos, int anim, int type=ANIMATION_FORWARD);
    void stopAnimation(int pos);

    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);

signals:
    void animationFinished(int);
    void animationsFinished(void);

protected:
    void drawPieces(QPainter*);

private slots:
    void animate();

protected:
    QVector<PiecePosition*> positions;
    QVector<PieceAnimation*> animations;

private:
    int freq;
    int xStart, yStart, realSize, pieceFraction;
    bool isRunning;
    QTimer *timer;
};


/* Piece Rendering Test */

class PieceTest: public PieceWidget
{
    Q_OBJECT
public:
    PieceTest(QWidget *parent=0);
protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
};

#endif // _PIECE_H_
