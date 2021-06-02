--TEST--
Test fix for #665
--DESCRIPTION--
Unprepared entries in static properties causing segfault
--FILE--
<?php
class SystemLoader extends \ThreadedBase
{
    private static $objConfig = null;

    public static function getConfig()
    {
        self::$objConfig = new SystemLoaderConfig();
    }
}

class SystemLoaderConfig extends \ThreadedBase { }

SystemLoader::getConfig();

class Test extends Thread {
	public function run(){
		echo SystemLoaderConfig::class;
	}
}

$objTestThread = new Test();
$objTestThread->start();
$objTestThread->join();
--EXPECT--
SystemLoaderConfig
