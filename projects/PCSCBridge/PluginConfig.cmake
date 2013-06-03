#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for PCSC Bridge
#
#\**********************************************************/

set(PLUGIN_NAME "PCSCBridge")
set(PLUGIN_PREFIX "PBR")
set(COMPANY_NAME "UbinitySAS")

# ActiveX constants:
set(FBTYPELIB_NAME PCSCBridgeLib)
set(FBTYPELIB_DESC "PCSCBridge 1.0 Type Library")
set(IFBControl_DESC "PCSCBridge Control Interface")
set(FBControl_DESC "PCSCBridge Control Class")
set(IFBComJavascriptObject_DESC "PCSCBridge IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "PCSCBridge ComJavascriptObject Class")
set(IFBComEventSource_DESC "PCSCBridge IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID c5d9a369-14b6-551e-8ab7-6ad37c6f6b80)
set(IFBControl_GUID 72c96b47-a0f8-5f4b-999c-2344d596291e)
set(FBControl_GUID 9d11c875-3716-58b4-b74f-990c54edfe21)
set(IFBComJavascriptObject_GUID 20650ca0-9fd3-547e-a281-cccfc3b4a69b)
set(FBComJavascriptObject_GUID 28dbfd79-6113-5a5d-bf67-eefb7b9b33dd)
set(IFBComEventSource_GUID cfc5da66-30cc-519e-8f10-3930ed87dca5)
if ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID a4d744f3-7b0d-53ed-a421-11e5e496c9c6)
else ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID 15081232-401d-5710-a188-4af8486950fa)
endif ( FB_PLATFORM_ARCH_32 )

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "UbinitySAS.PCSCBridge")
set(MOZILLA_PLUGINID "ubinity.com/PCSCBridge")

# strings
set(FBSTRING_CompanyName "Ubinity SAS")
set(FBSTRING_PluginDescription "PCSC Winscard API Bridge")
set(FBSTRING_PLUGIN_VERSION "0.2.0.0")
set(FBSTRING_LegalCopyright "Copyright 2013 Ubinity SAS")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "PCSC Bridge")
set(FBSTRING_FileExtents "")
if ( FB_PLATFORM_ARCH_32 )
    set(FBSTRING_PluginName "PCSC Bridge")  # No 32bit postfix to maintain backward compatability.
else ( FB_PLATFORM_ARCH_32 )
    set(FBSTRING_PluginName "PCSC Bridge_${FB_PLATFORM_ARCH_NAME}")
endif ( FB_PLATFORM_ARCH_32 )
set(FBSTRING_MIMEType "application/x-pcscbridge")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 0)
set(FBMAC_USE_COCOA 0)
set(FBMAC_USE_COREGRAPHICS 0)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)
