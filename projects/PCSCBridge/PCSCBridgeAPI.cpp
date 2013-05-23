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
static const std::string HEXDIGITS = "0123456789ABCDEF";

static int digitToHex(int digit) {
    if ((digit >= '0') && (digit <= '9')) {
        return digit - '0';
    }
    else
        if ((digit >= 'A') && (digit <= 'F')) {
            return (digit - 'A') + 10;
        }
        else
            if ((digit >= 'a') && (digit <= 'f')) {
                return (digit - 'a') + 10;
            }
    return -1;
}

static int hexToBinary(const std::string& data, unsigned char *targetBin) {
    int dataLength = data.length();
    if ((dataLength % 2) != 0) {
        return -1;
    }
    for (int i=0; i<dataLength; i += 2) {
        int d1, d2;
        d1 = digitToHex(data[i]);
        d2 = digitToHex(data[i + 1]);
        if ((d1 < 0) || (d2 < 0)) {
            return -1;
        }
        targetBin[i / 2] = (d1 << 4) + d2;
    } 
    return (dataLength / 2);
}

static std::string binaryToHex(const unsigned char *sourceBinary, int length) {
    std::string result;
    for (int i=0; i<length; i++) {
        int digit = sourceBinary[i];
        result.push_back(HEXDIGITS[(digit >> 4) & 0x0f]);
        result.push_back(HEXDIGITS[digit & 0x0f]);
    }
    return result;
}


//##########################################################################
//##########################################################################
//                            PCSCBridgeAPI
//##########################################################################
//##########################################################################


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

// Read-only property version
std::string PCSCBridgeAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}


bool PCSCBridgeAPI::init() {
    //open PCSC
    scard_error = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    return  SCARD_S_SUCCESS == scard_error;
}

#ifdef WIN32
typedef std::basic_string<TCHAR, std::char_traits<TCHAR>> tstring;

std::string LPTSTR2string(const LPTSTR s) {
    char cstr[256];

    memset(cstr,0,256);
    WideCharToMultiByte(CP_UTF8,0,s,-1,
                        cstr,256,
                        NULL,NULL);
    std::string str(cstr);
    return str; 
}
#endif

std::vector<std::string>  PCSCBridgeAPI::listReaders() {
    std::vector<std::string> readers;    

    //if not inited return empty list
    if (!hContext) {
        return readers;
    }

    //list readers names
    LPTSTR mszReaders = NULL;
    DWORD dwReaders   = SCARD_AUTOALLOCATE;
    scard_error = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (SCARD_S_SUCCESS != scard_error) {
        return readers;
    }

    //vectorize...
    LPTSTR  preader = mszReaders;
    int len = dwReaders;
    while (len && (*preader != '\0')) {
        int l;
#ifdef WIN32
        char cstr[256];
        memset(cstr,0,256);
        WideCharToMultiByte(CP_UTF8,0,preader,-1,
                            cstr,256,
                            NULL,NULL);

        readers.push_back((char*)&cstr[0]);     
        /*
          std::string str;
          str = LPTSTR2string(preader);
          readers.push_back(str);
        */
        l = wcslen(preader)+1;
#else
        readers.push_back(preader);
        l = strlen(preader)+1;
#endif
        preader  += l;
        len     -=l;
    }
    scard_error = SCardFreeMemory( hContext,mszReaders );
    return readers;
}


PCSCReaderAPIPtr PCSCBridgeAPI::selectReader(std::string rname) {   
    return boost::make_shared<PCSCReaderAPI>(rname, hContext);
}

