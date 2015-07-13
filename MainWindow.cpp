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

#include "MainWindow.h"
#include "BoardWidget.h"
#include "Network.h"

#include <QStatusBar>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(Network *n)
{
#ifdef QT_DBUS_SUPPORT
    QDBusConnection con = QDBusConnection::sessionBus();
    con.registerObject("/Qenolaba", this,
		       QDBusConnection::ExportScriptableSlots);
#endif

    createActions();
    createMenu();
    createToolbar();

    _statusbar = statusBar();
    _statusLabel = new QLabel(_statusbar);
    _statusbar->addWidget(_statusLabel, 1);

    _computerDepth = 3;
    _network = n;
    _board = new Board();
    _board->setDepth(_computerDepth);
    _boardWidget = new BoardWidget(*_board);
    _boardWidget->renderPieces(true);
    setCentralWidget(_boardWidget);

    connect(_boardWidget, SIGNAL(moveChoosen(Move&)),
	    SLOT(draw(Move&)));
    if (n)
	connect(n, SIGNAL(gotPosition(const char*)),
		SLOT(newPosition(const char*)));

    setWindowIcon(QIcon(":/app.png"));
    updateStatus();
}

void MainWindow::createActions()
{
    // file menu actions
    _newAction = new QAction(tr("&New Game"), this);
    _newAction->setShortcuts(QKeySequence::New);
    _newAction->setStatusTip(tr("Start new game"));
    connect(_newAction, SIGNAL(triggered()), this, SLOT(newGame()));

    _quitAction = new QAction(tr("&Quit"), this);
    _quitAction->setShortcut(tr("Ctrl+Q"));
    _quitAction->setStatusTip(tr("Quit the application"));
    connect(_quitAction, SIGNAL(triggered()), this, SLOT(close()));

    _startAction = new QAction(tr("Red &starts game"), this);
    _startAction->setCheckable(true);
    _startAction->setChecked(true);
    _startAction->setShortcut(tr("Ctrl+S"));
    _startAction->setStatusTip(tr("Red side about to start the game"));

    _redAction = new QAction(tr("Computer plays &Red"), this);
    _redAction->setCheckable(true);
    _redAction->setShortcut(tr("Ctrl+R"));
    _redAction->setStatusTip(tr("Red side played by computer"));

    _yellowAction = new QAction(tr("Computer plays &Yellow"), this);
    _yellowAction->setCheckable(true);
    _yellowAction->setChecked(true);
    _yellowAction->setShortcut(tr("Ctrl+Y"));
    _yellowAction->setStatusTip(tr("Yellow side played by computer"));

    _d1Action = new QAction(tr("Level &1: Weak"), this);
    _d1Action->setCheckable(true);
    _d2Action = new QAction(tr("Level &2: Medium"), this);
    _d2Action->setCheckable(true);
    _d3Action = new QAction(tr("Level &3: Strong"), this);
    _d3Action->setCheckable(true);
    _d4Action = new QAction(tr("Level &4: Challange"), this);
    _d4Action->setCheckable(true);

    _depthGroup = new QActionGroup(this);
    _depthGroup->addAction(_d1Action);
    _depthGroup->addAction(_d2Action);
    _depthGroup->addAction(_d3Action);
    _depthGroup->addAction(_d4Action);
    QAction* a = _d2Action;
    switch(_computerDepth) {
    case 2: a = _d1Action; break;
    case 4: a = _d3Action; break;
    case 5: a = _d4Action; break;
    }
    a->setChecked(true);
    connect(_depthGroup, SIGNAL(triggered(QAction*)),
	    SLOT(depthSet(QAction*)));

    // help menu actions
    _aboutAction = new QAction(tr("&About Qenolaba..."), this);
    _aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

}

