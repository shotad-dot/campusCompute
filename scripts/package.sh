#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
# Submission archive contains source/build materials, not generated log evidence or binaries.
zip -q -r CampusCompute_Submission.zip src include tests scripts Makefile README.md SCREENSHOT_GUIDE.md -x '*/.*' 'src/*.o'
echo "archive created: CampusCompute_Submission.zip"
unzip -l CampusCompute_Submission.zip
unzip -t CampusCompute_Submission.zip
