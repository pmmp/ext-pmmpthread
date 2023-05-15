<?php
/**
* This file serves as a benchmark for thread initialization/destruction
* usage: php-zts examples/Benchmark.php [threads] [samples]
*   threads - the number of threads to create, default=100
*   samples - the number of times to run the test, default=5
*/

/**
* Nothing
*/

use pmmp\thread\Thread;

$max = @$argv[1] ? $argv[1] : 100;
$sample = @$argv[2] ? $argv[2] : 5;

class MyThread extends Thread{
	public function run() : void{

	}
}

printf("Start(%d) ...", $max);
$it = 0;
do {
    $s = microtime(true);
    /* begin test */
    $ts = [];
    while (count($ts)<$max) {
        $t = new MyThread();
        $t->start(Thread::INHERIT_ALL); //TODO: customizable option args?
        $ts[]=$t;
    }
    $ts = [];
    /* end test */
    
    $ti [] = $max/(microtime(true)-$s);
    printf(".");
} while ($it++ < $sample);

printf(" %.3f tps\n", array_sum($ti) / count($ti));
?>
