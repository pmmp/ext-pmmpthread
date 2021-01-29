# Threading for PHP - Share Nothing, Do Everything :)

[![Build Status](https://travis-ci.com/pmmp/pthreads.svg?branch=fork)](https://travis-ci.com/pmmp/pthreads)
[![Build status](https://ci.appveyor.com/api/projects/status/929kgwur23p40n1y/branch/fork?svg=true)](https://ci.appveyor.com/project/pmmp/pthreads/branch/fork)
<!---
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/krakjoe/pthreads.svg)](http://isitmaintained.com/project/krakjoe/pthreads "Average time to resolve an issue")
[![Percentage of issues still open](http://isitmaintained.com/badge/open/krakjoe/pthreads.svg)](http://isitmaintained.com/project/krakjoe/pthreads "Percentage of issues still open")
[![Join the chat at https://gitter.im/krakjoe/pthreads](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/krakjoe/pthreads?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
-->
This project provides multi-threading that is compatible with PHP based on Posix Threads.

This is a fork of the now-abandoned [krakjoe/pthreads](https://github.com/krakjoe/pthreads) extension.

## Focus
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

## Highlights

* An easy to use, quick to learn OO Threading API for PHP 7.3+
* Execute any and all predefined and user declared methods and functions, including closures.
* Ready made synchronization included
* A world of possibilities ...

## Requirements

* PHP 7.3+
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

### Windows Support

Yes !! Windows support is offered thanks to the pthread-w32 library.

##### Simple Windows Installation

* Add `pthreadVC2.dll` (included with the Windows releases) to the same directory as `php.exe` eg. `C:\xampp\php`
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

### SAPI Support

pthreads v3 is restricted to operating in CLI only: I have spent many years trying to explain that threads in a web server just don't make sense, after 1,111 commits to pthreads I have realised that, my advice is going unheeded.

So I'm promoting the advice to hard and fast fact: *you can't use pthreads safely and sensibly anywhere but CLI.*

Thanks for listening ;)

### Documentation 

Documentation can be found in the PHP manual: http://docs.php.net/manual/en/book.pthreads.php, and some examples can be found in the "examples" folder in the master repository.

Here are some links to articles I have prepared for users: everybody should read them before they do anything else:

 - https://gist.github.com/krakjoe/6437782
 - https://gist.github.com/krakjoe/9384409

### Feedback

Please submit issues, and send your feedback and suggestions as often as you have them.

### Reporting Bugs

If you believe you have found a bug in pthreads, please open an issue: Include in your report *minimal, executable, reproducing code*.

Minimal:     reduce your problem to the smallest amount of code possible; This helps with hunting the bug, but also it helps with integration and regression testing once the bug is fixed.

Executable:  include all the information required to execute the example code, code snippets are not helpful.

Reproducing: some bugs don't show themselves on every execution, that's fine, mention that in the report and give an idea of how often you encounter the bug.

__It is impossible to help without reproducing code, bugs that are opened without reproducing code will be closed.__

Please include version and operating system information in your report.

### Developers

There is no defined API for you to create your own threads in your extensions, this project aims to provide Userland threading, it does not aim to provide a threading API for extension developers. I suggest you allow users to decide what they thread and keep your own extension focused on your functionality.
