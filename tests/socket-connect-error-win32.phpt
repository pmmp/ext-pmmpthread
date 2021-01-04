--TEST--
Test error cases when creating a socket on win32 only
--CREDITS--
Copied from php/php-src and adjusted, originally created by 
Russell Flynn <russ@redpill-linpro.com>
#PHPTestFest2009 Norway 2009-06-09 \o/
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) != 'WIN') {
	die('skip.. only valid for Windows');
}
--INI--
error_reporting=E_ALL
display_errors=1
--FILE--
<?php
  try{
    // Test with no arguments
    $server = new ThreadedSocket();
  }catch(\ArgumentCountError $e){
    echo $e->getMessage() . PHP_EOL;
  }

  try{
    // Test with less arguments than required
    $server = new ThreadedSocket(\ThreadedSocket::SOCK_STREAM, getprotobyname('tcp'));
  }catch(\ArgumentCountError $e){
    echo $e->getMessage() . PHP_EOL;
  }

  try {
    // Test with non integer parameters
    $server = new ThreadedSocket(array(), 1, 1);
  } catch(Throwable $throwable) {
    var_dump($throwable->getMessage());
  }

  try {
    // Test with unknown domain
    $server = new ThreadedSocket(\ThreadedSocket::AF_INET + 1000, \ThreadedSocket::SOCK_STREAM, 0);
  } catch(Throwable $throwable) {
    var_dump($throwable->getMessage());
  }
  
?>
--EXPECTF--
ThreadedSocket::__construct() expects exactly 3 parameters, 0 given
ThreadedSocket::__construct() expects exactly 3 parameters, 2 given
string(%d) "Argument 1 passed to ThreadedSocket::__construct() must be of the type %s, array given"
string(%d) "Unable to create socket (%d): An address incompatible with the requested protocol was used.
"