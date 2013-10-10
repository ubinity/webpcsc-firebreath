webpcsc-firebreath
====================

Browser plug-in interface to PCSC using FireBreath, exposing methods through JavaScript interfaces (for Linux, Windows, and oneday OS X)

How to use 
-----------

See dist/doc/PCSCBridge.html for documentation and projects/PCSCBridge/test/test.html for a short sample 


How to build
-------------


### Short version:

  * Download FireBreath from http://www.firebreath.org/
  * Rebuild FireBreath "makefiles" with prepmake - see http://colonelpanic.net/2010/11/firebreath-tips-working-with-source-control/
  * Build FireBreath plug-in for your target platform(s) - http://www.firebreath.org/display/documentation/Building+FireBreath+Plugins


### Long version:

  * Download FireBreath from http://www.firebreath.org/
  * Let's say FBPlugins is the firebreath root 
  * Git clone pcsc project, you should have:   $(FBPlugins)/projects/PCSCBridge/PCSCBridge.cpp

Note that this README.md will overide the one of firebreath if you git-clone in place.

#### Under Linux:

     Before you start, please install libpcsclite libs to compile:
     apt-get install pcscd libpcsclite1 libpcsclite-dev

     Copy $(FBPlugins)/ext/FindPCSCLite.cmake into /usr/share/cmake-2.8/Modules/ (or something like that)
     
     In $(FBPlugins)
       prepmake.sh  [-D CMAKE_BUILD_TYPE="Debug"]
       cd build
       make
     
     .so is here:
        $(FBPlugins)/build/bin/PCSCBridge/npPCSCBridge.so
     Copy .so to ~/.mozilla/plugins
    


#### Under Windows

     Open  $(FBPlugins)/projects/PCSCBridge/projectDef.cmake
     
     Check path at line:
           set_target_properties(${PROJECT_NAME} 
                                 PROPERTIES LINK_FLAGS  "/LIBPATH:\"C:\\WinDDK\\7600.16385.1\\lib\\ATL\\i386\" winscard.lib")
     
     In $(FBPlugins)
       ./prep2010.cmd
     open project $(FBPlugins)/build/projects/PCSCBridge/PCSCBridge.sln with VCexpress 2010
     build
     
     dll is here
       $(FBPlugins)/build/bin/PCSCBridge/Debug/npPCSCBridge.dll
     register it:
       regserv32 npPCSCBridge.dll






License
-------

	Copyright (c) 2013 UBINITY SAS 

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

       	 http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.


