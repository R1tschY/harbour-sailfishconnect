#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from changelog import find_first_header, ChangelogHeader, release, format_datetime
from textwrap import dedent
import datetime


def apply_inplace(text, fn, *args, **kwargs):
    change = fn(changelog=text, args=args, **kwargs)
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

    assert dedent("""
        * Fri Jan 03 2020 the author <mail@example.org> 0.1-1
    """) == apply_inplace(changelog, release, date=datetime.datetime(2020, 1, 3))

