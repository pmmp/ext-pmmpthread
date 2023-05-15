--TEST--
Test doc comments are copied properly
--DESCRIPTION--
Test that doc comments are copied, no leaking/errors
--FILE--
<?php
/**
* Comment
* @doc comment
* @package package
* @subpackage subpackage
*/

/**
* Comment
*/
class T extends \pmmp\thread\Thread {  
    /**
    * @var testing
    */
    public $content;
    /**
    * Method comment
    * @doc comment
    * @package package
    * @subpackage subpackage
    */
    public function run() : void{
       $reflect = new ReflectionMethod("T", "run");
       var_dump($reflect);
       var_dump($reflect->getDocComment());
    }
}

$t = new T();
$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->join();

$reflect = new ReflectionMethod("T", "run");
var_dump($reflect);
var_dump($reflect->getDocComment());

?>
--EXPECTF--
object(ReflectionMethod)#2 (2) {
  ["name"]=>
  string(3) "run"
  ["class"]=>
  string(1) "T"
}
string(%d) "/**
    * Method comment
    * @doc comment
    * @package package
    * @subpackage subpackage
    */"
object(ReflectionMethod)#2 (2) {
  ["name"]=>
  string(3) "run"
  ["class"]=>
  string(1) "T"
}
string(%d) "/**
    * Method comment
    * @doc comment
    * @package package
    * @subpackage subpackage
    */"
