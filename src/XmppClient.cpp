#include <sstream>

#include "Cinder/app/App.h"
#include "Cinder/System.h"

#include "XmppClient.h"

using namespace ci;
using namespace ci::app;
using namespace gloox;

XmppClient::XmppClient()
{
}

XmppClient::~XmppClient()
{
}

void XmppClient::connect(const std::string& xmppUser, const std::string& xmppServer)
{	
	mListener.setup(xmppUser, xmppServer);
	
	mListenerThread = ThreadRef( new std::thread(&XmppListener::openConnection, &mListener, this) );
}

void XmppClient::onConnect()
{	
	mSignalConnect();
}

void XmppClient::onDisconnect( ConnectionError e )
{
	switch(e) {
		case ConnNoError:
			break;
		
		case ConnStreamError:
		case ConnStreamClosed:
		case ConnIoError:
			throw XmppStreamExc();
			break;
			
		case ConnProxyAuthRequired:
		case ConnProxyAuthFailed:
		case ConnProxyNoSupportedAuth:
		case ConnTlsFailed:
		case ConnTlsNotAvailable:
		case ConnNoSupportedAuth:
		case ConnAuthenticationFailed:
			throw XmppAuthExc();
			break;
			
		case ConnParseError:
		case ConnStreamVersionError:
			throw XmppProtocolExc();
			break;
			
		case ConnUserDisconnected:
		case ConnNotConnected:
		case ConnDnsError:
		case ConnConnectionRefused:
			throw XmppFailedConnectionExc();
			break;
			
		case ConnOutOfMemory:
		case ConnCompressionFailed:
		default:
			throw XmppConnectionExc();
	}
	
	mSignalDisconnect();
}

bool XmppClient::onTLSConnect( const CertInfo& info )
{
	mSignalTlsConnect();
}

void XmppClient::handleMessage( const Message& msg, MessageSession* sess )
{
	// process information from message
	std::string name = sess->target().username();

	std::unordered_map<std::string, XmppPeer>::const_iterator peer_itr = mUserMap.find(name);
	XmppPeer peer;
	if (peer_itr != mUserMap.end()) 
		peer = peer_itr->second();
	
	mSignalHandleMsg( peer, msg.subject(), msg.body() );
}

void XmppClient::handleRoster( const Roster& roster )
{
	mUserMap.clear();
	
	Roster::const_iterator it;
	for ( it = roster.begin(); it != roster.end(); ++it ) {
		std::string username = (*it).second->jid();
		size_t index = username.find("@");
		username = username.substr(0,index);
		mUserMap.insert(std::pair<std::string, XmppPeer>(username, XmppPeer(username)));
	}
	
	mSignalRoster();
}

void XmppClient::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& message )
{	
	std::string name = item.jid();
	size_t index = name.find("@");
	name = name.substr(0,index);
	
	// add/update clients
	std::unordered_map<std::string, XmppPeer>::const_iterator peer_itr;
	if (presence == Presence::Available) {
		peer_itr = mUserMap.find(name);
		
		if (peer_itr != mUserMap.end()) {
			peer_itr->second().setStatus(presence);
		}
	}
	else if (presence==Presence::Invalid || presence==Presence::Unavailable) {
		peer_itr = mUserMap.find(name);

		if (peer_itr != mUserMap.end()) {
			peer_itr->second().setStatus(presence);
			peer_itr->second().setConnected(false);
		}
	}	
	
	mSignalRosterPresence( peer_itr->second(), resource, presence, message );
}

bool XmppClient::sendMessage(const std::string& recipient, const std::string& message, const std::string& subject)
{
	std::string r(recipient);
	r += "@";
	r += mListener.server();
	JID jid(r);
	
	return mListener.sendMessage(jid, message, subject);
}
