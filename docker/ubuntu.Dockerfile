FROM ffornari/s3fs-rgw-dependencies:ubuntu
ENV LD_LIBRARY_PATH=/usr/local/lib
RUN git clone https://baltig.infn.it/fornari/s3fs-rgw-iam-lib.git && \
    cd s3fs-rgw-iam-lib && \
    git checkout authorization_code && \
    cmake -S . -B build && \
    cd build && make install && cd ../.. && \
    rm -rf s3fs-rgw-iam-lib
RUN adduser --disabled-password --gecos '' docker
USER docker
RUN mkdir -p $HOME/.aws && \
    printf '%s\n%s\n%s\n%s\n%s\n' \
    "[default]" \
    "aws_access_key_id = access_key" \
    "aws_secret_access_key = secret_key" \
    "aws_session_token = access_token" \
    ";aws_refresh_token = refresh_token" \
    > $HOME/.aws/credentials && \
    chmod 600 $HOME/.aws/credentials && \
    mkdir -p $HOME/mnt/rgw && \
    echo "access:secret" > $HOME/.passwd-s3fs && \
    chmod 600 $HOME/.passwd-s3fs
CMD s3fs $BUCKET_NAME $HOME/mnt/rgw \
  -o use_path_request_style \
  -o url=$ENDPOINT_URL \
  -o endpoint=$REGION_NAME \
  -o credlib=librgw-sts.so \
  -o credlib_opts=Info -f
