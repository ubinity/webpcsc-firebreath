WEBPCSC: SCARDJS
================

V.2.99.0
Browser plug-in interface to PCSC using FireBreath, exposing methods through JavaScript interfaces (for Linux, Windows, and oneday OS X)

Note
-----
Chrome targeted only for now, see README.md at project root 


A word...
---------
This API relies on Q-Promise and user/extension architecture.

It means that any scardjs call  involving a PCSC API is sent asynchronously to the extension managing the PCSC native call.
The scardjs  function return a promise for the result.

In a user point of view, it means the code to send an APDU look like that:
  
        function handleRapdu(rapdu) {
                 //do something
        }
        read.send("00A4040000").then(handresp)


or more concisely:

        read.send("00A4040000").then(function(rapdu) { 
                 //do something
        });


 - See dist/test/test.html
 - See dist/doc/old/
 - See https://github.com/kriskowal/q/tree/v0.9

 - See http://pcsclite.alioth.debian.org/api/


License
-------

	Copyright (c) 2013 UBINITY SAS, <cedric.mesnil@ubinity.com>

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

       	 http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.


