--TEST--
Resource constants should not be copied
--DESCRIPTION--
pthreads was assuming that only the STDIN/STDOUT/STDERR constants were resource constants, but define() can create them too.
Since we can't copy or safely share resources, we skip resource constants during copying. This isn't an ideal fix, but it's safe.
--FILE--
<?php

define('A_CUSTOM_RESOURCE_CONSTANT', fopen(sys_get_temp_dir() . '/test.txt', 'a+b'));
if(!is_resource(A_CUSTOM_RESOURCE_CONSTANT)){
	die("failed to create resource constant");
}

define('A_NORMAL_CONSTANT', 1);

$t = new class extends \pmmp\thread\Thread{
	public function run() : void{
		var_dump(
			defined('STDIN'),
			defined('STDOUT'),
			defined('STDERR'),
			defined('A_CUSTOM_RESOURCE_CONSTANT'),
			defined('A_NORMAL_CONSTANT')
		);
	}
};

$t->start(\pmmp\thread\Thread::INHERIT_CONSTANTS);
$t->join();
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)		
