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

  Auto-generated PCSCBridgeAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include <winscard.h>
#ifdef linux
#include <reader.h>
#endif
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "PCSCBridge.h"

#ifndef H_PCSCBridgeAPI
#define H_PCSCBridgeAPI


//##########################################################################
//##########################################################################
//                           SCardAPI 
//##########################################################################
//##########################################################################
class SCardAPI : public FB::JSAPIAuto {
 public:
    SCardAPI();
    ~SCardAPI();

    LONG  EstablishContext(FB::JSObjectPtr args);
    LONG  ReleaseContext(FB::JSObjectPtr args);
    LONG  Connect(FB::JSObjectPtr args);
    LONG  Disconnect(FB::JSObjectPtr args);
    LONG  Status(FB::JSObjectPtr args);
    LONG  Transmit(FB::JSObjectPtr args);
    LONG  ListReaders(FB::JSObjectPtr args);  
    LONG  IsValidContext(FB::JSObjectPtr args);
    LONG  BeginTransaction (FB::JSObjectPtr args);
    LONG  EndTransaction(FB::JSObjectPtr args);
    LONG  Cancel(FB::JSObjectPtr args);
    LONG  GetStatusChange (FB::JSObjectPtr args);
    LONG  Control (FB::JSObjectPtr args);
    LONG  GetAttrib (FB::JSObjectPtr args);
    LONG  SetAttrib(FB::JSObjectPtr args);
};
typedef boost::shared_ptr<SCardAPI> SCardAPIPtr;

//##########################################################################
//##########################################################################
//                           ReaderAPI 
//##########################################################################
//##########################################################################

class PCSCReaderAPI : public FB::JSAPIAuto
{
 public:    
    PCSCReaderAPI();
    PCSCReaderAPI(const std::string rname, SCARDCONTEXT c);
    ~PCSCReaderAPI();

    //method
    std::string  powerUp();
    std::string  exchangeAPDU(std::string apdu, bool extendedAPDU);
    std::string  transmit(std::string apdu);
    void  powerDown();

    //properties R-only
    std::string get_atr();
    int get_protocol();
    int get_error();


    //properties RW
    bool get_autoChaining();
    void set_autoChaining(bool x);
    bool get_autoReissue();
    void set_autoReissue(bool x);
    bool get_autoGetResponse();
    void set_autoGetResponse(bool x);
    int  get_scardMode();
    void set_scardMode(int x);  
    int  get_preferredProtocols();
    void set_preferredProtocols(int x);  
    bool get_extendedLengthSupported();
    void set_extendedLengthSupported(bool x);
 
private:    
    bool checkSW(LPBYTE pbRecvBuffer, DWORD dwRecvLength, int sw, int swmask);
    int get_SW(LPBYTE pbRecvBuffer, DWORD dwRecvLength);

    //R-only properties
    std::string       ATR;
    DWORD             protocol;
    LONG              scard_error;
    std::string       readerName;

    //RW properties
    bool autoChaining;
    bool autoReissue;
    bool autoGetResponse;
    int  scardMode;
    int  preferredProtocols;
    bool extendedLengthSupported;

    //internals
    SCARDCONTEXT      hContext;
    SCARDHANDLE       hCard;
    SCARD_IO_REQUEST  pioSendPci;        
};
typedef boost::shared_ptr<PCSCReaderAPI> PCSCReaderAPIPtr;


//##########################################################################
//##########################################################################
//                           TerminalAPI 
//##########################################################################
//##########################################################################
class PCSCTerminalAPI : public FB::JSAPIAuto
{
 private:
    LONG              scard_error;
    SCARDCONTEXT      hContext;

 public:    
    PCSCTerminalAPI();    
    ~PCSCTerminalAPI();

    std::vector<std::string>  listReaders();
    PCSCReaderAPIPtr selectReader(std::string rname);
    LONG reinit();
};
typedef boost::shared_ptr<PCSCTerminalAPI> PCSCTerminalAPIPtr;

//##########################################################################
//##########################################################################
//                            PCSCBridgeAPI
//##########################################################################
//##########################################################################

class PCSCBridgeAPI : public FB::JSAPIAuto
{
 private:
    PCSCBridgeWeakPtr   m_plugin;
    FB::BrowserHostPtr  m_host;
    SCardAPIPtr         scard_api;
    PCSCTerminalAPIPtr  terminal_api;

 public:
    ////////////////////////////////////////////////////////////////////////////
    /// @fn PCSCBridgeAPI::PCSCBridgeAPI(const PCSCBridgePtr& plugin, const FB::BrowserHostPtr host)
    ///
    /// @brief  Constructor for your JSAPI object.
    ///         You should register your methods, properties, and events
    ///         that should be accessible to Javascript from here.
    ///
    /// @see FB::JSAPIAuto::registerMethod
    /// @see FB::JSAPIAuto::registerProperty
    /// @see FB::JSAPIAuto::registerEvent
    ////////////////////////////////////////////////////////////////////////////
    PCSCBridgeAPI(const PCSCBridgePtr& plugin, const FB::BrowserHostPtr& host) ;
   

