--TEST--
Test that nothing bad happens after the owner of a resource deletes it from a thread-safe object
--DESCRIPTION--
This test ensures that we don't get a UAF in PTHREADS_ZG(resources) after the pthreads_storage which originally referenced it is deleted.
--FILE--
<?php

$t1 = new class(STDIN) extends \Thread{

	public bool $dereferenced = false;
	public bool $ownerDestroyed = false;

	public function __construct(
		public mixed $foreignResource
	){}

	public function run() : void{
		$resource = $this->foreignResource;

		$this->synchronized(function() : void{
			$this->dereferenced = true;
			$this->notify();
		});

		$this->synchronized(function() : void{
			while(!$this->ownerDestroyed){
				$this->wait();
			}
		});
	}
};
$t1->start();

$t1->synchronized(function() use ($t1) : void{
	while(!$t1->dereferenced){
		$t1->wait();
	}
});
unset($t1->foreignResource);
$t1->synchronized(function() use ($t1) : void{
	$t1->ownerDestroyed = true;
	$t1->notify();
});
$t1->join();

echo "OK\n";
?>
--EXPECT--
OK
