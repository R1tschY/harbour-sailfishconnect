TEMPLATE = subdirs

SUBDIRS = lib app tests

app.depends = lib
tests.depends = lib

# Translations

TRANSLATION_TARGET = harbour-sailfishconnect
TRANSLATION_SOURCES = app/src app/qml lib/sailfishconnect

include(translations/translations.pri)

# Other files
DISTFILES += *.md



