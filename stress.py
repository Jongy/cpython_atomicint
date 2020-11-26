from threading import Thread
import sys


def main(argv):
    assert len(argv) == 4, f"{argv[0]} n_threads n_iters atomic?"
    n_threads = int(argv[1])
    n_iters = int(argv[2])
    atomic = argv[3] == "y"

    if atomic:
        from atomicint import AtomicInt
        n = AtomicInt()
        print("atomic")
    else:
        n = 0
        print("not atomic")

    def adder():
        nonlocal n
        i = 0
        while i < n_iters:
            n += 1
            i += 1

    threads = []
    for _ in range(n_threads):
        t = Thread(target=adder)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    print(int(n))


if __name__ == "__main__":
    main(sys.argv)
