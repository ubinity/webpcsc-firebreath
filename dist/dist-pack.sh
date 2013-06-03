#!/bin/sh

VER=0.2.0


echo 'Enter the signing key password (release@ubinity.com)':
stty -echo
read signpass
stty echo


##################################
###          DOC           ###
##################################
echo '-------------------------------'
echo '>>> Making doc <<<'

rm -rf doc
mkdir doc
a2x -f xhtml -D doc  ../projects/PCSCBridge/doc/PCSCBridge.asc


##################################
###          FIREFOX           ###
##################################

echo '-------------------------------'
echo '>>> Making xpi for firefox <<<<'
echo 

mkdir -p firefox
rm -rf firefox/*

rm -rf PCSCBridgeExtension
mkdir -p PCSCBridgeExtension
mkdir -p PCSCBridgeExtension/plugins

### set the rdf
echo "Generate RDF..."

cat > PCSCBridgeExtension/install.rdf <<EOF
<RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:em="http://www.mozilla.org/2004/em-rdf#">
  <Description about="urn:mozilla:install-manifest">
    <em:id>PCSCBridge@ubinity.com</em:id>
    <em:version>${VER}</em:version>
    <em:type>2</em:type>

    <em:targetApplication>
      <Description>
        <em:id>{ec8030f7-c20a-464f-9b0e-13a3a9e97384}</em:id>
        <em:minVersion>1.5</em:minVersion>
        <em:maxVersion>*</em:maxVersion>
      </Description>
    </em:targetApplication>
    <em:unpack>true</em:unpack>

    <em:name>PCSC bridge</em:name>
    <em:description>PCSC API bridge based on firebreath framework</em:description>
    <em:homepageURL>https://github.com/ubinity/webpcsc-firebreath</em:homepageURL>
    <em:creator>Cédric Mesnil</em:creator>

  </Description>
</RDF>
EOF

### set libs
echo "Retrieve resources..."
cp ./libs/* PCSCBridgeExtension/plugins/

### xpi
echo "Pack xpi"
(cd  PCSCBridgeExtension && zip -r ../firefox/PCSCBridgeExtension.xpi *)

gpg -s --local-user releases@ubinity.com --passphrase "$signpass"  --output firefox/PCSCBridgeExtension.xpi.sig firefox/PCSCBridgeExtension.xpi

echo 

##################################
###           CHROME           ###
##################################
#http://developer.chrome.com/extensions/crx.html

echo '--------------------------------'
echo '>>>> Making crx for chrome <<<<'
echo 

mkdir -p chrome
rm -rf chrome/*

rm -rf PCSCBridgeExtension
mkdir -p PCSCBridgeExtension

### set the manifest
echo "Generate Manifest..."

cat > PCSCBridgeExtension/manifest.json <<EOF
{
  "name": "PCSCBridge",
  "version": "${VER}",
  "manifest_version": 2,
  "description": "PCSC API bridge based on firebreath framework",
  "homepage_url": "https://github.com/ubinity/webpcsc-firebreath",
  "update_url": "https://github.com/ubinity/webpcsc-firebreath/dist/chrome/PCSCBridge-chrome-updates.xml",
    "plugins": [
EOF

libs=`ls libs`; 
while [ -n "$libs" ]; 
do
    l=`echo $libs|cut -f1 -d' '`
    libs=`echo $libs|cut -s -f2- -d' '`
    echo  -n "      { \"path\": \"$l\", \"public\": true }" >>PCSCBridgeExtension/manifest.json;
    if [ -n "$libs" ];
    then
        echo   ","  >>PCSCBridgeExtension/manifest.json;
    fi;
        
done;

cat >> PCSCBridgeExtension/manifest.json <<EOF

    ]
}
EOF

### set libs
echo "Retrieve resources..."
cp ./libs/* PCSCBridgeExtension/

### crx 
echo "Pack crx"
# google-chrome  --pack-extension=PCSCBridgeExtension  --pack-extension-key=pem/PCSCBridgeExtension.pem
./crxmake.sh PCSCBridgeExtension pem/PCSCBridgeExtension.pem
mv PCSCBridgeExtension.crx chrome

gpg -s --local-user releases@ubinity.com --passphrase "$signpass"  --output chrome/PCSCBridgeExtension.crx.sig chrome/PCSCBridgeExtension.crx

##################################
###            clean           ###
##################################
rm -rf PCSCBridgeExtension
