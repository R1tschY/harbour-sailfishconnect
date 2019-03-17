#!/bin/bash -e

SCRIPT_DIR=`dirname "${BASH_SOURCE[0]}"`

docker build \
    --build-arg local_uid=$(id -u $USER) \
    --build-arg local_gid=$(id -g $USER) \
    -t sailfishconnect-buildenv-local \
    "$SCRIPT_DIR"
