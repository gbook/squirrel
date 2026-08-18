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
   `Write()` stages files at the filesystem root and fails. The CLI does not need
   this because it runs with command-line mode enabled.

## Example (Qt: worker `QObject` + `QThread` + polling `QTimer`)

The worker builds and writes the package on its own thread and reports when it is
done. The GUI thread never touches the squirrel object except through the
thread-safe log/progress accessors, which it reads on a timer.

```cpp
// ---- worker.h ----
#include <QObject>
#include <atomic>
#include "squirrel.h"

class PackageWorker : public QObject {
    Q_OBJECT
public:
    // sqrl is created here but NOT used yet; it is only touched inside run(),
    // which executes on the worker thread. The atomic pointer lets the GUI
    // thread reach the log/progress accessors safely once run() publishes it.
    std::atomic<squirrel*> live{nullptr};

public slots:
    void run() {
        squirrel sqrl;                       // constructed on the worker thread
        sqrl.SetPackagePath("/data/out.sqrl");
        sqrl.SetSystemTempDir("/var/tmp");   // required in library mode
        sqrl.SetOverwritePackage(true);
        sqrl.DataFormat = "orig";

        // ... add subjects / studies / series, Store(), Resequence*() ...

        live = &sqrl;                        // publish for the poller
        bool ok = sqrl.Write();              // the long call
        live = nullptr;                      // retract before sqrl is destroyed
        emit finished(ok);
    }

signals:
    void finished(bool ok);
};
```

```cpp
// ---- starting it from the GUI ----
auto *thread = new QThread(this);
auto *worker = new PackageWorker;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &PackageWorker::run);
connect(worker, &PackageWorker::finished, this, [=](bool ok) {
    // drain nothing here; final lines were already polled below
    thread->quit();
});
connect(thread, &QThread::finished, worker, &QObject::deleteLater);
connect(thread, &QThread::finished, thread, &QObject::deleteLater);

// Poll every 250 ms from the GUI thread while the worker runs.
auto *poll = new QTimer(this);
connect(poll, &QTimer::timeout, this, [=]() {
    squirrel *s = worker->live.load();
    if (!s) return;                          // not started yet, or already finished
    QString chunk = s->GetLogBuffer();       // thread-safe: new log lines
    if (!chunk.isEmpty())
        appendToLogView(chunk);              // your UI update
    setProgressBar(s->GetProgress());        // thread-safe: 0–100
});
connect(worker, &PackageWorker::finished, poll, &QTimer::stop);

poll->start(250);
thread->start();
```

## Example (plain `std::thread`)

The same shape without Qt's threading classes. The worker publishes a pointer to
its stack-local squirrel object; the polling loop reads the thread-safe accessors
until the worker signals completion.

```cpp
#include <thread>
#include <atomic>

std::atomic<bool>      done{false};
std::atomic<bool>      result{false};
std::atomic<squirrel*> live{nullptr};

std::thread worker([&]() {
    squirrel sqrl;
    sqrl.SetPackagePath("/data/out.sqrl");
    sqrl.SetSystemTempDir("/var/tmp");
    sqrl.SetOverwritePackage(true);
    sqrl.DataFormat = "orig";
    // ... add subjects / studies / series ...

    live = &sqrl;
    result = sqrl.Write();
    live = nullptr;               // retract BEFORE sqrl leaves scope
    done  = true;                 // signal only after retracting
});

while (!done.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    squirrel *s = live.load();
    if (!s) continue;
    QString chunk = s->GetLogBuffer();
    if (!chunk.isEmpty())
        fputs(chunk.toLocal8Bit().constData(), stdout);
    printf("progress: %.0f%%\n", s->GetProgress());
}
worker.join();
```

Note the ordering in the worker: retract `live` **before** the squirrel object
goes out of scope, and set `done` **after** retracting, so the poller can never
dereference the object while it is being destroyed.

## What you will see

During a real write the poller receives, in order:

- staging lines as each series is prepared (`"Preparing series [S001-1-1]..."`),
- then `GetProgress()` climbing `0 → 100` through the compression phase, with a
  throttled `"Compressing package... N%"` line appearing in the drained buffer at
  each whole percent,
- and finally `"Finished writing package [...]. Size is [...] bytes"`.

`GetLog()` still returns the complete log at any time if you want the whole thing
rather than the incremental drain.
