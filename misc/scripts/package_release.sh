#!/bin/bash

set -e
set -x

ARTIFACTS=${ARTIFACTS:-"artifacts"}
DESTINATION=${DESTINATION:-"release"}
NAME=${NAME:-"godot-git-plugin"}

mkdir -p ${DESTINATION}
ls -R ${DESTINATION}
ls -R ${ARTIFACTS}

DESTDIR="${DESTINATION}/addons/${NAME}"

mkdir -p ${DESTDIR}/lib

find "${ARTIFACTS}" -maxdepth 5 -wholename "*/addons/${NAME}/lib/*" | xargs cp -r -t "${DESTDIR}/lib/"
find "${ARTIFACTS}" -wholename "*/LICENSE*" | xargs cp -t "${DESTDIR}/"
find "${ARTIFACTS}" -wholename "*/addons/${NAME}/*.gdextension" | head -n 1 | xargs cp -t "${DESTDIR}/"

CURDIR=$(pwd)
cd "${DESTINATION}/"
zip -r ${NAME}.zip addons
cd "$CURDIR"

ls -R ${DESTINATION}
