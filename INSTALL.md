## Setting up dependencies

Run the `/deps/install-dependent_packages` build appropriate for your system.  The script names indicate development and/or target systems.

## The build so far

    ./bootstrap.sh
    ./configure
    make
    make check           #optional
    make check-valgrind  #optional
    make install

`.configure` respects `--prefix`.

`make check` and `make check-valgrind` both require a disk image be present.  If you want to run these tests, I used the [CFREDS "Basic Mac" image](http://www.cfreds.nist.gov/v2/Basic_Mac_Image.html).
