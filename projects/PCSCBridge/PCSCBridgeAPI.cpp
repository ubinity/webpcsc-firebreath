/*
************************************************************************
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
*************************************************************************
*/


/**********************************************************\

  Auto-generated PCSCBridgeAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "PCSCBridgeAPI.h"


//##########################################################################
//##########################################################################
//                            Helpers
//##########################################################################
//##########################################################################
#define RECV_BUFFER_SIZE 0x10000

static LPTSTR string2LPTSTR(std::string str) {
    int strlen = str.length();
    char* cstr = new char[strlen+1];
    memset(cstr, 0, strlen+1);
    str.copy(cstr, strlen, 0);
    
#if defined(WIN32) && defined(UNICODE)
    int   lpstrlen = 2*strlen;
    LPTSTR lpstr = new WCHAR[lpstrlen+1];
    memset(lpstr, 0, lpstrlen+1);
    MultiByteToWideChar(CP_UTF8, 0, 
                        cstr, -1,
                        lpstr, lpstrlen);
    delete[] cstr;
    return lpstr;
#else
    return cstr;
#endif    

}

static std::string  LPTSTR2string(LPTSTR lpstr) {
    std::string stdstr;
#if defined(WIN32) && defined(UNICODE)
    int lplen = wcslen(lpstr);

    char* str = new char[lplen+1];
    memset(str, 0, lplen+1);
    WideCharToMultiByte(CP_UTF8,0,lpstr,-1,
                        str,lplen,
                        NULL,NULL);
    stdstr.assign(str);
    delete[] str;
#else
    stdstr.assign(lpstr);
#endif
    return stdstr;
}


static const std::string HEXDIGITS = "0123456789ABCDEF";

static int digitToHex(int digit) {
    if ((digit >= '0') && (digit <= '9')) {
        return digit - '0';
    }  else if ((digit >= 'A') && (digit <= 'F')) {
        return (digit - 'A') + 10;
    } else  if ((digit >= 'a') && (digit <= 'f')) {
        return (digit - 'a') + 10;
    }
    return -1;
}

static unsigned char * hexstrToBinary(const std::string& data,  int *length) {
    unsigned char * targetBin;
    int dataLength = data.length();

    *length = -1;
    if ((dataLength % 2) != 0) {
        return NULL;
    }    
    targetBin = new unsigned char[dataLength];    
    for (int i=0; i<dataLength; i += 2) {
        int d1, d2;
        d1 = digitToHex(data[i]);
        d2 = digitToHex(data[i + 1]);
        if ((d1 < 0) || (d2 < 0)) {
            delete[] targetBin;
            return NULL;
        }
        targetBin[i / 2] = (d1 << 4) + d2;
    } 
    *length = dataLength/2;
    return targetBin;
}

static std::string binaryToHexstr(const unsigned char *sourceBinary, int length) {
    std::string result;
    for (int i=0; i<length; i++) {
        int digit = sourceBinary[i];
        result.push_back(HEXDIGITS[(digit >> 4) & 0x0f]);
        result.push_back(HEXDIGITS[digit & 0x0f]);
    }
    return result;
}

static LONG getReaderList(SCARDCONTEXT hContext, std::vector<std::string> &readers) {
    LONG   error;

    if ((error = SCardIsValidContext(hContext)) !=  SCARD_S_SUCCESS) {
        return error;
    }
    
    //list readers names
    LPTSTR mszReaders = NULL;
    DWORD dwReaders   = SCARD_AUTOALLOCATE;
    error = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (SCARD_S_SUCCESS != error) {
        return error;
    }

    //vectorize...
    LPTSTR  preader = mszReaders;
    int len = dwReaders;
    while (len && (*preader != '\0')) {
        int l;
        readers.push_back(LPTSTR2string(preader));
#if defined(WIN32) && defined(UNICODE)
        l = wcslen(preader)+1;
#else
        l = strlen(preader)+1;
#endif
        preader  += l;
        len     -=l;
    }
    SCardFreeMemory(hContext,mszReaders);
    return SCARD_S_SUCCESS;
}


//##########################################################################
//##########################################################################
//                            PCSCBridgeAPI
//##########################################################################
//##########################################################################
PCSCBridgeAPI::PCSCBridgeAPI(const PCSCBridgePtr& plugin, const FB::BrowserHostPtr& host) :
    m_plugin(plugin), m_host(host) {

    scard_api = boost::make_shared<SCardAPI>();
    terminal_api = boost::make_shared<PCSCTerminalAPI>();
    
    // --------------------- GET LEVEL API -------------------------- 
    registerMethod("getSCardAPI",     make_method(this, &PCSCBridgeAPI::getSCardAPI));
    registerMethod("getTerminalAPI",  make_method(this, &PCSCBridgeAPI::getTerminalAPI));
    registerMethod("strError",        make_method(this, &PCSCBridgeAPI::strError));

    /// --------------------- READ ONLY -------------------------- 
    //version
    registerProperty("version",
                     make_property(this, 
                                   &PCSCBridgeAPI::get_version));
    
    //scard mode
    registerProperty("SCARD_SHARE_SHARED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_SHARE_SHARED));
    registerProperty("SCARD_SHARE_EXCLUSIVE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_SHARE_EXCLUSIVE));
    registerProperty("SCARD_SHARE_DIRECT ",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_SHARE_DIRECT ));
    // protocol 
    registerProperty("SCARD_PROTOCOL_T0",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_PROTOCOL_T0));
    registerProperty("SCARD_PROTOCOL_T1",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_PROTOCOL_T1));
    registerProperty("SCARD_PROTOCOL_RAW",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_PROTOCOL_RAW));        
    //scard disposition
    registerProperty("SCARD_LEAVE_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_LEAVE_CARD));
    registerProperty("SCARD_RESET_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_RESET_CARD));
    registerProperty("SCARD_UNPOWER_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_UNPOWER_CARD));
    registerProperty("SCARD_EJECT_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_EJECT_CARD));
    //scard status
    registerProperty("SCARD_UNKNOWN",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_UNKNOWN));
    registerProperty("SCARD_ABSENT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ABSENT));
    registerProperty("SCARD_PRESENT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_PRESENT));
    //scard ???
    registerProperty("SCARD_SWALLOWED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_SWALLOWED));
    registerProperty("SCARD_POWERED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_POWERED));
    registerProperty("SCARD_NEGOTIABLE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_NEGOTIABLE));
    registerProperty("SCARD_SPECIFIC",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_SPECIFIC));
    //scard state
    registerProperty("SCARD_STATE_UNAWARE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_UNAWARE));
    registerProperty("SCARD_STATE_IGNORE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_IGNORE));
    registerProperty("SCARD_STATE_CHANGED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_CHANGED));
    registerProperty("SCARD_STATE_UNKNOWN",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_UNKNOWN));
    registerProperty("SCARD_STATE_UNAVAILABLE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_UNAVAILABLE));
    registerProperty("SCARD_STATE_EMPTY",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_EMPTY));
    registerProperty("SCARD_STATE_PRESENT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_PRESENT));
    registerProperty("SCARD_STATE_ATRMATCH",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_ATRMATCH));
    registerProperty("SCARD_STATE_EXCLUSIVE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_EXCLUSIVE));
    registerProperty("SCARD_STATE_INUSE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_INUSE));
    registerProperty("SCARD_STATE_MUTE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_MUTE));
    registerProperty("SCARD_STATE_UNPOWERED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_STATE_UNPOWERED));
    //error/warning
    registerProperty("SCARD_S_SUCCESS",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_S_SUCCESS));
    registerProperty("SCARD_F_INTERNAL_ERROR",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_F_INTERNAL_ERROR));
    registerProperty("SCARD_E_CANCELLED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_CANCELLED));
    registerProperty("SCARD_E_INVALID_HANDLE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INVALID_HANDLE));
    registerProperty("SCARD_E_INVALID_PARAMETER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INVALID_PARAMETER));
    registerProperty("SCARD_E_INVALID_TARGET",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INVALID_TARGET));
    registerProperty("SCARD_E_NO_MEMORY",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_MEMORY));
    registerProperty("SCARD_F_WAITED_TOO_LONG",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_F_WAITED_TOO_LONG));
    registerProperty("SCARD_E_INSUFFICIENT_BUFFER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INSUFFICIENT_BUFFER));
    registerProperty("SCARD_E_UNKNOWN_READER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_UNKNOWN_READER));
    registerProperty("SCARD_E_TIMEOUT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_TIMEOUT));
    registerProperty("SCARD_E_SHARING_VIOLATION",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_SHARING_VIOLATION));
    registerProperty("SCARD_E_NO_SMARTCARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_SMARTCARD));
    registerProperty("SCARD_E_UNKNOWN_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_UNKNOWN_CARD));
    registerProperty("SCARD_E_CANT_DISPOSE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_CANT_DISPOSE));
    registerProperty("SCARD_E_PROTO_MISMATCH",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_PROTO_MISMATCH));
    registerProperty("SCARD_E_NOT_READY",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NOT_READY));
    registerProperty("SCARD_E_INVALID_VALUE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INVALID_VALUE));
    registerProperty("SCARD_E_SYSTEM_CANCELLED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_SYSTEM_CANCELLED));
    registerProperty("SCARD_F_COMM_ERROR",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_F_COMM_ERROR));
    registerProperty("SCARD_F_UNKNOWN_ERROR",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_F_UNKNOWN_ERROR));
    registerProperty("SCARD_E_INVALID_ATR",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INVALID_ATR));
    registerProperty("SCARD_E_NOT_TRANSACTED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NOT_TRANSACTED));
    registerProperty("SCARD_E_READER_UNAVAILABLE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_READER_UNAVAILABLE));
    registerProperty("SCARD_P_SHUTDOWN",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_P_SHUTDOWN));
    registerProperty("SCARD_E_PCI_TOO_SMALL",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_PCI_TOO_SMALL));
    registerProperty("SCARD_E_READER_UNSUPPORTED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_READER_UNSUPPORTED));
    registerProperty("SCARD_E_DUPLICATE_READER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_DUPLICATE_READER));
    registerProperty("SCARD_E_CARD_UNSUPPORTED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_CARD_UNSUPPORTED));
    registerProperty("SCARD_E_NO_SERVICE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_SERVICE));
    registerProperty("SCARD_E_SERVICE_STOPPED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_SERVICE_STOPPED));
    registerProperty("SCARD_E_UNEXPECTED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_UNEXPECTED));
    registerProperty("SCARD_E_UNSUPPORTED_FEATURE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_UNSUPPORTED_FEATURE));
    registerProperty("SCARD_E_ICC_INSTALLATION",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_ICC_INSTALLATION));
    registerProperty("SCARD_E_ICC_CREATEORDER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_ICC_CREATEORDER));
    registerProperty("SCARD_E_DIR_NOT_FOUND",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_DIR_NOT_FOUND));
    registerProperty("SCARD_E_FILE_NOT_FOUND",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_FILE_NOT_FOUND));
    registerProperty("SCARD_E_NO_DIR",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_DIR));
    registerProperty("SCARD_E_NO_FILE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_FILE));
    registerProperty("SCARD_E_NO_ACCESS",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_ACCESS));
    registerProperty("SCARD_E_WRITE_TOO_MANY",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_WRITE_TOO_MANY));
    registerProperty("SCARD_E_BAD_SEEK",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_BAD_SEEK));
    registerProperty("SCARD_E_INVALID_CHV",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_INVALID_CHV));
    registerProperty("SCARD_E_UNKNOWN_RES_MNG",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_UNKNOWN_RES_MNG));
    registerProperty("SCARD_E_NO_SUCH_CERTIFICATE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_SUCH_CERTIFICATE));
    registerProperty("SCARD_E_CERTIFICATE_UNAVAILABLE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_CERTIFICATE_UNAVAILABLE));
    registerProperty("SCARD_E_NO_READERS_AVAILABLE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_READERS_AVAILABLE));
    registerProperty("SCARD_E_COMM_DATA_LOST",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_COMM_DATA_LOST));
    registerProperty("SCARD_E_NO_KEY_CONTAINER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_NO_KEY_CONTAINER));
    registerProperty("SCARD_E_SERVER_TOO_BUSY",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_E_SERVER_TOO_BUSY));
    registerProperty("SCARD_W_UNSUPPORTED_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_UNSUPPORTED_CARD));
    registerProperty("SCARD_W_UNRESPONSIVE_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_UNRESPONSIVE_CARD));
    registerProperty("SCARD_W_UNPOWERED_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_UNPOWERED_CARD));
    registerProperty("SCARD_W_RESET_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_RESET_CARD));
    registerProperty("SCARD_W_REMOVED_CARD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_REMOVED_CARD));
    registerProperty("SCARD_W_SECURITY_VIOLATION",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_SECURITY_VIOLATION));
    registerProperty("SCARD_W_WRONG_CHV",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_WRONG_CHV));
    registerProperty("SCARD_W_CHV_BLOCKED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_CHV_BLOCKED));
    registerProperty("SCARD_W_EOF",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_EOF));
    registerProperty("SCARD_W_CANCELLED_BY_USER",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_CANCELLED_BY_USER));
    registerProperty("SCARD_W_CARD_NOT_AUTHENTICATED",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_W_CARD_NOT_AUTHENTICATED));


    //attribute
    registerProperty("SCARD_ATTR_VENDOR_NAME",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_NAME));
    registerProperty("SCARD_ATTR_VENDOR_IFD_TYPE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_IFD_TYPE));
    registerProperty("SCARD_ATTR_VENDOR_IFD_VERSION",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_IFD_VERSION));
    registerProperty("SCARD_ATTR_VENDOR_IFD_SERIAL_NO",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_IFD_SERIAL_NO));
    registerProperty("SCARD_ATTR_CHANNEL_ID",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CHANNEL_ID));
#ifdef linux
    registerProperty("SCARD_ATTR_ASYNC_PROTOCOL_TYPES",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ASYNC_PROTOCOL_TYPES));
#endif
    registerProperty("SCARD_ATTR_DEFAULT_CLK",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_DEFAULT_CLK));
    registerProperty("SCARD_ATTR_MAX_CLK",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_MAX_CLK));
    registerProperty("SCARD_ATTR_DEFAULT_DATA_RATE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_DEFAULT_DATA_RATE));
    registerProperty("SCARD_ATTR_MAX_DATA_RATE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_MAX_DATA_RATE));
    registerProperty("SCARD_ATTR_MAX_IFSD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_MAX_IFSD));
#ifdef linux
    registerProperty("SCARD_ATTR_SYNC_PROTOCOL_TYPES",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_SYNC_PROTOCOL_TYPES));
#endif
    registerProperty("SCARD_ATTR_POWER_MGMT_SUPPORT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_POWER_MGMT_SUPPORT));
    registerProperty("SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE));
    registerProperty("SCARD_ATTR_USER_AUTH_INPUT_DEVICE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_USER_AUTH_INPUT_DEVICE));
    registerProperty("SCARD_ATTR_CHARACTERISTICS",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CHARACTERISTICS));
    registerProperty("SCARD_ATTR_CURRENT_PROTOCOL_TYPE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_PROTOCOL_TYPE));
    registerProperty("SCARD_ATTR_CURRENT_CLK",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_CLK));
    registerProperty("SCARD_ATTR_CURRENT_F",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_F));
    registerProperty("SCARD_ATTR_CURRENT_D",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_D));
    registerProperty("SCARD_ATTR_CURRENT_N",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_N));
    registerProperty("SCARD_ATTR_CURRENT_W",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_W));
    registerProperty("SCARD_ATTR_CURRENT_IFSC",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_IFSC));
    registerProperty("SCARD_ATTR_CURRENT_IFSD",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_IFSD));
    registerProperty("SCARD_ATTR_CURRENT_BWT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_BWT));
    registerProperty("SCARD_ATTR_CURRENT_CWT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_CWT));
    registerProperty("SCARD_ATTR_CURRENT_EBC_ENCODING",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_EBC_ENCODING));
    registerProperty("SCARD_ATTR_EXTENDED_BWT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_EXTENDED_BWT));
    registerProperty("SCARD_ATTR_ICC_PRESENCE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ICC_PRESENCE));
    registerProperty("SCARD_ATTR_ICC_INTERFACE_STATUS",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ICC_INTERFACE_STATUS));
    registerProperty("SCARD_ATTR_CURRENT_IO_STATE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_IO_STATE));
    registerProperty("SCARD_ATTR_ATR_STRING",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ATR_STRING));
    registerProperty("SCARD_ATTR_ICC_TYPE_PER_ATR",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ICC_TYPE_PER_ATR));
    registerProperty("SCARD_ATTR_ESC_RESET",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ESC_RESET));
    registerProperty("SCARD_ATTR_ESC_CANCEL",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ESC_CANCEL));
    registerProperty("SCARD_ATTR_ESC_AUTHREQUEST",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_ESC_AUTHREQUEST));
    registerProperty("SCARD_ATTR_MAXINPUT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_MAXINPUT));
    registerProperty("SCARD_ATTR_DEVICE_UNIT",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_UNIT));
    registerProperty("SCARD_ATTR_DEVICE_IN_USE",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_IN_USE));
    registerProperty("SCARD_ATTR_DEVICE_FRIENDLY_NAME",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_FRIENDLY_NAME));
    registerProperty("SCARD_ATTR_DEVICE_SYSTEM_NAME",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_SYSTEM_NAME));
    registerProperty("SCARD_ATTR_SUPRESS_T1_IFS_REQUEST",
                     make_property(this,
                                   &PCSCBridgeAPI::get_SCARD_ATTR_SUPRESS_T1_IFS_REQUEST));
}

/*
SCARD_ATTR_VENDOR_NAME
SCARD_ATTR_VENDOR_IFD_TYPE
SCARD_ATTR_VENDOR_IFD_VERSION
SCARD_ATTR_VENDOR_IFD_SERIAL_NO
SCARD_ATTR_CHANNEL_ID
SCARD_ATTR_ASYNC_PROTOCOL_TYPES
SCARD_ATTR_DEFAULT_CLK
SCARD_ATTR_MAX_CLK
SCARD_ATTR_DEFAULT_DATA_RATE
SCARD_ATTR_MAX_DATA_RATE
SCARD_ATTR_MAX_IFSD
SCARD_ATTR_SYNC_PROTOCOL_TYPES
SCARD_ATTR_POWER_MGMT_SUPPORT
SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE
SCARD_ATTR_USER_AUTH_INPUT_DEVICE
SCARD_ATTR_CHARACTERISTICS
SCARD_ATTR_CURRENT_PROTOCOL_TYPE
SCARD_ATTR_CURRENT_CLK
SCARD_ATTR_CURRENT_F
SCARD_ATTR_CURRENT_D
SCARD_ATTR_CURRENT_N
SCARD_ATTR_CURRENT_W
SCARD_ATTR_CURRENT_IFSC
SCARD_ATTR_CURRENT_IFSD
SCARD_ATTR_CURRENT_BWT
SCARD_ATTR_CURRENT_CWT
SCARD_ATTR_CURRENT_EBC_ENCODING
SCARD_ATTR_EXTENDED_BWT
SCARD_ATTR_ICC_PRESENCE
SCARD_ATTR_ICC_INTERFACE_STATUS
SCARD_ATTR_CURRENT_IO_STATE
SCARD_ATTR_ATR_STRING
SCARD_ATTR_ICC_TYPE_PER_ATR
SCARD_ATTR_ESC_RESET
SCARD_ATTR_ESC_CANCEL
SCARD_ATTR_ESC_AUTHREQUEST
SCARD_ATTR_MAXINPUT
SCARD_ATTR_DEVICE_UNIT
SCARD_ATTR_DEVICE_IN_USE
SCARD_ATTR_DEVICE_FRIENDLY_NAME
SCARD_ATTR_DEVICE_SYSTEM_NAME
SCARD_ATTR_SUPRESS_T1_IFS_REQUEST
*/

