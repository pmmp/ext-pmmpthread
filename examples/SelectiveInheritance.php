<?php
/*
* In a large application, the overhead of each thread having to copy the entire context may become undesireable.
* Selective Inheritance serves as a way to choose which parts of the environment are available in threading contexts
* Following is some code that demonstrates the use of this feature
*
* Note: the included_files table is only populated where Thread::INHERIT_INCLUDES is set
*
* Ideally you want to inherit as little stuff as possible. Inheriting INI is fine, but generally speaking,
* you should avoid inheriting classes/functions/constants if they can be autoloaded, as inheriting code will use a lot
* of memory, as well as making thread startup slower.
* Code inheritance is supported for the cases where this isn't possible, such as single-file scripts like the one below.
*/

use pmmp\thread\Thread;

/*
 * When starting a Thread, many things must be copied from the parent thread's context. This includes:
 * - INI settings set by ini_set()
 * - User-defined code (classes, functions, constants, list of included files)
 *
 * This process is extremely costly and bug-prone, and should be avoided wherever possible.
 *
 * When to use inheritance options:
 *
 * - Thread::INHERIT_NONE: Copies nothing. Use when you can autoload all the code you need (best for performance and memory usage)
 * - Thread::INHERIT_INI: Copies INI settings but no code. Use this if you've used ini_set() and want to make sure the settings are copied, and your code can't re-apply them inside the thread.
 * - Thread::INHERIT_ALL: Copies everything. Use in single-file scripts, or when you can't autoload parts of your code (slow, wastes memory)
 * - Everything else: Never, they are legacy leftovers and have no good use case
 *
 * NOTE: The following things are **always** available on threads, regardless of INHERIT_* options:
 * - Built-in classes, functions and constants (provided by extensions or the PHP core)
 * - Preloaded user-defined classes, functions and constants (https://www.php.net/manual/en/opcache.configuration.php#ini.opcache.preload)
 */

class my_class{
}

function my_function(){
	return __FUNCTION__;
}

define("my_constant", 1);

ini_set('memory_limit', '123456789');

class Selective extends Thread{
	public function run() : void{
		/* functions exist where Thread::INHERIT_FUNCTIONS is set */
		echo "User-defined function inherited: " . (function_exists("my_function") ? "yes" : "no") . "\n";
		/* classes exist where Thread::INHERIT_CLASSES is set **BE CAREFUL** */
		echo "User-defined class inherited: " . (class_exists("my_class") ? "yes" : "no") . "\n";
		/* constants exist where Thread::INHERIT_CONSTANTS is set */
		echo "User-defined constant inherited: " . (defined("my_constant") ? "yes" : "no") . "\n";

		/* INI entries exist where Thread::INHERIT_INI is set */
		echo "ini_set() INI entries inherited: " . (ini_get('memory_limit') === '123456789' ? "yes" : "no") . "\n";

		/* built-in classes, functions and constants are always available */
		echo "Built-in classes available: " . (class_exists("stdClass") ? "yes" : "no") . "\n";
		echo "Built-in functions available: " . (function_exists("var_dump") ? "yes" : "no") . "\n";
		echo "Built-in constants available: " . (defined("PHP_VERSION") ? "yes" : "no") . "\n";
	}
}

foreach([
	"INHERIT_NONE" => Thread::INHERIT_NONE,
	"INHERIT_INI" => Thread::INHERIT_INI,
	"INHERIT_ALL" => Thread::INHERIT_ALL
] as $name => $value){
	echo "--- $name ---\n";
	$test = new Selective();
	$test->start($value);
	$test->join();
}
