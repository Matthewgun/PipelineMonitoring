QT += core gui network widgets

TARGET = PipelineMonitor
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    main.cpp \
    pipeline_monitor.cpp \
    pipeline_widget.cpp

HEADERS += \
    pipeline_monitor.h \
    pipeline_widget.h

FORMS += pipeline_monitor.ui  # Раскомментируйте если используете .ui файл

win32 {
    LIBS += -lws2_32
}

# Дополнительные настройки
DEFINES += QT_DEPRECATED_WARNINGS
