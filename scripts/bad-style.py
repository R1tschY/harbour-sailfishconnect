#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
from pathlib import Path
from fnmatch import fnmatch
import xml.etree.ElementTree as ET


class Failure:

    def __init__(
            self,
            file: Path,
            location: "Union[None, int, Tuple[int, int]]",
            message: str):
        self.file = file
        self.location = location
        self.message = message

    def print(self):
        if self.location is None:
            print(f"{self.file}: error: {self.message}")

        if isinstance(self.location, int):
            content = open(self.file, "r", encoding="utf-8").read()
            startLine = content.rfind('\n', None, self.location) + 1
            endLine = content.find('\n', self.location, None)
            if endLine == -1:
                endLine = len(content)
            print(f"{self.file}: error: {self.message}")
            print(" " + content[startLine:endLine])
            print(" " * (self.location - startLine + 1) + "^")

        if isinstance(self.location, tuple):
            content = open(self.file, "r", encoding="utf-8").read()
            startLine = content.rfind('\n', None, self.location[0]) + 1
            endLine = content.find('\n', self.location[0], None)
            if endLine == -1:
                endLine = len(content)
            print(f"{self.file}: error: {self.message}")
            print(" " + content[startLine:endLine])
            print(" " * (self.location[0] - startLine + 1) + "^" * (self.location[1] - self.location[0]))



class Rule:
    def canHandle(self, file: Path) -> bool:
        return True

    def handleFile(self, file: Path, content: str) -> "Iterable[Failure]":
        raise NotImplementedError()


class MatchRegexRule:

    def __init__(self, regex: str, files: str = None, message: str = None):
        self.regex = re.compile(regex)
        self.files = files
        self.message = message

    def canHandle(self, file: Path):
        return fnmatch(file.name, self.files)

    def handleFile(self, file: Path, content: str):
        if not self.regex.match(content):
            return [Failure(file, None, self.message)]
        else:
            return []


class DontMatchRegexRule:

    def __init__(self, regex: str, *, files: str = None, message: str = None):
        self.regex = re.compile(regex)
        self.files = files
        self.message = message

    def canHandle(self, file: Path):
        return fnmatch(file.name, self.files)

    def handleFile(self, file: Path, content: str):
        return [
            Failure(file, match.span(0), self.message)
            for match in self.regex.finditer(content)
        ]

registry = {
    "match-regex": MatchRegexRule,
    "dont-match-regex": DontMatchRegexRule,
}


def main():
    # read config
    config = sys.argv[1]
    if not os.path.exists(config):
        print(f"{config} does not exist")
        sys.exit(1)

    tree = ET.parse(config)
    root = tree.getroot()
    assert root.tag == "bad-style"

    # create rules
    rules = []
    for element in root:
        if element.text is None:
            rules.append(registry[element.tag](**element.attrib))
        else:
            rules.append(registry[element.tag](
                element.text.strip(), **element.attrib))

    # apply
    failures = []
    matchedFiles = 0
    for root, dirs, files in os.walk('.'):
        for file in files:
            content = None
            path = Path(root) / file
            for rule in rules:
                if rule.canHandle(path):
                    if content is None:
                        matchedFiles += 1
                        content = open(path, "r", encoding="utf-8").read()

                    failures.extend(rule.handleFile(path, content))

    # result
    print(f"matched files {matchedFiles}")
    if failures:
        print(f"\n{len(failures)} Failures:")
        for failure in failures:
            failure.print()

        sys.exit(5)


if __name__ == '__main__':
    main()
