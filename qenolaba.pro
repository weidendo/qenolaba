TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

HEADERS += Move.h Board.h EvalScheme.h  \
    Piece.h BoardWidget.h Network.h

SOURCES += Move.cpp Board.cpp EvalScheme.cpp \
    Piece.cpp BoardWidget.cpp Network.cpp
