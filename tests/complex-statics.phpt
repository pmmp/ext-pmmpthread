--TEST--
Test complex statics bug #32
--DESCRIPTION--
This test verifies that complex static types are ignored when creating thread contexts, leading to predictable stable behaviour
--FILE--
<?php
class sql {
	public static $connection;

	public static function __callstatic($method, $args){
		$tid = \pmmp\thread\Thread::getCurrentThreadId();
		if (isset(self::$connection[$tid])) {
			return call_user_func_array(array(self::$connection[$tid], "_{$method}"), $args);
		} else {
			self::$connection[$tid] = new sql();
			if (isset(self::$connection[$tid]))
				return call_user_func_array(array(self::$connection[$tid], "_{$method}"), $args);
		}
	}
	
	public function _query($sql){
		printf("%s: %s\n", __METHOD__, $sql);
	}
}

class UserThread extends \pmmp\thread\Thread {
    public function run () : void{
        /* execute queries */
		sql::query("SELECT * FROM mysql.user");
		sql::query("SELECT * FROM mysql.user");
		
    }
}

sql::query("SELECT * FROM mysql.user");
sql::query("SELECT * FROM mysql.user");

$thread = new UserThread();
$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
?>
--EXPECT--
sql::_query: SELECT * FROM mysql.user
sql::_query: SELECT * FROM mysql.user
sql::_query: SELECT * FROM mysql.user
sql::_query: SELECT * FROM mysql.user

