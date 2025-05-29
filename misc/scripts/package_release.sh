#!/bin/bash

set -e
set -x

ARTIFACTS=${ARTIFACTS:-"artifacts"}
RELEASE=${RELEASE:-"release"}

mkdir -p ${RELEASE}/addons/godot-git-plugin/lib
ls -R ${RELEASE}
ls -R ${ARTIFACTS}

find "${ARTIFACTS}" -maxdepth 5 -wholename "*/lib/*" | xargs cp -r -t "${RELEASE}/addons/godot-git-plugin/lib/"
find "${ARTIFACTS}" -wholename "*/LICENSE*" | xargs cp -t "${RELEASE}/addons/godot-git-plugin"
cp misc/git_plugin.gdextension "${RELEASE}/addons/godot-git-plugin/"

CURDIR=$(pwd)
cd "${RELEASE}/"
# Clear unneded windows files
rm addons/godot-git-plugin/lib/*.pdb addons/godot-git-plugin/lib/*.exp addons/godot-git-plugin/lib/*.lib || echo "Nothing to delete"
cd "$CURDIR"

ls -R ${RELEASE}
