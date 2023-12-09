#!/bin/bash
ROOTDIR=$(git rev-parse --show-toplevel)
FILE="${ROOTDIR}/profiles/rgw-sts-profile-$1"
if [ -f "$FILE" ]; then
  docker run --name=s3fs-client$1 --rm \
           --net=host -d \
           --device /dev/fuse \
           --cap-add SYS_ADMIN \
           --privileged \
           --env-file $FILE \
           -v "${ROOTDIR}/certs/certs-client$1/private.key":/home/docker/private.key \
           -v "${ROOTDIR}/certs/certs-client$1/public.crt":/home/docker/public.crt \
           ffornari/s3fs-rgw-iam:auth-code-ubuntu \
           sh -c \
           "s3fs \$BUCKET_NAME \$HOME/mnt/rgw \
           -o use_cache=/tmp \
           -o use_path_request_style \
           -o url=\$ENDPOINT_URL \
           -o endpoint=\$REGION_NAME \
           -o credlib=librgw-sts.so \
           -o credlib_opts=Info -f"
fi
