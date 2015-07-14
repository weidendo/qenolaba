# qenolaba
Board game with computer player and network connectivity


### Overview

This is a resurrection of Kenolaba from KDE 3.5.10 without KDE dependency.
It compiles both with Qt4 and Qt5.

The computer player does Alpha/Beta search with iterative deepening and
adaptive depth search. It does not use transposition tables.

Network connectivity works by exchanging updated board positions.
Multiple Qenolaba instances find each other when started on same system;
to connect instances on different machines, provide the host name of 
the machine the 1st instance is running as argument to the 2nd instance.


### Compile and Install

With qmake from Qt4 or Qt5 in $PATH, run

    qmake; make

To install the single resulting binary in /usr/local/bin, run

    make install

