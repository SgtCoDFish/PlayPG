# PlayPG / Python3 Setup
PlayPG uses Python3 along with libclang and mako to generate boilerplate source code and save time.

Setup is relatively simple under Linux (tested on Ubuntu 15.10) with one small caveat. The following is roughly the process,
assuming that you have python3 set up.

Note that the official clang bindings do not support Python 3 (i.e. you can't just sudo pip3 install clang) but there are 
ports. One is given here.

```
sudo apt-get install libclang-dev
sudo apt-get install python-dev
pip3 install mako
pip3 install clang
```

By default on Ubuntu there is no visible libclang.so after installing; you'll probably want to do something like (assuming clang 3.6):

```
sudo ln -s /usr/lib/x86_64-linux-gnu/libclang-3.6.so /usr/local/lib/libclang.so
```
