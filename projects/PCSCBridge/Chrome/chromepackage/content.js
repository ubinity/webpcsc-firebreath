
// Dumb bridge between the web page and the extension
var port = chrome.runtime.connect();

//extention --> user page

function ext2page(msg) {
        //console.log("SCard -> User: ");
        //console.log(msg);
        window.postMessage(msg, "*");
};
port.onMessage.addListener(ext2page, false);

//user page --> extention

function page2ext(event) {
    // We only accept messages from ourselves
        if (event.source != window)
                return;
    
        if (event.data.destination && (event.data.destination == "SCARD")) {
                //console.log("User -> SCard: ");
                //console.log(event.data);
                port.postMessage(event.data);
        }
};

window.addEventListener("message", page2ext, false);

