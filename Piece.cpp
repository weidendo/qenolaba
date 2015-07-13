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

#include <math.h>

#include <QTimer>
#include <QBitmap>
#include <QImage>
#include <QPixmap>
#include <QPainter>

#include "Piece.h"

Ball* Ball::first = 0;
//QImage Ball::back;
int Ball::sizeX, Ball::sizeY;
double Ball::lightX, Ball::lightY, Ball::lightZ;
QColor Ball::lightColor;
double Ball::rippleCount, Ball::rippleDepth;

/* set global Ball parameter */
void Ball::setSize(int x, int y)
{
    sizeX = x;
    sizeY = y;

    invalidate();
}

void Ball::invalidate()
{
    Ball *b;

    /* invalidate all Balls... */
    for(b=first;b!=0;b=b->next)
	b->pm = QPixmap(0,0);
}

void Ball::setLight(int x, int y, int z, const QColor& c)
{
    double len = sqrt(double(x*x + y*y + z*z));

    lightX = x/len;
    lightY = y/len;
    lightZ = z/len;

    lightColor = c;

    invalidate();
}


void Ball::setTexture(double c, double d)
{
    rippleCount = c;
    rippleDepth = d;

    invalidate();
}

// default static setting
double Ball::zoom = 1.05;
double Ball::flip = 2.0;
double Ball::limit = 0;

Ball::Ball(const QColor& c, double a, int t)
{
    if (first ==0) {
	sizeX = sizeY = -1;
	setLight();
	setTexture(7,.3);
    }

    bColor = c;
    an = a;
    sina = sin(a), cosa = cos(a);

    tex = t;

    next = first;
    first = this;
}

Ball::~Ball()
{
    Ball* b;

    if (first == this)
	first = next;
    else {
	for(b = first; b!=0; b=b->next)
	    if (b->next == this) break;
	if (b!=0)
	    b->next = next;
    }
}

QPixmap* Ball::pixmap()
{
    if (pm.isNull() && sizeX>0 && sizeY>0)
	render();
    return &pm;
}

void Ball::render()
{
    int x,y;
    double xx,yy,zz, ll,lll, red,green,blue;

    if (sizeX==0 || sizeY==0)
	return;

    QImage image(sizeX,sizeY, QImage::Format_RGB32);
    image.fill(0);

    double vv=2./(sizeX+sizeY);

    /* Go through all pixels, mapping x/y to (-1..1,-1..1) */
    for(y=0;y<sizeY;y++) {
	yy = (2.*y-sizeY)/(sizeY-2) *zoom;
	for(x=0;x<sizeX;x++) {
	    xx = (2.*x-sizeX)/(sizeX-2) *zoom;

	    /* Change only if inside the ball */
	    zz = 1 - (xx*xx + yy*yy);

	    if (zz>flip) zz=2*flip-zz;
	    else {
		zz -= limit;
	    }

	    if (zz>-vv) {
		zz = (zz<0) ? 0 : sqrt(zz);

		/* ll: light intensity at this point */
		ll = xx*lightX + yy*lightY + zz*lightZ;

		/* some face modification */
		double mapx = xx*(2-zz);
		double mapy = yy*(2-zz);
		double rmapx =  cosa*mapx + sina*mapy; /* rotate */
		double rmapy = -sina*mapx + cosa*mapy;

		if (tex>0)
		    ll += rippleDepth* cos(rippleCount*rmapx)*cos(rippleCount*rmapy);

		ll = (ll<0.01) ? 0.0 : (ll>.99) ? 1.0 : ll;
		lll = ll*ll;

		//	printf("x %f, y %f, z %f : ll %f lll %f\n", xx,yy,zz,ll,lll);


		/* mix ball+light */
		red   = lll * lightColor.red() +   (1-lll) * bColor.red();
		green = lll * lightColor.green() + (1-lll) * bColor.green();
		blue  = lll * lightColor.blue() +  (1-lll) * bColor.blue();

		/* lightness */
		red   = .2 * bColor.red()   + .8 * ll * red;
		green = .2 * bColor.green() + .8 * ll * green;
		blue  = .2 * bColor.blue()  + .8 * ll * blue;

		image.setPixel(x,y, qRgb( (int)red,  (int)green, (int)blue ));
	    }
	}
    }
    const QImage iMask = image.createHeuristicMask();
    QBitmap bMask;
    bMask = QBitmap::fromImage( iMask );
    pm = QPixmap::fromImage( image );
    pm.setMask(bMask);
}


