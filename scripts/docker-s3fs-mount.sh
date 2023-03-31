#!/bin/bash
ROOTDIR=$(git rev-parse --show-toplevel)
FILE="${ROOTDIR}/rgw-sts-profile"
if [ -f "$FILE" ]; then
  docker run --name=s3fs --rm \
           --net=host -ti \
           --device /dev/fuse \
           --cap-add SYS_ADMIN \
           --privileged \
           --env-file $FILE \
           ffornari/s3fs-rgw-sts
fi