const std::string PCSCBridgeAPI::strError(LONG code) {

    switch(code) {
    case SCARD_S_SUCCESS                 : //((LONG)0x00000000) 
        return std::string("SCARD_S_SUCCESS");
    case SCARD_F_INTERNAL_ERROR          : //((LONG)0x80100001) 
        return std::string("SCARD_F_INTERNAL_ERROR");
    case SCARD_E_CANCELLED               : //((LONG)0x80100002) 
        return std::string("SCARD_E_CANCELLED");
    case SCARD_E_INVALID_HANDLE          : //((LONG)0x80100003) 
        return std::string("SCARD_E_INVALID_HANDLE");
    case SCARD_E_INVALID_PARAMETER       : //((LONG)0x80100004) 
        return std::string("SCARD_E_INVALID_PARAMETER");
    case SCARD_E_INVALID_TARGET          : //((LONG)0x80100005) 
        return std::string("SCARD_E_INVALID_TARGET");
    case SCARD_E_NO_MEMORY               : //((LONG)0x80100006) 
        return std::string("SCARD_E_NO_MEMORY");
    case SCARD_F_WAITED_TOO_LONG         : //((LONG)0x80100007) 
        return std::string("SCARD_F_WAITED_TOO_LONG");
    case SCARD_E_INSUFFICIENT_BUFFER     : //((LONG)0x80100008) 
        return std::string("SCARD_E_INSUFFICIENT_BUFFER");
    case SCARD_E_UNKNOWN_READER          : //((LONG)0x80100009) 
        return std::string("SCARD_E_UNKNOWN_READER");
    case SCARD_E_TIMEOUT                 : //((LONG)0x8010000A) 
        return std::string("SCARD_E_TIMEOUT");
    case SCARD_E_SHARING_VIOLATION       : //((LONG)0x8010000B) 
        return std::string("SCARD_E_SHARING_VIOLATION");
    case SCARD_E_NO_SMARTCARD            : //((LONG)0x8010000C) 
        return std::string("SCARD_E_NO_SMARTCARD");
    case SCARD_E_UNKNOWN_CARD            : //((LONG)0x8010000D) 
        return std::string("SCARD_E_UNKNOWN_CARD");
    case SCARD_E_CANT_DISPOSE            : //((LONG)0x8010000E) 
        return std::string("SCARD_E_CANT_DISPOSE");
    case SCARD_E_PROTO_MISMATCH          : //((LONG)0x8010000F) 
        return std::string("SCARD_E_PROTO_MISMATCH");
    case SCARD_E_NOT_READY               : //((LONG)0x80100010) 
        return std::string("SCARD_E_NOT_READY");
    case SCARD_E_INVALID_VALUE           : //((LONG)0x80100011) 
        return std::string("SCARD_E_INVALID_VALUE");
    case SCARD_E_SYSTEM_CANCELLED        : //((LONG)0x80100012) 
        return std::string("SCARD_E_SYSTEM_CANCELLED");
    case SCARD_F_COMM_ERROR              : //((LONG)0x80100013) 
        return std::string("SCARD_F_COMM_ERROR");
    case SCARD_F_UNKNOWN_ERROR           : //((LONG)0x80100014) 
        return std::string("SCARD_F_UNKNOWN_ERROR");
    case SCARD_E_INVALID_ATR             : //((LONG)0x80100015) 
        return std::string("SCARD_E_INVALID_ATR");
    case SCARD_E_NOT_TRANSACTED          : //((LONG)0x80100016) 
        return std::string("SCARD_E_NOT_TRANSACTED");
    case SCARD_E_READER_UNAVAILABLE      : //((LONG)0x80100017) 
        return std::string("SCARD_E_READER_UNAVAILABLE");
    case SCARD_P_SHUTDOWN                : //((LONG)0x80100018) 
        return std::string("SCARD_P_SHUTDOWN");
    case SCARD_E_PCI_TOO_SMALL           : //((LONG)0x80100019) 
        return std::string("SCARD_E_PCI_TOO_SMALL");
    case SCARD_E_READER_UNSUPPORTED      : //((LONG)0x8010001A) 
        return std::string("SCARD_E_READER_UNSUPPORTED");
    case SCARD_E_DUPLICATE_READER        : //((LONG)0x8010001B) 
        return std::string("SCARD_E_DUPLICATE_READER");
    case SCARD_E_CARD_UNSUPPORTED        : //((LONG)0x8010001C) 
        return std::string("SCARD_E_CARD_UNSUPPORTED");
    case SCARD_E_NO_SERVICE              : //((LONG)0x8010001D) 
        return std::string("SCARD_E_NO_SERVICE");
    case SCARD_E_SERVICE_STOPPED         : //((LONG)0x8010001E) 
        return std::string("SCARD_E_SERVICE_STOPPED");
    case SCARD_E_UNEXPECTED              : //((LONG)0x8010001F) 
        return std::string("SCARD_E_UNEXPECTED");
        //case SCARD_E_UNSUPPORTED_FEATURE     : //((LONG)0x8010001F) 
        //return std::string("SCARD_E_UNSUPPORTED_FEATURE/SCARD_E_UNEXPECTED");
    case SCARD_E_ICC_INSTALLATION        : //((LONG)0x80100020) 
        return std::string("SCARD_E_ICC_INSTALLATION");
    case SCARD_E_ICC_CREATEORDER         : //((LONG)0x80100021) 
        return std::string("SCARD_E_ICC_CREATEORDER");
        /* case SCARD_E_UNSUPPORTED_FEATURE  : //((LONG)0x80100022) / **< This smart card does not support the requested feature. */
    case SCARD_E_DIR_NOT_FOUND           : //((LONG)0x80100023) 
        return std::string("SCARD_E_DIR_NOT_FOUND");
    case SCARD_E_FILE_NOT_FOUND          : //((LONG)0x80100024) 
        return std::string("SCARD_E_FILE_NOT_FOUND");
    case SCARD_E_NO_DIR                  : //((LONG)0x80100025) 
        return std::string("SCARD_E_NO_DIR");
    case SCARD_E_NO_FILE                 : //((LONG)0x80100026) 
        return std::string("SCARD_E_NO_FILE");
    case SCARD_E_NO_ACCESS               : //((LONG)0x80100027) 
        return std::string("SCARD_E_NO_ACCESS");
    case SCARD_E_WRITE_TOO_MANY          : //((LONG)0x80100028) 
        return std::string("SCARD_E_WRITE_TOO_MANY");
    case SCARD_E_BAD_SEEK                : //((LONG)0x80100029) 
        return std::string("SCARD_E_BAD_SEEK");
    case SCARD_E_INVALID_CHV             : //((LONG)0x8010002A) 
        return std::string("SCARD_E_INVALID_CHV");
    case SCARD_E_UNKNOWN_RES_MNG         : //((LONG)0x8010002B) 
        return std::string("SCARD_E_UNKNOWN_RES_MNG");
    case SCARD_E_NO_SUCH_CERTIFICATE     : //((LONG)0x8010002C) 
        return std::string("SCARD_E_NO_SUCH_CERTIFICATE");
    case SCARD_E_CERTIFICATE_UNAVAILABLE : //((LONG)0x8010002D) 
        return std::string("SCARD_E_CERTIFICATE_UNAVAILABLE");
    case SCARD_E_NO_READERS_AVAILABLE    : //((LONG)0x8010002E) 
        return std::string("SCARD_E_NO_READERS_AVAILABLE");
    case SCARD_E_COMM_DATA_LOST          : //((LONG)0x8010002F) 
        return std::string("SCARD_E_COMM_DATA_LOST");
    case SCARD_E_NO_KEY_CONTAINER        : //((LONG)0x80100030) 
        return std::string("SCARD_E_NO_KEY_CONTAINER");
    case SCARD_E_SERVER_TOO_BUSY         : //((LONG)0x80100031) 
        return std::string("SCARD_E_SERVER_TOO_BUSY");
    case SCARD_W_UNSUPPORTED_CARD        : //((LONG)0x80100065) 
        return std::string("SCARD_W_UNSUPPORTED_CARD");
    case SCARD_W_UNRESPONSIVE_CARD       : //((LONG)0x80100066) 
        return std::string("SCARD_W_UNRESPONSIVE_CARD");
    case SCARD_W_UNPOWERED_CARD          : //((LONG)0x80100067) 
        return std::string("SCARD_W_UNPOWERED_CARD");
    case SCARD_W_RESET_CARD              : //((LONG)0x80100068) 
        return std::string("SCARD_W_RESET_CARD");
    case SCARD_W_REMOVED_CARD            : //((LONG)0x80100069) 
        return std::string("SCARD_W_REMOVED_CARD");
    case SCARD_W_SECURITY_VIOLATION      : //((LONG)0x8010006A) 
        return std::string("SCARD_W_SECURITY_VIOLATION");
    case SCARD_W_WRONG_CHV               : //((LONG)0x8010006B) 
        return std::string("SCARD_W_WRONG_CHV");
    case SCARD_W_CHV_BLOCKED             : //((LONG)0x8010006C) 
        return std::string("SCARD_W_CHV_BLOCKED");
    case SCARD_W_EOF                     : //((LONG)0x8010006D) 
        return std::string("SCARD_W_EOF");
    case SCARD_W_CANCELLED_BY_USER       : //((LONG)0x8010006E) 
        return std::string("SCARD_W_CANCELLED_BY_USER");
    case SCARD_W_CARD_NOT_AUTHENTICATED  : //((LONG)0x8010006F) 
        return std::string("SCARD_W_CARD_NOT_AUTHENTICATED");
    }
    return std::string("SCARD_WAT");
}

