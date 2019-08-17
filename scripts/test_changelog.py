from changelog import find_first_header, ChangelogHeader, release, format_datetime
from textwrap import dedent
import datetime


def apply_inplace(text, fn, args):
    change = fn(changelog=text, args=args)
    if change:
        return text[:change.start] + change.replacement + text[change.end:]
    else:
        return text


def test_header_regex():
    actual = find_first_header(dedent("""
        * Sat Mar 02 2000 the author <mail@example.org> 0.1-1
    """))

    assert "Sat Mar 02 2000" == actual.date
    assert "the author" == actual.author
    assert "mail@example.org" == actual.email
    assert "0.1" == actual.version
    assert "1" == actual.release


def test_release():
    changelog = dedent("""
        * Sat Mar 02 2000 the author <mail@example.org> 0.1-1
    """)

    date = format_datetime(datetime.datetime.now())
    assert dedent("""
        * {} the author <mail@example.org> 0.1-1
    """.format(date)) == apply_inplace(changelog, release, [])