///////////////////////////////////////////////////////////////////////////////
/// @fn PCSCBridgePtr PCSCBridgeAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
PCSCBridgePtr PCSCBridgeAPI::getPlugin()
{
    PCSCBridgePtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}


/* --------------------- GET LEVEL API -------------------------- */

SCardAPIPtr PCSCBridgeAPI::getSCardAPI() {   
    return scard_api;  
}

PCSCTerminalAPIPtr PCSCBridgeAPI::getTerminalAPI() {   
    return terminal_api;  
}


/* --------------------- Read only const ------------------------ */

//version
std::string PCSCBridgeAPI::get_version() {
    return FBSTRING_PLUGIN_VERSION;
}


//scard mode
int  PCSCBridgeAPI::get_SCARD_SHARE_SHARED() {
    return SCARD_SHARE_SHARED;
}
int  PCSCBridgeAPI::get_SCARD_SHARE_EXCLUSIVE() {
    return SCARD_SHARE_EXCLUSIVE;
}
int  PCSCBridgeAPI::get_SCARD_SHARE_DIRECT() {
    return SCARD_SHARE_DIRECT;
}

//preferred protocol 
int  PCSCBridgeAPI::get_SCARD_PROTOCOL_T0() {
    return SCARD_PROTOCOL_T0;
}
int  PCSCBridgeAPI::get_SCARD_PROTOCOL_T1() {
    return SCARD_PROTOCOL_T1;
}
int  PCSCBridgeAPI::get_SCARD_PROTOCOL_RAW() {
    return SCARD_PROTOCOL_RAW;
}
#ifdef linux
int  PCSCBridgeAPI::get_SCARD_PROTOCOL_T15() {
    return SCARD_PROTOCOL_T15;
}
#endif

