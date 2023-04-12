#!/bin/bash
ROOTDIR=$(git rev-parse --show-toplevel)
FILE="${ROOTDIR}/rgw-sts-profile"
if [ -f "$FILE" ]; then
  docker run --name=s3fs-client$1 --rm \
           --net=host -ti \
           --device /dev/fuse \
           --cap-add SYS_ADMIN \
           --privileged \
           --env-file $FILE \
           ffornari/s3fs-rgw-iam \
           sh -c \
           "s3fs \$BUCKET_NAME \$HOME/mnt/rgw \
           -o use_cache=/tmp \
           -o use_path_request_style \
           -o url=\$ENDPOINT_URL \
           -o endpoint=\$REGION_NAME \
           -o credlib=librgw-sts.so \
           -o credlib_opts=Info -f"
fi
