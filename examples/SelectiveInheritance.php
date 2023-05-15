<?php
/*
* This is for advanced users ONLY !!
*
* In a large application, the overhead of each thread having to copy the entire context may become undesireable.
* Selective Inheritance serves as a way to choose which parts of the environment are available in threading contexts
* Following is some code that demonstrates the use of this feature
*
* Note: the included_files table is only populated where Thread::INHERIT_INCLUDES is set
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
$test->start();
$test->join();
?>
