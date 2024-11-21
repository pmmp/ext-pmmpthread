*If you know how to write phpt tests, please open a PR with environment information, and your test, this will cause CI to test your issue before a human is able to look*

Environment
==========

  * PHP: *can be obtained with `php -v`*
  * pthreads: *can be obtained with `php -dextension=pmmpthread.so --ri pmmpthread | grep Version`*
  * OS: *if Windows, include arch*
  * OPcache: *yes/no (check for `Zend OPcache` in the output of `php -m`)*
  * JIT: *can be obtained with `php -i | grep opcache.jit`*

Summary
======

A summary isn't always necessary, code > words ... delete this section, as applicable.

Reproducing Code
==============

Bug reports *must* come with reproducing code, please ensure code is formatted correctly.

If the code cannot be a single script, please link to a gist, or a github repository containing the reproducing code. 

Please, do not link to application repositories.

*Please ensure code is self contained, and executable: if the code cannot be executed, the bug cannot be reproduced.*

Expected Output
=============

Please include the expected output, properly formatted.

Actual Output
============

Please include the actual output, properly formatted.
