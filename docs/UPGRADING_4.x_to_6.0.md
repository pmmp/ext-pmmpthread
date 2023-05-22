# Upgrading from pthreads 4.x to pmmpthread 6.0

Since 4.x, various API changes have been made, simplifying it and removing various magic behaviours.

A condensed list of API changes is below. Other improvements, such as performance improvements, are not listed here.

If you prefer to read the original changelogs, here is a list of significant ones:
- [5.0.0](https://github.com/pmmp/ext-pmmpthread/releases/tag/5.0.0) - simplification, API redesign
- [5.1.0](https://github.com/pmmp/ext-pmmpthread/releases/tag/5.1.0) - support for dereferencing thread-safe objects created by a dead thread or worker task
- [5.2.0](https://github.com/pmmp/ext-pmmpthread/releases/tag/5.2.0) - user-defined `join()` override now called on automatic join when thread variable goes out of scope
- [5.3.0](https://github.com/pmmp/ext-pmmpthread/releases/tag/5.3.0) - closure improvements, removal of automatic serialization of non-thread-safe objects
- [6.0.0-beta1](https://github.com/pmmp/ext-pmmpthread/releases/tag/6.0.0-beta1) - API cleanup, extension renamed to `pmmpthread`, thread shared globals

For an overview of what classes, functions and constants are available, you can check out the [stubs](https://github.com/pmmp/ext-pmmpthread/tree/fork/stubs).

## New features
- It's now possible (in most cases) to dereference a `ThreadSafe` object created by a worker task or thread which has already exited. This was previously impossible, as the object would have been destroyed as soon as the task or thread stopped running.
- `ThreadSafe` descendents once again support full mutability, facilitated by an object store modcount to minimize performance cost.
- User-defined `Thread::join()` overrides are now called when the thread has no remaining references, allowing for improved cleanup logic.
- Added `Thread::getSharedGlobals() : ThreadSafeArray` - a thread-safe array automatically available to all threads unconditionally, useful for sharing global state such as autoloader paths or global locks
- Added `ThreadSafeArray::fromArray()` to explicitly convert an `array` to a `ThreadSafeArray`.

## API changes
- `Threaded` has been split into several new classes:
  - `pmmp\thread\ThreadSafe` - a basic thread-safe object. In most cases, `extends \Threaded` can be directly replaced with `extends \pmmp\thread\ThreadSafe`. Implements `wait()`, `notify()`, `notifyOne()` and `synchronized()`.
  - `pmmp\thread\ThreadSafeArray` (`final`) extends `ThreadSafe` - offers `array`-like behaviour. Implements `shift()`, `pop()`, `merge()`, `chunk()`, `count()`, `fromArray()`, `ArrayAccess` API, `Countable` API, `IteratorAggregate` API.
  - `pmmp\thread\Runnable` (`abstract`) extends `ThreadSafe` - base class for `Worker` tasks and `Thread`. Implements `run()`, `isRunning()` and `isTerminated()`.
- The following classes have been renamed:
  - `Pool` -> `pmmp\thread\Pool`
  - `Thread` -> `pmmp\thread\Thread`
  - `ThreadedConnectionException` -> `pmmp\thread\ConnectionException`
  - `Worker` -> `pmmp\thread\Worker`
- All `PTHREADS_*` global constants have been moved to `pmmp\thread\Thread::` class constants.
- The following have been removed:
  - `ThreadedSocket` - the native PHP `Socket` class can be shared provided that `ext-sockets` was compiled
  - `Volatile` - use `pmmp\thread\ThreadSafe` or `pmmp\thread\ThreadSafeArray` instead; all `ThreadSafe` objects now support mutation without performance losses
  - `Collectable` - this interface was useless and can be removed entirely
  - `Threaded::addRef()`, `Threaded::delRef()`, `Threaded::getRefCount()`
  - `Threaded::extend()` - this function made no sense and violated various php-src and ext-opcache assumptions
- `ArrayAccess` now works when implemented by `ThreadSafe` descendents. This did not work prior to v5.
- Thread-safe objects are no longer serializable. Previously, they serialized to a native pointer to themselves, which was used for the old non-thread-safe serialize hacks.
- All APIs now use native parameter and return types.
- Property types are now verified when writing to properties of classes extending `ThreadSafe`.
- `ThreadSafe` now behaves much more similarly to a regular non-thread-safe object, throwing similar errors and warnings.
- Removed the magic `worker` field that appeared on `Worker` tasks just before executing them. This can be achieved by using `pmmp\thread\Thread::getCurrentThread()` instead.
- `Thread::start()` now requires the `$options` to be specified. This is because there's no default that works for all use cases - `INHERIT_NONE` is preferable for large applications, but `INHERIT_ALL` is easier for single-file scripts.

## Behaviour changes
- Assigning a non-thread-safe (NTS) variables to a thread-safe object will now throw an error, instead of attempting to magically make it thread-safe.
  - Array implicit coercion to `Volatile` is removed. Arrays may be converted to `ThreadSafeArray` using `ThreadSafeArray::fromArray()`.
  - Serialization of non-thread-safe variables is removed.
  - The following types are now considered thread-safe, and may be assigned to `ThreadSafe` objects:
    - Scalar variables (`bool`, `int`, `float`, `string`)
    - `NULL`
    - Objects extending `pmmp\thread\ThreadSafe`
    - `Closure`s which do not reference `$this` (use `static function`), do not `use(&$var)`, do not use non-thread-safe variables, and do not have `static` variables
    - `Socket`s which are not created from `socket_import_stream()`
- Class static properties now always use their default values on new threads. Previously, they would inconsistently inherit the values from their parent at the time of thread start. This was a giant mess because statics would end up in broken states if they contained non-thread-safe values, and would behave differently if OPcache was used.
- `Worker::stack()` now throws an exception if the worker is not running.
- `Worker` now exits if an uncaught error occurs during a task. This can be detected via `isTerminated()`, similar to a regular thread. Previously, the worker would keep executing tasks while the worker context was in an undefined state.
