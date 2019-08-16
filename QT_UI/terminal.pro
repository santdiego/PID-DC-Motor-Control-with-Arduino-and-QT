QT += widgets serialport
QT += printsupport
requires(qtConfig(combobox))
CONFIG += static
TARGET = terminal
TEMPLATE = app

SOURCES += \
    ../serialframedecoder.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    motorcontrol.cpp \
    ../QCustomPlot/qcustomplot.cpp \
    axistag.cpp

HEADERS += \
    ../serialframedecoder.h \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    motorcontrol.h \
    ../QCustomPlot/qcustomplot.h \
    axistag.h

INCLUDEPATH +=../
INCLUDEPATH += QCustomPlot/
FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    motorcontrol.ui

RESOURCES += \
    resources.qrc


