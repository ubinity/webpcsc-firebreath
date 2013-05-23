/**********************************************************\

  Auto-generated PCSCBridgeAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include <winscard.h>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "PCSCBridge.h"

#ifndef H_PCSCBridgeAPI
#define H_PCSCBridgeAPI

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
    bool checkSW(int sw, int swmask);

    //properties R-only
    std::string get_atr();
    int get_protocol();
    int get_error();
    int get_SW();
	int get_SCARD_SHARE_SHARED();
    int get_SCARD_SHARE_EXCLUSIVE();
    int get_SCARD_SHARE_DIRECT();
    int get_SCARD_PROTOCOL_T0();
    int get_SCARD_PROTOCOL_T1();
    int get_SCARD_PROTOCOL_RAW();

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

    DWORD             dwRecvLength;  
    BYTE              pbRecvBuffer[0x10000];    
    DWORD             dwSendLength;  
    BYTE              pbSendBuffer[0x10000];

  
    
};

typedef boost::shared_ptr<PCSCReaderAPI> PCSCReaderAPIPtr;

class PCSCBridgeAPI : public FB::JSAPIAuto
{
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
    PCSCBridgeAPI(const PCSCBridgePtr& plugin, const FB::BrowserHostPtr& host) :
        m_plugin(plugin), m_host(host)
    {
        registerMethod("init",         make_method(this, &PCSCBridgeAPI::init));
        registerMethod("listReaders",  make_method(this, &PCSCBridgeAPI::listReaders));
        registerMethod("selectReader", make_method(this, &PCSCBridgeAPI::selectReader));
        registerMethod("strError", make_method(this, &PCSCBridgeAPI::strError));
        // Read-only property
        registerProperty("version",    make_property(this, &PCSCBridgeAPI::get_version));
   }

    ///////////////////////////////////////////////////////////////////////////////
    /// @fn PCSCBridgeAPI::~PCSCBridgeAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~PCSCBridgeAPI() {};

    PCSCBridgePtr getPlugin();

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    // Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));

    // Method test-event
    void testEvent();


    // HERE START PCSC BRIDGE
    bool init();    
    std::vector<std::string>  listReaders();
    PCSCReaderAPIPtr selectReader(std::string rname);
    const std::string strError(LONG code);
private:
    PCSCBridgeWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    // HERE START PCSC BRIDGE
    std::string       readerName;
    SCARDCONTEXT      hContext;
    LONG              scard_error;
};

#endif // H_PCSCBridgeAPI