//##########################################################################
//##########################################################################
//                           PCSCReaderAPI 
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
    registerMethod("checkSW",      make_method(this, &PCSCReaderAPI::checkSW));    

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
    registerProperty("SW",
                     make_property(this,
                                   &PCSCReaderAPI::get_SW));
    registerProperty("SCARD_SHARE_SHARED",
                     make_property(this,
                                   &PCSCReaderAPI::get_SCARD_SHARE_SHARED));
    registerProperty("SCARD_SHARE_EXCLUSIVE",
                     make_property(this,
                                   &PCSCReaderAPI::get_SCARD_SHARE_EXCLUSIVE));
    registerProperty("SCARD_SHARE_DIRECT ",
                     make_property(this,
                                   &PCSCReaderAPI::get_SCARD_SHARE_DIRECT ));
    registerProperty("SCARD_PROTOCOL_T0",
                     make_property(this,
                                   &PCSCReaderAPI::get_SCARD_PROTOCOL_T0));
    registerProperty("SCARD_PROTOCOL_T1",
                     make_property(this,
                                   &PCSCReaderAPI::get_SCARD_PROTOCOL_T1));
    registerProperty("SCARD_PROTOCOL_RAW",
                     make_property(this,
                                   &PCSCReaderAPI::get_SCARD_PROTOCOL_RAW));

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

