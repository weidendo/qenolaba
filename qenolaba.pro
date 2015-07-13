TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

RESOURCES = qenolaba.qrc

HEADERS += Move.h Board.h EvalScheme.h  \
    Piece.h BoardWidget.h Network.h \
    MainWindow.h

SOURCES += Move.cpp Board.cpp EvalScheme.cpp \
    Piece.cpp BoardWidget.cpp Network.cpp \
    MainWindow.cpp main.cpp
