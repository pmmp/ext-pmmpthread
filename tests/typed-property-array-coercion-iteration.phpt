--TEST--
Tests that 7.4 property typehints don't break iteration of Threaded/Volatile
--FILE--
<?php

class Test1 extends \Threaded {
	public array $players;

	public function encode(): void{
		foreach((array) $this->players as $player){
			echo($player . PHP_EOL);
		}
	}
}
class Test2 extends \Threaded {
	public array $players;

	public function encode(): void{
		foreach($this->players as $player){
			echo($player . PHP_EOL);
		}
	}
}

$packet = new Test1();
$packet->players = new Volatile();
$packet->players[] = "test";
echo ("Running Test-1.." . PHP_EOL);
$packet->encode();
echo("Test-1 completed" . PHP_EOL);

$packet = new Test2();
$packet->players = new Volatile();
$packet->players[] = "test";
echo ("Running Test-2.." . PHP_EOL);
$packet->encode();
echo("Test-2 completed" . PHP_EOL);
?>
--EXPECT--
Running Test-1..
test
Test-1 completed
Running Test-2..
test
Test-2 completed
