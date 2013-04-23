# nautilus-sendto-its



## Latest Version

http://www.matbooth.co.uk/archive/nautilus-sendto-trac-0.4.tar.gz nautilus-sendto-trac-0.4.tar.gz
http://www.matbooth.co.uk/archive/nautilus-sendto-trac-0.4.sha256sum nautilus-sendto-trac-0.4.sha256sum

## Building from Source

Download the source and verify its integrity:

    $ wget http://www.matbooth.co.uk/archive/nautilus-sendto-trac-0.4.tar.gz
    $ wget http://www.matbooth.co.uk/archive/nautilus-sendto-trac-0.4.sha256sum
    $ sha256sum -c nautilus-sendto-trac-0.4.sha256sum

Untar and build:

    $ tar xvf nautilus-sendto-trac-0.4.tar.gz
    $ cd nautilus-sendto-trac-0.4
    $ ./configure
    $ make
    $ sudo make install

## Release Engineering

To create source tar balls:

    $ svn co https://www.matbooth.co.uk/svn/tags/nautilus-sendto-trac-VERSION/
    $ cd nautilus-sendto-trac-VERSION
    $ autoreconf -i
    $ ./configure && make dist
    $ sha256sum nautilus-sendto-trac-VERSION.tar.gz > nautilus-sendto-trac-VERSION.sha256sum

Where VERSION is the desired version.

## Licence

This project is licenced under the [GNU GPL, Version 2][GPL2].

[GPL2]: http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
