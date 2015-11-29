# BuzZ

!!! experimental version !!!

BuzZ is a extensible and configurable library for network data collect from social medias with realtime (stream) suport. This works based on two interfaces: buzz and buzzData.

The table bellow show the current support:


### Build

```
~$ autoreconf --install
~$ ./configure
~$ make
~# make install
```

build tests:

`~$ make -C test`

### buzz Interface
Implements the connection with the network and how the library will communicate with social medias (eg.: twitter, facebook, instagram, etc.)


### buzzData Interface
Implements the way that data will be stored (eg.: postgresql, file, stdout, etc.).


### TODO: 
- [Documentation](https://github.com/leonardoms/BuzZ/wiki)
