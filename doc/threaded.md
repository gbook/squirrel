# Running `squirrel` in a Worker Thread

`squirrel::Write()` can run for minutes to hours (the archive-compression phase
dominates). If a parent application calls it directly it will block until the
whole package is written, with no way to show progress in the meantime.

The library supports a better pattern: run the squirrel object on a **worker
thread** and, from another thread, poll two thread-safe accessors while the work
is in progress:

- `QString GetLogBuffer()` — returns *and clears* the log lines accumulated since
  the last call (a "give me what's new" drain).
- `double GetProgress()` — progress of the current long operation, `0.0`–`100.0`.

Both take an internal mutex, so they are safe to call from a different thread than
the one running `Write()`.

## Two rules that matter

1. **One thread owns the squirrel object.** The object holds a `QSqlDatabase`
   (SQLite) connection, and Qt SQL connections have thread affinity. The entire
   lifecycle — construct the object, add subjects/studies/series, call `Write()` —
   must happen on the *same* thread. Only `GetLogBuffer()`, `GetLog()`, and
   `GetProgress()` may be called from another thread. Constructing on one thread
   and writing on another fails with *"database does not belong to the calling
   thread."*

2. **Set a temp directory in library mode.** When squirrel is used as a library
   (not the CLI), call `SetSystemTempDir()` before `Write()`. Without it,
   `Write()` stages files at the filesystem root and fails. The bundled `squirrel`
   command-line tool does not need this because it enables command-line mode
   (`SetCommandLineExecution(true)`); an application that *embeds* the library
   does need it, even when that application is itself a command-line program.

## Example (single-threaded Qt command-line app)

A command-line program built on `QCoreApplication` normally does everything on the
main thread. To keep printing progress while the package is written, offload just
the squirrel work to a background thread and poll the accessors from `main()`.

The cleanest way in Qt is `QtConcurrent::run`, which runs a task on the global
thread pool and hands back a `QFuture`. No event loop (`app.exec()`) is required —
`main()` stays a simple procedural function and polls in a plain loop. Add
`QT += concurrent` to your `.pro`.

The squirrel object is created *inside* the task, so it lives entirely on the
worker thread (rule 1). The task publishes a pointer to it through a `std::atomic`
so the main thread can reach the thread-safe accessors while `Write()` runs.

```cpp
#include <QCoreApplication>
#include <QtConcurrent>
#include <QFuture>
#include <QThread>
#include <atomic>
#include <cstdio>
#include "squirrel.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::atomic<squirrel*> live{nullptr};

    // Run the whole squirrel lifecycle on a pool thread. Quiet mode (the second
    // constructor argument) stops the library from writing to stdout itself, so
    // the only console output is what we print from the polling loop below.
    QFuture<bool> future = QtConcurrent::run([&live]() -> bool {
        squirrel sqrl(false, true /* quiet */);
        sqrl.SetPackagePath("/data/out.sqrl");
        sqrl.SetSystemTempDir("/var/tmp");   // required when embedding the library
        sqrl.SetOverwritePackage(true);
        sqrl.DataFormat = "orig";

        // ... add subjects / studies / series, Store(), Resequence*() ...

        live = &sqrl;                        // publish for the poller
        bool ok = sqrl.Write();              // the long call
        live = nullptr;                      // retract before sqrl is destroyed
        return ok;
    });

    // Main thread: poll until the task finishes. Still single-threaded control
    // flow - we just sleep and print. No event loop needed.
    while (!future.isFinished()) {
        QThread::msleep(250);
        squirrel *s = live.load();
        if (!s) continue;                    // not published yet / already retracted

        QString chunk = s->GetLogBuffer();   // thread-safe: log lines since last call
        if (!chunk.isEmpty())
            fputs(chunk.toLocal8Bit().constData(), stdout);

        printf("\rprogress: %3.0f%%", s->GetProgress());  // thread-safe: 0-100
        fflush(stdout);
    }

    bool ok = future.result();               // waits for the task and returns its value
    printf("\nWrite() %s\n", ok ? "succeeded" : "failed");
    return ok ? 0 : 1;
}
```

If you would rather not link `QtConcurrent`, a raw `std::thread` gives the same
result with an explicit `done` flag instead of a `QFuture`:

```cpp
#include <thread>
#include <atomic>

std::atomic<bool>      done{false};
std::atomic<bool>      result{false};
std::atomic<squirrel*> live{nullptr};

std::thread worker([&]() {
    squirrel sqrl(false, true /* quiet */);
    sqrl.SetPackagePath("/data/out.sqrl");
    sqrl.SetSystemTempDir("/var/tmp");
    sqrl.SetOverwritePackage(true);
    sqrl.DataFormat = "orig";
    // ... add subjects / studies / series ...

    live   = &sqrl;
    result = sqrl.Write();
    live   = nullptr;             // retract BEFORE sqrl leaves scope
    done   = true;                // signal only after retracting
});

while (!done.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    squirrel *s = live.load();
    if (!s) continue;
    QString chunk = s->GetLogBuffer();
    if (!chunk.isEmpty())
        fputs(chunk.toLocal8Bit().constData(), stdout);
    printf("\rprogress: %3.0f%%", s->GetProgress());
    fflush(stdout);
}
worker.join();
```

Note the ordering in the worker: retract `live` **before** the squirrel object
goes out of scope, and (in the `std::thread` version) set `done` **after**
retracting, so the main thread can never dereference the object while it is being
destroyed.

## What you will see

During a real write the poller receives, in order:

- staging lines as each series is prepared (`"Preparing series [S001-1-1]..."`),
- then `GetProgress()` climbing `0 → 100` through the compression phase, with a
  throttled `"Compressing package... N%"` line appearing in the drained buffer at
  each whole percent,
- and finally `"Finished writing package [...]. Size is [...] bytes"`.

`GetLog()` still returns the complete log at any time if you want the whole thing
rather than the incremental drain.
