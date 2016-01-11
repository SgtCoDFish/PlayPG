PlayPG
======

An MMO client/server architecture with RPG elements to gameplay. Servers are designed to be highly modular and scalable and for multiple instances to run concurrently while minimising fragmentation between game worlds; that is, for each "realm" every player should be able to interact with each other.

Each server should run functionally on a Raspberry Pi 2 for a small number of players, meaning that testing and early deployment is very cheap. Each type of server also supports IPv4 and IPv6 as standard.

Setup should be as simple as installing dependencies, running:

```
mkdir build && cd build
cmake ..
make -j4
```

and going! You'll need to install [APG](https://github.com/SgtCoDFish/APG) first, which is the helper library associated with the game.

Target Platforms
----------------
### Client
- Windows (Vista or later, not XP)   
In theory, PlayPG should run on Vista and above (but not XP). In practice, it's tested on Windows 10 and will work best there. Uses OpenGL, but a DirectX renderer would be fantastic if it were added to APG.

- Linux   
Tested on Ubuntu using proprietary graphics drivers. Would be nice to run on a Pi, but untested as of yet.

### Server
- Linux (Tested on Ubuntu 15.10, Debian + Raspbian Jessie)   

The server only targets Linux flavours for deployment, although at the time of writing runs fine on Windows with the caveat that some native winsock2 features fail (SDL2_net seems to work fine). It would be nice, but isn't essential, for this to continue.

### Other Platforms
The code should be easily portable: any areas where cross-platform differences appear should be clearly marked and easy to change in the future, meaning that while *BSD/OS X are not supported, they should be easy enough to do in theory. Likely this would also require changes to [APG](https://github.com/SgtCoDFish/APG), espcially for native POSIX sockets.

Getting Started with Development
================================

Platform Specific Tips
----------------------

### Ubuntu 15.10
#### General

#### ODB
ODB is built using the old ABI and must be rebuilt. This isn't too hard; for example, using MySQL:

```
apt-get source libodb-dev
apt-get source libodb-mysql-dev

> extract both files
> then, in each directory...

./configure CXXFLAGS=-std=c++1y -DGLIB_CXX_USE_CXX11_ABI=1
make -j4
sudo make install
```

#### Boost
By default, the boost packages with Ubuntu 15.10 (wily) are built with the old C++ ABI and are therefore completely useless if you're otherwise using the standard toolset for modern C++.

To build boost on the new abi (assuming you've downloaded the latest source of boost from [the boost downloads page](http://www.boost.org/users/download/)):

```
tar xf boost_1_59_0.tar.gz # replace version as appropriate
cd boost_1_59_0
./bootstrap.sh --with-toolset=<clang|gcc> --with-libraries=program_options,filesystem,system # toolset and libraries as needed
./b2 toolset=<clang|gcc> cxxflags="-std=c++14 -w" define=_GLIBCXX_USE_CXX11_ABI=1 -j4
```

### Raspberry Pi (Raspbian/DietPi Jessie)
#### ODB
*Note: This also applies to Debian Jessie*
No libraries for ODB are provided in DietPi repos (unsure about Raspbian), so you need to compile yourself, although the process is as standard as you can get:

```
sudo apt-get install libmysqld-dev
./configure
make -j4
sudo make install
```

The harder part is the lack of an executable; the error message you'll get if you charge ahead is that g++ was compiled without plugin support, which it does actually have. You just need to install some more libraries relating to plugin development.

There's also a bug under g++ 4.9.2 which will stop you from compiling odb-2.4.0, see [here](http://www.codesynthesis.com/pipermail/odb-users/2015-February/002378.html). This is fixed by passing -fno-devirtualize to ./configure.

```
sudo apt-get install gcc-4.9-plugin-dev libcutl-dev libexpat-dev
cd odb-2.4.0
./configure CXXFLAGS=-fno-devirtualize
make -j4
sudo make install
```
#### Boost
You'll have to recompile from scratch because the Boost libraries provided by default are really out-of-date (1.55 at time of writing, when 1.60 is out). It's not pleasant and it's strongly recommended that if you have a cross compiler for the Pi you use it, because compiling natively is likely going to take a long time. PlayPG requires at least 1.56 but if you're compiling from scratch, why not use a more modern version anyway?

```
cd boost_1_60_0 # or whatever version you're using
./bootstrap.sh --with-libraries=filesystem,system,program_options # build them all by removing this option
																  # but if you're compiling natively and you do that, you're nuts
./b2 cxxflags="-std=c++1y" link=static threading=multi
sudo ./b2 install
```

### Windows
Boost is best built manually if you've not already got an installation of it, and you might need to set -DBOOST_ROOT=<path>.

Similar goes for OpenSSL; see [this guide](http://developer.covenanteyes.com/building-openssl-for-visual-studio/) for details (run 7-zip as admin to unzip and preserve symlinks)

When using CMake, testing is done for -G "NMake Makefiles". Generating projects files should work (but if you have experience in this area, testing is appreciated!)

### OS X
No attempt is made to run either the client or server on OS X due to a lack of hardware for testing and experience. If you want to implement it, feel free! In theory, there's nothing stopping the OS X deployment being similar to Linux, but platform-specific bugs/differences always crop up. 