int  PCSCReaderAPI::get_SW() {
    if (dwRecvLength <2) {
        return 0;
    }
    int sw = 
        ((pbRecvBuffer[dwRecvLength-2]&0xFF) <<8) |  
        (pbRecvBuffer[dwRecvLength-1]&0xFF);
    return sw;
}

//scardMode const
int  PCSCReaderAPI::get_SCARD_SHARE_SHARED() {
    return SCARD_SHARE_SHARED;
}
int  PCSCReaderAPI::get_SCARD_SHARE_EXCLUSIVE() {
    return SCARD_SHARE_EXCLUSIVE;
}
int  PCSCReaderAPI::get_SCARD_SHARE_DIRECT() {
    return SCARD_SHARE_DIRECT;
}

//preferred protocol const
int  PCSCReaderAPI::get_SCARD_PROTOCOL_T0() {
    return SCARD_PROTOCOL_T0;
}
int  PCSCReaderAPI::get_SCARD_PROTOCOL_T1() {
    return SCARD_PROTOCOL_T1;
}
int  PCSCReaderAPI::get_SCARD_PROTOCOL_RAW() {
    return SCARD_PROTOCOL_RAW;
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
#ifdef WIN32
    WCHAR         mszReaderName[256];
#else
    char          mszReaderName[256];
#endif
    unsigned char atr[32];
    DWORD         readerLen, state, atrLen;
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
    memset(mszReaderName,0, sizeof(mszReaderName));
#ifdef WIN32
    char * cstr = new char [readerName.length()+1];
    std::strcpy (cstr, readerName.c_str());     
    MultiByteToWideChar(CP_UTF8, 0, 
                        cstr, -1,
                        mszReaderName, 256);
    delete[] cstr;
#else
    std::strcpy (mszReaderName, readerName.c_str()); 
#endif

    // connect to selected reader
    scard_error = SCardConnect(hContext, mszReaderName, 
                               scardMode, preferredProtocols, 
                               &hCard, &dwActiveProtocol);
    if (SCARD_S_SUCCESS != scard_error) {
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
    if (SCARD_S_SUCCESS != scard_error) {
        return ATR;
    }   
    ATR = binaryToHex(atr, atrLen);
    return ATR;
}

bool  PCSCReaderAPI::checkSW(int sw, int swmask) {
    int r = get_SW();
    return  (r&swmask) == sw;
}


std::string  PCSCReaderAPI::transmit(std::string apdu) {
    std::string err;

    //check ctx
    scard_error = SCardIsValidContext(hContext);
    if (SCARD_S_SUCCESS != scard_error) {
        return ATR;
    }

    //transmit
    memset(pbSendBuffer, 0, sizeof(pbSendBuffer));
    dwSendLength = 0;
    memset(pbRecvBuffer, 0, sizeof(pbRecvBuffer));
    dwSendLength = hexToBinary(apdu, pbSendBuffer);

    if (dwSendLength<0) {
        scard_error = SCARD_E_INVALID_PARAMETER;
        return err;
    }

    dwRecvLength = sizeof(pbRecvBuffer);
    scard_error = SCardTransmit(hCard, &pioSendPci, 
                                pbSendBuffer, dwSendLength,
                                NULL, pbRecvBuffer, &dwRecvLength);
    if (SCARD_S_SUCCESS != scard_error) {
        return err;
    }
    return binaryToHex(pbRecvBuffer,dwRecvLength);
}


std::string  PCSCReaderAPI::exchangeAPDU(std::string apdu, bool extendedAPDU) {
    int offset;
    std::string err;

    //check ctx
    scard_error = SCardIsValidContext(hContext);
    if (SCARD_S_SUCCESS != scard_error) {
        return err;
    }

    //convert apdu
    memset(pbSendBuffer, 0, sizeof(pbSendBuffer));
    dwSendLength = 0;
    memset(pbRecvBuffer, 0, sizeof(pbRecvBuffer));
    dwSendLength = hexToBinary(apdu, pbSendBuffer);
    if (dwSendLength<0) {
        return err;
    }

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
                dwRecvLength = sizeof(pbRecvBuffer);
                scard_error = SCardTransmit(hCard, &pioSendPci, 
                                            pbSendBuffer, 5+232,
                                            NULL, pbRecvBuffer, &dwRecvLength);
                if (SCARD_S_SUCCESS != scard_error) {
                    return err;
                }
                if(!checkSW(0x9000, 0xFFFF)) {
                    return err;
                }
                offset += 232;
                lc -= 232;
            }
        }
        pbSendBuffer[0] &= ~0x10;
        pbSendBuffer[4] = lc;
        memmove(pbSendBuffer+5, pbSendBuffer+offset, lc);
        dwRecvLength = sizeof(pbRecvBuffer);
        scard_error = SCardTransmit(hCard, &pioSendPci, 
                                    pbSendBuffer, 5+lc,
                                    NULL, pbRecvBuffer, &dwRecvLength);
        if (SCARD_S_SUCCESS != scard_error) {
            return err;
        }

    } 
    //else just send and see....
    else {
        dwRecvLength = sizeof(pbRecvBuffer);
        scard_error = SCardTransmit(hCard, &pioSendPci, 
                                    pbSendBuffer, dwSendLength,
                                    NULL, pbRecvBuffer, &dwRecvLength);
        if (SCARD_S_SUCCESS != scard_error) {
            return err;
        }
    }

    //handle 6Cxx, if autoReissue set
    if ( ((get_SW() &0xFF00) == 0x6C00) && autoReissue) {
        pbSendBuffer[4] = get_SW() &0x00FF;
        dwRecvLength = sizeof(pbRecvBuffer);
        scard_error = SCardTransmit(hCard, &pioSendPci, 
                                    pbSendBuffer, dwSendLength,
                                    NULL, pbRecvBuffer, &dwRecvLength);
        if (SCARD_S_SUCCESS != scard_error) {
            return err;
        }
    }

    //handle 61xx, if autoGetResponse set
    offset = dwRecvLength-2;
    if (((get_SW() &0xFF00) == 0x6100) && autoGetResponse) {
        while ((get_SW() &0xFF00) == 0x6100) {
            //next offset for data to receive, 61xx removed
            pbSendBuffer[0] = 0x00;
            pbSendBuffer[1] = 0xC0;
            pbSendBuffer[2] = 0x00;
            pbSendBuffer[3] = 0x00;
            pbSendBuffer[4] = get_SW()&0xFF;
            dwSendLength = 5;
            
            dwRecvLength = sizeof(pbRecvBuffer)-offset;
            
            scard_error = SCardTransmit(hCard, &pioSendPci, 
                                        pbSendBuffer, dwSendLength,
                                        NULL, pbRecvBuffer+offset, &dwRecvLength);
            
            if (SCARD_S_SUCCESS != scard_error) {
                return err;
            }
            dwRecvLength += offset;
            offset = dwRecvLength-2;
        }   
    }

    return binaryToHex(pbRecvBuffer,dwRecvLength);    
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