    ///////////////////////////////////////////////////////////////////////////////
    /// @fn PCSCBridgeAPI::~PCSCBridgeAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~PCSCBridgeAPI() {};


    // --- method ---
    PCSCBridgePtr      getPlugin();
    SCardAPIPtr        getSCardAPI();
    PCSCTerminalAPIPtr getTerminalAPI();
    const std::string  strError(LONG code);

    // --- properties R-only ---

    //version
    std::string get_version();

    //scard mode
    int get_SCARD_SHARE_SHARED();
    int get_SCARD_SHARE_EXCLUSIVE();
    int get_SCARD_SHARE_DIRECT();
    int get_SCARD_PROTOCOL_T0();
    int get_SCARD_PROTOCOL_T1();
    int get_SCARD_PROTOCOL_RAW();
#ifdef linux
    int get_SCARD_PROTOCOL_T15();
#endif
    //scard disposition
    int get_SCARD_LEAVE_CARD();
    int get_SCARD_RESET_CARD();
    int get_SCARD_UNPOWER_CARD();
    int get_SCARD_EJECT_CARD();
    
    //scard status
    int get_SCARD_UNKNOWN();
    int get_SCARD_ABSENT();
    int get_SCARD_PRESENT();

    //scard ???
    int get_SCARD_SWALLOWED() ;
    int get_SCARD_POWERED();
    int get_SCARD_NEGOTIABLE() ;
    int get_SCARD_SPECIFIC() ;
    
    //scard state
    int get_SCARD_STATE_UNAWARE();
    int get_SCARD_STATE_IGNORE() ;
    int get_SCARD_STATE_CHANGED() ;
    int get_SCARD_STATE_UNKNOWN();
    int get_SCARD_STATE_UNAVAILABLE();
    int get_SCARD_STATE_EMPTY() ;
    int get_SCARD_STATE_PRESENT() ;
    int get_SCARD_STATE_ATRMATCH() ;
    int get_SCARD_STATE_EXCLUSIVE();
    int get_SCARD_STATE_INUSE() ;
    int get_SCARD_STATE_MUTE() ;
    int get_SCARD_STATE_UNPOWERED() ;

    //attribute
    int get_SCARD_ATTR_VENDOR_NAME();
    int get_SCARD_ATTR_VENDOR_IFD_TYPE();
    int get_SCARD_ATTR_VENDOR_IFD_VERSION();
    int get_SCARD_ATTR_VENDOR_IFD_SERIAL_NO();
    int get_SCARD_ATTR_CHANNEL_ID();
#ifdef linux
    int get_SCARD_ATTR_ASYNC_PROTOCOL_TYPES();
#endif
    int get_SCARD_ATTR_DEFAULT_CLK();
    int get_SCARD_ATTR_MAX_CLK();
    int get_SCARD_ATTR_DEFAULT_DATA_RATE();
    int get_SCARD_ATTR_MAX_DATA_RATE();
    int get_SCARD_ATTR_MAX_IFSD();
#ifdef linux
    int get_SCARD_ATTR_SYNC_PROTOCOL_TYPES();
#endif
    int get_SCARD_ATTR_POWER_MGMT_SUPPORT();
    int get_SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE();
    int get_SCARD_ATTR_USER_AUTH_INPUT_DEVICE();
    int get_SCARD_ATTR_CHARACTERISTICS();
    int get_SCARD_ATTR_CURRENT_PROTOCOL_TYPE();
    int get_SCARD_ATTR_CURRENT_CLK();
    int get_SCARD_ATTR_CURRENT_F();
    int get_SCARD_ATTR_CURRENT_D();
    int get_SCARD_ATTR_CURRENT_N();
    int get_SCARD_ATTR_CURRENT_W();
    int get_SCARD_ATTR_CURRENT_IFSC();
    int get_SCARD_ATTR_CURRENT_IFSD();
    int get_SCARD_ATTR_CURRENT_BWT();
    int get_SCARD_ATTR_CURRENT_CWT();
    int get_SCARD_ATTR_CURRENT_EBC_ENCODING();
    int get_SCARD_ATTR_EXTENDED_BWT();
    int get_SCARD_ATTR_ICC_PRESENCE();
    int get_SCARD_ATTR_ICC_INTERFACE_STATUS();
    int get_SCARD_ATTR_CURRENT_IO_STATE();
    int get_SCARD_ATTR_ATR_STRING();
    int get_SCARD_ATTR_ICC_TYPE_PER_ATR();
    int get_SCARD_ATTR_ESC_RESET();
    int get_SCARD_ATTR_ESC_CANCEL();
    int get_SCARD_ATTR_ESC_AUTHREQUEST();
    int get_SCARD_ATTR_MAXINPUT();
    int get_SCARD_ATTR_DEVICE_UNIT();
    int get_SCARD_ATTR_DEVICE_IN_USE();
    int get_SCARD_ATTR_DEVICE_FRIENDLY_NAME();
    int get_SCARD_ATTR_DEVICE_SYSTEM_NAME();
    int get_SCARD_ATTR_SUPRESS_T1_IFS_REQUEST();

