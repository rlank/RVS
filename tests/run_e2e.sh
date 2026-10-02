#!/usr/bin/env bash
# ==============================================================================
# RVS E2E Test Suite Launcher
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

export PYTHONPATH="${REPO_ROOT}:${SCRIPT_DIR}"

if command -v python3 &>/dev/null; then
    PYTHON_BIN="python3"
elif [ -f "${REPO_ROOT}/.venv/bin/python" ]; then
    PYTHON_BIN="${REPO_ROOT}/.venv/bin/python"
else
    echo "Error: Python 3 executable not found." >&2
    exit 1
fi

exec "${PYTHON_BIN}" "${SCRIPT_DIR}/e2e_runner.py" "$@"
