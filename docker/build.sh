#!/bin/bash
docker build -t ffornari/s3fs-rgw-dependencies:ubuntu -f dependencies-ubuntu.Dockerfile .
docker build -t ffornari/s3fs-rgw-dependencies:alpine -f dependencies-alpine.Dockerfile .
docker build -t ffornari/s3fs-rgw-iam:auth-code-ubuntu -f ubuntu.Dockerfile .
docker build -t ffornari/s3fs-rgw-iam:auth-code-alpine -f alpine.Dockerfile .
