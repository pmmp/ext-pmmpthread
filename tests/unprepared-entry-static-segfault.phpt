--TEST--
Test fix for #665
--DESCRIPTION--
Unprepared entries in static properties causing segfault
--FILE--
<?php
class SystemLoader extends \pmmp\thread\ThreadSafe
{
    private static $objConfig = null;

    public static function getConfig()
    {
        self::$objConfig = new SystemLoaderConfig();
    }
}

class SystemLoaderConfig extends \pmmp\thread\ThreadSafe { }

SystemLoader::getConfig();

class Test extends \pmmp\thread\Thread {
	public function run() : void{
		echo SystemLoaderConfig::class;
	}
}

$objTestThread = new Test();
$objTestThread->start();
$objTestThread->join();
--EXPECT--
SystemLoaderConfig
