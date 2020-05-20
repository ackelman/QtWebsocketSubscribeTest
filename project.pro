QT = websockets

TARGET = server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    server.cpp

HEADERS += \
    server.h