/* Class BallAnimation */

BallAnimation::BallAnimation(int s, Ball* ball1, Ball* ball2)
{
    QColor c1 = ball1->ballColor();
    double a1 = ball1->angle();
    int r1 = c1.red(), g1 = c1.green(), b1 = c1.blue();

    QColor c2 = ball2->ballColor();
    double a2 = ball2->angle();
    int r2 = c2.red(), g2 = c2.green(), b2 = c2.blue();

    QColor c;
    double a;
    int i;

    steps = s;
    s--;

    balls.append( new Ball( c1,a1 ) );

    for(i=1; i< s; i++) {
	c.setRgb( r1+(r2-r1)*i/s, g1+(g2-g1)*i/s, b1+(b2-b1)*i/s );
	a = a1+(a2-a1)*i/s;

	balls.append( new Ball( c,a ) );
    }

    balls.append( new Ball( c2,a2 ) );
}


/* Class BallPosition */
BallPosition::BallPosition(int xp,int yp, Ball* d)
{
    x=xp;
    y=yp;
    def=d;
    actStep = -1;
    actType = ANIMATION_STOPPED;
    actAnimation=0;
}


/*  Class BallWidget */

BallWidget::BallWidget( int _freq, int bFr, QWidget *parent )
    : QWidget(parent), positions(MAX_POSITION), animations(MAX_ANIMATION)
{
    int i;

    for(i=0;i<MAX_POSITION;i++)
	positions[i] = 0;

    for(i=0;i<MAX_ANIMATION;i++)
	animations[i] = 0;

    freq = _freq;
    isRunning = false;
    ballFraction = bFr;
    realSize = -1;
    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(animate()) );
}

BallWidget::~BallWidget()
{
    if (timer !=0)
	delete timer;
}

void BallWidget::createBlending(int no, int s, Ball* b1, Ball* b2)
{
    if (no<0 || no>= MAX_ANIMATION) return;

    if (animations[no] !=0)
	delete animations[no];

    animations[no] = new BallAnimation(s,b1,b2);
}


/* X, Y are coordinates in a virtual 1000x1000 area */
void BallWidget::createBallPosition(int no, int x, int y, Ball* def)
{
    if (no<0 || no>= MAX_POSITION) return;

    if (positions[no] !=0)
	delete positions[no];

    positions[no] = new BallPosition(x,y, def);
}

void BallWidget::startAnimation(int pos, int anim, int type)
{
    BallPosition *p;

    if (pos<0 || pos>=MAX_POSITION || positions[pos]==0) return;
    if (anim<0 || anim>=MAX_ANIMATION || animations[anim]==0) return;

    p = positions.at(pos);
    p->actAnimation = animations.at(anim);

    /* One step *BEFORE* start */
    p->actStep = -1;
    p->actDir = 1;
    p->actType = type;

    if (!isRunning) {
	isRunning = true;
	timer->setSingleShot(true);
	timer->start( 0 );
    }
}

/* If LOOP: Set to ONESHOT, otherwise set to last frame */
void BallWidget::stopAnimation(int pos)
{
    BallPosition *p;

    if (pos<0 || pos>=MAX_POSITION || positions[pos]==0) return;

    p = positions.at(pos);
    if (p->actType == ANIMATION_STOPPED ||
	p->actAnimation == 0) return;

    if (p->actType == ANIMATION_LOOP ||
	p->actType == ANIMATION_CYCLE) {
	p->actType = ANIMATION_FORWARD;
	//    return;
    }
    /* Set last step: animate() does the rest */
    p->actDir = 1;
    p->actStep = p->actAnimation->steps;
}

void BallWidget::resizeEvent(QResizeEvent *)
{
    int w = width() *10/12, h = height();

    realSize = (w>h) ? h:w;

    Ball::setSize( realSize/ballFraction, realSize/ballFraction );
    update();
}

void BallWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawBalls(&p);
}

