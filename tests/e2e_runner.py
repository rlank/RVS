#!/usr/bin/env python3
"""
RVS Opaque-Box Requirement-Driven E2E Test Suite Runner.
Executes test tiers, collects performance and outcome metrics,
outputs human-readable tabular results and writes machine-readable JSON summaries.
"""

import argparse
import json
import os
import sys
import time
import unittest
from datetime import UTC, datetime

# Ensure project root and tests/ are in PYTHONPATH
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)


class E2ETestResult(unittest.TestResult):
    """Custom TestResult capturing per-test execution duration and outcome."""

    def __init__(self):
        super().__init__()
        self.test_records = []
        self._current_start_time = None

    def startTest(self, test):
        super().startTest(test)
        self._current_start_time = time.perf_counter()

    def addSuccess(self, test):
        super().addSuccess(test)
        start = self._current_start_time or time.perf_counter()
        duration = time.perf_counter() - start
        self._record_outcome(test, "PASS", duration, None)

    def addFailure(self, test, err):
        super().addFailure(test, err)
        start = self._current_start_time or time.perf_counter()
        duration = time.perf_counter() - start
        self._record_outcome(test, "FAIL", duration, str(err[1]))

    def addError(self, test, err):
        super().addError(test, err)
        start = self._current_start_time or time.perf_counter()
        duration = time.perf_counter() - start
        self._record_outcome(test, "ERROR", duration, str(err[1]))

    def addSkip(self, test, reason):
        super().addSkip(test, reason)
        start = self._current_start_time or time.perf_counter()
        duration = time.perf_counter() - start
        self._record_outcome(test, "SKIP", duration, reason)

    def _record_outcome(self, test, status, duration, message):
        test_id = (
            test.id()
            if hasattr(test, "id")
            else f"{test.__class__.__name__}.{test._testMethodName}"
        )
        parts = test_id.split(".")
        tier = "unknown"
        module_name = "unknown"
        for part in parts:
            if part.startswith("test_tier"):
                tier = part
            elif (
                part.startswith("test_f")
                or part.startswith("test_b")
                or part.startswith("test_c")
                or part.startswith("test_app")
                or part.startswith("test_adv")
            ):
                module_name = part

        test_name = parts[-1]
        self.test_records.append(
            {
                "test_id": test_id,
                "tier": tier,
                "module": module_name,
                "test_name": test_name,
                "status": status,
                "duration_seconds": round(duration, 5),
                "error_message": message,
            }
        )


def _resolve_selected_dirs(tiers: list[str] | None, tier_dirs: dict[str, str]) -> list[str]:
    if not tiers:
        return list(tier_dirs.values())
    selected = []
    for t in tiers:
        t_key = t.lower().replace("-", "").replace("_", "")
        if t_key in tier_dirs:
            selected.append(tier_dirs[t_key])
        elif t in tier_dirs.values():
            selected.append(t)
    return selected


def discover_tests(
    tests_dir: str,
    tiers: list[str] | None = None,
    features: list[int] | int | None = None,
) -> unittest.TestSuite:
    """Discovers test suites filtered by selected tiers or feature numbers."""
    suite = unittest.TestSuite()
    tier_dirs = {
        "tier1": "test_tier1_features",
        "tier2": "test_tier2_boundaries",
        "tier3": "test_tier3_combinations",
        "tier4": "test_tier4_applications",
        "tier5": "test_tier5_adversarial",
    }

    selected_dirs = _resolve_selected_dirs(tiers, tier_dirs)
    loader = unittest.TestLoader()

    feature_list = (
        [features]
        if isinstance(features, int)
        else (list(features) if features is not None else None)
    )

    for d in selected_dirs:
        dir_path = os.path.join(tests_dir, d)
        if not os.path.isdir(dir_path):
            continue

        if feature_list and d == "test_tier1_features":
            for f_num in feature_list:
                pattern = f"test_f{f_num:02d}_*.py"
                sub_suite = loader.discover(
                    start_dir=dir_path, pattern=pattern, top_level_dir=REPO_ROOT
                )
                suite.addTest(sub_suite)
        else:
            pattern = "test_*.py"
            sub_suite = loader.discover(
                start_dir=dir_path, pattern=pattern, top_level_dir=REPO_ROOT
            )
            suite.addTest(sub_suite)

    return suite


