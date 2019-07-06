TS = $${_PRO_FILE_PWD_}/translations/$${TRANSLATION_TARGET}.ts

# absolute paths for sources
for (source, TRANSLATION_SOURCES) {
    _TRANSLATION_SOURCES += $${_PRO_FILE_PWD_}/$$source
}

# find translations
_TRANSLATIONS_IN = $$files($${_PRO_FILE_PWD_}/translations/$${TRANSLATION_TARGET}-*.ts)

# absolute paths for translation output
for (trans, _TRANSLATIONS_IN) {
    _TRANSLATIONS_OUT += $${OUT_PWD}/$$relative_path($$trans, $${_PRO_FILE_PWD_})
}

# qm target
qm.files = $$replace(_TRANSLATIONS_OUT, \.ts, .qm)
qm.path = /usr/share/$${TRANSLATION_TARGET}/translations
qm.CONFIG += no_check_exist
qm.commands += \
    lupdate -noobsolete $${_TRANSLATION_SOURCES} -ts $${TS} $$_TRANSLATIONS_IN && \
    mkdir -p $${OUT_PWD}/translations && \
    [ \"$${OUT_PWD}\" != \"$${_PRO_FILE_PWD_}\" ] && \
    cp -af $${_TRANSLATIONS_IN} $${OUT_PWD}/translations ; \
    lrelease $${_TRANSLATIONS_OUT}

INSTALLS += qm
QMAKE_EXTRA_TARGETS += qm

DISTFILES += $${TS} $${_TRANSLATIONS_IN}
