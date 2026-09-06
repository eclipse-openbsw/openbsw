#!/usr/bin/env bash
# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************
#
# Build S32K148 GCC size snapshots for a revision and its baseline, then report
# their ROM, RAM, binary, and section deltas.
#
# Usage: ws/s32k148-gcc.delta.sh [--baseline BASELINE] [REF]
#   no REF or "."         compare the working tree with HEAD
#   "+"                   compare the Git index with HEAD
#   REF                   compare a commit with its first parent
#   --baseline BASELINE   compare against BASELINE instead of the default

set -euo pipefail

usage() {
    printf 'usage: %s [--baseline BASELINE] [REF]\n' "$(basename "$0")" >&2
}

baseline_ref=""
input_ref=""
while (($# > 0)); do
    case "$1" in
        "--baseline")
            if (($# < 2)) || [[ -n "${baseline_ref}" ]]; then
                usage
                exit 2
            fi
            baseline_ref="$2"
            shift 2
            ;;
        "--")
            shift
            if (($# > 1)) || { (($# == 1)) && [[ -n "${input_ref}" ]]; }; then
                usage
                exit 2
            fi
            if (($# == 1)); then
                input_ref="$1"
            fi
            break
            ;;
        -*)
            usage
            exit 2
            ;;
        *)
            if [[ -n "${input_ref}" ]]; then
                usage
                exit 2
            fi
            input_ref="$1"
            shift
            ;;
    esac
done
input_ref="${input_ref:-.}"
repository_root="$(git rev-parse --show-toplevel)"
cd "${repository_root}"

case "${input_ref}" in
    ".")
        default_baseline_ref="HEAD"
        temporary_directory="$(mktemp -d)"
        trap 'rm -rf "${temporary_directory}"' EXIT
        temporary_index="${temporary_directory}/index"
        GIT_INDEX_FILE="${temporary_index}" git read-tree HEAD
        GIT_INDEX_FILE="${temporary_index}" git add --all
        current_tree="$(GIT_INDEX_FILE="${temporary_index}" git write-tree)"
        ;;
    "+")
        default_baseline_ref="HEAD"
        current_tree="$(git write-tree)"
        ;;
    *)
        commit="$(git rev-parse --verify "${input_ref}^{commit}")"
        default_baseline_ref="${commit}^1"
        current_tree="$(git rev-parse "${commit}^{tree}")"
        ;;
esac

baseline_ref="${baseline_ref:-${default_baseline_ref}}"
baseline_commit="$(git rev-parse --verify "${baseline_ref}^{commit}")"
baseline_tree="$(git rev-parse "${baseline_commit}^{tree}")"

source_filter="$(<ws/size-delta/s32k148-gcc-source.josh)"
compose_filter=":[baseline=:\$input=${baseline_tree}:/input${source_filter},current=:\$input=${current_tree}:/input${source_filter},::docker/development/,::ws/common/dev-image.josh,::ws/size-delta/]:+ws/size-delta/s32k148-gcc"

JOSH_EXPERIMENTAL_FEATURES=1 \
    josh compose run . "${compose_filter}"
