#!/bin/sh 

VER=0.2.99

if [ -n "$SIGPWD" ];
then
    signpass=$SIGPWD
else
    echo 'Enter the signing key password (release@ubinity.com)':
    stty -echo
    read signpass
    stty echo
fi;


##################################
###             JS             ###
##################################
echo '-------------------------------'
echo '>>> Making js <<<'
rm -rf js
mkdir js
cp -r ../projects/PCSCBridge/js/* js/
rm -f js/*~

##################################
###            DOC             ###
##################################
echo '-------------------------------'
echo '>>> Making doc <<<'

rm -rf doc
mkdir doc doc/old
#a2x -f xhtml -D doc/old  ../projects/PCSCBridge/doc/PCSCBridge.asc
/opt/jsdoc/jsdoc -c ../projects/PCSCBridge/doc/doc-conf.json

##################################
###          FIREFOX           ###
##################################
echo '-------------------------------'
echo '>>> Making xpi for firefox <<<<'
echo 


rm -fr firefox-unpaked firefox PCSCBridgeExtension

mkdir firefox-unpaked firefox
cat > firefox-unpaked/README.TXT <<EOF

NO YET SUPPORTED

EOF
cp firefox-unpaked/README.TXT firefox

# mkdir -p firefox
# rm -rf firefox/*

# rm -rf PCSCBridgeExtension
# mkdir -p PCSCBridgeExtension
# mkdir -p PCSCBridgeExtension/plugins

# ### set the rdf
# echo "Generate RDF..."

# cat > PCSCBridgeExtension/install.rdf <<EOF
# <RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:em="http://www.mozilla.org/2004/em-rdf#">
#   <Description about="urn:mozilla:install-manifest">
#     <em:id>PCSCBridge@ubinity.com</em:id>
#     <em:version>${VER}</em:version>
#     <em:type>2</em:type>

#     <em:targetApplication>
#       <Description>
#         <em:id>{ec8030f7-c20a-464f-9b0e-13a3a9e97384}</em:id>
#         <em:minVersion>1.5</em:minVersion>
#         <em:maxVersion>*</em:maxVersion>
#       </Description>
#     </em:targetApplication>
#     <em:unpack>true</em:unpack>

#     <em:name>PCSC bridge</em:name>
#     <em:description>PCSC API bridge based on firebreath framework</em:description>
#     <em:homepageURL>https://github.com/ubinity/webpcsc-firebreath</em:homepageURL>
#     <em:creator>Cédric Mesnil</em:creator>

#   </Description>
# </RDF>
# EOF

# ### set libs
# echo "Retrieve resources..."
# cp ./libs/* PCSCBridgeExtension/plugins/

# ### xpi
# echo "Pack xpi"
# (cd  PCSCBridgeExtension && zip -r ../firefox/PCSCBridgeExtension.xpi *)
# if [ x"$signpass" != x ]; 
# then
#   gpg -s --local-user releases@ubinity.com --passphrase "$signpass"  --output firefox/PCSCBridgeExtension.xpi.sig firefox/PCSCBridgeExtension.xpi
# fi
# echo 

##################################
###           CHROME           ###
##################################
#http://developer.chrome.com/extensions/crx.html

echo '--------------------------------'
echo '>>>> Making crx for chrome <<<<'
echo 

rm -rf chrome chrome-unpaked PCSCBridgeExtension
mkdir PCSCBridgeExtension

### set the extention files
echo "Generate Manifest..."
cp ../projects/PCSCBridge/Chrome/chromepackage/* PCSCBridgeExtension
cp libs/* PCSCBridgeExtension
rm -f PCSCBridgeExtension/*~
sed -i -e  "s/@VER@/${VER}/" PCSCBridgeExtension/manifest.json 

### crx 
echo "Pack crx"
# google-chrome  --pack-extension=PCSCBridgeExtension  --pack-extension-key=pem/PCSCBridgeExtension.pem
mkdir chrome
./crxmake.sh PCSCBridgeExtension pem/PCSCBridgeExtension.pem
mv PCSCBridgeExtension.crx chrome

if [ x"$signpass" != x ]; 
then
    gpg -s --local-user releases@ubinity.com --passphrase "$signpass"  --output chrome/PCSCBridgeExtension.crx.sig chrome/PCSCBridgeExtension.crx
fi
mv PCSCBridgeExtension chrome-unpaked

##################################
###            clean           ###
##################################
