#!/bin/bash
ROOTDIR=$(git rev-parse --show-toplevel)
FILE="${ROOTDIR}/rgw-sts-profile"
if [ -f "$FILE" ]; then
  export $(grep -v '^#' $FILE | xargs -d '\n')
fi
echo "access:secret" > $HOME/.passwd-s3fs
chmod 600 $HOME/.passwd-s3fs
mkdir -p $HOME/.aws
printf '%s\n%s\n%s\n%s\n%s\n' \
  "[default]" \
  "aws_access_key_id = access_key" \
  "aws_secret_access_key = secret_key" \
  "aws_session_token = access_token" \
  ";aws_refresh_token = refresh_token" \
  > $HOME/.aws/credentials
chmod 600 $HOME/.aws/credentials
mkdir -p $HOME/mnt/rgw
s3fs $BUCKET_NAME $HOME/mnt/rgw \
  -o use_path_request_style \
  -o url=$ENDPOINT_URL \
  -o endpoint=$REGION_NAME \
  -o credlib=librgw-sts.so \
  -o use_cache=/tmp/s3fs \
  -o credlib_opts=Debug -f -d
