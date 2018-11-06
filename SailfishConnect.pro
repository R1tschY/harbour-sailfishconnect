TEMPLATE = subdirs

SUBDIRS = app lib tests

app.depends = lib
