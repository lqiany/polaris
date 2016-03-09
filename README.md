Overview
=========
POLARIS provides libraries and tools to help transportation 
engineers to create transportation system simulations. 
In particular to simulate a Traffic Management 
Center along with ITS infrastructure. 
It provides implementation of vented models 
in transportation community so that those 
can be reused by application developers.

The project is distributed under BSD liense.

Please cite POLARIS using the following reference:
Auld, J., Hope, M., Ley, H., Sokolov, V., Xu, B., Zhang, K. (2016). POLARIS: Agent-based modeling framework development and 
	implementation for integrated travel demand and network and operations simulations. Transportation Research Part C:
	Emerging Technologies. doi:10.1016/j.trc.2015.07.017

POLARIS Dependencies
--------------------
Here is the list of dependencies:
* Boost 1.54
* gperftools-2.1
* odb-2.2.2
* odb-qlite-2.2.2
* sqlite3
* spatialite4
In addition to the dependencies above, if you would like to build an application that uses the graphical
front end of Polaris - Antares, you will need to have the following dependencies
* wxWidgets-2.9.3
* plplot
* glew-1.7.0
Also, if you wold like to compile unit tests, you will need to have
* gtest-1.7.0

We do distribute compiled all of the dependencies for a windows machine,
so you do not have to go through pain of downloading/installing those.
You can download compiled dependencies from [here](https://sourceforge.net/projects/polaris-t/files/latest/download?source=files "Dependencies for Windows").
Extract to c:\opt. The binaries provided in the polarisdeps folder are compiled for x64 platform. Make sure you build an application for an x64 platform as well.

We've tested the application on a 64-bit windows 8/7 machines. We haven't tested it under any of Linux platforms yet.

There is a convenience polaris_env-x64.bat script that appends all of the paths with necessary dlls to the PATH varaible so those can be found when you run an application.

Building Using Cmake
--------------------
You can download cmake from here [here](http://www.cmake.org/files/v2.8/cmake-2.8.12-win32-x86.zip "cmake").
Make sure it is added to the PATH

Open CmakeLists.txt and make sure that BOOST_ROOT is set up properly

run the following commands:
mkdir build
cd build
cmake -G"Visual Studio 10 Win64" ..
This will generate a polaris.sln file you can open in Visual Studio 10
Switch to MinSizeRel and build




