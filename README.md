run with e.g. `uv run --python=3.11 python -c "import numpy_deadlock_test"`

You should see something like:

```
Using CPython 3.11.10
Removed virtual environment at: .venv
Creating virtual environment at: .venv
      Built numpy-deadlock-test @ file:///Users/goldbaum/Documents/numpy-deadlock
Installed 2 packages in 8ms
B: Acquiring allocator
B: Acquire allocator, not trying to acquire the GIL
B: acquired GIL (no deadlock)
A: acquired GIL, now trying to get PyThread lock
A: acquired lock (no deadlock)
```
