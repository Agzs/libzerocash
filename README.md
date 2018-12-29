libzerocash: a C++ library implementing the Zerocash protocol
================================================================================

forked from: [https://github.com/Zerocash/libzerocash](https://github.com/Zerocash/libzerocash)

refer to: [https://github.com/Electric-Coin-Company/libzerocash](https://github.com/Electric-Coin-Company/libzerocash)

The libzerocash library implements the Zerocash protocol, which provides a *privacy-preserving* version of [Bitcoin](https://bitcoin.org/) (or a similar currency).

Zerocash fixes an inherent weakness of Bitcoin: every user's payment history is recorded in public view on the [block chain](https://en.bitcoin.it/wiki/Block_chain), and is thus readily available to *anyone*. While there are techniques to obfuscate this information, they are problematic and ineffective. Instead, in Zerocash, users may pay one another directly, via payment transactions that reveal neither the origin, destination, or amount of the payment. This is a marked improvement compared to Bitcoin (and similar decentralized digital currencies), where every payment's information is made public for the whole world to see.

The basic Zerocash protocol is described in this paper:

* Eli Ben-Sasson, Alessandro Chiesa, Christina Garman, Matthew Green, Ian Miers, Eran Tromer, Madars Virza, _Zerocash: Decentralized Anonymous Payments from Bitcoin_, [proceedings of the IEEE Symposium on Security & Privacy (Oakland) 2014](http://www.ieee-security.org/TC/SP2014/), 459-474, IEEE, 2014

It improves on an earlier protocol, [Zerocoin](http://zerocoin.org/), developed by some of the same authors, both in functionality (Zerocoin only hides a payment's origin, but not its destination or amount) and in efficiency (Zerocash transactions are less than 1KB and take less than 6ms to verify).

Zerocash relies on Succinct Noninteractive Arguments of Knowledge (SNARK) proofs, as implemented in the open-source  [libsnark](https://github.com/scipr-lab/libsnark) library by [SCIPR Lab](http://www.scipr-lab.org/).

Notes on getting it running
----------------------------------------------------------------------------------

* 1. ``./get-libsnark``,r un get-libsnark to git pull libsnark from github.com, 
* 2. checkout the version when lightning\_circuit was generated.
``cd depsrc/libsnark/ && git checkout 746ade7ce0f30a6f6e612e50450294c8e7ade9a4``
* 3. Comment ``git pull`` in get-libsnark 
* 4. run get-libsnark again, ``./get-libsnark``
* 5. ``make``

There are two bugs: 
- fatal error: curses.h: No such file or directory <br/>
 solution: `sudo apt-get install libncurses5-dev libncursesw5-dev` 

- error: base class ‘class CryptoPP::AbstractRing<CryptoPP::Integer>’ should be explicitly initialized in the copy constructor [-Werror=extra] <br/>
solution: removed the -Werror and -Wfatal-errors flags from the Makefile. [zcash issue](https://github.com/zcash/zcash/issues/449)

For more information see the [Zerocash web site](http://zerocash-project.org).

Warning
--------------------------------------------------------------------------------

This is an academic proof-of-concept prototype. This implementation is *not ready for production use*. It does not yet contain all the features, careful code review, tests and integration that are needed for a deployment. Future changes to the cryptographic protocol and data formats are likely.

Authors
--------------------------------------------------------------------------------

* [Eli Ben-Sasson](http://eli.net.technion.ac.il/) (Technion)
* [Alessandro Chiesa](http://www.eecs.berkeley.edu/~alexch/) (UC Berkeley)
* [Christina Garman](http://hms.isi.jhu.edu/index.php/people/7.html) (Johns Hopkins University)
* [Matthew Green](http://spar.isi.jhu.edu/~mgreen/) (Johns Hopkins University)
* [Ian Miers](http://hms.isi.jhu.edu/index.php/people/11.html) (Johns Hopkins University)
* [Eran Tromer](http://www.cs.tau.ac.il/~tromer/) (Tel Aviv University)
* [Madars Virza](https://madars.org/) (MIT)
