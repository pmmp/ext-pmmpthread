# Threading for PHP - Share Nothing, Do Everything :)

[![CI](https://github.com/pmmp/pthreads/actions/workflows/main.yml/badge.svg)](https://github.com/pmmp/pthreads/actions/workflows/main.yml)
[![Build status](https://ci.appveyor.com/api/projects/status/929kgwur23p40n1y/branch/fork?svg=true)](https://ci.appveyor.com/project/pmmp/pthreads/branch/fork)

This project provides limited support for threading in PHP CLI.

This is a fork of the now-abandoned [krakjoe/pthreads](https://github.com/krakjoe/pthreads) extension.

## Managing expectations

While the idea of PHP threading may seem great, the barrier to using threads is much higher than other languages, due to severe limitations imposed by the design of the Zend Engine. Many things are not possible with threads in PHP, or are simply far too performance-intensive to be worthwhile.

You can learn more about pthreads at the following links:
- https://doc.pmmp.io/en/rtfd/developer-reference/threading-in-php-wtf.html
- https://gist.github.com/krakjoe/6437782
- https://gist.github.com/krakjoe/9384409

## Documentation 
Documentation can be found in the `stub.php` files in the `stubs` folder, and some examples can be found in the `examples` folder in the master repository.

Legacy documentation for pthreads v2/v3 can be found [here](http://docs.php.net/manual/en/book.pthreads.php).

## Fork focus
This fork is used in production on thousands of [PocketMine-MP](https://github.com/pmmp/PocketMine-MP) servers worldwide. Therefore, the focus is on performance and stability.

## Changes compared to the original
- PHP 7.4 and 8.0 support
- Many bug fixes which were never merged upstream
- Performance improvements
- Memory usage improvements
- Integration with [OPcache](https://www.php.net/manual/en/book.opcache.php) on PHP 7.4+ (pthreads leverages opcache SHM to reuse classes and functions, saving lots of memory)
- OPcache JIT support on PHP 8.0.2+

## [OPcache](https://www.php.net/manual/en/book.opcache.php) compatibility
Despite popular belief, OPcache is still useful in a CLI environment - as long as it's a threaded one :)
Every thread in pthreads is like a web server "request", so while OPcache doesn't offer as big an advantage to an application using pthreads as it does to a web server, it's far from useless.

If you're using PHP 7.4+, using [OPcache](https://www.php.net/manual/en/book.opcache.php) with pthreads is **strongly recommended**, as you'll get various benefits from doing so:

- Reduced memory usage when the same class is used on several threads
- Better performance of starting new threads when threads inherit classes and functions

Preloading classes and functions is also supported on PHP 7.4, which will make classes available to all threads without an autoloader.

OPcache isn't enabled in the CLI by default, so you'll need to add
```
opcache.enable_cli=1
```
to your `php.ini` file.

## Why not drop pthreads and move on to something newer and easier to work with, like [krakjoe/parallel](https://github.com/krakjoe/parallel)?
- We found parallel too limited for the use cases we needed it for. If it had some thread-safe class base like `Threaded`, it might be more usable.
- It's possible to implement parallel's API using pthreads, but not the other way round.
- parallel requires significant migration efforts for code using pthreads.

Some specific nitpicks which were deal-breakers for parallel usage in PocketMine-MP:
- parallel has confusing and inconsistent behaviour surrounding object copying. While pthreads also has various inconsistencies and isn't exactly the easiest to understand thing in the world, its faults are well known (better the devil we know than the devil we don't).
- parallel has uncontrollable behaviour around its object copying routine (it's not possible to customise copies or prevent copies from occurring).

Updating pthreads to PHP 7.4 allowed PocketMine-MP users to immediately gain the benefits of PHP 7.4 without needing to suffer API breaks that would affect plugins. In addition, PHP 7.4 introduced various new internal features which are highly beneficial specifically to pthreads, such as immutable classes and op_arrays.

## Requirements

* PHP 7.4+
* PHP CLI (**only** CLI is supported; pthreads is not intended for usage on a webserver)
* ZTS Enabled ( Thread Safety )
* Posix Threads Implementation (pthread-w32 / pthreads4w on Windows)

Testing has been carried out on x86, x64 and ARM, in general you just need a compiler and pthread.h

##### Unix-based Building from Source

Building pthreads from source is quite simple on Unix-based OSs. The instructions are as follows:
 * Clone this repository and checkout the release to use (or master for the latest updates)
 * `cd pthreads`
 * `phpize`
 * `./configure`
 * `make`
 * `make install` (may need sudo)
 * Update your php.ini file to load the `pthreads.so` file using the `extension` directive

### Windows-based Building from Source
Yes !! Windows support is offered thanks to the pthread-w32 library.

##### Simple Windows Installation

* Add `pthreadVC2.dll` or `pthreadVC3.dll` (included with the Windows releases) to the same directory as `php.exe` eg. `C:\xampp\php`
* Add `php_pthreads.dll` to PHP extension folder eg. `C:\xampp\php\ext`

### Mac OSX Support

Yes !! Users of Mac will be glad to hear that pthreads is now tested on OSX as part of the development process.

### Hello World

As is customary in our line of work:

```php
<?php
$thread = new class extends Thread {
	public function run() {
		echo "Hello World\n";
	}
};

$thread->start() && $thread->join();
?>
```

## Feedback

Please submit issues, and send your feedback and suggestions as often as you have them.

## Reporting Bugs

If you believe you have found a bug in pthreads, please open an issue: Include in your report *minimal, executable, reproducing code*.

Minimal:     reduce your problem to the smallest amount of code possible; This helps with hunting the bug, but also it helps with integration and regression testing once the bug is fixed.

Executable:  include all the information required to execute the example code, code snippets are not helpful.

Reproducing: some bugs don't show themselves on every execution, that's fine, mention that in the report and give an idea of how often you encounter the bug.

__It is impossible to help without reproducing code, bugs that are opened without reproducing code will be closed.__

Please include version and operating system information in your report.

## Developers

There is no defined API for you to create your own threads in your extensions, this project aims to provide Userland threading, it does not aim to provide a threading API for extension developers. I suggest you allow users to decide what they thread and keep your own extension focused on your functionality.
