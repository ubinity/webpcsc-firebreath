webpcsc-firebreath
====================

Browser plug-in interface to PCSC using FireBreath, exposing methods through JavaScript interfaces (for Linux, Windows, and oneday OS X)

INPORTANT NOTE
--------------

AS google announced the end of NAPI support and Firefox is going to do the same thing, webpcsc is 
under refactor to anticipate the failing  removal.
the previous version is available under "0.2" branch.

The master branch, targeting 0.3 version, is under active development with the following design in mind:

  * only target chrome browser
  * abstract the PCSC function in is stable user .js file (what has been started in v0.2)
  * pcsc calls (or aka native) are moved in a chrome extension
  * asynchronous call in user page thanks to usage of extension

We try to keep the master branch in a functional state, but documentation maybe broken 
and code sometime buggy. Do not hesitate to report and contribute.

For now, the native back-end stay the same still relies on FireBreath, as this framework will certainly be
upgraded too.


How to use 
-----------

See dist/doc/index.html for documentation and dist/tests/test.html for a short sample 


How to build
-------------


### Short version:

  * Download FireBreath from http://www.firebreath.org/
  * Rebuild FireBreath "makefiles" with prepmake - see http://colonelpanic.net/2010/11/firebreath-tips-working-with-source-control/
  * Build FireBreath plug-in for your target platform(s) - http://www.firebreath.org/display/documentation/Building+FireBreath+Plugins


### Long version:

  * Download FireBreath from http://www.firebreath.org/
  * Let's say FBPlugins is the FireBreath root 
  * Git clone pcsc project, you should have:   $(FBPlugins)/projects/PCSCBridge/PCSCBridge.cpp

Note that this README.md will override the one of FireBreath if you git-clone in place.


#### Rebuild the native GNU/Linux plugin:

     Before you start, please install libpcsclite libs to compile:
     apt-get install pcscd libpcsclite1 libpcsclite-dev

     Copy $(FBPlugins)/ext/FindPCSCLite.cmake into /usr/share/cmake-2.8/Modules/ (or something like that)
     
     In $(FBPlugins)
       prepmake.sh  [-D CMAKE_BUILD_TYPE="Debug"]
       cd build
       make
     
     .so is here:
        $(FBPlugins)/build/bin/PCSCBridge/npPCSCBridge.so
     Copy .so to  $(FBPlugins)/dist/lib
     

#### Rebuild the native Windows plugin:

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
     Copy dll to  $(FBPlugins)/dist/lib


#### Building browser extension

Assuming you have built the GNU/linux and windows native libraries.

   $(FBPlugins)/dist/
      ./dist-pack.sh

Extension are placed in 

     $(FBPlugins)/dist/chrome
     $(FBPlugins)/dist/firefox

Unpacked extension, useful for debugging, are kept here:

     $(FBPlugins)/dist/chrome-unpaked
     $(FBPlugins)/dist/firefox-unpaked


When loading chrome packed extension, tick the 'Allow access to file URLs' once loaded.

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


