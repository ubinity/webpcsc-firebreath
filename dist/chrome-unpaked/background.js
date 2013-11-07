
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

/** scardProxy domain */
var scardProxy = {
        
        /* ============================== VERSION ============================== */
        VERSION: "0.2.1",
        
       /* ============================== Bridge ============================== */
        bridge: undefined,
        
        /* ============================== PCSC ============================== */
        
        /** 
         * Retrieve  primary webpcsc plugin interface.
         * Contains root API and SCARD constants
         *
         * @return bridge root entry point or undefined 
         */
        getPCSCBridge: function () {
                if (!scardProxy.bridge) {
                        scardProxy.bridge = document.getElementById('pcscbridge');
                }
                return scardProxy.bridge;
        },
        
        /**
         * Retrieve the SCARD API Object
         */
        getSCardAPI: function () {
                var bridge     = scardProxy.getPCSCBridge();
                var scard = undefined;
                if (bridge) {
                        scard     = bridge.getSCardAPI();
                }
                return scard;
        },
        
        /**
         * Retrieve the SCARD constants
         */
        getSCardConsts: function() {
                //huggly code, but do not want recompile native plugin now
                var bridge     = scardProxy.getPCSCBridge()
                var csts = {};
                csts.SCARD_SHARE_SHARED = bridge.SCARD_SHARE_SHARED;
                csts.SCARD_SHARE_EXCLUSIVE = bridge.SCARD_SHARE_EXCLUSIVE;
                csts.SCARD_SHARE_DIRECT  = bridge.SCARD_SHARE_DIRECT ;
                csts.SCARD_PROTOCOL_T0 = bridge.SCARD_PROTOCOL_T0;
                csts.SCARD_PROTOCOL_T1 = bridge.SCARD_PROTOCOL_T1;
                csts.SCARD_PROTOCOL_RAW = bridge.SCARD_PROTOCOL_RAW;
                csts.SCARD_LEAVE_CARD = bridge.SCARD_LEAVE_CARD;
                csts.SCARD_RESET_CARD = bridge.SCARD_RESET_CARD;
                csts.SCARD_UNPOWER_CARD = bridge.SCARD_UNPOWER_CARD;
                csts.SCARD_EJECT_CARD = bridge.SCARD_EJECT_CARD;
                csts.SCARD_UNKNOWN = bridge.SCARD_UNKNOWN;
                csts.SCARD_ABSENT = bridge.SCARD_ABSENT;
                csts.SCARD_PRESENT = bridge.SCARD_PRESENT;
                csts.SCARD_SWALLOWED = bridge.SCARD_SWALLOWED;
                csts.SCARD_POWERED = bridge.SCARD_POWERED;
                csts.SCARD_NEGOTIABLE = bridge.SCARD_NEGOTIABLE;
                csts.SCARD_SPECIFIC = bridge.SCARD_SPECIFIC;
                csts.SCARD_STATE_UNAWARE = bridge.SCARD_STATE_UNAWARE;
                csts.SCARD_STATE_IGNORE = bridge.SCARD_STATE_IGNORE;
                csts.SCARD_STATE_CHANGED = bridge.SCARD_STATE_CHANGED;
                csts.SCARD_STATE_UNKNOWN = bridge.SCARD_STATE_UNKNOWN;
                csts.SCARD_STATE_UNAVAILABLE = bridge.SCARD_STATE_UNAVAILABLE;
                csts.SCARD_STATE_EMPTY = bridge.SCARD_STATE_EMPTY;
                csts.SCARD_STATE_PRESENT = bridge.SCARD_STATE_PRESENT;
                csts.SCARD_STATE_ATRMATCH = bridge.SCARD_STATE_ATRMATCH;
                csts.SCARD_STATE_EXCLUSIVE = bridge.SCARD_STATE_EXCLUSIVE;
                csts.SCARD_STATE_INUSE = bridge.SCARD_STATE_INUSE;
                csts.SCARD_STATE_MUTE = bridge.SCARD_STATE_MUTE;
                csts.SCARD_STATE_UNPOWERED = bridge.SCARD_STATE_UNPOWERED;
                csts.SCARD_S_SUCCESS = bridge.SCARD_S_SUCCESS;
                csts.SCARD_F_INTERNAL_ERROR = bridge.SCARD_F_INTERNAL_ERROR;
                csts.SCARD_E_CANCELLED = bridge.SCARD_E_CANCELLED;
                csts.SCARD_E_INVALID_HANDLE = bridge.SCARD_E_INVALID_HANDLE;
                csts.SCARD_E_INVALID_PARAMETER = bridge.SCARD_E_INVALID_PARAMETER;
                csts.SCARD_E_INVALID_TARGET = bridge.SCARD_E_INVALID_TARGET;
                csts.SCARD_E_NO_MEMORY = bridge.SCARD_E_NO_MEMORY;
                csts.SCARD_F_WAITED_TOO_LONG = bridge.SCARD_F_WAITED_TOO_LONG;
                csts.SCARD_E_INSUFFICIENT_BUFFER = bridge.SCARD_E_INSUFFICIENT_BUFFER;
                csts.SCARD_E_UNKNOWN_READER = bridge.SCARD_E_UNKNOWN_READER;
                csts.SCARD_E_TIMEOUT = bridge.SCARD_E_TIMEOUT;
                csts.SCARD_E_SHARING_VIOLATION = bridge.SCARD_E_SHARING_VIOLATION;
                csts.SCARD_E_NO_SMARTCARD = bridge.SCARD_E_NO_SMARTCARD;
                csts.SCARD_E_UNKNOWN_CARD = bridge.SCARD_E_UNKNOWN_CARD;
                csts.SCARD_E_CANT_DISPOSE = bridge.SCARD_E_CANT_DISPOSE;
                csts.SCARD_E_PROTO_MISMATCH = bridge.SCARD_E_PROTO_MISMATCH;
                csts.SCARD_E_NOT_READY = bridge.SCARD_E_NOT_READY;
                csts.SCARD_E_INVALID_VALUE = bridge.SCARD_E_INVALID_VALUE;
                csts.SCARD_E_SYSTEM_CANCELLED = bridge.SCARD_E_SYSTEM_CANCELLED;
                csts.SCARD_F_COMM_ERROR = bridge.SCARD_F_COMM_ERROR;
                csts.SCARD_F_UNKNOWN_ERROR = bridge.SCARD_F_UNKNOWN_ERROR;
                csts.SCARD_E_INVALID_ATR = bridge.SCARD_E_INVALID_ATR;
                csts.SCARD_E_NOT_TRANSACTED = bridge.SCARD_E_NOT_TRANSACTED;
                csts.SCARD_E_READER_UNAVAILABLE = bridge.SCARD_E_READER_UNAVAILABLE;
                csts.SCARD_P_SHUTDOWN = bridge.SCARD_P_SHUTDOWN;
                csts.SCARD_E_PCI_TOO_SMALL = bridge.SCARD_E_PCI_TOO_SMALL;
                csts.SCARD_E_READER_UNSUPPORTED = bridge.SCARD_E_READER_UNSUPPORTED;
                csts.SCARD_E_DUPLICATE_READER = bridge.SCARD_E_DUPLICATE_READER;
                csts.SCARD_E_CARD_UNSUPPORTED = bridge.SCARD_E_CARD_UNSUPPORTED;
                csts.SCARD_E_NO_SERVICE = bridge.SCARD_E_NO_SERVICE;
                csts.SCARD_E_SERVICE_STOPPED = bridge.SCARD_E_SERVICE_STOPPED;
                csts.SCARD_E_UNEXPECTED = bridge.SCARD_E_UNEXPECTED;
                csts.SCARD_E_UNSUPPORTED_FEATURE = bridge.SCARD_E_UNSUPPORTED_FEATURE;
                csts.SCARD_E_ICC_INSTALLATION = bridge.SCARD_E_ICC_INSTALLATION;
                csts.SCARD_E_ICC_CREATEORDER = bridge.SCARD_E_ICC_CREATEORDER;
                csts.SCARD_E_DIR_NOT_FOUND = bridge.SCARD_E_DIR_NOT_FOUND;
                csts.SCARD_E_FILE_NOT_FOUND = bridge.SCARD_E_FILE_NOT_FOUND;
                csts.SCARD_E_NO_DIR = bridge.SCARD_E_NO_DIR;
                csts.SCARD_E_NO_FILE = bridge.SCARD_E_NO_FILE;
                csts.SCARD_E_NO_ACCESS = bridge.SCARD_E_NO_ACCESS;
                csts.SCARD_E_WRITE_TOO_MANY = bridge.SCARD_E_WRITE_TOO_MANY;
                csts.SCARD_E_BAD_SEEK = bridge.SCARD_E_BAD_SEEK;
                csts.SCARD_E_INVALID_CHV = bridge.SCARD_E_INVALID_CHV;
                csts.SCARD_E_UNKNOWN_RES_MNG = bridge.SCARD_E_UNKNOWN_RES_MNG;
                csts.SCARD_E_NO_SUCH_CERTIFICATE = bridge.SCARD_E_NO_SUCH_CERTIFICATE;
                csts.SCARD_E_CERTIFICATE_UNAVAILABLE = bridge.SCARD_E_CERTIFICATE_UNAVAILABLE;
                csts.SCARD_E_NO_READERS_AVAILABLE = bridge.SCARD_E_NO_READERS_AVAILABLE;
                csts.SCARD_E_COMM_DATA_LOST = bridge.SCARD_E_COMM_DATA_LOST;
                csts.SCARD_E_NO_KEY_CONTAINER = bridge.SCARD_E_NO_KEY_CONTAINER;
                csts.SCARD_E_SERVER_TOO_BUSY = bridge.SCARD_E_SERVER_TOO_BUSY;
                csts.SCARD_W_UNSUPPORTED_CARD = bridge.SCARD_W_UNSUPPORTED_CARD;
                csts.SCARD_W_UNRESPONSIVE_CARD = bridge.SCARD_W_UNRESPONSIVE_CARD;
                csts.SCARD_W_UNPOWERED_CARD = bridge.SCARD_W_UNPOWERED_CARD;
                csts.SCARD_W_RESET_CARD = bridge.SCARD_W_RESET_CARD;
                csts.SCARD_W_REMOVED_CARD = bridge.SCARD_W_REMOVED_CARD;
                csts.SCARD_W_SECURITY_VIOLATION = bridge.SCARD_W_SECURITY_VIOLATION;
                csts.SCARD_W_WRONG_CHV = bridge.SCARD_W_WRONG_CHV;
                csts.SCARD_W_CHV_BLOCKED = bridge.SCARD_W_CHV_BLOCKED;
                csts.SCARD_W_EOF = bridge.SCARD_W_EOF;
                csts.SCARD_W_CANCELLED_BY_USER = bridge.SCARD_W_CANCELLED_BY_USER;
                csts.SCARD_W_CARD_NOT_AUTHENTICATED = bridge.SCARD_W_CARD_NOT_AUTHENTICATED;
                csts.SCARD_ATTR_VENDOR_NAME = bridge.SCARD_ATTR_VENDOR_NAME;
                csts.SCARD_ATTR_VENDOR_IFD_TYPE = bridge.SCARD_ATTR_VENDOR_IFD_TYPE;
                csts.SCARD_ATTR_VENDOR_IFD_VERSION = bridge.SCARD_ATTR_VENDOR_IFD_VERSION;
                csts.SCARD_ATTR_VENDOR_IFD_SERIAL_NO = bridge.SCARD_ATTR_VENDOR_IFD_SERIAL_NO;
                csts.SCARD_ATTR_CHANNEL_ID = bridge.SCARD_ATTR_CHANNEL_ID;
                csts.SCARD_ATTR_ASYNC_PROTOCOL_TYPES = bridge.SCARD_ATTR_ASYNC_PROTOCOL_TYPES;
                csts.SCARD_ATTR_DEFAULT_CLK = bridge.SCARD_ATTR_DEFAULT_CLK;
                csts.SCARD_ATTR_MAX_CLK = bridge.SCARD_ATTR_MAX_CLK;
                csts.SCARD_ATTR_DEFAULT_DATA_RATE = bridge.SCARD_ATTR_DEFAULT_DATA_RATE;
                csts.SCARD_ATTR_MAX_DATA_RATE = bridge.SCARD_ATTR_MAX_DATA_RATE;
                csts.SCARD_ATTR_MAX_IFSD = bridge.SCARD_ATTR_MAX_IFSD;
                csts.SCARD_ATTR_SYNC_PROTOCOL_TYPES = bridge.SCARD_ATTR_SYNC_PROTOCOL_TYPES;
                csts.SCARD_ATTR_POWER_MGMT_SUPPORT = bridge.SCARD_ATTR_POWER_MGMT_SUPPORT;
                csts.SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE = bridge.SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE;
                csts.SCARD_ATTR_USER_AUTH_INPUT_DEVICE = bridge.SCARD_ATTR_USER_AUTH_INPUT_DEVICE;
                csts.SCARD_ATTR_CHARACTERISTICS = bridge.SCARD_ATTR_CHARACTERISTICS;
                csts.SCARD_ATTR_CURRENT_PROTOCOL_TYPE = bridge.SCARD_ATTR_CURRENT_PROTOCOL_TYPE;
                csts.SCARD_ATTR_CURRENT_CLK = bridge.SCARD_ATTR_CURRENT_CLK;
                csts.SCARD_ATTR_CURRENT_F = bridge.SCARD_ATTR_CURRENT_F;
                csts.SCARD_ATTR_CURRENT_D = bridge.SCARD_ATTR_CURRENT_D;
                csts.SCARD_ATTR_CURRENT_N = bridge.SCARD_ATTR_CURRENT_N;
                csts.SCARD_ATTR_CURRENT_W = bridge.SCARD_ATTR_CURRENT_W;
                csts.SCARD_ATTR_CURRENT_IFSC = bridge.SCARD_ATTR_CURRENT_IFSC;
                csts.SCARD_ATTR_CURRENT_IFSD = bridge.SCARD_ATTR_CURRENT_IFSD;
                csts.SCARD_ATTR_CURRENT_BWT = bridge.SCARD_ATTR_CURRENT_BWT;
                csts.SCARD_ATTR_CURRENT_CWT = bridge.SCARD_ATTR_CURRENT_CWT;
                csts.SCARD_ATTR_CURRENT_EBC_ENCODING = bridge.SCARD_ATTR_CURRENT_EBC_ENCODING;
                csts.SCARD_ATTR_EXTENDED_BWT = bridge.SCARD_ATTR_EXTENDED_BWT;
                csts.SCARD_ATTR_ICC_PRESENCE = bridge.SCARD_ATTR_ICC_PRESENCE;
                csts.SCARD_ATTR_ICC_INTERFACE_STATUS = bridge.SCARD_ATTR_ICC_INTERFACE_STATUS;
                csts.SCARD_ATTR_CURRENT_IO_STATE = bridge.SCARD_ATTR_CURRENT_IO_STATE;
                csts.SCARD_ATTR_ATR_STRING = bridge.SCARD_ATTR_ATR_STRING;
                csts.SCARD_ATTR_ICC_TYPE_PER_ATR = bridge.SCARD_ATTR_ICC_TYPE_PER_ATR;
                csts.SCARD_ATTR_ESC_RESET = bridge.SCARD_ATTR_ESC_RESET;
                csts.SCARD_ATTR_ESC_CANCEL = bridge.SCARD_ATTR_ESC_CANCEL;
                csts.SCARD_ATTR_ESC_AUTHREQUEST = bridge.SCARD_ATTR_ESC_AUTHREQUEST;
                csts.SCARD_ATTR_MAXINPUT = bridge.SCARD_ATTR_MAXINPUT;
                csts.SCARD_ATTR_DEVICE_UNIT = bridge.SCARD_ATTR_DEVICE_UNIT;
                csts.SCARD_ATTR_DEVICE_IN_USE = bridge.SCARD_ATTR_DEVICE_IN_USE;
                csts.SCARD_ATTR_DEVICE_FRIENDLY_NAME = bridge.SCARD_ATTR_DEVICE_FRIENDLY_NAME;
                csts.SCARD_ATTR_DEVICE_SYSTEM_NAME = bridge.SCARD_ATTR_DEVICE_SYSTEM_NAME;
                csts.SCARD_ATTR_SUPRESS_T1_IFS_REQUEST = bridge.SCARD_ATTR_SUPRESS_T1_IFS_REQUEST;
                return csts;                
        },

};

chrome.runtime.onConnect.addListener(function(port) {

        port.onMessage.addListener(function(msg) {
                
                if ((typeof msg.destination == "undefined") || (msg.destination != "SCARD")) {
                        return; // unhandled
                }

                var sc   = scardProxy.getSCardAPI();
                if ((msg.args == undefined) || 
                    (msg.func == undefined)) {
                        msg.args.err = -1;
                } else if (msg.func == "getSCardConsts") {
                        msg.args = scardProxy.getSCardConsts();
                        msg.err  = 0;
                } else if (sc[msg.func] != undefined) {
                        msg.args.err = sc[msg.func](msg.args);
                } else {
                        msg.args.err = -2;
                }
                msg.destination="DRACS"
                port.postMessage(msg);
        });
        
        port.onDisconnect.addListener(function(msg) {
        });
});
