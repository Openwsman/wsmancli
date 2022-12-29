# Openwsman command line client

## Motivation

The primary purpose of this repository is sample code for the
Openwsman client libraries (C and C++)

The secondary (and arguably more useful) purpose is a command line
client for running WSMAN requests.

However, it is recommended to use the Openwsman language bindings
(available for Ruby, Perl, Python, Java) for more serious wsman usage.

## How to build

Install the openwsman client libraries before attempting to compile.

This follows standard GNU autotools principles and can be compiled as
follows

```
./bootstrap
./configure
make
```

It will produce a `wsman` binary below `src`


## Examples for Openwsman C++ API

Openwsman comes with a(n incomplete) C++ API implemented as
`libwsman_clientpp.so.1`

The `examples` subdirectory contains sample code for using this API.

## Further reading

The [Openwsman wiki](https://github.com/Openwsman/openwsman/wiki) has more [wsmancli
documentation](https://github.com/Openwsman/openwsman/wiki/openwsman-command-line-client)