//scard disposition
int PCSCBridgeAPI::get_SCARD_LEAVE_CARD() {
    return SCARD_LEAVE_CARD;
}
int PCSCBridgeAPI::get_SCARD_RESET_CARD() {
    return SCARD_RESET_CARD;
}
int PCSCBridgeAPI::get_SCARD_UNPOWER_CARD() {
    return SCARD_UNPOWER_CARD;
}
int PCSCBridgeAPI::get_SCARD_EJECT_CARD() {
    return SCARD_EJECT_CARD;
}

//scard status
int PCSCBridgeAPI::get_SCARD_UNKNOWN() {
    return SCARD_UNKNOWN;
}
int PCSCBridgeAPI::get_SCARD_ABSENT() {
    return SCARD_ABSENT;
}
int PCSCBridgeAPI::get_SCARD_PRESENT() {
    return SCARD_PRESENT;
}

//scard ???
int PCSCBridgeAPI::get_SCARD_SWALLOWED() {
    return SCARD_SWALLOWED;
}
int PCSCBridgeAPI::get_SCARD_POWERED() {
    return SCARD_POWERED;
}
int PCSCBridgeAPI::get_SCARD_NEGOTIABLE() {
    return SCARD_NEGOTIABLE;
}
int PCSCBridgeAPI::get_SCARD_SPECIFIC() {
    return SCARD_SPECIFIC;
}

//scard state
int PCSCBridgeAPI::get_SCARD_STATE_UNAWARE() {
    return SCARD_STATE_UNAWARE;
}
int PCSCBridgeAPI::get_SCARD_STATE_IGNORE() {
    return SCARD_STATE_IGNORE;
}
int PCSCBridgeAPI::get_SCARD_STATE_CHANGED() {
    return SCARD_STATE_CHANGED;
}
int PCSCBridgeAPI::get_SCARD_STATE_UNKNOWN() {
    return SCARD_STATE_UNKNOWN;
}
int PCSCBridgeAPI::get_SCARD_STATE_UNAVAILABLE() {
    return SCARD_STATE_UNAVAILABLE;
}
int PCSCBridgeAPI::get_SCARD_STATE_EMPTY() {
    return SCARD_STATE_EMPTY;
}
int PCSCBridgeAPI::get_SCARD_STATE_PRESENT() {
    return SCARD_STATE_PRESENT;
}
int PCSCBridgeAPI::get_SCARD_STATE_ATRMATCH() {
    return SCARD_STATE_ATRMATCH;
}
int PCSCBridgeAPI::get_SCARD_STATE_EXCLUSIVE() {
    return SCARD_STATE_EXCLUSIVE;
}
int PCSCBridgeAPI::get_SCARD_STATE_INUSE() {
    return SCARD_STATE_INUSE;
}
int PCSCBridgeAPI::get_SCARD_STATE_MUTE() {
    return SCARD_STATE_MUTE;
}
int PCSCBridgeAPI::get_SCARD_STATE_UNPOWERED() {
    return SCARD_STATE_UNPOWERED;
}

