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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QActionGroup>

#include "Move.h"

class Board;
class BoardWidget;
class MoveList;
class Network;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "io.github.qenolaba")

public:
  MainWindow(Network* n = 0);

  void createActions();
  void createMenu();
  void createToolbar();

public slots:
  void newGame();
  void depthSet(QAction*);
  void draw(Move& m);
  void draw();
  void newPosition(const char*);

private:
  void initInput();
  bool updateStatus();

  int _computerDepth;
  Move _moveToDraw;

  QAction *_newAction, *_quitAction, *_startAction;
  QAction *_redAction, *_yellowAction;
  QAction *_d1Action, *_d2Action, *_d3Action, *_d4Action;
  QActionGroup* _depthGroup;
  QStatusBar* _statusbar;
  QLabel* _statusLabel;

  MoveList _moveList;
  Board* _board;
  BoardWidget* _boardWidget;
  Network* _network;
};


#endif // MAINWINDOW_H

