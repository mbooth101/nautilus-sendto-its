# nautilus-sendto-its

![Screen Shot][ScreenShot]

## Supported ITSs

* **Trac** - Trac version 0.11 or newer. Your Trac or Bloodhound instance must be running the XML-RPC plug-in.

## Building from Source

Build from git:

    $ git clone git://github.com/mbooth101/nautilus-sendto-its.git
    $ cd nautilus-sendto-its
    $ git checkout <VERSION>
    $ autoreconf -i
    $ ./configure
    $ make
    $ sudo make install

Build from tarball:

    $ wget https://github.com/mbooth101/nautilus-sendto-its/archive/nautilus-sendto-its-<VERSION>.tar.gz
    $ tar xvf nautilus-sendto-its-<VERSION>.tar.gz
    $ cd nautilus-sendto-its-<VERSION>
    $ autoreconf -i
    $ ./configure
    $ make
    $ sudo make install

Substituting <VERSION> for the desired version.

## Licence

This project is licenced under the [GNU GPL, Version 2][GPL2].

[ScreenShot]: https://raw.github.com/mbooth101/nautilus-sendto-its/master/screenshot.png "Screenshot of nautilus-sendto-its in action."
[GPL2]: http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
