<?php

use pmmp\thread\Thread;

/*
 * This example shows multiple threads listening for connections on the same socket.
 * This could be useful to distribute sessions across multiple threads.
 *
 * NOTE: Socket objects can only be shared between threads if ext-sockets headers and libs were available at compile time
 * If these were not present, attempting to share a Socket object will result in a NonThreadSafeValueError being thrown.
 */

class Test extends Thread{
	public function __construct(private Socket $socket){}

	public function run() : void{
		$clients = 0;
		while(++$clients < 10 && ($client = socket_accept($this->socket)) !== false){
			printf("Accept in %lu\n", $this->getThreadId());

			$requestHeaders = [];
			$emptyHeaders = 0;
			while(($chars = socket_read($client, 1024, PHP_NORMAL_READ))){
				$trimmed = trim($chars);
				if(strlen($trimmed) === 0){
					$emptyHeaders++;
					if($emptyHeaders > 1){
						break;
					}else{
						continue;
					}
				}
				$requestHeaders[] = trim($chars);
			}
			echo "Request headers:\n" . implode("\n", $requestHeaders) . "\n";

			$responseHeaders = [];
			foreach($requestHeaders as $header){
				if(strlen($header) > 0){
					$responseHeaders[] = $header;
				}
			}

			socket_getpeername($client, $address, $port);

			$body = [];
			$body[] = "<html>";
			$body[] = "<head>";
			$body[] = "<title>Multithread Sockets PHP ({$address}:{$port})</title>";
			$body[] = "</head>";
			$body[] = "<body>";
			$body[] = "<pre>";
			foreach($responseHeaders as $header)
				$body[] = "{$header}";
			$body[] = "Request ID: $clients on thread " . $this->getThreadId();
			$body[] = "</pre>";
			$body[] = "</body>";
			$body[] = "</html>";

			$rawBody = implode("\r\n", $body);

			socket_write($client, implode("\r\n", [
				"HTTP/1.0 200 OK",
				"Content-Type: text/html",
				sprintf("Content-Length: %d", strlen($rawBody))
			]));
			socket_write($client, "\r\n\r\n");
			socket_write($client, $rawBody);

			socket_close($client);
		}

		echo "Shutting down after serving $clients requests\n";
	}
}

if(count($argv) < 2){
	fwrite(STDERR, "Required args: listen port\n");
	exit(1);
}

$sock = socket_create_listen($argv[1]);
if($sock === false){
	throw new \RuntimeException("Failed to create socket: " . socket_strerror(socket_last_error()));
}

$workers = [];
while(count($workers) < 5){
	$worker = new Test($sock);
	/* we can use INHERIT_NONE here since the thread isn't using any other code */
	$worker->start(Thread::INHERIT_NONE);
	$workers[] = $worker;
}
printf("%d threads waiting on port %d\n", count($workers), $argv[1]);

/*
 * All the threads are waiting for connections, so this code will block until the threads have finished accepting
 * connections and closed their sockets.
 */
foreach($workers as $thread){
	$thread->join();
}
