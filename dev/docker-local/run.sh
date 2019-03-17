#!/bin/bash -e

IMAGE_NAME="sailfishconnect-buildenv-local"

SCRIPT_DIR=`dirname "${BASH_SOURCE[0]}"`
SDK=`$SCRIPT_DIR/get-sdk-install-folder`

docker run --rm -it \
    --cap-add SYS_PTRACE \
    --network=host \
    --volume "/tmp:/home/mersdk/tmp" \
    --volume "$HOME:/home/mersdk/share" \
    --volume "$HOME:/home/src1" \
    --volume "$SDK/mersdk/ssh:/etc/ssh/authorized_keys" \
    --volume "$SDK/mersdk/targets:/host_targets" \
    --volume "$SDK/vmshare:/etc/mersdk/share" \
    "$IMAGE_NAME" \
    "$@"
