TARGET = "HashMan"

macx {
  # Include spaces in the file name if on Mac OS X.
  TARGET = "HashMan"
  QMAKE_INFO_PLIST += Info.plist
  QMAKE_MAC_SDK = 10.13
  OTHER_FILES += Info.plist
}

QT += widgets \
    gui \
    core

HEADERS = hashcalcapplication.h \
    hashproject/sourcedirectory.h \
    hashproject/hashproject.h \
    hashproject/filelist.h \
    gui/sourcedirectorywidget.h \
    gui/menuactions.h \
    gui/mainwindow.h \
    gui/filedrop.h \
    gui/statusboxwidget.h \
    workers/filefinder.h \
    workers/hasher.h \
    algorithms/crc32algorithm.h \
    algorithms/hashalgorithm.h \
    algorithms/qtcryptoalgorithms.h


SOURCES = hashcalcapplication.cpp \
    main.cpp \
    hashproject/sourcedirectory.cpp \
    hashproject/filelist.cpp \
    hashproject/hashproject.cpp \
    gui/sourcedirectorywidget.cpp \
    gui/mainwindow.cpp \
    gui/menuactions.cpp \
    gui/filedrop.cpp \
    gui/statusboxwidget.cpp \
    workers/filefinder.cpp \
    workers/hasher.cpp \
    algorithms/crc32algorithm.cpp \
    algorithms/qtcryptoalgorithms.cpp

RESOURCES += HashMan.qrc
RC_ICONS += images/mainicon.ico
ICON += images/mainicon.icns
