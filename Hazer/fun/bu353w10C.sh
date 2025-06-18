#!/bin/bash
# Copyright 2021-2025 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Like the bu353w10 script but reads a concatenation of stored data files.

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

export LC_ALL=en_US.UTF-8

uudecode $(readlink -e $(dirname ${0})/../../../dat)/hazer/bu353w10-*.u64 | coreable gpstool -R
