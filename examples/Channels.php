<?php

declare(strict_types=1);

/*
 * pmmpthread doesn't have a dedicated type for channels at the time of writing (Nov 2023).
 * However, ThreadSafeArray can be used for this purpose reasonably well.
 */

use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;

function readChannel(ThreadSafeArray $channel) : mixed{
	return $channel->synchronized(function() use ($channel) : mixed{
		while($channel->count() === 0){
			//wait for data to be available
			$channel->wait();
		}

		return $channel->shift();
	});
}

function writeChannel(ThreadSafeArray $channel, mixed $data) : void{
	$channel->synchronized(function() use ($channel, $data) : void{
		$channel[] = $data;
		$channel->notify();
	});
}

class CompressorThread extends Thread{

	public function __construct(
		private ThreadSafeArray $inChannel,
		private ThreadSafeArray $outChannel
	){}

	public function run() : void{
		//fetch this into a local variable to avoid unnecessary synchronization
		$inChannel = $this->inChannel;
		$outChannel = $this->outChannel;

		while(true){
			$buffer = readChannel($inChannel);

			if($buffer === null){
				//null is the shutdown signal
				break;
			}

			$compressed = zlib_encode($buffer, ZLIB_ENCODING_GZIP, 9);

			writeChannel($outChannel, $compressed);
		}
	}
}

$inChannel = new ThreadSafeArray();
$outChannel = new ThreadSafeArray();
$thread = new CompressorThread($inChannel, $outChannel);
$thread->start(Thread::INHERIT_ALL);

for($loops = 0; $loops < 100; $loops++){
	for($in = 0; $in < 100; $in++){
		writeChannel($inChannel, "Hello world! This is loop $loops");
	}

	//process any available results
	$collected = 0;
	while($outChannel->count() > 0){
		$compressed = readChannel($outChannel);
		$collected++;
	}
	echo "Collected $collected results\n";
}

writeChannel($inChannel, null); //shutdown

$thread->join();

//process any remaining results
$collected = 0;
while($outChannel->count() > 0){
	$compressed = readChannel($outChannel);
	$collected++;
}
echo "Collected $collected results\n";
