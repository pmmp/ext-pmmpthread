--TEST--
Test attributes copying
--FILE--
<?php

class FunctionAttribute{

	public function __construct(public int $param1, public string $param2){}
}

const TARGETS = [
	\Attribute::TARGET_CLASS => "class",
	\Attribute::TARGET_FUNCTION => "function",
	\Attribute::TARGET_METHOD => "method",
	\Attribute::TARGET_PROPERTY => "property",
	\Attribute::TARGET_CLASS_CONSTANT => "class constant",
	\Attribute::TARGET_PARAMETER => "parameter",
];
function dumpAttribute(\ReflectionAttribute $attribute) : void{
	echo "--- Attribute " . "\"" . $attribute->getName() . "\" ---\n";
	$targets = [];
	foreach(TARGETS as $flag => $name){
		if(($attribute->getTarget() & $flag) !== 0){
			$targets[] = $name;
		}
	}
	echo "\tTarget=" . implode(", ", $targets) . "\n";
	echo "\tRepeated=" . ($attribute->isRepeated() ? "yes" : "no") . "\n";
	echo "\tArguments=" . print_r($attribute->getArguments(), true) . "\n";
	echo "--- End of attribute ---\n";
}

function dumpAttributes(array $attributes) : void{
	foreach($attributes as $attribute){
		dumpAttribute($attribute);
	}
}

#[FunctionAttribute(1, "hello"), FunctionAttribute(2, "world"), NotRepeated]
function t1(#[ParameterAttribute("i", "don't", "know")] int $p1) : void{

}

function test() : void{
	$func = new \ReflectionFunction('t1');
	dumpAttributes($func->getAttributes());
	foreach($func->getParameters() as $parameter){
		dumpAttributes($parameter->getAttributes());
	}
}

echo "--- main thread start ---\n";
test();
echo "--- main thread end ---\n";

$thread = new class extends \Thread{
	public function run() : void{
		echo "--- child thread start ---\n";
		test();
		echo "--- child thread end ---\n";
	}
};
$thread->start();
$thread->join();
echo "OK\n";
?>
--EXPECTF--
--- main thread start ---
--- Attribute "FunctionAttribute" ---
	Target=function
	Repeated=yes
	Arguments=Array
(
    [0] => 1
    [1] => hello
)

--- End of attribute ---
--- Attribute "FunctionAttribute" ---
	Target=function
	Repeated=yes
	Arguments=Array
(
    [0] => 2
    [1] => world
)

--- End of attribute ---
--- Attribute "NotRepeated" ---
	Target=function
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "ParameterAttribute" ---
	Target=parameter
	Repeated=no
	Arguments=Array
(
    [0] => i
    [1] => don't
    [2] => know
)

--- End of attribute ---
--- main thread end ---
--- child thread start ---
--- Attribute "FunctionAttribute" ---
	Target=function
	Repeated=yes
	Arguments=Array
(
    [0] => 1
    [1] => hello
)

--- End of attribute ---
--- Attribute "FunctionAttribute" ---
	Target=function
	Repeated=yes
	Arguments=Array
(
    [0] => 2
    [1] => world
)

--- End of attribute ---
--- Attribute "NotRepeated" ---
	Target=function
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "ParameterAttribute" ---
	Target=parameter
	Repeated=no
	Arguments=Array
(
    [0] => i
    [1] => don't
    [2] => know
)

--- End of attribute ---
--- child thread end ---
OK
