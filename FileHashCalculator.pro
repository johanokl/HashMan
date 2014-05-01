QT += widgets \
  core

HEADERS     = gui/mainwindow.h \
    hashproject/sourcedirectory.h \
    gui/sourcedirectorywidget.h \
    gui/menuactions.h \
    hashproject/hashproject.h \
    algorithms/crc32algorithm.h \
    algorithms/hashalgorithm.h \
    workers/filefinder.h \
    workers/hasher.h \
    hashproject/filelist.h \
    gui/filedrop.h \
    gui/statusboxwidget.h \
    algorithms/qtcryptoalgorithms.h \
    hashcalcapplication.h

SOURCES     = gui/mainwindow.cpp \
              main.cpp \
    hashproject/sourcedirectory.cpp \
    gui/sourcedirectorywidget.cpp \
    gui/menuactions.cpp \
    hashproject/hashproject.cpp \
    workers/filefinder.cpp \
    workers/hasher.cpp \
    hashproject/filelist.cpp \
    algorithms/crc32algorithm.cpp \
    gui/filedrop.cpp \
    gui/statusboxwidget.cpp \
    algorithms/qtcryptoalgorithms.cpp \
    hashcalcapplication.cpp

TARGET = "File Hash Calculator"

QMAKE_INFO_PLIST += Info.plist
OTHER_FILES += Info.plist

RESOURCES += FileHashCalculator.qrc
RC_ICONS += images/mainicon.ico

ICON += images/mainicon.icns

