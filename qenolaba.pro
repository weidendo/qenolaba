TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

HEADERS += Move.h Board.h \
    Piece.h EvalScheme.h

SOURCES += Move.cpp Board.cpp \
    Piece.cpp EvalScheme.cpp
