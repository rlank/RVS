#!/usr/bin/env python3
"""
Entry point for RVS E2E test runner.
Delegates to tests/e2e_runner.py.
"""

import os

if __name__ == "__main__":
    runner_path = os.path.join(os.path.dirname(__file__), "e2e_runner.py")
    # Execute e2e_runner.py with all CLI arguments
    with open(runner_path) as f:
        code = compile(f.read(), runner_path, "exec")
        exec(code, {"__name__": "__main__", "__file__": runner_path})
