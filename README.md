Cinder-XMPP
===========

CinderBlock that wraps the Gloox v1.0.3 for use with Cinder v0.8.5.

Overview
-----------
Wrapper classes include XmppClient and XmppListener. The XmppListener is a "thin" wrapper of gloox constructs. To gain roster and status information, one would have to interface with gloox datatypes.

The XmppClient class provides a more complete wrapper of the library including spawning a thread that the listener runs on to receive messages. It provides access to gloox constructs using C++11 STL datatypes only.

Usage
-----------
```cpp
void connected()
{
  std::cout << "connected" << std::endl;
}

void roster(const list<string>& roster)
{
	std::cout() << "roster size:" << roster.size() << std::endl;
	for (auto iter = roster.begin(); iter != roster.end(); iter++) {
		std::cout() << " user: " << *iter << std::endl;
	}
}

client = new xmpp::Client();
client->getSignalConnect().connect(std::bind(&connected));
client->getSignalRoster().connect(std::bind(&roster, std::_1));
client->connect("username", "password", "server");
```

Status
-----------
Works on Mac platform
Works on PC platform
 - static library is not included (too large)
 - compiling static library using VC10/VC11 is supported by Gloox library

Notes
-----------
Tested using ejabberd server implementation
CinderBlock defintion does not include sample
