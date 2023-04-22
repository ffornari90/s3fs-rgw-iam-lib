#!/bin/bash
docker build -t ffornari/s3fs-rgw-dependencies -f dependencies.Dockerfile .
docker build -t ffornari/s3fs-rgw-iam:auth-code -f Dockerfile .
