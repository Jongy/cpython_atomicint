A (WIP) simple attempt at atomic integers for CPython, which complies to the
number protocol (so operations like += etc are available).

Actually, only `+=` is implemented now :)

So, instead of needing such code for atomic increase/decrease/... on integers:

```py
import threading
n = 0
lock = threading.Lock()

# in multithreaded environment:
with lock:
    n += 1
```

(or employing other suggestions, as given [here](https://stackoverflow.com/questions/23547604/python-counter-atomic-increment)), you can use:

```py
from atomicint import AtomicInt
n = AtomicInt()

# in multithreaded environment:
n += 1  # atomic with regard to all Python threads
```

Examples with `stress.py`:
```
$ python3 stress.py 50 100000 n
not atomic
4650574
$ python3 stress.py 50 100000 n
not atomic
4062557
$ python3 stress.py 50 100000 n
not atomic
4538181
$ python3 stress.py 50 100000 y
atomic
5000000
$ python3 stress.py 50 100000 y
atomic
5000000
```

## How

Python numbers are immutable. Any operation between 2 numbers always creates a new one.

Even for operations like `+=` which are seemingly a "combined operation", the result is computed with
the 2 operands, then re-assigned to the operand.

Let's inspect the bytecode of a simple function, for instance.
```py
def f(x):
    x += 1
```

we get:

```
In [3]: dis.dis(f)
  2           0 LOAD_FAST                0 (x)
              2 LOAD_CONST               1 (1)
              4 INPLACE_ADD
              6 STORE_FAST               0 (x)
              8 LOAD_CONST               0 (None)
             10 RETURN_VALUE
```

`INPLACE_ADD` is the `+=`, and afterwards, its result is assigned to `x`. CPython bytecode may be interrupted
between (almost) any 2 opcodes - this is to relinquish control from a thread holding the GIL for too long; giving
time for other threads to run.
So, if our thread is preempted between e.g `INPLACE_ADD` and `STORE_FAST`, we get a classic race condition.

But C code can't be preempted like Python code, so by implementing the "add" logic in C, and returning the
*same*, modified object, we get atomic integers with respect to Python. No need to actually use "atomic operations"
in the CPU level, because of the GIL.
