<?php

use pmmp\thread\ThreadSafe;
use pmmp\thread\Thread;

/*
* Because we plan for two contexts to manipulate this object we extend the ThreadSafe declaration
* This connects automatically the reference in the creating context and the reference in the threading context
*/
class Response extends ThreadSafe {
	
	public function __construct($url){
		$this->url = $url;
		$this->start = 0;
		$this->finish = 0;
		$this->data = null;
		$this->length = 0;
	}
	
	public function setStart($start)	{ $this->start = $start; }
	public function setFinish($finish)	{ $this->finish = $finish; }
	public function setData($data)		{ 
		$this->data = $data; 
		$this->length = strlen($this->data);
	}
	
	public function getUrl()		{ return $this->url; }
	public function getDuration()	{ return $this->finish - $this->start; }
	public function getLength()		{ return $this->length; }
	public function getData()		{ return $this->data; }
	public function getStart()		{ return $this->start; }
	public function getFinish()		{ return $this->finish; }
}

class Request extends Thread {
	/*
	* You could have more parameters but an object can hold everything ...
	*/
	public function __construct($response){
		$this->response = $response;
	}
	
	/*
	* Populate the response object for creating context
	*/
	public function run() : void {
		/*
  	    * Every time a ThreadSafe object's properties are read or written, locking of the object context will occur (in this case the Request object).
		* It's recommended (but not required) to fetch frequently-used ThreadSafe properties into local variables while you're using them.
  		* This avoids unnecessary locking and unlocking and improves performance.
		*/
		$response = $this->response;

		if($response->getUrl()){
			$response->setStart(microtime(true));
			$response->setData(file_get_contents($response->getUrl()));
			$response->setFinish(microtime(true));
		}
	}
}

/*
* We can retain the reference to the response object !!
*/
$response = new Response(
	sprintf("http://www.google.com/?q=%s", md5(rand()*time()))
);

/*
* Initialize a new request with a response object as the only parameter
*/
$request = new Request($response);

/*
* Tell you all about it ...
*/
printf("Fetching: %s ", $response->getUrl());
/*
 * This is using INHERIT_ALL because we are in a single-file script, but you should
 * prefer INHERIT_NONE (or perhaps INHERIT_INI) if you can autoload your code.
 */
if($request->start(Thread::INHERIT_ALL)){
	/* do something during runtime */
	while($request->isRunning()) {
		echo ".";
		$request->synchronized(function() use($request) {
			$request->wait(100);
		});
	}
	/* 
		you do not need to join:
			when a thread returns false for isRunning then your code is no longer being executed
			join() will be called automatically when the object goes out of scope
	*/
	printf(" got %d bytes in %f seconds\n", $response->getLength(), $response->getDuration());
}
$request->join();
?>