//attribute
int PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_NAME() {
    return SCARD_ATTR_VENDOR_NAME;
}
int PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_IFD_TYPE() {
    return SCARD_ATTR_VENDOR_IFD_TYPE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_IFD_VERSION() {
    return SCARD_ATTR_VENDOR_IFD_VERSION;
}
int PCSCBridgeAPI::get_SCARD_ATTR_VENDOR_IFD_SERIAL_NO() {
    return SCARD_ATTR_VENDOR_IFD_SERIAL_NO;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CHANNEL_ID() {
    return SCARD_ATTR_CHANNEL_ID;
}
#ifdef linux
int PCSCBridgeAPI::get_SCARD_ATTR_ASYNC_PROTOCOL_TYPES() {
    return SCARD_ATTR_ASYNC_PROTOCOL_TYPES;
}
#endif
int PCSCBridgeAPI::get_SCARD_ATTR_DEFAULT_CLK() {
    return SCARD_ATTR_DEFAULT_CLK;
}
int PCSCBridgeAPI::get_SCARD_ATTR_MAX_CLK() {
    return SCARD_ATTR_MAX_CLK;
}
int PCSCBridgeAPI::get_SCARD_ATTR_DEFAULT_DATA_RATE() {
    return SCARD_ATTR_DEFAULT_DATA_RATE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_MAX_DATA_RATE() {
    return SCARD_ATTR_MAX_DATA_RATE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_MAX_IFSD() {
    return SCARD_ATTR_MAX_IFSD;
}
#ifdef linux
int PCSCBridgeAPI::get_SCARD_ATTR_SYNC_PROTOCOL_TYPES() {
    return SCARD_ATTR_SYNC_PROTOCOL_TYPES;
}
#endif
int PCSCBridgeAPI::get_SCARD_ATTR_POWER_MGMT_SUPPORT() {
    return SCARD_ATTR_POWER_MGMT_SUPPORT;
}
int PCSCBridgeAPI::get_SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE() {
    return SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_USER_AUTH_INPUT_DEVICE() {
    return SCARD_ATTR_USER_AUTH_INPUT_DEVICE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CHARACTERISTICS() {
    return SCARD_ATTR_CHARACTERISTICS;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_PROTOCOL_TYPE() {
    return SCARD_ATTR_CURRENT_PROTOCOL_TYPE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_CLK() {
    return SCARD_ATTR_CURRENT_CLK;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_F() {
    return SCARD_ATTR_CURRENT_F;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_D() {
    return SCARD_ATTR_CURRENT_D;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_N() {
    return SCARD_ATTR_CURRENT_N;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_W() {
    return SCARD_ATTR_CURRENT_W;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_IFSC() {
    return SCARD_ATTR_CURRENT_IFSC;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_IFSD() {
    return SCARD_ATTR_CURRENT_IFSD;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_BWT() {
    return SCARD_ATTR_CURRENT_BWT;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_CWT() {
    return SCARD_ATTR_CURRENT_CWT;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_EBC_ENCODING() {
    return SCARD_ATTR_CURRENT_EBC_ENCODING;
}
int PCSCBridgeAPI::get_SCARD_ATTR_EXTENDED_BWT() {
    return SCARD_ATTR_EXTENDED_BWT;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ICC_PRESENCE() {
    return SCARD_ATTR_ICC_PRESENCE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ICC_INTERFACE_STATUS() {
    return SCARD_ATTR_ICC_INTERFACE_STATUS;
}
int PCSCBridgeAPI::get_SCARD_ATTR_CURRENT_IO_STATE() {
    return SCARD_ATTR_CURRENT_IO_STATE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ATR_STRING() {
    return SCARD_ATTR_ATR_STRING;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ICC_TYPE_PER_ATR() {
    return SCARD_ATTR_ICC_TYPE_PER_ATR;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ESC_RESET() {
    return SCARD_ATTR_ESC_RESET;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ESC_CANCEL() {
    return SCARD_ATTR_ESC_CANCEL;
}
int PCSCBridgeAPI::get_SCARD_ATTR_ESC_AUTHREQUEST() {
    return SCARD_ATTR_ESC_AUTHREQUEST;
}
int PCSCBridgeAPI::get_SCARD_ATTR_MAXINPUT() {
    return SCARD_ATTR_MAXINPUT;
}
int PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_UNIT() {
    return SCARD_ATTR_DEVICE_UNIT;
}
int PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_IN_USE() {
    return SCARD_ATTR_DEVICE_IN_USE;
}
int PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_FRIENDLY_NAME() {
    return SCARD_ATTR_DEVICE_FRIENDLY_NAME;
}
int PCSCBridgeAPI::get_SCARD_ATTR_DEVICE_SYSTEM_NAME() {
    return SCARD_ATTR_DEVICE_SYSTEM_NAME;
}
int PCSCBridgeAPI::get_SCARD_ATTR_SUPRESS_T1_IFS_REQUEST() {
    return SCARD_ATTR_SUPRESS_T1_IFS_REQUEST;
}

//error/warning
LONG PCSCBridgeAPI::get_SCARD_S_SUCCESS() {
    return SCARD_S_SUCCESS;
}
LONG PCSCBridgeAPI::get_SCARD_F_INTERNAL_ERROR() {
    return SCARD_F_INTERNAL_ERROR;
}
LONG PCSCBridgeAPI::get_SCARD_E_CANCELLED() {
    return SCARD_E_CANCELLED;
}
LONG PCSCBridgeAPI::get_SCARD_E_INVALID_HANDLE() {
    return SCARD_E_INVALID_HANDLE;
}
LONG PCSCBridgeAPI::get_SCARD_E_INVALID_PARAMETER() {
    return SCARD_E_INVALID_PARAMETER;
}
LONG PCSCBridgeAPI::get_SCARD_E_INVALID_TARGET() {
    return SCARD_E_INVALID_TARGET;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_MEMORY() {
    return SCARD_E_NO_MEMORY;
}
LONG PCSCBridgeAPI::get_SCARD_F_WAITED_TOO_LONG() {
    return SCARD_F_WAITED_TOO_LONG;
}
LONG PCSCBridgeAPI::get_SCARD_E_INSUFFICIENT_BUFFER() {
    return SCARD_E_INSUFFICIENT_BUFFER;
}
LONG PCSCBridgeAPI::get_SCARD_E_UNKNOWN_READER() {
    return SCARD_E_UNKNOWN_READER;
}
LONG PCSCBridgeAPI::get_SCARD_E_TIMEOUT() {
    return SCARD_E_TIMEOUT;
}
LONG PCSCBridgeAPI::get_SCARD_E_SHARING_VIOLATION() {
    return SCARD_E_SHARING_VIOLATION;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_SMARTCARD() {
    return SCARD_E_NO_SMARTCARD;
}
LONG PCSCBridgeAPI::get_SCARD_E_UNKNOWN_CARD() {
    return SCARD_E_UNKNOWN_CARD;
}
LONG PCSCBridgeAPI::get_SCARD_E_CANT_DISPOSE() {
    return SCARD_E_CANT_DISPOSE;
}
LONG PCSCBridgeAPI::get_SCARD_E_PROTO_MISMATCH() {
    return SCARD_E_PROTO_MISMATCH;
}
LONG PCSCBridgeAPI::get_SCARD_E_NOT_READY() {
    return SCARD_E_NOT_READY;
}
LONG PCSCBridgeAPI::get_SCARD_E_INVALID_VALUE() {
    return SCARD_E_INVALID_VALUE;
}
LONG PCSCBridgeAPI::get_SCARD_E_SYSTEM_CANCELLED() {
    return SCARD_E_SYSTEM_CANCELLED;
}
LONG PCSCBridgeAPI::get_SCARD_F_COMM_ERROR() {
    return SCARD_F_COMM_ERROR;
}
LONG PCSCBridgeAPI::get_SCARD_F_UNKNOWN_ERROR() {
    return SCARD_F_UNKNOWN_ERROR;
}
LONG PCSCBridgeAPI::get_SCARD_E_INVALID_ATR() {
    return SCARD_E_INVALID_ATR;
}
LONG PCSCBridgeAPI::get_SCARD_E_NOT_TRANSACTED() {
    return SCARD_E_NOT_TRANSACTED;
}
LONG PCSCBridgeAPI::get_SCARD_E_READER_UNAVAILABLE() {
    return SCARD_E_READER_UNAVAILABLE;
}
LONG PCSCBridgeAPI::get_SCARD_P_SHUTDOWN() {
    return SCARD_P_SHUTDOWN;
}
LONG PCSCBridgeAPI::get_SCARD_E_PCI_TOO_SMALL() {
    return SCARD_E_PCI_TOO_SMALL;
}
LONG PCSCBridgeAPI::get_SCARD_E_READER_UNSUPPORTED() {
    return SCARD_E_READER_UNSUPPORTED;
}
LONG PCSCBridgeAPI::get_SCARD_E_DUPLICATE_READER() {
    return SCARD_E_DUPLICATE_READER;
}
LONG PCSCBridgeAPI::get_SCARD_E_CARD_UNSUPPORTED() {
    return SCARD_E_CARD_UNSUPPORTED;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_SERVICE() {
    return SCARD_E_NO_SERVICE;
}
LONG PCSCBridgeAPI::get_SCARD_E_SERVICE_STOPPED() {
    return SCARD_E_SERVICE_STOPPED;
}
LONG PCSCBridgeAPI::get_SCARD_E_UNEXPECTED() {
    return SCARD_E_UNEXPECTED;
}
LONG PCSCBridgeAPI::get_SCARD_E_UNSUPPORTED_FEATURE() {
    return SCARD_E_UNSUPPORTED_FEATURE;
}
LONG PCSCBridgeAPI::get_SCARD_E_ICC_INSTALLATION() {
    return SCARD_E_ICC_INSTALLATION;
}
LONG PCSCBridgeAPI::get_SCARD_E_ICC_CREATEORDER() {
    return SCARD_E_ICC_CREATEORDER;
}
LONG PCSCBridgeAPI::get_SCARD_E_DIR_NOT_FOUND() {
    return SCARD_E_DIR_NOT_FOUND;
}
LONG PCSCBridgeAPI::get_SCARD_E_FILE_NOT_FOUND() {
    return SCARD_E_FILE_NOT_FOUND;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_DIR() {
    return SCARD_E_NO_DIR;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_FILE() {
    return SCARD_E_NO_FILE;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_ACCESS() {
    return SCARD_E_NO_ACCESS;
}
LONG PCSCBridgeAPI::get_SCARD_E_WRITE_TOO_MANY() {
    return SCARD_E_WRITE_TOO_MANY;
}
LONG PCSCBridgeAPI::get_SCARD_E_BAD_SEEK() {
    return SCARD_E_BAD_SEEK;
}
LONG PCSCBridgeAPI::get_SCARD_E_INVALID_CHV() {
    return SCARD_E_INVALID_CHV;
}
LONG PCSCBridgeAPI::get_SCARD_E_UNKNOWN_RES_MNG() {
    return SCARD_E_UNKNOWN_RES_MNG;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_SUCH_CERTIFICATE() {
    return SCARD_E_NO_SUCH_CERTIFICATE;
}
LONG PCSCBridgeAPI::get_SCARD_E_CERTIFICATE_UNAVAILABLE() {
    return SCARD_E_CERTIFICATE_UNAVAILABLE;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_READERS_AVAILABLE() {
    return SCARD_E_NO_READERS_AVAILABLE;
}
LONG PCSCBridgeAPI::get_SCARD_E_COMM_DATA_LOST() {
    return SCARD_E_COMM_DATA_LOST;
}
LONG PCSCBridgeAPI::get_SCARD_E_NO_KEY_CONTAINER() {
    return SCARD_E_NO_KEY_CONTAINER;
}
LONG PCSCBridgeAPI::get_SCARD_E_SERVER_TOO_BUSY() {
    return SCARD_E_SERVER_TOO_BUSY;
}
LONG PCSCBridgeAPI::get_SCARD_W_UNSUPPORTED_CARD() {
    return SCARD_W_UNSUPPORTED_CARD;
}
LONG PCSCBridgeAPI::get_SCARD_W_UNRESPONSIVE_CARD() {
    return SCARD_W_UNRESPONSIVE_CARD;
}
LONG PCSCBridgeAPI::get_SCARD_W_UNPOWERED_CARD() {
    return SCARD_W_UNPOWERED_CARD;
}
LONG PCSCBridgeAPI::get_SCARD_W_RESET_CARD() {
    return SCARD_W_RESET_CARD;
}
LONG PCSCBridgeAPI::get_SCARD_W_REMOVED_CARD() {
    return SCARD_W_REMOVED_CARD;
}
LONG PCSCBridgeAPI::get_SCARD_W_SECURITY_VIOLATION() {
    return SCARD_W_SECURITY_VIOLATION;
}
LONG PCSCBridgeAPI::get_SCARD_W_WRONG_CHV() {
    return SCARD_W_WRONG_CHV;
}
LONG PCSCBridgeAPI::get_SCARD_W_CHV_BLOCKED() {
    return SCARD_W_CHV_BLOCKED;
}
LONG PCSCBridgeAPI::get_SCARD_W_EOF() {
    return SCARD_W_EOF;
}
LONG PCSCBridgeAPI::get_SCARD_W_CANCELLED_BY_USER() {
    return SCARD_W_CANCELLED_BY_USER;
}
LONG PCSCBridgeAPI::get_SCARD_W_CARD_NOT_AUTHENTICATED() {
    return SCARD_W_CARD_NOT_AUTHENTICATED;
}


const std::string PCSCBridgeAPI::strError(LONG code) {

    switch(code) {
    case SCARD_S_SUCCESS                 : 
        return std::string("SCARD_S_SUCCESS");
    case SCARD_F_INTERNAL_ERROR          : 
        return std::string("SCARD_F_INTERNAL_ERROR");
    case SCARD_E_CANCELLED               : 
        return std::string("SCARD_E_CANCELLED");
    case SCARD_E_INVALID_HANDLE          : 
        return std::string("SCARD_E_INVALID_HANDLE");
    case SCARD_E_INVALID_PARAMETER       : 
        return std::string("SCARD_E_INVALID_PARAMETER");
    case SCARD_E_INVALID_TARGET          : 
        return std::string("SCARD_E_INVALID_TARGET");
    case SCARD_E_NO_MEMORY               : 
        return std::string("SCARD_E_NO_MEMORY");
    case SCARD_F_WAITED_TOO_LONG         : 
        return std::string("SCARD_F_WAITED_TOO_LONG");
    case SCARD_E_INSUFFICIENT_BUFFER     : 
        return std::string("SCARD_E_INSUFFICIENT_BUFFER");
    case SCARD_E_UNKNOWN_READER          : 
        return std::string("SCARD_E_UNKNOWN_READER");
    case SCARD_E_TIMEOUT                 : 
        return std::string("SCARD_E_TIMEOUT");
    case SCARD_E_SHARING_VIOLATION       : 
        return std::string("SCARD_E_SHARING_VIOLATION");
    case SCARD_E_NO_SMARTCARD            : 
        return std::string("SCARD_E_NO_SMARTCARD");
    case SCARD_E_UNKNOWN_CARD            : 
        return std::string("SCARD_E_UNKNOWN_CARD");
    case SCARD_E_CANT_DISPOSE            : 
        return std::string("SCARD_E_CANT_DISPOSE");
    case SCARD_E_PROTO_MISMATCH          : 
        return std::string("SCARD_E_PROTO_MISMATCH");
    case SCARD_E_NOT_READY               : 
        return std::string("SCARD_E_NOT_READY");
    case SCARD_E_INVALID_VALUE           : 
        return std::string("SCARD_E_INVALID_VALUE");
    case SCARD_E_SYSTEM_CANCELLED        : 
        return std::string("SCARD_E_SYSTEM_CANCELLED");
    case SCARD_F_COMM_ERROR              : 
        return std::string("SCARD_F_COMM_ERROR");
    case SCARD_F_UNKNOWN_ERROR           : //((LONG)0x80100014) 
        return std::string("SCARD_F_UNKNOWN_ERROR");
    case SCARD_E_INVALID_ATR             : 
        return std::string("SCARD_E_INVALID_ATR");
    case SCARD_E_NOT_TRANSACTED          : 
        return std::string("SCARD_E_NOT_TRANSACTED");
    case SCARD_E_READER_UNAVAILABLE      : 
        return std::string("SCARD_E_READER_UNAVAILABLE");
    case SCARD_P_SHUTDOWN                : 
        return std::string("SCARD_P_SHUTDOWN");
    case SCARD_E_PCI_TOO_SMALL           : 
        return std::string("SCARD_E_PCI_TOO_SMALL");
    case SCARD_E_READER_UNSUPPORTED      : 
        return std::string("SCARD_E_READER_UNSUPPORTED");
    case SCARD_E_DUPLICATE_READER        : 
        return std::string("SCARD_E_DUPLICATE_READER");
    case SCARD_E_CARD_UNSUPPORTED        : 
        return std::string("SCARD_E_CARD_UNSUPPORTED");
    case SCARD_E_NO_SERVICE              : 
        return std::string("SCARD_E_NO_SERVICE");
    case SCARD_E_SERVICE_STOPPED         : 
        return std::string("SCARD_E_SERVICE_STOPPED");
    case SCARD_E_UNEXPECTED              : 
        return std::string("SCARD_E_UNEXPECTED");                
    case SCARD_E_ICC_INSTALLATION        : 
        return std::string("SCARD_E_ICC_INSTALLATION");
    case SCARD_E_ICC_CREATEORDER         : 
        return std::string("SCARD_E_ICC_CREATEORDER");
        /* case SCARD_E_UNSUPPORTED_FEATURE  :  / **< This smart card does not support the requested feature. */
    case SCARD_E_DIR_NOT_FOUND           : 
        return std::string("SCARD_E_DIR_NOT_FOUND");
    case SCARD_E_FILE_NOT_FOUND          : 
        return std::string("SCARD_E_FILE_NOT_FOUND");
    case SCARD_E_NO_DIR                  : 
        return std::string("SCARD_E_NO_DIR");
    case SCARD_E_NO_FILE                 : 
        return std::string("SCARD_E_NO_FILE");
    case SCARD_E_NO_ACCESS               : 
        return std::string("SCARD_E_NO_ACCESS");
    case SCARD_E_WRITE_TOO_MANY          : 
        return std::string("SCARD_E_WRITE_TOO_MANY");
    case SCARD_E_BAD_SEEK                : 
        return std::string("SCARD_E_BAD_SEEK");
    case SCARD_E_INVALID_CHV             : 
        return std::string("SCARD_E_INVALID_CHV");
    case SCARD_E_UNKNOWN_RES_MNG         : 
        return std::string("SCARD_E_UNKNOWN_RES_MNG");
    case SCARD_E_NO_SUCH_CERTIFICATE     : 
        return std::string("SCARD_E_NO_SUCH_CERTIFICATE");
    case SCARD_E_CERTIFICATE_UNAVAILABLE : 
        return std::string("SCARD_E_CERTIFICATE_UNAVAILABLE");
    case SCARD_E_NO_READERS_AVAILABLE    : 
        return std::string("SCARD_E_NO_READERS_AVAILABLE");
    case SCARD_E_COMM_DATA_LOST          : 
        return std::string("SCARD_E_COMM_DATA_LOST");
    case SCARD_E_NO_KEY_CONTAINER        : 
        return std::string("SCARD_E_NO_KEY_CONTAINER");
    case SCARD_E_SERVER_TOO_BUSY         : 
        return std::string("SCARD_E_SERVER_TOO_BUSY");
    case SCARD_W_UNSUPPORTED_CARD        : 
        return std::string("SCARD_W_UNSUPPORTED_CARD");
    case SCARD_W_UNRESPONSIVE_CARD       : 
        return std::string("SCARD_W_UNRESPONSIVE_CARD");
    case SCARD_W_UNPOWERED_CARD          : 
        return std::string("SCARD_W_UNPOWERED_CARD");
    case SCARD_W_RESET_CARD              : 
        return std::string("SCARD_W_RESET_CARD");
    case SCARD_W_REMOVED_CARD            : 
        return std::string("SCARD_W_REMOVED_CARD");
    case SCARD_W_SECURITY_VIOLATION      : 
        return std::string("SCARD_W_SECURITY_VIOLATION");
    case SCARD_W_WRONG_CHV               : 
        return std::string("SCARD_W_WRONG_CHV");
    case SCARD_W_CHV_BLOCKED             : 
        return std::string("SCARD_W_CHV_BLOCKED");
    case SCARD_W_EOF                     : 
        return std::string("SCARD_W_EOF");
    case SCARD_W_CANCELLED_BY_USER       : 
        return std::string("SCARD_W_CANCELLED_BY_USER");
    case SCARD_W_CARD_NOT_AUTHENTICATED  : 
        return std::string("SCARD_W_CARD_NOT_AUTHENTICATED");
    }
    return std::string("SCARD_WAT");
}




//##########################################################################
//##########################################################################
//                           SCardAPI 
//##########################################################################
//##########################################################################

SCardAPI::SCardAPI() {
    /* --------------------- LOW LEVEL API -------------------------- */
    registerMethod("EstablishContext", make_method(this, &SCardAPI::EstablishContext));
    registerMethod("ReleaseContext",   make_method(this, &SCardAPI::ReleaseContext));
    registerMethod("Connect",          make_method(this, &SCardAPI::Connect));
    registerMethod("Disconnect",       make_method(this, &SCardAPI::Disconnect));
    registerMethod("Transmit",         make_method(this, &SCardAPI::Transmit));
    registerMethod("Status",           make_method(this, &SCardAPI::Status));
    registerMethod("IsValidContext",   make_method(this, &SCardAPI::IsValidContext));
    registerMethod("BeginTransaction", make_method(this, &SCardAPI::BeginTransaction));
    registerMethod("EndTransaction",   make_method(this, &SCardAPI::EndTransaction));
    registerMethod("ListReaders",      make_method(this, &SCardAPI::ListReaders));
    registerMethod("Cancel",           make_method(this, &SCardAPI::Cancel));
    registerMethod("GetStatusChange",  make_method(this, &SCardAPI::GetStatusChange));
    registerMethod("Control",          make_method(this, &SCardAPI::Control));
    registerMethod("GetAttrib",        make_method(this, &SCardAPI::GetAttrib));
    registerMethod("SetAttrib",        make_method(this, &SCardAPI::SetAttrib));

}
SCardAPI::~SCardAPI() {}


/* --------------------- LOW LEVEL API -------------------------- */

/*
 * LONG SCardEstablishContext(DWORD 	dwScope,
 *                            LPCVOID 	pvReserved1,
 *                            LPCVOID 	pvReserved2,
 *                            LPSCARDCONTEXT 	phContext 
 *                           )	
 *
 * Expected JS args: 
 * {
 *   {number} dwScope, 
 *   {number} hContext,
 * }
 */
LONG  SCardAPI::EstablishContext(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;


    DWORD 	dwScope;               //in
    //LPCVOID 	pvReserved1,
    //LPCVOID 	pvReserved2,
    SCARDCONTEXT 	hContext ;     //out
        
    //convert args
    var = args->GetProperty("dwScope");
    dwScope = var.convert_cast<DWORD>();

    //call
    error = SCardEstablishContext(dwScope, NULL, NULL, &hContext);
   
    //set result
    args->SetProperty("hContext", (double)hContext);   
    
    return error;
}


/* 
 * LONG SCardReleaseContext(SCARDCONTEXT hContext)
 *
 *  Expected JS args: 
 * {
 *   {number} hContext,
 * }
 */
LONG  SCardAPI::ReleaseContext(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDCONTEXT 	hContext ;

    //convert args
    var = args->GetProperty("hContext");
    hContext = var.convert_cast<SCARDCONTEXT>();

    //call
    error = SCardReleaseContext(hContext);

    return error;
}
/*
 * LONG SCardConnect(SCARDCONTEXT    hContext,
 *                   LPCSTR          szReader,
 *                   DWORD           dwShareMode,
 *                   DWORD           dwPreferredProtocols,
 *                   LPSCARDHANDLE   phCard,
 *                   LPDWORD         pdwActiveProtocol 
 * )       
 *
 * Expected JS args:
 * {
 *   {number}  hContext,
 *   {string}  strReader,
 *   {number}  dwShareMode,
 *   {number}  dwPreferredProtocols,
 *   {number}  hCard,
 *   {nnumber} dwActiveProtocol,
 * }
 */
LONG  SCardAPI::Connect(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    
    SCARDCONTEXT    hContext;              // in
    /*
#ifdef WIN32
    WCHAR          szReader[256];          // in
#else
    char           szReader[256];          //in
#endif
    */
    LPTSTR          szReader;              //in
    DWORD           dwShareMode;           // in
    DWORD           dwPreferredProtocols;  // in
    SCARDHANDLE     hCard;                 // out
    DWORD           dwActiveProtocol;      // out

    szReader = NULL;

    // --- convert args ---
    try {
      //> hContext
      var =  args->GetProperty("hContext");
      hContext = var.convert_cast<SCARDCONTEXT>();
      //> szReader
      var =  args->GetProperty("strReader");    
      std::string readerName = var.convert_cast<std::string>();
      szReader = string2LPTSTR(readerName);
      //> dwShareMode
      var =  args->GetProperty("dwShareMode"); 
      dwShareMode = var.convert_cast<DWORD>();
      //> dwPreferredProtocols
      var =  args->GetProperty("dwPreferredProtocols"); 
      dwPreferredProtocols = var.convert_cast<DWORD>();
      
      // --- Call --- 
      hCard = -1;
      dwActiveProtocol  = -1;
      error = SCardConnect(hContext, szReader, dwShareMode, dwPreferredProtocols, 
                           &hCard, &dwActiveProtocol);

      // --- set  ---
      args->SetProperty("hCard", (double)hCard); 
      args->SetProperty("dwActiveProtocol", (double)dwActiveProtocol); 
    
    } catch(...) {
      error = -1;
    }
    delete[] szReader;
    return error;
}

/*
 * LONG SCardDisconnect(SCARDHANDLE     hCard,
 *                      DWORD   dwDisposition 
 *                     )       
 * Expected JS args:
 * {
 *   {number}  hCard,
 *   {number}  dwDisposition,
 * }
 */
LONG  SCardAPI::Disconnect(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDHANDLE     hCard;   //in
    DWORD   dwDisposition ;  //in

    // --- convert args ---
     //> hCard
    var =  args->GetProperty("hCard");
    hCard = var.convert_cast<SCARDHANDLE>();
    //>ioSendPci
    var =  args->GetProperty("dwDisposition");
    dwDisposition  = var.convert_cast<DWORD>();

    // --- Call ---
    error = SCardDisconnect(hCard, dwDisposition);

    return error;
}

/*
 * LONG SCardStatus(SCARDHANDLE     hCard,
 *                  LPSTR           mszReaderName,
 *                  LPDWORD         pcchReaderLen,
 *                  LPDWORD         pdwState,
 *                  LPDWORD         pdwProtocol,
 *                  LPBYTE  pbAtr,
 *                  LPDWORD         pcbAtrLen 
 *                )   
 * 
 * Expected JS args:
 * {    
 *   {number}    hCard,
 *   {string}    strReaderName,
 *   {number}    dwState,
 *   {number}    dwProtocol,
 *   {hexstring} bATR,
 * }
 */
LONG  SCardAPI::Status(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;
    
    SCARDHANDLE    hCard;              // in
#ifdef WIN32
    WCHAR          szReader[512];      // out
#else
    char           szReader[512];      //out
#endif
    DWORD          dwReaderLen;        //in,out
    DWORD          dwState;            // out
    DWORD          dwProtocol;   // out
    unsigned char  bATR[128];
    DWORD          bATRLen;

    // --- convert args ---
    //> hContext
    var =  args->GetProperty("hCard");
    hCard = var.convert_cast<SCARDHANDLE>();

    // --- Call --- 
    memset(szReader, 0, sizeof(szReader));    
    dwReaderLen = 512;
    dwState = 0;
    dwProtocol = 0;
    memset(bATR, 0, sizeof(bATR));    
    bATRLen =  sizeof(bATR);
    error = SCardStatus(hCard,
                        szReader, &dwReaderLen, 
                        &dwState, &dwProtocol,
                        bATR, &bATRLen);
    
    // --- Set Result ---
    //> strReaderName
    var = szReader;
    args->SetProperty("strReaderName", var); 

    //> dwState    
    args->SetProperty("dwState", (double)dwState);

    //> dwProtocol
    args->SetProperty("dwProtocol", (double)dwProtocol);

    //> bATR
    var = binaryToHexstr(bATR, bATRLen);
    args->SetProperty("bATR", var);

    return error;
}

/*
 * LONG SCardTransmit(SCARDHANDLE              hCard,
 *                    const SCARD_IO_REQUEST * pioSendPci,
 *                    LPCBYTE                  pbSendBuffer,
 *                    DWORD                    cbSendLength,
 *                    SCARD_IO_REQUEST *       pioRecvPci,
 *                    LPBYTE                   pbRecvBuffer,
 *                    LPDWORD                  pcbRecvLength
 *                   )       
 *
 * Expected JS args:
 * {
 *   {number}    hCard,
 *   {number}    ioSendPci,
 *   {hexstring} bSendBuffer,
 *   {number}    ioRecvPci,
 *   {hexstring} bRecvBuffer,
 *  }
 */
LONG  SCardAPI::Transmit(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDHANDLE                hCard;            //in
    DWORD/*SCARD_IO_REQUEST*/  ioSendPci;        //in/out
    std::string                bSendBuffer;      //in
    DWORD                      dwSendLength;     //in
    std::string                bRecvBuffer;      //in
    DWORD                      dwRecvLength;     //in/out

    LPBYTE                 pbSendBuffer;
    LPBYTE                 pbRecvBuffer;
    SCARD_IO_REQUEST       sPCI;
    SCARD_IO_REQUEST       rPCI;
    int                    len;
 
    pbSendBuffer = NULL;
    pbRecvBuffer = NULL;
    try {
      // --- convert args ---
      //> hCard
      var =  args->GetProperty("hCard");
      hCard = var.convert_cast<SCARDHANDLE>();
      //>ioSendPci
      var =  args->GetProperty("ioSendPci");
      ioSendPci  = var.convert_cast<DWORD>();
      //>pbSendBuffer
      var =  args->GetProperty("bSendBuffer");
      bSendBuffer = var.convert_cast<std::string>();
      
      pbSendBuffer = hexstrToBinary(bSendBuffer, &len);
      dwSendLength = len;
      if (dwSendLength<0) {
        return SCARD_E_INVALID_PARAMETER;
      }
      
      dwRecvLength = RECV_BUFFER_SIZE;
      pbRecvBuffer = new unsigned char[dwRecvLength];
      memset(pbRecvBuffer, 0, RECV_BUFFER_SIZE);
      
      //--- Call ---
      switch(ioSendPci) {
      case SCARD_PROTOCOL_T0:
        sPCI = *SCARD_PCI_T0;
        break;
        
      case SCARD_PROTOCOL_T1:
        sPCI = *SCARD_PCI_T1;
        break;
      case SCARD_PROTOCOL_RAW:
        sPCI = *SCARD_PCI_RAW;
        break;        
      }
      error = SCardTransmit(hCard, &sPCI, 
                            pbSendBuffer, dwSendLength,
                            NULL, pbRecvBuffer, &dwRecvLength);
      // --- set result ---
      //>ioSendPci
      args->SetProperty("ioSendPci",  (double)ioSendPci); 
      //>ioRecvPci
      ;  //ignored
      //>bRecvBuffer
      var = binaryToHexstr(pbRecvBuffer,dwRecvLength);
      args->SetProperty("bRecvBuffer", var); 
    } catch (...) {
      error = -1;
    }
    delete[] pbSendBuffer;
    delete[] pbRecvBuffer;    
    return error;
}




/**
 * LONG  SCardIsValidContext (SCARDCONTEXT hContext)
 * 
 * Expected JS args:
 * {
 *   {number}    hcontext,
 * } 
 */
LONG  SCardAPI::IsValidContext (FB::JSObjectPtr args)  {
    LONG   error;
    FB::variant var;

    SCARDCONTEXT 	hContext ;     //in
    
    // --- convert args ---
    var = args->GetProperty("hContext");
    hContext = var.convert_cast<SCARDCONTEXT>();

    // --- Call --- 
    error = SCardIsValidContext(hContext);

    return error;
}

/*
 *   LONG  SCardBeginTransaction (SCARDHANDLE hCard);
 * Expected JS args:
 * {
 *   {number}    hCard,
 * } 
 */
LONG  SCardAPI::BeginTransaction (FB::JSObjectPtr args)  {
    LONG   error;
    FB::variant var;
    
    SCARDHANDLE     hCard;   //in

    // --- convert args ---
     //> hCard
    var =  args->GetProperty("hCard");
    hCard = var.convert_cast<SCARDHANDLE>();

    // --- Call ---
    error = SCardBeginTransaction(hCard);

    return error;
}

/*
 *   LONG  SCardEndTransaction(SCARDHANDLE hCard,DWORD dwDisposition);
 * Expected JS args:
 * {
 *   {number}  hCard,
 *   {number}  dwDisposition,
 * } 
 */
LONG  SCardAPI::EndTransaction(FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDHANDLE     hCard;   //in
    DWORD   dwDisposition ;  //in

    // --- convert args ---
     //> hCard
    var =  args->GetProperty("hCard");
    hCard = var.convert_cast<SCARDHANDLE>();
    //> dwDisposition
    var =  args->GetProperty("dwDisposition");
    dwDisposition  = var.convert_cast<DWORD>();

    // --- Call ---
    error = SCardEndTransaction(hCard, dwDisposition);

    return error;
}

/*
 * LONG  SCardListReaders(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders);
 *
 * Expected JS args:
 * {
 *   {number}    hContext,
 *   {string}    strGroups,
 *   {string}    strReaders[]
 * } 
 */
 LONG  SCardAPI::ListReaders(FB::JSObjectPtr args) {
     LONG   error;
     FB::variant var;
     
     SCARDCONTEXT 	hContext ;     //in
     std::vector<std::string> readers;
     // --- convert args ---
     //> hContext
     var = args->GetProperty("hContext");
     hContext = var.convert_cast<SCARDCONTEXT>();
     //> mszGroups
     ;  //no used
     
     // --- Call ---
     error = getReaderList(hContext, readers);
     
     // --- Set ---
     args->SetProperty("strReaders", readers); 
     
    return error;
}

/*
 * LONG  SCardCancel (SCARDCONTEXT hContext);
 *
 * Expected JS args:
 * {
 *   {number}    hcontext,
 * } 
 */
LONG  SCardAPI::Cancel (FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDCONTEXT 	hContext ;     //in
    
    // --- convert args ---
    var = args->GetProperty("hContext");
    hContext = var.convert_cast<SCARDCONTEXT>();

    // --- Call --- 
    error = SCardCancel(hContext);

    return error;
}

/*
 * LONG 	SCardGetStatusChange (SCARDCONTEXT hContext, DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders)
 *
 * Expected JS args:
 * {
 *   {number}      hcontext,
 *   {number}      dwTimeout,
 *   {readerState} readerStates[]
 * }
 *
 * readerState {
 *   {sring}      strReader;           // in, Reader name
 *   {number}     dwCurrentState;      // in, Current state of reader
 *   {number]     dwEventState;        // out, Reader state after a state change
 *   {hexstring}  bAtr;                // out
 *  }
 */
LONG 	SCardAPI::GetStatusChange (FB::JSObjectPtr args) {
    LONG                error;
    FB::variant         var;
    FB::JSObjectPtr     readerStates; 

    SCARDCONTEXT 	hContext ;       //in
    DWORD               dwTimeout;       //in
    SCARD_READERSTATE   *rgReaderStates; //in,out
    DWORD               cReaders;        //in


    rgReaderStates = NULL;
    try {
      // --- convert args ---
      var = args->GetProperty("hContext");
      hContext = var.convert_cast<SCARDCONTEXT>();
      
      var = args->GetProperty("dwTimeout");
      dwTimeout = var.convert_cast<DWORD>();
      
      var = args->GetProperty("readerStates");
      readerStates = var.convert_cast<FB::JSObjectPtr>();
      
      /*
        typedef struct {
        LPCTSTR szReader;
        LPVOID  pvUserData;
        DWORD   dwCurrentState;
        DWORD   dwEventState;
        DWORD   cbAtr;
        BYTE    rgbAtr[36];
        } SCARD_READERSTATE;
      */
      var = readerStates->GetProperty("length");
      cReaders = var.convert_cast<DWORD>();
      rgReaderStates = new SCARD_READERSTATE[cReaders];
      memset(rgReaderStates, 0, sizeof(SCARD_READERSTATE)*cReaders);  
      
      for (unsigned int i = 0; i < cReaders; i++) {
        FB::JSObjectPtr readerelt;
        var = readerStates->GetProperty(i);                
        readerelt = var.convert_cast<FB::JSObjectPtr>();
        
        var = readerelt->GetProperty("strReader");
        rgReaderStates[i].szReader       = string2LPTSTR(var.convert_cast<std::string>());

        var = readerelt->GetProperty("dwCurrentState"); 
        rgReaderStates[i].dwCurrentState = var.convert_cast<DWORD>();
      }
      
      // --- Call ---
      error = SCardGetStatusChange(hContext, dwTimeout, rgReaderStates, cReaders);
      
      // --- Set result ---
      for (unsigned int i = 0; i < cReaders; i++) {
        FB::JSObjectPtr  readerelt;
        var = readerStates->GetProperty(i);                
        readerelt = var.convert_cast<FB::JSObjectPtr>();
        
        readerelt->SetProperty("dwEventState",(double)rgReaderStates[i].dwEventState); 
        
        var = binaryToHexstr(rgReaderStates[i].rgbAtr, rgReaderStates[i].cbAtr);
        readerelt->SetProperty("bAtr",var);
        
      }
    } catch (...) {
      error = -1;
    }

    for (unsigned int i = 0; i < cReaders; i++) {
        delete[] rgReaderStates[i].szReader;
    }
    delete[] rgReaderStates;
 
    return error;
}

/*
 * LONG SCardControl (SCARDHANDLE hCard, DWORD dwControlCode, 
 *                    LPCVOID pbSendBuffer, DWORD cbSendLength, 
 *                    LPVOID  pbRecvBuffer, DWORD cbRecvLength, 
 *                    LPDWORD lpBytesReturned)
 *
 * Expected JS args:
 * {
 *   {number}     hCard,
 *   {number}     dwControlCode,
 *   {hexstring}  bSendBuffer
 *   {hexstring}  bRecvBuffer
 * }
 */
LONG 	SCardAPI::Control (FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDHANDLE                hCard;            //in
    DWORD                      dwControlCode;    //in
    std::string                bSendBuffer;      //in
    std::string                bRecvBuffer;      //out

    LPBYTE                     pbSendBuffer;
    DWORD                      cbSendLength;     
    LPBYTE                     pbRecvBuffer;
    DWORD                      cbRecvLength;     
    DWORD                      cbBytesReturned;   
    int                        len;

    pbSendBuffer = NULL;
    pbRecvBuffer = NULL;

    try {
      // --- convert args ---
      //> hCard
      var =  args->GetProperty("hCard");
      hCard = var.convert_cast<SCARDHANDLE>();
      //>dwControlCode
      var =  args->GetProperty("dwControlCode");
      dwControlCode = var.convert_cast<DWORD>();
      //>pbSendBuffer
      var =  args->GetProperty("bSendBuffer");
      bSendBuffer = var.convert_cast<std::string>();
      
      
      pbSendBuffer = hexstrToBinary(bSendBuffer, &len);
      cbSendLength = len;
      if (cbSendLength<0) {
        return SCARD_E_INVALID_PARAMETER;
      }
      
      cbRecvLength = RECV_BUFFER_SIZE;
      pbRecvBuffer = new unsigned char[RECV_BUFFER_SIZE];
      memset(pbRecvBuffer, 0, cbRecvLength);
      
      cbBytesReturned = 0;
      // --- Call ---
      error = SCardControl(hCard, dwControlCode, 
                           pbSendBuffer, cbSendLength,
                           pbRecvBuffer, cbRecvLength,
                           &cbBytesReturned);
      
      // --- set result ---
      var = binaryToHexstr(pbRecvBuffer, cbBytesReturned);
      args->SetProperty("bRecvBuffer", var); 
    } catch(...) {
      error = -1;
    }
    delete[] pbSendBuffer;
    delete[] pbRecvBuffer;
    return error;
}

/*
 * LONG 	SCardGetAttrib (SCARDHANDLE hCard, DWORD dwAttrId, LPBYTE pbAttr, LPDWORD pcbAttrLen)
 *
 * Expected JS args:
 * {
 *   {number}     hCard,
 *   {number}     dwAttrId,
 *   {hexstring}  bAttr
 * }
 */
LONG 	SCardAPI::GetAttrib (FB::JSObjectPtr args) {
    LONG   error;
    FB::variant var;

    SCARDHANDLE                hCard;        //in
    DWORD                      dwAttrId;     //in
    std::string                bAttr;        //in

    LPBYTE   pbAttr = NULL;
    DWORD    pcbAttrLen = SCARD_AUTOALLOCATE;

    // --- convert args ---
    //> hCard
    var =  args->GetProperty("hCard");
    hCard = var.convert_cast<SCARDHANDLE>();
    //>dwControlCode
    var =  args->GetProperty("dwAttrId");
    dwAttrId = var.convert_cast<DWORD>();

    // --- Call ---
    error = SCardGetAttrib(hCard, dwAttrId, (LPBYTE)&pbAttr, &pcbAttrLen);

    // --- set result ---
    if (error == SCARD_S_SUCCESS) {
        var = binaryToHexstr(pbAttr, pcbAttrLen);
        args->SetProperty("bAttr", var); 
    }
    
    SCardFreeMemory(hCard, pbAttr);
    return error;
}

/*
 * LONG 	SCardSetAttrib (SCARDHANDLE hCard, DWORD dwAttrId, LPCBYTE pbAttr, DWORD cbAttrLen)
 *
 * Expected JS args:
 * {
 *   {number}     hCard,
 *   {number}     dwAttrId,
 *   {hexstring}  bAttr
 * }
 */
LONG 	SCardAPI::SetAttrib (FB::JSObjectPtr args) {
      LONG   error;
    FB::variant var;

    SCARDHANDLE                hCard;        //in
    DWORD                      dwAttrId;     //in
    std::string                bAttr;        //in

    LPBYTE   pbAttr;
    DWORD    cbAttrLen;
    int      len;

    pbAttr = NULL;

    try {
      // --- convert args ---
      //> hCard
      var =  args->GetProperty("hCard");
      hCard = var.convert_cast<SCARDHANDLE>();
      //>dwControlCode
      var =  args->GetProperty("dwAttrId");
      dwAttrId = var.convert_cast<DWORD>();
      //>bAttr
      var =  args->GetProperty("bAttr");
      bAttr = var.convert_cast<std::string>();
      
      
      pbAttr = hexstrToBinary(bAttr, &len);
      cbAttrLen = len;
      
      // --- Call ---
      error = SCardSetAttrib(hCard, dwAttrId, 
                             pbAttr,
                             cbAttrLen);
    } catch(...){
      error = -1;
    }
    delete[] pbAttr;
    return error;
}



//##########################################################################
//##########################################################################
//                           TerminalAPI 
//##########################################################################
//##########################################################################
PCSCTerminalAPI::PCSCTerminalAPI() {
    registerMethod("listReaders",  make_method(this, &PCSCTerminalAPI::listReaders));
    registerMethod("selectReader", make_method(this, &PCSCTerminalAPI::selectReader));
    registerMethod("reinit",       make_method(this, &PCSCTerminalAPI::reinit));
    scard_error = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
}

PCSCTerminalAPI::~PCSCTerminalAPI() { 
    if (SCardIsValidContext(hContext) == SCARD_S_SUCCESS) {
        SCardReleaseContext(hContext);
    }
}

LONG PCSCTerminalAPI::reinit() {
    if (SCardIsValidContext(hContext) == SCARD_S_SUCCESS) {
        SCardReleaseContext(hContext);
    }
    scard_error = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    return scard_error;
}

std::vector<std::string>  PCSCTerminalAPI::listReaders() {
    std::vector<std::string> readers;    

    scard_error = getReaderList(hContext, readers);
    return readers;
}

PCSCReaderAPIPtr PCSCTerminalAPI::selectReader(std::string rname) {   
    return boost::make_shared<PCSCReaderAPI>(rname, hContext);
}




//##########################################################################
//##########################################################################
//                           ReaderAPI 
//##########################################################################
//##########################################################################

PCSCReaderAPI::PCSCReaderAPI() {
}

PCSCReaderAPI::PCSCReaderAPI(const std::string rname, SCARDCONTEXT c) : readerName(rname),ATR() {

    //internals
    hContext         = c;
    hCard            = 0;

    /* Method */
    registerMethod("powerUp",      make_method(this, &PCSCReaderAPI::powerUp));
    registerMethod("powerDown",    make_method(this, &PCSCReaderAPI::powerDown));    
    registerMethod("transmit",     make_method(this, &PCSCReaderAPI::transmit));  
    registerMethod("exchangeAPDU", make_method(this, &PCSCReaderAPI::exchangeAPDU));

    /* Read-only properties */
    registerProperty("atr",
                     make_property(this,
                                   &PCSCReaderAPI::get_atr));
    protocol         = -1;
    registerProperty("protocol",
                     make_property(this,
                                   &PCSCReaderAPI::get_protocol));
    registerProperty("error",
                     make_property(this,
                                   &PCSCReaderAPI::get_error));

    /* RW properties*/
    autoChaining      = false;
    registerProperty("autoChaining",  
                     make_property(this, 
                                   &PCSCReaderAPI::get_autoChaining, 
                                   &PCSCReaderAPI::set_autoChaining));    
    autoReissue      = false;
    registerProperty("autoReissue",  
                     make_property(this, 
                                   &PCSCReaderAPI::get_autoReissue, 
                                   &PCSCReaderAPI::set_autoReissue));    

    autoGetResponse  = false;
    registerProperty("autoGetResponse", 
                     make_property(this, 
                                   &PCSCReaderAPI::get_autoGetResponse, 
                                   &PCSCReaderAPI::set_autoGetResponse));   

    scardMode        = SCARD_SHARE_SHARED;  
    registerProperty("scardMode", 
                     make_property(this, 
                                   &PCSCReaderAPI::get_scardMode, 
                                   &PCSCReaderAPI::set_scardMode));  

    preferredProtocols = SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1;
    registerProperty("preferredProtocols", 
                     make_property(this, 
                                   &PCSCReaderAPI::get_preferredProtocols, 
                                   &PCSCReaderAPI::set_preferredProtocols));  

    extendedLengthSupported = false;
    registerProperty("extendedLengthSupported", 
                     make_property(this, 
                                   &PCSCReaderAPI::get_extendedLengthSupported, 
                                   &PCSCReaderAPI::set_extendedLengthSupported));  

}

PCSCReaderAPI::~PCSCReaderAPI() {
    if(hCard) {
        SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
        hCard = 0;
    }
}

/////////////////////
//R-only properties//
/////////////////////
std::string PCSCReaderAPI::get_atr() {
    return ATR;
}

int PCSCReaderAPI::get_protocol() {
    return protocol;
}

int PCSCReaderAPI::get_error() {
    return scard_error;
}

int  PCSCReaderAPI::get_SW(LPBYTE pbRecvBuffer, DWORD dwRecvLength) {
    if (dwRecvLength <2) {
        return 0;
    }
    int sw = 
        ((pbRecvBuffer[dwRecvLength-2]&0xFF) <<8) |  
        (pbRecvBuffer[dwRecvLength-1]&0xFF);
    return sw;
}

/////////////////
//RW properties//
/////////////////
//autoChaining
bool PCSCReaderAPI::get_autoChaining() {
    return autoChaining;
}
void PCSCReaderAPI::set_autoChaining(bool x) {
    autoChaining = x ;
}
//autoReissue
bool PCSCReaderAPI::get_autoReissue() {
    return autoReissue;
}
void PCSCReaderAPI::set_autoReissue(bool x) {
    autoReissue = x ;
}
//autoGetResponse
bool PCSCReaderAPI::get_autoGetResponse() {
    return autoGetResponse;
}
void PCSCReaderAPI::set_autoGetResponse(bool x) {
    autoGetResponse = x ;
}

//scardMode
void PCSCReaderAPI::set_scardMode(int x) {
    switch (x) {
    case SCARD_SHARE_SHARED:
    case SCARD_SHARE_EXCLUSIVE:
    case SCARD_SHARE_DIRECT:
        scardMode = x;
    }
}
int PCSCReaderAPI::get_scardMode() {
    return scardMode;
}

//preferredProtocols
int PCSCReaderAPI::get_preferredProtocols() {
    return preferredProtocols;
}
void PCSCReaderAPI::set_preferredProtocols(int x) {
    if (x & ~(SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1|SCARD_PROTOCOL_RAW)) {
        return;
    }
    preferredProtocols = x;
}

//extendedLengthSupported
bool PCSCReaderAPI::get_extendedLengthSupported() {
    return extendedLengthSupported;
}
void PCSCReaderAPI::set_extendedLengthSupported(bool x) {
    extendedLengthSupported = x ;
}


///////////
//Methods//
///////////
std::string  PCSCReaderAPI::powerUp() {
    /*
#ifdef WIN32
    WCHAR         mszReaderName[256];
#else
    char          mszReaderName[256];
#endif
    */
    LPTSTR       mszReaderName;
    unsigned char atr[32];
    DWORD         readerLen, atrLen;
    DWORD         dwActiveProtocol, dwState;

    //check ctx
    scard_error = SCardIsValidContext(hContext);
    if (SCARD_S_SUCCESS != scard_error) {
        return ATR;
    }

    //clean up
    ATR.clear();
    protocol = -1;
    if (hCard) {
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
    }
    hCard = 0;

    //convert reader name
    mszReaderName = string2LPTSTR(readerName);

    // connect to selected reader
    scard_error = SCardConnect(hContext, mszReaderName, 
                               scardMode, preferredProtocols, 
                               &hCard, &dwActiveProtocol);
    if (SCARD_S_SUCCESS != scard_error) {
        delete[] mszReaderName;
        return ATR;
    }
    protocol = dwActiveProtocol;

    switch(dwActiveProtocol)  {
    case SCARD_PROTOCOL_T0:
        pioSendPci = *SCARD_PCI_T0;
        break;
    
    case SCARD_PROTOCOL_T1:
        pioSendPci = *SCARD_PCI_T1;
        break;
    }

    readerLen = 128;
    atrLen = 32;
    scard_error = SCardStatus(hCard,
                              mszReaderName, &readerLen, 
                              &dwState, &dwActiveProtocol,
                              atr, &atrLen);
    delete[] mszReaderName;
    
    if (SCARD_S_SUCCESS != scard_error) {
        return ATR;
    }   
    ATR = binaryToHexstr(atr, atrLen);
    return ATR;
}

bool  PCSCReaderAPI::checkSW(LPBYTE pbRecvBuffer, DWORD dwRecvLength, int sw, int swmask) {
    int r = get_SW(pbRecvBuffer, dwRecvLength);
    return  (r&swmask) == sw;
}


std::string  PCSCReaderAPI::transmit(std::string apdu) {
    std::string err;
    std::string rapdu;
    int         len; 
    LPBYTE      pbSendBuffer;
    LPBYTE      pbRecvBuffer;
    DWORD       dwRecvLength;
    DWORD       dwSendLength; 

    //check ctx
    scard_error = SCardIsValidContext(hContext);
    if (SCARD_S_SUCCESS != scard_error) {
        return err;
    }

    //transmit
    pbSendBuffer = hexstrToBinary(apdu, &len);
    dwSendLength = len;
    if (dwSendLength<0) {
        scard_error = SCARD_E_INVALID_PARAMETER;
        return rapdu;
    }

    dwRecvLength = RECV_BUFFER_SIZE;
    pbRecvBuffer = new unsigned char[dwRecvLength];
    memset(pbRecvBuffer, 0, dwRecvLength);

    scard_error = SCardTransmit(hCard, &pioSendPci, 
                                pbSendBuffer, dwSendLength,
                                NULL, pbRecvBuffer, &dwRecvLength);
    if (SCARD_S_SUCCESS != scard_error) {
        return err;
    }
    
    rapdu = binaryToHexstr(pbRecvBuffer,dwRecvLength);
    delete[] pbSendBuffer;
    delete[] pbRecvBuffer;
    return rapdu;
}


std::string  PCSCReaderAPI::exchangeAPDU(std::string apdu, bool extendedAPDU) {
    int         offset;
    std::string rapdu;
    int         len;
    LPBYTE      pbSendBuffer;
    LPBYTE      pbRecvBuffer;
    DWORD       dwRecvLength;
    DWORD       dwSendLength; 

    //check ctx
    scard_error = SCardIsValidContext(hContext);
    if (SCARD_S_SUCCESS != scard_error) {
        return rapdu;
    }

    //convert apdu
    pbSendBuffer = hexstrToBinary(apdu, &len);
    dwSendLength = len;
    if (dwSendLength<0) {
        scard_error = SCARD_E_INVALID_PARAMETER;
        return rapdu;
    }

    dwRecvLength = RECV_BUFFER_SIZE;
    pbRecvBuffer = new unsigned char[dwRecvLength];
    memset(pbRecvBuffer, 0, dwRecvLength);


    //if outgoing extended APDU, and card does not support it, 
    //handle chaining automagically
    if (extendedAPDU && !extendedLengthSupported && autoChaining) {
        int lc = ((pbSendBuffer[5]&0xFF)<<8) | (pbSendBuffer[6]&0xFF);
        offset = 7;
        if (lc >232) {
            pbSendBuffer[0] |= 0x10;
            pbSendBuffer[4] = 232;
            while (lc > 232) {
                memmove(pbSendBuffer+5, pbSendBuffer+offset, 232);
                dwRecvLength = RECV_BUFFER_SIZE;
                scard_error = SCardTransmit(hCard, &pioSendPci, 
                                            pbSendBuffer, 5+232,
                                            NULL, pbRecvBuffer, &dwRecvLength);
                if (SCARD_S_SUCCESS != scard_error) {
                    return rapdu;
                }
                if(!checkSW(pbRecvBuffer, dwRecvLength, 0x9000, 0xFFFF)) {
                    return rapdu;
                }
                offset += 232;
                lc -= 232;
            }
        }
        pbSendBuffer[0] &= ~0x10;
        pbSendBuffer[4] = lc;
        memmove(pbSendBuffer+5, pbSendBuffer+offset, lc);
        dwRecvLength = RECV_BUFFER_SIZE;
        scard_error = SCardTransmit(hCard, &pioSendPci, 
                                    pbSendBuffer, 5+lc,
                                    NULL, pbRecvBuffer, &dwRecvLength);
        if (SCARD_S_SUCCESS != scard_error) {
            return rapdu;
        }

    } 
    //else just send and see....
    else {
        dwRecvLength = RECV_BUFFER_SIZE;
        scard_error = SCardTransmit(hCard, &pioSendPci, 
                                    pbSendBuffer, dwSendLength,
                                    NULL, pbRecvBuffer, &dwRecvLength);
        if (SCARD_S_SUCCESS != scard_error) {
            return rapdu;
        }
    }

    //handle 6Cxx, if autoReissue set
    if ( ((get_SW(pbRecvBuffer, dwRecvLength) &0xFF00) == 0x6C00) && autoReissue) {
        pbSendBuffer[4] = get_SW(pbRecvBuffer, dwRecvLength) &0x00FF;
        dwRecvLength = RECV_BUFFER_SIZE;
        scard_error = SCardTransmit(hCard, &pioSendPci, 
                                    pbSendBuffer, dwSendLength,
                                    NULL, pbRecvBuffer, &dwRecvLength);
        if (SCARD_S_SUCCESS != scard_error) {
            return rapdu;
        }
    }

    //handle 61xx, if autoGetResponse set
    offset = dwRecvLength-2;
    if (((get_SW(pbRecvBuffer, dwRecvLength) &0xFF00) == 0x6100) && autoGetResponse) {
        while ((get_SW(pbRecvBuffer, dwRecvLength) &0xFF00) == 0x6100) {
            //next offset for data to receive, 61xx removed
            pbSendBuffer[0] = 0x00;
            pbSendBuffer[1] = 0xC0;
            pbSendBuffer[2] = 0x00;
            pbSendBuffer[3] = 0x00;
            pbSendBuffer[4] = get_SW(pbRecvBuffer, dwRecvLength)&0xFF;
            dwSendLength = 5;
            
            dwRecvLength = RECV_BUFFER_SIZE-offset;
            
            scard_error = SCardTransmit(hCard, &pioSendPci, 
                                        pbSendBuffer, dwSendLength,
                                        NULL, pbRecvBuffer+offset, &dwRecvLength);
            
            if (SCARD_S_SUCCESS != scard_error) {
                return rapdu;
            }
            dwRecvLength += offset;
            offset = dwRecvLength-2;
        }   
    }

    rapdu = binaryToHexstr(pbRecvBuffer,dwRecvLength);    
    delete[] pbSendBuffer;
    delete[] pbRecvBuffer;
    return rapdu;
}

void  PCSCReaderAPI::powerDown() {
    //check ctx
    scard_error = SCardIsValidContext(hContext);
    if (SCARD_S_SUCCESS != scard_error) {
        return;
    }
    //down
    if (hCard) {
        scard_error = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
        hCard = 0;
    }
}
