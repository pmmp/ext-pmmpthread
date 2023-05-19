<?php

use pmmp\thread\Thread;

class Client extends Thread {
	public function __construct($socket){
		$this->socket = $socket;

		/* INHERIT_NONE can be used if the thread doesn't need any code that can't be autoloaded */
		$this->start(Thread::INHERIT_NONE);
	}
	public function run() : void {
		$client = $this->socket;
		if ($client) {
			$header = 0;
			while(($chars = socket_read($client, 1024, PHP_NORMAL_READ))) {
				$head[$header]=trim($chars);
				if ($header>0) {
					if (!$head[$header] && !$head[$header-1])
						break;
				}
				$header++;
			}
			foreach($head as $header) {
				if ($header) {
					$headers[]=$header;	
				}
			}

			$response = array(	
				"head" => array(
					"HTTP/1.0 200 OK",
					"Content-Type: text/html"
				), 
				"body" => array()
			);
			
			socket_getpeername($client, $address, $port);
		
			$response["body"][]="<html>";
			$response["body"][]="<head>";
			$response["body"][]="<title>Multithread Sockets PHP ({$address}:{$port})</title>";
			$response["body"][]="</head>";
			$response["body"][]="<body>";
			$response["body"][]="<pre>";
			foreach($headers as $header)
				$response["body"][]="{$header}";
			$response["body"][]="</pre>";
			$response["body"][]="</body>";
			$response["body"][]="</html>";
			$response["body"] = implode("\r\n", $response["body"]);
			$response["head"][] = sprintf("Content-Length: %d", strlen($response["body"]));
			$response["head"] = implode("\r\n", $response["head"]);
				
			socket_write($client, $response["head"]);
			socket_write($client, "\r\n\r\n");
			socket_write($client, $response["body"]);
					
			socket_close($client);
		}
	}
}
/* ladies and gentlemen, the world first multi-threaded socket server in PHP :) */
$server = socket_create_listen(13000);
while(($client = socket_accept($server))){
	$clients[]=new Client($client);
	/* we will serve a few clients and quit, to show that memory is freed and there are no errors on shutdown (hopefully) */
	/* in the real world, do something here to ensure clients not running are destroyed */
	/* the nature of a socket server is an endless loop, 
		if you do not do something to explicitly destroy clients you create this may leak */
}
/* that is all */
?>
