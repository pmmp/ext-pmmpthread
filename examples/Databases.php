<?php

use pmmp\thread\Worker;
use pmmp\thread\Pool;
use pmmp\thread\Runnable;

/*
 * It's often desirable to interact with things like databases from multiple threads in an application.
 * However, database objects (PDO, mysqli, etc.) cannot be made thread-safe, so they cannot be shared between threads.
 *
 * An ideal solution to this problem is to create Workers for interacting with the database, and create an independent
 * connection per Worker. This avoids the need for sharing unsafe objects, and also improves performance (database are
 * often capable of processing multiple queries in parallel).
 *
 * Runnables provided to the worker can then use the Worker's thread-local connection to interact with the database.
 *
 * The following is a basic example of using PDO in a Pool of Workers to interact with a SQLite database. This kind of
 * setup can be altered to work with any other database (e.g. mysqli, leveldb, etc.)
 */

class PDOWorker extends Worker{
	/*
	 * static properties are thread-local - changes on one thread won't be seen by other threads
	 */
	private static \PDO $connection;

	private string $config;

	/*
	* Note that we only pass in the configuration to create the connection
	*/

	public function __construct(array $config){
		$this->config = serialize($config);
	}

	public function run() : void{
		//set up the connection for tasks to use
		self::$connection = new PDO(...unserialize($this->config));
	}

	/**
	 * Returns the PDO connection for the current Worker thread
	 */
	public static function getConnection() : \PDO{ return self::$connection; }
}

/*
 * When the Pool starts new Worker threads, they will construct the
 * PDO object before any Runnable tasks are executed.
 */
$pool = new Pool(4, PDOWorker::class, [["sqlite:example.db"]]);

/*
 * Now we can submit work to the Pool
 */

for($i = 0; $i < 10; $i++){
	$pool->submit(new class extends Runnable{
		public function run() : void{
			var_dump(\PDOWorker::getConnection());
		}
	});
}

$pool->shutdown();
