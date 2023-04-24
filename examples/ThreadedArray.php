<?php
/**
*	Regular PHP arrays cannot be shared between threads, as they cannot be made thread-safe.
*	For this purpose, pthreads has a ThreadedArray class, which provides similar functionality to a regular array.
*/

/* you might want to set this to 100 before running if you're running on older ( dual core ) hardware */
/* never, _ever_, _ever_, _ever_ create 1000 threads in a PHP application, if you think there's a need to create that many threads:
	you are doing it wrong */
/* the number is high to show that manipulating a stackable as an array in this way is completely safe and reliable */
$hammers = 500;
/** $hammers threads are about to edit this array */

/* a thread for editing */
class T extends Thread {
	public function __construct($test){
		$this->test = $test;
		$this->start();
	}
	public function run() : void{
		/*
		* Every individual operation on a thread-safe object (read/write property, read/write dimension)
		* causes the object to be locked and unlocked. This guarantees that memory will never be corrupted.
		* Simple operations like this one can therefore be done without a synchronized() block.
		*
		* However, if you need to do multiple operations atomically (e.g. checking the count of a field before reading from it),
		* you should use a synchronized() block to prevent races.
		*/
		$this->test[]=rand(0, 10);
	}
}
/* create the array here for passing */
$s = new ThreadedArray();
/* set a pointless value */
$s[]="h";
/* show it was set */
print_r($s);
$ts = array();
/* hammer the crap out of the array */
while(@$i++ < $hammers){
	$ts[]=new T($s);
}
printf("GOT %d T's\n", count($ts));
/* we want all threads to complete */
foreach($ts as $t)
	$t->join();
/* show it was all set without corruption */
print_r($s);
?>
