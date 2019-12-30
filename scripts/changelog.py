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

import argparse
import sys
import re
import locale
import datetime
from pathlib import Path
from collections import namedtuple
from typing import Match

CHANGELOG_FILE = (
    Path(__file__).parent.parent / "rpm" / "harbour-sailfishconnect.changes"
)
HEADER_RE = re.compile(
    r"^\* (?P<date>\w+ \w+ \d+ \d+) (?P<author>[^<]+) <(?P<email>[^>]+)> (?P<version>[\w.~]+)-(?P<release>\d+)",
    re.M,
)


class ChangelogHeader:
    def __init__(self, match: Match):
        self.match = match

    @property
    def date(self):
        return self.match.group("date")

    @property
    def author(self):
        return self.match.group("author")

    @property
    def email(self):
        return self.match.group("email")

    @property
    def version(self):
        return self.match.group("version")

    @property
    def release(self):
        return self.match.group("release")

    @property
    def date_loc(self):
        return (self.match.start("date"), self.match.end("date"))

    @property
    def author_loc(self):
        return (self.match.start("author"), self.match.end("author"))

    @property
    def email_loc(self):
        return (self.match.start("email"), self.match.end("email"))

    @property
    def version_loc(self):
        return (self.match.start("version"), self.match.end("version"))

    @property
    def release_loc(self):
        return (self.match.start("release"), self.match.end("release"))


class Change(namedtuple("Change", ["start", "end", "replacement"])):
    pass



def find_first_header(changelog: str) -> ChangelogHeader:
    match = HEADER_RE.search(changelog)
    if not match:
        raise RuntimeError("No changelog entry found. Right format?")

    return ChangelogHeader(match)


def format_datetime(datetime):
    locale.setlocale(locale.LC_ALL, "C")
    return datetime.strftime("%a %b %e %Y")


def apply_commandfn(fn, args):
    with CHANGELOG_FILE.open("r") as fp:
        changelog = fp.read()

    change = fn(changelog=changelog, args=args)

    if change:
        with CHANGELOG_FILE.open("w") as fp:
            fp.write(changelog[:change.start])
            fp.write(change.replacement)
            fp.write(changelog[change.end:])


def release(changelog: str, args) -> Change:
    header = find_first_header(changelog)
    return Change(
        start=header.date_loc[0],
        end=header.date_loc[1],
        replacement=format_datetime(datetime.datetime.now())
    )


COMMANDS = {"release": release}


def main():
    argparser = argparse.ArgumentParser(description="Edit changelog")
    argparser.add_argument("command", choices=COMMANDS.keys(), help="One of " + ", ".join(COMMANDS.keys()))
    argparser.add_argument("args", nargs=argparse.REMAINDER)
    args = argparser.parse_args()

    commandfn = COMMANDS.get(args.command)
    if commandfn is None:
        print(
            "Unknown command: "
            + args.command
            + ". Use one of "
            + ", ".join(COMMANDS.keys())
            + ".",
            file=sys.stderr,
        )
        sys.exit(1)

    apply_commandfn(commandfn, args.args)



if __name__ == "__main__":
    main()
