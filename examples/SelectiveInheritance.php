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

class my_class {}

function my_function(){
    return __FUNCTION__;
}

define ("my_constant", 1);

class Selective extends Thread {
    public function run() : void {
        /* functions exist where Thread::INHERIT_FUNCTIONS is set */
        var_dump(function_exists("my_function"));
        /* classes exist where Thread::INHERIT_CLASSES is set **BE CAREFUL** */
        var_dump(class_exists("my_class"));
        /* constants exist where Thread::INHERIT_CONSTANTS is set */
        var_dump(defined("my_constant"));
    }
}

?>
expect:
    bool(false)
    bool(false)
    bool(false)
<?php
$test = new Selective();
/*
 * this is the most performant option and should be used wherever possible
 * if all your code can be autoloaded you should be able to use this
 */
$test->start(Thread::INHERIT_NONE);
$test->join();
?>
=======================================
expect:
    bool(false)
    bool(true)
    bool(true)
<?php
$test = new Selective();
$test->start(Thread::INHERIT_ALL & ~Thread::INHERIT_FUNCTIONS);
$test->join();
?>
=======================================
expect:
    bool(false)
    bool(false)
    bool(true)
<?php
$test = new Selective();
$test->start(Thread::INHERIT_INI | Thread::INHERIT_CONSTANTS);
$test->join();
?>
=======================================
expect:
    bool(true)
    bool(true)
    bool(true)
<?php
$test = new Selective();
/*
 * You really, really don't want to use INHERIT_ALL in a production application - it's really slow and wastes lots of memory
 * Prefer INHERIT_NONE if you can autoload your code and don't set any INI entries
 */
$test->start(Thread::INHERIT_ALL);
$test->join();
?>
