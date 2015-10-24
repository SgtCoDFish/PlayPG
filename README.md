PlayPG
======

A test of [APG](https://github.com/SgtCoDFish/APG).

```
mkdir build && cd build
cmake ..
make -j4
```

and go! You'll need to install APG first.

Ubuntu 15.10 + Boost
====================
By default, the boost packages with Ubuntu 15.10 (wily) are built with the old c++ ABI and are therefore completely useless if you're otherwise using a standard toolset.

To build boost on the new abi assuming you've downloaded the latest source of boost from [the boost downloads page](http://www.boost.org/users/download/):

```
tar xf boost_1_59_0.tar.gz # replace version as appropriate
cd boost_1_59_0
./bootstrap.sh --with-toolset=<clang|gcc> --with-libraries=program_options # toolset and libraries as needed
./b2 toolset=<clang|gcc> cxxflags="-std=c++14 -w" define=_GLIBCXX_USE_CXX11_ABI=1 -j4
```