void BallWidget::drawBalls(QPainter* painter)
{
    int i;
    BallPosition *pos;
    int xReal, yReal;

    int w = width(), h = height();

    if (realSize<0) return;

    for(i=0;i<MAX_POSITION;i++) {
	pos = positions.at(i);
	if (pos==0) continue;

	xReal = (w + pos->x * realSize / 500 - Ball::w() )/2;
	yReal = (h + pos->y * realSize / 500 - Ball::h() )/2;

	if (pos->actAnimation==0 || pos->actStep==-1) {
	    if (pos->def !=0 ) {
		painter->drawPixmap( xReal, yReal, *pos->def->pixmap() );
	    }
	}
	else {
	    int s = pos->actStep;
	    if (s>= pos->actAnimation->steps)
		s = pos->actAnimation->steps-1;
	    Ball* b = pos->actAnimation->balls.at(s);
	    painter->drawPixmap( xReal, yReal, *b->pixmap() );
	}
    }
}

void BallWidget::animate()
{
    bool doAnimation = false;

    int i;
    BallPosition *p;
    int xReal, yReal;

    int w = width(), h = height();

    for(i=0;i<MAX_POSITION;i++) {
	p = positions.at(i);
	if (p==0) continue;

	if (p->actType == ANIMATION_STOPPED ||
	    p->actAnimation ==0) continue;

	p->actStep += p->actDir;
	if (p->actStep <= -1) {
	    p->actDir = 1;
	    p->actStep = 1;
	    doAnimation = true;
	}
	else if (p->actStep >= p->actAnimation->steps) {
	    if (p->actType == ANIMATION_CYCLE) {
		p->actDir = -1;
		p->actStep = p->actAnimation->steps -2;
		doAnimation = true;
	    }
	    else if (p->actType == ANIMATION_LOOP) {
		p->actStep = 1; /*skip first frame for smooth animation */
		doAnimation = true;
	    }
	    else {
		p->actType = ANIMATION_STOPPED;
		p->actAnimation = 0;
		emit animationFinished(i);
	    }
	}
	else {
	    doAnimation = true;
	}

	xReal = (w + p->x * realSize / 500 - Ball::w() )/2;
	yReal = (h + p->y * realSize / 500 - Ball::h() )/2;

	if (p->actAnimation==0 || p->actStep==-1) {
	    if (p->def !=0 )
		update( xReal, yReal, Ball::w()+1, Ball::h()+1);
	}
	else
	    update( xReal, yReal, Ball::w()+1, Ball::h()+1);

    }
    if (!doAnimation) {
	isRunning = false;
	emit animationsFinished();
    }
    else {
	timer->setSingleShot(true);
	timer->start(1000/freq);
    }
}



/* Ball Test */


BallTest::BallTest( QWidget *parent )
    : BallWidget(10,2,parent)
{
    int w,h;

    w = h = 500;
    resize(w,h);
    //Ball::setSize( w/2, h/2, this );

    Ball *b1 = new Ball( Qt::green );
    Ball *b2 = new Ball( Qt::yellow );
    Ball *b3 = new Ball( Qt::red );
    Ball *b4 = new Ball( Qt::red, 3.14/2 );

    createBlending(0,5,b1,b2);
    createBallPosition( 0, -250, -250, b1);

    createBlending(1,10,b1,b3);
    createBallPosition(1, -250, 250, b1);

    createBlending(2,15,b3,b2);
    createBallPosition( 2, 250, -250, b3);

    createBlending(3,20,b3,b4);
    createBallPosition(3, 250, 250, b3);
}

/*
void BallTest::paintEvent( QPaintEvent * )
{
  bitBlt(this,0,0, b.pixmap());
}
*/

void BallTest::mousePressEvent( QMouseEvent * )
{
    startAnimation(0,0, ANIMATION_CYCLE);
    startAnimation(1,1);
    startAnimation(2,2);
    startAnimation(3,3, ANIMATION_LOOP);
}

void BallTest::mouseReleaseEvent( QMouseEvent * )
{
    stopAnimation(0);
    stopAnimation(1);
    stopAnimation(3);
}

/* Test... */

#include <QApplication>
int main(int argc, char *argv[])
{
    // set zoom, flip, limit
    Ball::setSpecials(.52, .85, .75);

    QApplication app(argc, argv);
    BallTest top;

    top.show();
    return app.exec();
}