    //error 
    LONG get_SCARD_S_SUCCESS();
    LONG get_SCARD_F_INTERNAL_ERROR();
    LONG get_SCARD_E_CANCELLED();
    LONG get_SCARD_E_INVALID_HANDLE();
    LONG get_SCARD_E_INVALID_PARAMETER();
    LONG get_SCARD_E_INVALID_TARGET();
    LONG get_SCARD_E_NO_MEMORY();
    LONG get_SCARD_F_WAITED_TOO_LONG();
    LONG get_SCARD_E_INSUFFICIENT_BUFFER();
    LONG get_SCARD_E_UNKNOWN_READER();
    LONG get_SCARD_E_TIMEOUT();
    LONG get_SCARD_E_SHARING_VIOLATION();
    LONG get_SCARD_E_NO_SMARTCARD();
    LONG get_SCARD_E_UNKNOWN_CARD();
    LONG get_SCARD_E_CANT_DISPOSE();
    LONG get_SCARD_E_PROTO_MISMATCH();
    LONG get_SCARD_E_NOT_READY();
    LONG get_SCARD_E_INVALID_VALUE();
    LONG get_SCARD_E_SYSTEM_CANCELLED();
    LONG get_SCARD_F_COMM_ERROR();
    LONG get_SCARD_F_UNKNOWN_ERROR();
    LONG get_SCARD_E_INVALID_ATR();
    LONG get_SCARD_E_NOT_TRANSACTED();
    LONG get_SCARD_E_READER_UNAVAILABLE();
    LONG get_SCARD_P_SHUTDOWN();
    LONG get_SCARD_E_PCI_TOO_SMALL();
    LONG get_SCARD_E_READER_UNSUPPORTED();
    LONG get_SCARD_E_DUPLICATE_READER();
    LONG get_SCARD_E_CARD_UNSUPPORTED();
    LONG get_SCARD_E_NO_SERVICE();
    LONG get_SCARD_E_SERVICE_STOPPED();
    LONG get_SCARD_E_UNEXPECTED();
    LONG get_SCARD_E_UNSUPPORTED_FEATURE();
    LONG get_SCARD_E_ICC_INSTALLATION();
    LONG get_SCARD_E_ICC_CREATEORDER();
    LONG get_SCARD_E_DIR_NOT_FOUND();
    LONG get_SCARD_E_FILE_NOT_FOUND();
    LONG get_SCARD_E_NO_DIR();
    LONG get_SCARD_E_NO_FILE();
    LONG get_SCARD_E_NO_ACCESS();
    LONG get_SCARD_E_WRITE_TOO_MANY();
    LONG get_SCARD_E_BAD_SEEK();
    LONG get_SCARD_E_INVALID_CHV();
    LONG get_SCARD_E_UNKNOWN_RES_MNG();
    LONG get_SCARD_E_NO_SUCH_CERTIFICATE();
    LONG get_SCARD_E_CERTIFICATE_UNAVAILABLE();
    LONG get_SCARD_E_NO_READERS_AVAILABLE();
    LONG get_SCARD_E_COMM_DATA_LOST();
    LONG get_SCARD_E_NO_KEY_CONTAINER();
    LONG get_SCARD_E_SERVER_TOO_BUSY();
    LONG get_SCARD_W_UNSUPPORTED_CARD();
    LONG get_SCARD_W_UNRESPONSIVE_CARD();
    LONG get_SCARD_W_UNPOWERED_CARD();
    LONG get_SCARD_W_RESET_CARD();
    LONG get_SCARD_W_REMOVED_CARD();
    LONG get_SCARD_W_SECURITY_VIOLATION();
    LONG get_SCARD_W_WRONG_CHV();
    LONG get_SCARD_W_CHV_BLOCKED();
    LONG get_SCARD_W_EOF();
    LONG get_SCARD_W_CANCELLED_BY_USER();
    LONG get_SCARD_W_CARD_NOT_AUTHENTICATED();

    ////////////
    /// TEST ///
    ////////////
    void set_testString(const std::string& val);
    // Method echo
    FB::variant echo(const FB::variant& msg);
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));

    // Method test-event
    void testEvent();

    // HERE START PCSC BRIDGE
    //    std::string       readerName;
};

#endif // H_PCSCBridgeAPI

