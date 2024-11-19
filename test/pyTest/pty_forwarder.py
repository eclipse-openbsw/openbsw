import os
import tty
import termios
from threading import Thread
from time import sleep
from pathlib import Path


class PtyForwarder():
    """ Create 2 pseudo-terminals (pty) and create 2 threads,
    each thread reads from master of one pty
    and writes to the master of the other pty.
    Anything can connect to the slave side of either pty
    and exchange bytes with anything connected to
    the slave side of the other pty.
    """

    PTY_SLAVE_A_ALIAS = "PTY_FORWARDER"

    def __init__(self):

        # Open 1st PTY
        self._master_a, self._slave_a = os.openpty()
        tty.setraw(self._master_a, termios.TCSANOW)

        # Open 2nd PTY
        self._master_b, self._slave_b = os.openpty()
        tty.setraw(self._master_b, termios.TCSANOW)

    @property
    def slave_a_path(self):
        return os.ttyname(self._slave_a)

    @property
    def slave_b_path(self):
        return os.ttyname(self._slave_b)

    def _run_func_a(self):
        while self._run:
            try:
                os.write(self._master_b, os.read(self._master_a, 1))
            except Exception:
                pass

    def _run_func_b(self):
        while self._run:
            try:
                os.write(self._master_a, os.read(self._master_b, 1))
            except Exception:
                pass

    def start(self):
        self._thread_a = Thread(
            target=self._run_func_a,
            daemon=True)
        self._thread_b = Thread(
            target=self._run_func_b,
            daemon=True)
        self._run = True
        self._thread_a.start()
        self._thread_b.start()

    def stop(self):
        self._run = False
        self._thread_a.join(0.1)
        self._thread_b.join(0.1)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description='Set up bidirectional connected pseudoterminals.')
    parser.add_argument(
        'link',
        nargs=2,
        help='name of link to slave PTY to create (2 are required)')
    args = parser.parse_args()

    for link in args.link:
        if Path(link).exists():
            print(f"{link} already exists!")
            exit()

    pty_forwarder = PtyForwarder()

    os.symlink(pty_forwarder.slave_a_path, args.link[0])
    os.symlink(pty_forwarder.slave_b_path, args.link[1])

    pty_forwarder.start()

    try:
        while True:
            sleep(10)
    except KeyboardInterrupt:
        print('interrupted!')

    pty_forwarder.stop()

    for link in args.link:
        os.unlink(link)