void MainWindow::createMenu()
{
    QMenuBar* mBar = menuBar();

    QMenu* fileMenu = mBar->addMenu(tr("&File"));
    fileMenu->addAction(_newAction);
    fileMenu->addSeparator();
    fileMenu->addAction(_quitAction);

    QMenu* optionMenu = mBar->addMenu(tr("&Options"));
    optionMenu->addAction(_startAction);
    optionMenu->addAction(_redAction);
    optionMenu->addAction(_yellowAction);
    optionMenu->addSeparator();
    optionMenu->addAction(_d1Action);
    optionMenu->addAction(_d2Action);
    optionMenu->addAction(_d3Action);
    optionMenu->addAction(_d4Action);

    QMenu* helpMenu = mBar->addMenu(tr("&Help"));
    helpMenu->addAction(_aboutAction);
}

void MainWindow::createToolbar()
{
    QToolBar* tb = new QToolBar(tr("Main Toolbar"), this);
    tb->setObjectName("main-toolbar");
    addToolBar(Qt::TopToolBarArea, tb);

    tb->addAction(_newAction);
    tb->addSeparator();
}


void MainWindow::initInput()
{
    qDebug("%s", qPrintable(_board->getState()));

    if (!updateStatus()) {
	_boardWidget->updatePosition(true);
	return;
    }

    if (((_board->actColor() == Board::color1) && _redAction->isChecked()) ||
	((_board->actColor() == Board::color2) && _yellowAction->isChecked())) {

	_moveToDraw = _board->bestMove();
	QTimer::singleShot(50, this, SLOT(draw()));
    }
    else {
	_board->generateMoves(_moveList);
	_boardWidget->choseMove(&_moveList);
    }
    _boardWidget->updatePosition(true);
}

bool MainWindow::updateStatus()
{
    if (_board->getColor1Count() == 0)
	_statusLabel->setText(tr("Game not started yet"));
    else if (_board->getColor1Count()<9)
	_statusLabel->setText(tr("Yellow wins the game"));
    else if (_board->getColor2Count()<9)
	_statusLabel->setText(tr("Red wins the game"));
    else if (_board->actColor() == Board::color1) {
	_statusLabel->setText(tr("Red about to move"));
	return true;
    }
    else if (_board->actColor() == Board::color2) {
	_statusLabel->setText(tr("Yellow about to move"));
	return true;
    }
    return false;
}

void MainWindow::newGame()
{
    _board->begin(_startAction->isChecked() ? Board::color1 : Board::color2);
    if (_network) _network->broadcast(_board);
    initInput();
}

void MainWindow::about()
{
    QString text, version;
    version = QLatin1String("0.1");
    text = QString("<h3>Qenolaba %1</h3>").arg(version);
    text += tr("<p>Qenolaba is inspired by a famous board game with "
	       "hexagonal grid. It has a computer player and network "
	       "connectivity (just try to run it twice).</p>"
	       "<p>Qenolaba is open-source, and it is distributed under the "
	       "terms of the GPL v2.</p>"
	       "Author: "
	       "<a href=\"mailto:Josef.Weidendorfer@gmx.de\">"
	       "Josef Weidendorfer</a>");

    QMessageBox::about(this, tr("About Qenolaba"), text);
}

void MainWindow::depthSet(QAction* a)
{
    if (a == _d1Action) _computerDepth = 2;
    if (a == _d2Action) _computerDepth = 3;
    if (a == _d3Action) _computerDepth = 4;
    if (a == _d4Action) _computerDepth = 5;
    _board->setDepth(_computerDepth);
}

void MainWindow::draw(Move& m)
{
    if (m.isValid()) {
	_board->playMove(m);
	if (_network) _network->broadcast(_board);
    }
    initInput();
}

void MainWindow::draw()
{
    if (_moveToDraw.isValid()) {
	_board->playMove(_moveToDraw);
	if (_network) _network->broadcast(_board);
    }
    initInput();
}


void MainWindow::newPosition(const char* p)
{
    QString s(p);
    _board->setState(s);
    qDebug("Got new position from network...");
    initInput();
}

