TEMPLATE = subdirs

SUBDIRS = lib tests
tests.depends = lib

packagesExist(sailfishapp) {
    SUBDIRS += app
    app.depends = lib
}

# Translations

TRANSLATION_TARGET = harbour-sailfishconnect
TRANSLATION_SOURCES = app/src app/qml lib/sailfishconnect

include(translations/translations.pri)

# Other files
DISTFILES += *.md



