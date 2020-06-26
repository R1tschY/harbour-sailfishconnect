#!/usr/bin/env python3

from argparse import ArgumentParser
from pathlib import Path
import json
import os


HOME = os.path.expanduser("~/")


def map_home_dir(entry):
    return entry.replace("/home/mersdk/share/", HOME)


def map_entry(entry):
    return {
        key: map_home_dir(value) for key, value in entry.items()
    }


def main():
    with open("./rpmbuilddir/compile_commands.json", "rb") as fp:
        compile_commands = json.load(fp)

    compile_commands = [
        map_entry(entry) for entry in compile_commands
    ]

    with open("./compile_commands.json", "w") as fp:
        json.dump(compile_commands, fp, indent=2)


if __name__ == "__main__":
    main()