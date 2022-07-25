--TEST--
Test that standard Socket objects get copied and overwritten properly
--SKIPIF--
<?php if(!extension_loaded("sockets")) die("skip ext-sockets is required for this test"); ?>
--FILE--
<?php

$threaded = new \Threaded;
$threaded->socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
socket_connect($threaded->socket, "127.0.0.1", 19132);

$thread = new class($threaded) extends \Thread{

	public bool $started = false;
	public bool $wait1 = false;

	public function __construct(public \Threaded $threaded){}

	public function run() : void{
		socket_getpeername($this->threaded->socket, $addr, $port);
		echo "child thread: $addr:$port\n";
		$this->synchronized(function() : void{
			$this->started = true;
			$this->notify();
		});
		$this->synchronized(function() : void{
			while(!$this->wait1) $this->wait();
		});
		socket_getpeername($this->threaded->socket, $addr, $port);
		echo "child thread: $addr:$port\n";
	}
};
$thread->start();

$thread->synchronized(function() use ($thread) : void{
	while(!$thread->started) $thread->wait();
});
$threaded->socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
socket_connect($threaded->socket, "127.0.0.1", 12345);
$thread->synchronized(function() use ($thread) : void{
	$thread->wait1 = true;
	$thread->notify();
});
$thread->join();

?>
--EXPECT--
child thread: 127.0.0.1:19132
child thread: 127.0.0.1:12345

