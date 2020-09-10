# Threading for PHP - Share Nothing, Do Everything :)

[![Build Status](https://travis-ci.com/pmmp/pthreads.svg?branch=fork)](https://travis-ci.com/pmmp/pthreads)
[![Build status](https://ci.appveyor.com/api/projects/status/929kgwur23p40n1y/branch/fork?svg=true)](https://ci.appveyor.com/project/pmmp/pthreads/branch/fork)
<!---
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/krakjoe/pthreads.svg)](http://isitmaintained.com/project/krakjoe/pthreads "Average time to resolve an issue")
[![Percentage of issues still open](http://isitmaintained.com/badge/open/krakjoe/pthreads.svg)](http://isitmaintained.com/project/krakjoe/pthreads "Percentage of issues still open")
[![Join the chat at https://gitter.im/krakjoe/pthreads](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/krakjoe/pthreads?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
-->
This project provides multi-threading that is compatible with PHP based on Posix Threads.

## Highlights

* An easy to use, quick to learn OO Threading API for PHP 7.3+
* Execute any and all predefined and user declared methods and functions, including closures.
* Ready made synchronization included
* A world of possibilities ...

## Technical Features

* High Level Threading
* Synchronization
* Worker Threads
* Thread Pools
* Complete Support for OO - ie. traits, interfaces, inheritance etc
* Full read/write/execute support for Threaded objects

## Requirements

* PHP 7.3+
* ZTS Enabled ( Thread Safety )
* Posix Threads Implementation

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

### Are you serious ?

Absolutely, this is not a hack, we _don't_ use forking or any other such nonsense, what you create are honest to goodness posix threads that are completely compatible with PHP and safe ... this is true multi-threading :)

### SAPI Support

pthreads v3 is restricted to operating in CLI only: I have spent many years trying to explain that threads in a web server just don't make sense, after 1,111 commits to pthreads I have realised that, my advice is going unheeded.

So I'm promoting the advice to hard and fast fact: *you can't use pthreads safely and sensibly anywhere but CLI.*

Thanks for listening ;)

### Documentation 

Documentation can be found in the PHP manual: http://docs.php.net/manual/en/book.pthreads.php, and some examples can be found in the "examples" folder in the master repository.

Further insights and occasional announcements can be read at the http://pthreads.org site where pthreads is developed and tested in the real world.

Here are some links to articles I have prepared for users: everybody should read them before they do anything else:

 - https://gist.github.com/krakjoe/6437782
 - https://gist.github.com/krakjoe/9384409

If you have had the time to put any cool demo's together and would like them showcased on pthreads.org please get in touch.

### Feedback

Please submit issues, and send your feedback and suggestions as often as you have them.

### Reporting Bugs

If you believe you have found a bug in pthreads, please open an issue: Include in your report *minimal, executable, reproducing code*.

Minimal:     reduce your problem to the smallest amount of code possible; This helps with hunting the bug, but also it helps with integration and regression testing once the bug is fixed.

Executable:  include all the information required to execute the example code, code snippets are not helpful.

Reproducing: some bugs don't show themselves on every execution, that's fine, mention that in the report and give an idea of how often you encounter the bug.

__It is impossible to help without reproducing code, bugs that are opened without reproducing code will be closed.__

Please include version and operating system information in your report.

*Please do not post requests to help with code on github; I spend a lot of time on Stackoverflow, a much better place for asking questions.*

Have patience; I am one human being.

### Developers

There is no defined API for you to create your own threads in your extensions, this project aims to provide Userland threading, it does not aim to provide a threading API for extension developers. I suggest you allow users to decide what they thread and keep your own extension focused on your functionality.
