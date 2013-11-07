/*
************************************************************************
Copyright (c) 2013 UBINITY SAS, Cédric Mesnil <cedric.mesnil@ubinity.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************
*/
/**
 * @project WEBPCSC
 * @author Cédric Mesnil <cedric.mesnil@ubinity.com>
 * @license Apache License, Version 2.0
 */



/**
 * SCard main space
 * Note:
 *   All function with return value noted {Q<x>},means the function return a promise for a value of type x
 * @namespace scardjs
 */
var scardjs = {
        /* ============================== VERSION ============================== */
        /** Revision */
        VERSION: "0.2.99",


        /* ============================== POSTMAN ============================== */

        /** @private */
        padlock: 0,

        /** @private */
        locks: {},

        /** 
         * Lock object for an asynchrone operation and create associated promise
         *
         * @param {any}    obj the this object the asynchone lock belongs
         * @param {number} id  the lock semaphore identifier
         * @private 
         */
        lock: function(obj, id) {
                if (scardjs.locks["_"+id]) {
                        throw "SCARD Pending lock for "+id;
                }
                var lock =  { 
                        deferred: Q.defer(),
                        this:     obj,
                };
                scardjs.locks["_"+id] = lock
                return lock;
        },
        
        /** 
         * Unlock object for an asynchrone operation and return the padlock
         *
         * @param {any}    obj the this object the asynchone lock belongs
         * @param {number} id  the lock semaphore identifier
         * @private 
         */
        unlock: function (id) {
                if (scardjs.locks["_"+id]==undefined) {
                        console.log("SCARD No pending lock for" +id);
                        return;
                }
                var lock =  scardjs.locks["_"+id];
                delete scardjs.locks["_"+id];
                return lock;
        },

        /**
         * the toString() for posted message...
         * @return {string} msg as string
         *
         * @private
         */
        msgStr: function msgStr(msg) {
                var str="";
                for(var name in msg) {
                        if (msg.hasOwnProperty(name)) {
                                if (name == "args") {
                                        str += "\n  args {";
                                        str += msgStr(msg[name])
                                        str += "\n  }";
                                } else {
                                        str = str+"\n"+name+":  "+msg[name];
                                }
                        }
                }
                return str+"\n";
        },

        /** 
         * Log to console.
         * Can be used as setLogger parameter.
         */
        consoleLogger: function(str) {
                console.log(msg);
        },

        /**
         * /dev/null logger.
         * Can be used as setLogger parameter.
         */
        noneLogger: function (str) {
                return;
        },

        /**
         * Set debug logger. 
         * fn shall be a function taking string as first argument and doing something with it...
         * 
         * @param {function} [fn] logger
         */
        setLogger: function(fn) {
                if (fn == undefined) {
                        fn = scardjs.noneLogger;
                }
                scardjs.log = fn;
        },


        /**
         * Postman for incoming message.
         * It unlock the object and resolve the pending promise
         * @private
         */
        scardHandler: function(event) {
                // We only accept messages from ourselves
                if (event.source != window) {
                        return;
                }
                if (event.data.destination && (event.data.destination == "SCARD")) {
                        scardjs.log("POST: " + scardjs.msgStr(event.data));
                        return;
                }
                if (event.data.destination && (event.data.destination == "DRACS")) {
                        scardjs.log("RECV: " + scardjs.msgStr(event.data));
                        var lock = scardjs.unlock(event.data.lock);
                        lock.deferred.resolve(event.data);
                }
        },

        /* ============================== CONSTANT ============================== */
        /** 
         * Initialize the scardjs lib. 
         * It shall be call once, before any other function
         */
        init: function() {
                var Z = scardjs.lock(this, "INIT");
                window.postMessage({ destination: "SCARD", 
                                     lock:        "INIT",
                                     func:        "getSCardConsts",
                                     args: {
                                     }
                                   },
                                   "*");
                return Z.deferred.promise.then(function(result) {
                        for (var name in result.args) {
                                scardjs[name] = result.args[name];
                        };
                        return result.ret;
                });
        },

        /* ============================== PCSC ============================== */
        
        /**
         * Build a new SCARD context 
         * @constructor
         */
        SCardContext: function () {
                var lockID = ++scardjs.padlock;

                /**
                 * Establish new PCSC context
                 * @params {number} [scope] as defined by pcss. default is SCARD_SCOPE_SYSTEM 
                 * @return hContext
                 */
                this.establish = function(scope) {      
                        var Z = scardjs.lock(this, lockID);
                        if (!scope) {
                                scope = 2;
                        }
                        this.hContext = -1;                        
                        window.postMessage({ destination: "SCARD", 
                                             lock:        lockID,
                                             func:        "EstablishContext",
                                             args: {
                                                     dwScope:  scope,
                                                     hContext: 0,
                                             }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function(result) {
                                Z.this.hContext = result.args.hContext;
                                result.ret = result.args.hContext                                
                                Z.message = result;
                                return result.ret;
                        });
                        return p;                        
                };
                
                /** 
                 * Return array of available reader names 
                 * @return {string[]} reader list
                 */
                this.listReaders = function() {
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({ destination: "SCARD", 
                                             lock:       lockID,
                                             func:       "ListReaders",
                                             args: {
                                                     hContext:   this.hContext,
                                                     strGroups:  "",
                                                     strReaders: undefined,
                                             }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function(result) {
                                if (result.args.err != scardjs.SCARD_S_SUCCESS) {
                                        return undefined;
                                }
                                
                                var readers  = [];
                                result.args.strReaders.forEach(function (r) {
                                        readers.push(r);
                                });
                                result.ret = readers;
                                Z.message = result;
                                return result.ret;
                        });                        
                        return p;
                };

                /**
                 * Release this scard context 
                 * This context is not usabel anymore
                 */
                this.release = function() {
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({ destination: "SCARD", 
                                             lock:      lockID,
                                             func:      "ReleaseContext",
                                             args: {
                                                     hContext:  this.hContext,
                                             }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function(result) {
                                Z.this.message = result;
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        this.hContext = -1;
                                        result.ret = true;

                                } else {
                                        result.ret = false;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                };

                /** 
                 * Build a scardjs.SCardReader for the given reader name.
                 * @param {string} reader Reader to get. Can be a regex.
                 * @return a fresh scardjs.SCardReader, or undefined if failed.
                 */
                this.getReader = function(reader) {
                        var fullname = undefined;
                        var scctx    = this;
                        var sc_reader = undefined;
                        var p = this.listReaders().then(function(readers) {
                                if (reader.length == 0) {
                                        return undefined;
                                }
                                if (!reader) {
                                        reader = readers[0];
                                } else {
                                        readers.every(function (r) {
                                                if ((r.indexOf(reader) == 0) ||
                                                    r.match(reader)) {
                                                        fullname = r;
                                                        return false;
                                                }
                                                return true;
                                        });
                                }
                                if (fullname) {
                                        sc_reader = new scardjs.SCardReader(fullname, scctx);
                                        return sc_reader;
                                }
                                fullname = "";
                                return undefined;
                        });
                        return p
                };

                /** 
                 * 
                 * @param {array}  currentStates array of {name: <readerName:str>, state: <currentState:int> }
                 *
                 * @param {number} timeout       number, scarjs.INFINITE means infinite
                 *
                 * @return {array} new state as an array of {name: <readerName:string>, state: <currentState:number>,  eventState: {event:number}, ATR: {currentATR:string}}
                 */
                this.getStatusChange = function(currentStates,timeout) {
                        if (!(this.hCard>0)) {
                                return false;
                        }
                        if (timeout == undefined) {
                                timeout = 0;
                        }
                        var Z = scardjs.lock(this, lockID);
                        var msg = {destination: "SCARD",
                                   lock:        lockID,
                                   func:        "GetStatusChange",
                                   args : {
                                           hContext:          this.scardCtx.hContext,
                                           dwTimeout:         timeout,
                                           readerStates:      []
                                   }
                                  };
                        currentStates.forEach(function (r) {
                                msg.args.readerStates.push({
                                        strReader:      r.name,
                                        dwCurrentState: r.state,
                                        dwEventState:   0,
                                        bAtr:           ""
                                });
                        });
                        window.postMessage(msg, "*");
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        var currentStates = [];
                                        result.args.readerStates.forEach(function(r) {
                                                currentStates.push({
                                                        name:       r.strReader,
                                                        state:      r.dwCurrentState,
                                                        eventState: r.dwEventState,
                                                        ATR:        r.bAtr
                                                });   
                                        });
                                        result.ret = currentStates;
                                } else {
                                        result.ret = undefined;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                }

                // PRIVATE
                this.err          = 0;
                this.hContext     = -1;    
        },



        
        /**
         * A reader object, build with an SCardContext instance from getReader interface.
         * @constructor
         * @param {String}       reader     the reader full-name
         * @param {SCardContext} scardCtx   SCardContext instance building this reader.
         */
        SCardReader: function (reader, scardCtx) {
                var lockID = ++scardjs.padlock;
                /** 
                 * Connect to reader.
                 * If mode is not specified, it is set SCARD_SHARE_SHARED
                 * Underlying PCSCAPI: SCardConnect.
                 * @param {number} mode  one of SCARD_SHARE_SHARED, SCARD_SHARE_EXCLUSIVE, SCARD_SHARE_DIRECT.
                 */
                this.connect = function (mode) {
                        if(!mode) {
                                mode = scardjs.SCARD_SHARE_SHARED;
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD", 
                                            lock:        lockID,
                                            func:        "Connect",
                                            args : {
                                                    hContext:             this.scardCtx.hContext,
                                                    strReader:            this.reader,
                                                    dwShareMode:          mode,
                                                    dwPreferredProtocols: 3,
                                                    hCard:                0,
                                                    dwActiveProtocol:     0
                                            }
                                           },
                                           "*");
                        
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err != scardjs.SCARD_S_SUCCESS) {
                                         result.ret = false;
                                } else {
                                        Z.this.hCard =  result.args.hCard;
                                        Z.this.dwProtocol =  result.args.dwActiveProtocol;
                                        result.ret = true;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                };

                /**
                 * Power up the card.
                 * Underlying PCSCAPI: SCardStatus.
                 * @return {array}  [{hexstring} atr, {number} reader full-name, {number}  state]
                 */
                this.status = function() {
                        if (!(this.hCard>0)) {
                                return Q(undefined);
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD", 
                                            lock:        lockID,
                                            func:        "Status",
                                            args : {
                                                    hCard:         this.hCard,
                                                    strReaderName: "",
                                                    dwState:       0,
                                                    dwProtocol:    0,
                                                    bATR:          ""
                                            }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function (result) {
                                Z.this.message = result;
                                if (result.args.err != scardjs.SCARD_S_SUCCESS) {
                                        return result.ret = undefined;
                                } else {
                                        Z.this.dwProtocol = result.args.dwProtocol;
                                        result.ret = [result.args.bATR, result.args.strReaderName, result.args.dwProtocol, result.args.dwState];
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                };

                /**
                 * Transmit APDU.
                 * Underlying PCSCAPI: SCardTransmit.
                 * @param  {hexstring}  apdu command  APDU
                 * @return {hexstring}       response APDU , including status word SW.
                 */
                this.transmit = function(apdu) {
                        if (!(this.hCard>0)) {
                                return Q(undefined);
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD", 
                                            lock:      lockID,
                                            func:       "Transmit",
                                            args : {
                                                    func:          arguments.callee.name,
                                                    hCard:         this.hCard,
                                                    ioSendPci:     this.dwProtocol,
                                                    bSendBuffer:   apdu,
                                                    ioRecvPci:     this.dwProtocol,
                                                    bRecvBuffer:   ""
                                            }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err != scardjs.SCARD_S_SUCCESS) {
                                        result.ret = undefined;
                                } else {
                                        result.ret = result.args.bRecvBuffer;
                                }
                                Z.message = result;
                                return result.ret
                        });
                        return p;
                };

                /**
                 * Disconnect from reader.
                 * Underlying PCSCAPI: SCardDisconect.
                 * @param {hexstring} disposition one of SCARD_LEAVE_CARD, SCARD_RESET_CARD, SCARD_UNPOWER_CARD, SCARD_EJECT_CARD.
                 * @return true if success, false else.
                 */
                this.disconnect = function(disposition){                
                        if (!(this.hCard>0)) {
                                return false;
                        }
                        var Z = scardjs.lock(this, lockID);
                        if (!disposition) {
                                disposition = scardjs.SCARD_LEAVE_CARD;
                        }
                        
                        window.postMessage({destination: "SCARD", 
                                            lock:        lockID,
                                            func:        "Disconnect",
                                            args : {
                                                    hCard:         this.hCard,
                                                    dwDisposition: disposition,
                                            }
                                           },
                                           "*");

                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        Z.this.hCard = 0;
                                        result.ret = true;
                                } else {
                                        result.ret = true;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                };

                /**
                 * Start a transaction
                 * Underlying PCSCAPI: SCardBeginTransaction.
                 * Not yet implemented
                 */
                this.beginTransaction = function() {
                        throw "Not yep implemented: beginTransaction"
                };
                

                /**
                 * End a transaction
                 * Underlying PCSCAPI: SCardEndTransaction.
                 * Not yet implemented
                 */
                this.endTransaction = function() {
                        throw "Not yep implemented: beginTransaction"
                };


                /**
                 * Cancel transaction
                 * Underlying PCSCAPI: SCardCancelTransaction.
                 * Not yet implemented
                 */
                this.cancelTransaction = function() {
                        throw "Not yep implemented: beginTransaction"
                };


                /** 
                 * Get an attribute from the IFD Handler
                 * Underlying PCSCAPI: SCardGetAttrib	
                 * @param {number} attrID  attribute identifier
                 * @return {string} attribute value
                 */
                this.getAttribute = function(attrID) {
                        if (!(this.hCard>0)) {
                                return false;
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD", 
                                            lock:        lockID,
                                            func:        "GetAttribute",
                                            args : {
                                                    hCard:     this.hCard,
                                                    dwAttrId : attrID,
                                                    bAttr:     undefined
                                            }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        result.ret = result.args.bAttr;
                                } else {
                                        result.ret = undefined;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                };

               /** 
                * Set an attribute of the IFD Handler.
                * Underlying PCSCAPI: SCardSetAttrib	
                * @param {number} attrID  attribute identifier
                * @param {string} attribute value
                * @return{number}  true if sucess, false else
                */
                this.setAttribute = function(attrID, attrVal) {
                        if (!(this.hCard>0)) {
                                return false;
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD", 
                                            lock:        lockID,
                                            func:        "SetAttribute",
                                            args : {
                                                    hCard:     this.hCard,
                                                    dwAttrId : attrID,
                                                    bAttr:     attrVal
                                            }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        result.ret = true;
                                } else {
                                        result.ret = false;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                };

                /**
                 * Sends a command directly to the IFD Handler to be processed by the reader.
                 * Underlying PCSCAPI:  SCardControl
                 * @param {number}    ctrlCode  operation controle code
                 * @param {hexstring} ctrlVal   command data
                 */
                this.control = function(ctrlCode, ctrlVal) {
                        if (!(this.hCard>0)) {
                                return false;
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD",
                                            lock:        lockID,
                                            func:        "Control",
                                            args : {
                                                    hCard:     this.hCard,
                                                    dwControlCode:        ctrlCode,
                                                    bSendBuffer:          ctrlVal,
                                                    bRecvBuffer:          ""

                                            }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        result.ret = result.args.bRecvBuffer;
                                } else {
                                        result.ret = undefined;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                }

                /** 
                 * Monitor a reader for change...
                 * Underlying PCSCAPI: SCardGetStatusChange
                 *
                 * @param {assoc}  currentState  Associatve array with {name: <readerName:string>, state: <currentState:number> }
                 * @param {number} timeout       timeout in ms, 0 means infinite
                 */
                this.getStatusChange = function(currentState,timeout) {
                        if (!(this.hCard>0)) {
                                return false;
                        }
                        if (timeout == undefined) {
                                timeout = 0;
                        }
                        var Z = scardjs.lock(this, lockID);
                        window.postMessage({destination: "SCARD",
                                            lock:        lockID,
                                            func:        "GetStatusChange",
                                            args : {
                                                    hContext:          this.scardCtx.hContext,
                                                    dwTimeout:         timeout,
                                                    readerStates: [ 
                                                            {
                                                                    strReader:  this.reader,
                                                                    dwCurrentState: currentState,
                                                                    dwEventState:   0,
                                                                    bAtr:           "",
                                                            }
                                                    ]
                                            }
                                           },
                                           "*");
                        var p = Z.deferred.promise.then(function (result) {
                                if (result.args.err == scardjs.SCARD_S_SUCCESS) {
                                        result.ret = {
                                                name:       result.args.readerStates[0].strReader,
                                                state:      result.args.readerStates[0].dwCurrentState,
                                                eventState: result.args.readerStates[0].dwEventState, 
                                                ATR:        result.args.readerStates[0].bAtr};
                                } else {
                                        result.ret = undefined;
                                }
                                Z.message = result;
                                return result.ret;
                        });
                        return p;
                }
                
                /** 
                 * Set this reader config. Configuration changes behavior of exchange function.
                 *
                 * @param {assoc} config  assoc array { autoGetResponse: true|false, autoReissue: true|false, autoChaining: true|false,  extendedLengthSupported: true|false }        
                 */
                this.setConfig = function(config) {
                        this.config.autoGetResponse         = config.autoGetResponse ;
                        this.config.autoReissue             = config.autoReissue ;
                        this.config.autoChaining            = config.autoChaining ;
                        this.config.extendedLengthSupported = config.extendedLengthSupported ;
                };

                /**
                 * Retrieve current config.
                 * See setConfig and exchange.
                 * @return {assoc} config
                 */
                this.getConfig = function() {
                        return {
                                autoGetResponse         : this.config.autoGetResponse,
                                autoReissue             : this.config.autoReissue,
                                autoChaining            : this.config.autoChaining,
                                extendedLengthSupported : this.config.extendedLengthSupported
                        };
                };
                

                /**
                 * Powerup the card.
                 * Short cut for connect then status
                 * @return {hexstring} ATR or undefined
                 */ 
                this.powerUp = function() {
                        var _this = this;
                        _this.powerDown()
                                .then(function(result) {
                                        return _this.connect();
                                })
                                .then(function(connected) {
                                        if (connected) {
                                                return _this.status();
                                        } else {
                                                return undefined;
                                        }
                                })
                                .then(function (s) {
                                        if (s) {
                                                return s[0];
                                        }
                                        return undefined;
                                });
                };

                /**
                 * Powerdown the card.
                 * Short cut for disconnect with mode equals to SCARD_UNPOWER_CARD
                 */ 
                this.powerDown = function() {
                        if (this.hCard>0) {
                                return this.disconnect(this.scardCtx.bridge.SCARD_UNPOWER_CARD);
                        } else {
                                return Q(false);
                        }
                };

                /**
                 * Reset the card.
                 * Short cut for disconnect with mode equals to SCARD_RESET_CARD
                 * @return {hexstring} ATR or undefined               
                 */ 
                this.reset = function() {
                        if (this.hCard>0) {
                                return this.disconnect(this.scardCtx.bridge.SCARD_RESET_CARD);
                        } else {
                                return Q(false);
                        }
                };


                /**
                 * Exchange APDU and manage some protocol behavior according to config.
                 *
                 * Declared extendedAPDU=true, must include extended lc and extended le in the data. 
                 * p3 value is ignored and set to zero.
                 * 
                 * autoGetResponse:
                 *
                 *   - if set, the GET_RESPONSE apdu is automatically send when receiving a 61xx status
                 *
                 *
                 * autoReissue:
                 *
                 *   - if set, the APDU is automatically reissued with adjusted le when receiving a 6Cxx status
                 *
                 *
                 * autoChaining:
                 *
                 *   - if set and if lc>255 and if extendedAPDU is false, ISO7816 chaining is automatically 
                 *     used and managed
                 *   - if set and  if extendedAPDU is true and if extendedLengthSupported is false, ISO7816 
                 *     chaining is automatically used and managed
                 *
                 *
                 * extendedLengthSupported:
                 *
                 *   - if set, extended APDU are sent as is, else it depends on autoChaining value
                 *
                 *
                 * @param  {hextring}   apdu            command to send
                 * @param  {boolean}    [extendedAPDU]  apdu is an extended one
                 * @return {hexstring}  response apdu
                 */
                this.exchange = function(apdu, extendedAPDU) {
                        var _this = this;

                        if (!(_this.hCard>0)) {
                                return Q(undefined);
                        }
                        var cla  = parseInt(apdu.substring(0,2), 16); 
                        var ins  = parseInt(apdu.substring(2,4), 16);
                        var p1   = parseInt(apdu.substring(4,6), 16);
                        var p2   = parseInt(apdu.substring(6,8), 16);
                        var p3   = parseInt(apdu.substring(8,10), 16);
                        var data = apdu.substring(10);
                        var lc, le;

                        var chaining = false;
                        var rdata = [];
                        var sw;

                        //if outgoing extended APDU, and card does not support it, 
                        //handle chaining automagically
                        if (extendedAPDU) {
                                switch (this.dwProtocol) {
                                        //in T1, nothing todo: APDU => TPDU
                                case 2:
                                        break;
                                        
                                case 1:
                                        //in T0, either envoppe or chaining
                                if (!_this.config.extendedLengthSupported && _this.config.autoChaining) {
                                        lc  = parseInt(apdu.substring(10,14), 16);
                                        le = parseInt(data.substring(data.length-4), 16);
                                        data = data.substring(4, data.length-4);
                                        if (lc <= 256) {
                                                apdu = 
                                                        scardjs.hexl1(cla) +
                                                        scardjs.hexl1(ins) +
                                                        scardjs.hexl1(p1)  +
                                                        scardjs.hexl1(p2)  +
                                                        scardjs.hexl1(lc)  + 
                                                        data;
                                        } else {
                                                chaining = true;       
                                        }
                                } else if (_this.config.extendedLengthSupported) {
                                        throw "Extended APDU with T0 ENVELOPPE not implemented";
                                } else {
                                        throw "Extended APDU not supported by card";
                                }
                                        break;

                                default:
                                        throw "Unhandled Exhange Protocol";
                                }
                        }

                        var p = Q(chaining).then(function(chain) {
                                if (chain) {
                                        function loopTransmit(rapdu) {
                                                var pbSendBuffer;
                                                sw = parseInt(rapdu.substring(rapdu.length-4), 16);
                                                if ((lc=0) || (sw != 0x9000)) {
                                                        return Q(rapdu);
                                                }
                                                if (lc <232) {
                                                        p3 =lc;
                                                        cla &= ~0x10;
                                                        lc = 0;
                                                } else {
                                                        cla |= 0x10;
                                                        p3   = 232;
                                                        lc  -= 232;
                                                }
                                                pbSendBuffer = 
                                                        scardjs.hexl1(cla) +
                                                        scardjs.hexl1(ins) +
                                                        scardjs.hexl1(p1)  +
                                                        scardjs.hexl1(p2)  +
                                                        scardjs.hexl1(p3)  +                                        
                                                        data.substring(0,p3*2);
                                                data=data.substring(p3*2);
                                                return  _this.transmit(pbSendBuffer).then(loopTransmit);
                                        }
                                        return loopTransmit("9000");
                                } 
                                //else just send and see....
                                else {
                                        return  _this.transmit(apdu);
                                }
                        }).then(function(rapdu) {
                                //handle 6Cxx, if autoReissue set
                                var pbSendBuffer;
                                sw = parseInt(rapdu.substring(rapdu.length-4), 16); 
                                rdata = rapdu.substring(0, rapdu.length-4); 
                                if ( ((sw & 0xFF00) == 0x6C00) && _this.config.autoReissue) {
                                        pbSendBuffer =
                                                scardjs.hexl1(cla) +
                                                scardjs.hexl1(ins) +
                                                scardjs.hexl1(p1)  +
                                                scardjs.hexl1(p2)  +
                                                scardjs.hexl1(sw&0xFF) +                                        
                                                data;
                                        return  _this.transmit(pbSendBuffer);
                                } else {
                                        return Q(rapdu);
                                }
                        }).then(function (rapdu) {
                                //handle 61xx, if autoGetResponse set
                                var pbSendBuffer;
                                sw = parseInt(rapdu.substring(rapdu.length-4), 16); 
                                if ( ((sw &0xFF00) == 0x6100) && _this.config.autoGetResponse) {
                                        function loopReceive(rapdu) {
                                                sw = parseInt(rapdu.substring(rapdu.length-4), 16);
                                                if ((sw&0xFF00) != 0x6100) {
                                                        return Q(rdata+rapdu);
                                                }
                                                rdata = rdata +  rapdu.substring(0, rapdu.length-4);
                                                p3 = sw&0xFF;
                                                pbSendBuffer =
                                                        scardjs.hexl1(0x00) +
                                                        scardjs.hexl1(0xC0) +
                                                        scardjs.hexl1(0x00) +
                                                        scardjs.hexl1(0x00) +
                                                        scardjs.hexl1(sw&0xFF);
                                                return  _this.transmit(pbSendBuffer).then(loopReceive);
                                        }
                                        rdata = "";
                                        return loopReceive(rapdu);
                                } else {
                                        return Q(rapdu);
                                }
                        });                          
                        return p;
                };

                //R ONLY
                this.reader     = reader;
                this.scardCtx   = scardCtx;        
                //RW
                this.hCard      = -1;        
                this.dwProtocol = -1;

                //config
                this.config ={
                        autoGetResponse         : true,
                        autoReissue             : true,
                        autoChaining            : true,
                        extendedLengthSupported : false,
                };

        },


         /* ============================== UTILS ============================== */
        
        /* 
         * Convert a number to hexstring
         * @param {nunber}   n   number to encode
         * @param {number]   l   minimal hexa-byte length of hexstring
         * @return{hextring}     String hex, without 'Ox'
         */
        hexl: function (n,l) {
                //round up l
                l = l*2; 
                var h = n.toString(16);
                
                if ((h.length&1) == 1) h = "0"+h;
                //pad
                while(h.length<l) {
                        h = "00"+h;
                }
                //trunk
                if (h.length>l) {
                        h = h.substr(h.length>l,l);
                }
                //ret
                return h.toUpperCase();
        },

        /** Same hexl(n,1), aka one byte, aka xx */
        hexl1: function (n) {
                return scardjs.hexl(n,1);
        },
        /** Same hexl(n,2), aka one byte, aka xxyy */
        hexl2: function (n) {
                return scardjs.hexl(n,2);
        },
        /** Same hexl(n,4), aka one byte, aka xxxxyyzz */
        hexl4: function (n) {
                return scardjs.hexl(n,4);
        },

};

window.addEventListener("message", scardjs.scardHandler, false);
