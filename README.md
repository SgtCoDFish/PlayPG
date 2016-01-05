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
- CMake can struggle to find mysqlcppconn; as such the dependency is there but `find_package` might not be.

#### Boost
By default, the boost packages with Ubuntu 15.10 (wily) are built with the old C++ ABI and are therefore completely useless if you're otherwise using a standard toolset for modern C++.

To build boost on the new abi (assuming you've downloaded the latest source of boost from [the boost downloads page](http://www.boost.org/users/download/)):

```
tar xf boost_1_59_0.tar.gz # replace version as appropriate
cd boost_1_59_0
./bootstrap.sh --with-toolset=<clang|gcc> --with-libraries=program_options,filesystem,system # toolset and libraries as needed
./b2 toolset=<clang|gcc> cxxflags="-std=c++14 -w" define=_GLIBCXX_USE_CXX11_ABI=1 -j4
```

### Windows
Boost is best built manually if you've not already got an installation of it, and you might need to set -DBOOST_ROOT=<path>.

Similar goes for OpenSSL; see [this guide](http://developer.covenanteyes.com/building-openssl-for-visual-studio/) for details (run 7-zip as admin to unzip and preserve symlinks)

When using CMake, testing is done for -G "NMake Makefiles". Generating projects files should work (but if you have experience in this area, testing is appreciated!)

### OS X
No attempt is made to run either the client or server on OS X due to a lack of hardware for testing and experience. If you want to implement it, feel free! In theory, there's nothing stopping the OS X deployment being similar to Linux, but platform-specific bugs/differences always crop up. 