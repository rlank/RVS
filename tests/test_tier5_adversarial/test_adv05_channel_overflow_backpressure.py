"""
Tier 5 - Adversarial 5: Channel Overflow & Backpressure Stress.
Tests behavior under producer flooding, bounded channel overflow, cancellation
interrupts during blocking wait, and structured error recovery.
"""

import queue
import threading
import time
import unittest


class TestAdv05ChannelOverflowBackpressure(unittest.TestCase):
    def test_producer_fast_flooding_bounded_queue(self):
        """Bounded channel rejects excessive items without buffer bloat."""
        q = queue.Queue(maxsize=3)
        # Push 3
        q.put(1)
        q.put(2)
        q.put(3)
        # 4th fails
        with self.assertRaises(queue.Full):
            q.put_nowait(4)

    def test_cancellation_unblocks_waiting_producer(self):
        """Cancellation token unblocks a producer waiting on full channel."""
        q = queue.Queue(maxsize=1)
        q.put("blocker")
        cancelled = threading.Event()
        unblocked_cleanly = False

        def blocked_producer():
            nonlocal unblocked_cleanly
            while not cancelled.is_set():
                try:
                    q.put("payload", timeout=0.01)
                except queue.Full:
                    continue
            unblocked_cleanly = True

        t = threading.Thread(target=blocked_producer)
        t.start()
        time.sleep(0.05)
        cancelled.set()
        t.join()

        self.assertTrue(unblocked_cleanly)

    def test_poison_pill_termination_clean(self):
        """Poison pill sentinel shuts down consumer without exceptions."""
        q = queue.Queue()
        q.put(10)
        q.put(20)
        q.put(None)  # Poison pill

        consumed = []
        while True:
            item = q.get()
            if item is None:
                break
            consumed.append(item)

        self.assertEqual(consumed, [10, 20])

    def test_consumer_timeout_on_idle_stream(self):
        """Consumer safely times out on stalled producer without hanging indefinitely."""
        q = queue.Queue()
        with self.assertRaises(queue.Empty):
            q.get(timeout=0.05)

    def test_semaphore_permit_exhaustion(self):
        """Bounded semaphore limits concurrent active tile evaluations."""
        sem = threading.Semaphore(2)
        self.assertTrue(sem.acquire(blocking=False))
        self.assertTrue(sem.acquire(blocking=False))
        # 3rd permit unavailable
        self.assertFalse(sem.acquire(blocking=False))
        sem.release()
        self.assertTrue(sem.acquire(blocking=False))


if __name__ == "__main__":
    unittest.main()
