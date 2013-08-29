
/*
************************************************************************
Copyright (c) 2013 Ubinity SAS - Cédric Mesnil

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

/** scardjs domain */
var scardjs = {
        /* ============================== VERSION ============================== */
        VERSION: "0.3",

        /* ============================== UTILS ============================== */
        
        /* 
         * Convert a number to hexstring
         * @param {nunber} n    number to encode
         * @param {number] l:   minimal hexa-byte length of hexstring
         * @return String hex, without 'Ox'
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

        /* Same hexl(n,1), aka one byte, aka xx */
        hexl1: function (n) {
                return scardjs.hexl(n,1);
        },
        /* Same hexl(n,2), aka one byte, aka xxyy */
        hexl2: function (n) {
                return scardjs.hexl(n,2);
        },
        /* Same hexl(n,4), aka one byte, aka xxxxyyzz */
        hexl4: function (n) {
                return scardjs.hexl(n,4);
        },

        /* ============================== PCSC ============================== */
        
        /** 
         * Retrieve  primary webpcsc plugin interface.
         * Contrains root API and SCARD constants
         *
         * @return bridge root entry point or undefined 
         */
        getPCSCBridge: function () {
                return document.getElementById('pcscbridge');
        },
        
        /**
         * Retrieve the SCARD API Object
         */
        getSCardAPI: function () {
                var bridge     = getPCSCBridge()
                var scard = undefined;
                if (bridge) {
                        scard     = bridge.getSCardAPI();
                }
                return scard;
        },
        
        /**
         * Build a new SCARD conntext 
         * @constructor
         */
        SCardContext: function () {
                
                /**
         * Establish new PCSC context
         *  @params {number} scope one of 
         * return hContext
         */
                function establish(scope) {      
                        if (!scope) {
                                scope = 2;
                        }
                        this.hContext = -1;
                        this.params = {
                                func:     arguments.callee.name,
                                dwScope:  scope,
                                hContext: 0
                        };
                        this.params.err = this.scard.EstablishContext(this.params);
                        if (this.params.err == this.bridge.SCARD_S_SUCCESS) {
                                this.hContext = this.params.hContext;
                        }
                        return this.hContext;
                };
                
                /**
                 * Lookup for a reader that match the givent name and return a new SCardReader if found.
                 * @return SCardReader or undefined
                 */
                function getReader(reader) {
                        var fullname = undefined;
                        var readers = this.listReaders();
                        if (reader.length == 0) {
                                return undefined;
                        }
                        if (!reader) {
                                reader = readers[0];
                        } else {
                                readers.every(function (r) {
                                        if (r.match(reader)) {
                                                fullname = r;
                                                return false;
                                        }
                                        return true;
                                });
                        }
                        if (fullname) {
                                return new scardjs.SCardReader(fullname, this);
                        }
                        return undefined;
                };
                
                /** Return array of available reader names */
                function listReaders() {
                        this.params = {
                                func:          arguments.callee.name,
                                hContext:      this.hContext,
                                strGroups:     "",
                                strReaders:    undefined,
                        }
                        this.params.err = this.scard.ListReaders(this.params); 
                        if (this.params.err != this.bridge.SCARD_S_SUCCESS) {
                                return undefined;
                        }
                        
                        var readers  = [];
                        this.params.strReaders.forEach(function (r) {
                                readers.push(r);
                        });
                        return readers;                
                };
                
                /** Release this scard context */
                function release() {
                        this.params = {
                                func:          arguments.callee.name,
                                hContext:      this.hContext
                        }
                        
                this.params.err = this.scard.ReleaseContext(this.params);
                        if (this.params.err == this.bridge.SCARD_S_SUCCESS) {
                                this.hContext = -1;
                                return true;
                        }
                        return false;
                }
        
                // PRIVATE
                this.bridge      = getPCSCBridge();
                this.scard       = this.bridge .getSCardAPI();
                this.err         = 0;
                this.hContext    = -1;    
                
                // INTERFACE
                this.establish   = establish;
                this.release     = release;
                this.getReader   = getReader;
                this.listReaders = listReaders;
        },

        
        /**
         * A reader object, build with an SCardContext instance from getReader interface.
         * @constructor
         * @param {String} r     eader     the reader full-name
         *  @param {SCardContext} scardCtx  SCardContext instance building this reader.
         * 
         */
        SCardReader: function (reader, scardCtx) {

                /** Connect to reader.
                 * If mode is not specified, it is set SCARD_SHARE_SHARED.
                 * Underlying PCSCAPI: SCardConnect.
                 * @param {number} mode  one of SCARD_SHARE_SHARED, SCARD_SHARE_EXCLUSIVE, SCARD_SHARE_DIRECT.
                 */
                function connect(mode) {
                        if(!mode) {
                                mode = this.scardCtx.bridge.SCARD_SHARE_SHARED;
                        }
                        
                        this.params = {
                                func:                 arguments.callee.name,
                                hContext:             this.scardCtx.hContext,
                                strReader:            this.reader,
                                dwShareMode:          mode,
                                dwPreferredProtocols: 3,
                                hCard:                0,
                                dwActiveProtocol:     0
                        };
                        
                        this.params.err = this.scardCtx.scard.Connect(this.params)&0xffffffffff;
                        if (this.params.err != this.scardCtx.bridge.SCARD_S_SUCCESS) {
                                return false;
                        }
                        this.hCard =  this.params.hCard;
                        this.dwProtocol =  this.params.dwActiveProtocol;
                        return true;
                };

                /**
                 * Power up the card.
                 * Underlying PCSCAPI: SCardStatus.
                 * @return array with [{hexstring} atr, {number} reader full-name, {number}  state]
                 */
                function status() {
                        if (!this.hCard>0) {
                                return undefined;
                        }
                        this.params = {
                                func:          arguments.callee.name,
                                hCard:         this.hCard,
                                strReaderName: "",
                                dwState:       0,
                                dwProtocol:    0,
                                bATR:          ""
                        };
                        this.params.err = this.scardCtx.scard.Status(this.params); 
                        if (this.params.err != this.scardCtx.bridge.SCARD_S_SUCCESS) {
                                return undefined;
                        }
                        this.dwProtocol = this.params.dwProtocol;
                        return [this.params.bATR, this.params.strReaderName, this.params.dwProtocol, this.params.dwState];
                };

                /**
                 * Transmit APDU.
                 * Underlying PCSCAPI: SCardTransmit.
                 * @param {hexstring} apdu
                 * @return rapdu as {hexstring}, including status word SW.
                 */
                function transmit(apdu){
                        if (!this.hCard>0) {
                                return undefined;
                        }
                        this.params = {
                                func:          arguments.callee.name,
                                hCard:         this.hCard,
                                ioSendPci:     this.dwProtocol,
                                bSendBuffer:   apdu,
                                ioRecvPci:     this.dwProtocol,
                                bRecvBuffer:   ""
                        };
                        
                        this.params.err = this.scardCtx.scard.Transmit(this.params); 
                        if (this.params.err != this.scardCtx.bridge.SCARD_S_SUCCESS) {
                                return undefined;
                        }
                        return this.params.bRecvBuffer;
                };

                /**
                 * Disconnect from reader.
                 * Underlying PCSCAPI: SCardDisconect.
                 * @param {hexstring} disposition one of SCARD_LEAVE_CARD, SCARD_RESET_CARD, SCARD_UNPOWER_CARD, SCARD_EJECT_CARD.
                 * @return true if success, false else.
                 */
                function disconnect(disposition){                
                        if (!this.hCard>0) {
                                return false;
                        }
                        if (!disposition) {
                                disposition = this.scardCtx.scardCtx.bridge.SCARD_LEAVE_CARD;
                        }
                        this.params = {
                                func:          arguments.callee.name,
                                hCard:         this.hCard,
                                dwDisposition: disposition,
                        }
                        this.params.err = this.scardCtx.scard.Disconnect(this.params);
                        if ( this.params.err == this.scardCtx.bridge.SCARD_S_SUCCESS) {
                                this.hCard = 0;
                                return 0;
                        }
                        return false;
                };


                
                /** 
                 * Set this redear config. Configuration changes behavior of exchange function.
                 *
                 * @param {assoc} config  assoc array { autoGetResponse: true|false, autoReissue: true|false, autoChaining: true|false,  extendedLengthSupported: true|false }        
                 */
                function setConfig(config) {
                        this.config.autoGetResponse         = config.autoGetResponse ;
                        this.config.autoReissue             = config.autoReissue ;
                        this.config.autoChaining            = config.autoChaining ;
                        this.config.extendedLengthSupported = config.extendedLengthSupported ;
                };

                /**
                 * Retrieve current config.
                 * See setConfig and exchange.
                 * @return {assox} config
                 */
                function getConfig() {
                        return {
                                autoGetResponse         : this.config.autoGetResponse,
                                autoReissue             : this.config.autoReissue,
                                autoChaining            : this.config.autoChaining,
                                extendedLengthSupported : this.config.extendedLengthSupported
                        };
                };
                

                /**
                 * Powerup the card.
                 * @return ATR as hexstring or undefined
                 */ 
                function powerUp() {
                        if (this.hCard>0) {
                                this.powerDown();
                        }
                        if (this.connect()) {
                                var s;
                                s = this.status();
                                if (s) {
                                        return s[0];
                                }
                        }
                        return undefined;
                };

                /**
                 * Powerdown the card.
                 */ 
                function powerDown() {
                        return this.disconnect(this.scardCtx.scardCtx.bridge.SCARD_UNPOWER_CARD);
                };

                /**
                 * Reset the card.
                 */ 
                function reset() {
                        return this.disconnect(this.scardCtx.scardCtx.bridge.SCARD_RESET_CARD);
                };


                /**
                 * Exchange APDU and manage some protocol behavior according to config.
                 *
                 * Declared extendedAPDU=true, must include extended lc and extended le in the data. 
                 * p3 value is ignored and set to zero.
                 * 
                 *  autoGetResponse: 
                 *    -if set, the GET_RESPONSE apdu is automatically send when receiving a 61xx status
                 *
                 *  autoReissue:     
                 *    - if set, the APDU is automatically reissued with adjusted le when receiving a 6Cxx status
                 *
                 * autoChaining: 
                 *    - if set and if lc>255 and if extendedAPDU is false, ISO7816 chaining is automatically 
                 *      used and managed
                 *    - if set and  if extendedAPDU is true and if extendedLengthSupported is false, ISO7816 
                 *      chaining is automatically used and managed
                 *  extendedLengthSupported: 
                 *    - if set, extended APDU are sent as is, else it depends on autoChaining value
                 *
                 *
                 */
                function exchange(cla, ins, p1, p2, p3, data, extendedAPDU) {
                        if (!this.hCard>0) {
                                return undefined;
                        }


                        //fix args
                        if (p3 == undefined) {
                                if (data) {
                                        p3 = data.length/2;
                                } else {
                                        p3 = 0;
                                }
                        }
                        if (data == undefined) {
                                data = [];
                        }

                        //if outgoing extended APDU, and card does not support it, 
                        //handle chaining automagically
                        var lc = p3;
                        var chaining = false;
                        var pbSendBuffer;
                        var pbRecvBuffer;
                        var sw;
                        if (extendedAPDU && !this.config.extendedLengthSupported && this.config.autoChaining) {
                                lc = parseInt(data.substring(0,4), 16);
                                data = data.substring(4);
                                var le = parseInt(data.substring(data.length-4), 16);
                                data = data.substring(0,data.length-4);
                                chaining = true;
                        }
                        else if (!extendedAPDU && (p3 > 255)) {
                                chaining = true;
                        }
                        if (chaining) {
                                cla |= 0x10;
                                while (lc > 0) {
                                        if (lc <232) {
                                                p3 =lc;
                                                cla &= ~0x10;
                                                lc -= 232;
                                        } else {
                                                p3=232;
                                                lc = 0;
                                        }
                                        pbSendBuffer = 
                                                scardjs.hexl1(cla) +
                                                scardjs.hexl1(p1) +
                                                scardjs.hexl1(p2) +
                                                scardjs.hexl1(p3) +                                        
                                                data.substring(0,p3*2);
                                        data=substring(p3*2);
                                        pbRecvBuffer = this.transmit(data);
                                        if (!pbRecvBuffer) {
                                                return undefined;
                                        }
                                        sw = parseInt(pbRecvBuffer.substring(pbRecvBuffer.length-4), 16);
                                        if (lc) {
                                                if (sw != 0x9000) {
                                                        return data;
                                                }
                                        } else {
                                                rdata = pbRecvBuffer.substring(0, pbRecvBuffer.length-4);     
                                        }
                                } 
                        }
                        //else just send and see....
                        else {
                                if (p3>255) {
                                        p3 = 0; //assume extended APDU
                                }
                                pbSendBuffer = 
                                        scardjs.hexl1(cla) +
                                        scardjs.hexl1(ins) +
                                        scardjs.hexl1(p1) +
                                        scardjs.hexl1(p2) +
                                        scardjs.hexl1(p3) +                                        
                                        data;
                                pbRecvBuffer = this.transmit(pbSendBuffer);
                                if (!pbRecvBuffer) {
                                        return undefined;
                                }
                                sw = parseInt(pbRecvBuffer.substring(pbRecvBuffer.length-4), 16); 
                                rdata = pbRecvBuffer.substring(0, pbRecvBuffer.length-4); 
                        }
                        
                        //handle 6Cxx, if autoReissue set
                        if ( ((sw & 0xFF00) == 0x6C00) && autoReissue) {
                                pbSendBuffer =
                                        scardjs.hexl1(cla) +
                                        scardjs.hexl1(ins) +
                                        scardjs.hexl1(p1) +
                                        scardjs.hexl1(p2) +
                                        scardjs.hexl1(sw&0xFF) +                                        
                                        data;
                                pbRecvBuffer = this.transmit(pbSendBuffer);
                                if (!pbRecvBuffer) {
                                        return undefined;
                                }
                                sw = parseInt(pbRecvBuffer.substring(pbRecvBuffer.length-4), 16); 
                                rdata = pbRecvBuffer.substring(0, pbRecvBuffer.length-4); 
                        }
                        
                        //handle 61xx, if autoGetResponse set
                        if ( ((sw &0xFF00) == 0x6100) && autoGetResponse) {
                                var rdata = "";
                                while ( (sw &0xFF00) == 0x6100) {
                                        //next offset for data to receive, 61xx removed
                                        pbSendBuffer =
                                                scardjs.hexl1(0x00) +
                                                scardjs.hexl1(0xC0) +
                                                scardjs.hexl1(0x00) +
                                                scardjs.hexl1(0x00) +
                                                scardjs.hexl1(sw&0xFF);
                                        pbRecvBuffer = this.transmit(pbSendBuffer);
                                        if (!pbRecvBuffer) {
                                                return undefined;
                                        }
                                        dwRecvLength += offset;
                                        offset = dwRecvLength-2;
                                }  
                                sw = parseInt(pbRecvBuffer.substring(pbRecvBuffer.length-4), 16); 
                                rdata = rdata+pbRecvBuffer.substring(0, pbRecvBuffer.length-4); 

                        }

                        return [rdata, sw];
                };

                //R ONLY
                this.reader     = reader;
                this.scardCtx   = scardCtx;        
                //RW
                this.hCard      = -1;        
                this.dwProtocol = -1;
                //tracking 
                this.params     = {};

                //config
                this.config ={
                        autoGetResponse         : true,
                        autoReissue             : true,
                        autoChaining            : true,
                        extendedLengthSupported : false,
                };

                //Interface
                this.connect    = connect;
                this.status     = status;
                this.transmit   = transmit;
                this.disconnect = disconnect;
                this.setConfig  = setConfig;
                this.getConfig  = getConfig;
                this.powerUp    = powerUp;
                this.powerDown  = powerDown;
                this.reset      = reset;
                this.exchange   = exchange;

        }
};