def print_human_report(records: list[dict], summary: dict):
    """Prints a clean, formatted ASCII report for human inspection."""
    print("\n" + "=" * 92)
    print("                RVS E2E REQUIREMENTS-DRIVEN TEST SUITE EXECUTION REPORT")
    print("=" * 92)

    tier_labels = {
        "test_tier1_features": "Tier 1: Feature Coverage",
        "test_tier2_boundaries": "Tier 2: Boundary & Corner Cases",
        "test_tier3_combinations": "Tier 3: Cross-Feature Interactions",
        "test_tier4_applications": "Tier 4: Real-World Applications",
        "test_tier5_adversarial": "Tier 5: Adversarial Hardening",
    }

    current_tier = None
    for r in records:
        if r["tier"] != current_tier:
            current_tier = r["tier"]
            label = tier_labels.get(current_tier, current_tier)
            print(f"\n--- {label} ---")
            print(f"{'Status':<8} | {'Module':<36} | {'Test Case':<32} | {'Duration'}")
            print("-" * 92)

        status_marker = f"[{r['status']}]"
        mod = r["module"][:36]
        tname = r["test_name"][:32]
        dur = f"{r['duration_seconds'] * 1000:6.1f} ms"
        print(f"{status_marker:<8} | {mod:<36} | {tname:<32} | {dur}")

    print("\n" + "=" * 92)
    print("                                 EXECUTION SUMMARY")
    print("=" * 92)
    print(f"Total Tests Executed : {summary['total_tests']}")
    print(f"Passed               : {summary['passed']}")
    print(f"Failed               : {summary['failed']}")
    print(f"Errors               : {summary['errors']}")
    print(f"Skipped              : {summary['skipped']}")
    print(f"Pass Rate            : {summary['pass_rate']:.2f}%")
    print(f"Total Elapsed Time   : {summary['total_duration_seconds']:.3f} seconds")
    status_str = "SUCCESS (100% PASS)" if summary["status"] == "PASS" else "FAILURE"
    print(f"Final Status         : {status_str}")
    print("=" * 92 + "\n")


def main():
    parser = argparse.ArgumentParser(description="RVS E2E Test Suite Runner")
    parser.add_argument("--tier", type=str, help="Comma-separated tiers to run (e.g. 1,2,3,4,5)")
    parser.add_argument(
        "--feature",
        type=str,
        help="Specific feature number or comma-separated numbers to run (e.g. 1 or 1,2,3,4,5,6,7,8)",
    )
    parser.add_argument(
        "--json-output", type=str, default="e2e_results.json", help="Path to write JSON summary"
    )
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")
    args = parser.parse_args()

    tests_dir = os.path.dirname(os.path.abspath(__file__))

    selected_tiers = None
    if args.tier:
        raw_tiers = [t.strip() for t in args.tier.split(",")]
        selected_tiers = [f"tier{t}" if not t.startswith("tier") else t for t in raw_tiers]

    selected_features = None
    if args.feature:
        selected_features = [int(f.strip()) for f in str(args.feature).split(",") if f.strip()]

    suite = discover_tests(tests_dir, tiers=selected_tiers, features=selected_features)

    start_wall_time = datetime.now(UTC).isoformat()
    start_time = time.perf_counter()

    result = E2ETestResult()
    suite.run(result)

    total_duration = time.perf_counter() - start_time

    total_tests = len(result.test_records)
    passed = sum(1 for r in result.test_records if r["status"] == "PASS")
    failed = len(result.failures)
    errors = len(result.errors)
    skipped = len(result.skipped)
    pass_rate = (passed / total_tests * 100.0) if total_tests > 0 else 0.0

    overall_status = "PASS" if (failed == 0 and errors == 0 and total_tests > 0) else "FAIL"

    summary = {
        "timestamp": start_wall_time,
        "total_tests": total_tests,
        "passed": passed,
        "failed": failed,
        "errors": errors,
        "skipped": skipped,
        "pass_rate": round(pass_rate, 2),
        "total_duration_seconds": round(total_duration, 4),
        "status": overall_status,
        "records": result.test_records,
    }

    # Print human-readable report
    print_human_report(result.test_records, summary)

    # Write JSON results
    json_path = os.path.abspath(os.path.join(REPO_ROOT, args.json_output))
    with open(json_path, "w") as f:
        json.dump(summary, f, indent=2)
    print(f"Machine-readable results written to: {json_path}")

    # Exit with code 0 on 100% pass, 1 on any failure
    sys.exit(0 if overall_status == "PASS" else 1)


if __name__ == "__main__":
    main()
