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

bool XmppClient::connect(const std::string& xmppUser, const std::string& xmppServer)
{
	if (xmppUser.empty() || xmppServer.empty()) return false;
	
	mListener.setup(xmppUser, xmppServer);
	
	mListenerThread = ThreadRef( new std::thread(&XmppListener::openConnection, &mListener, this) );
	
	return true;
}

void XmppClient::onConnect()
{	
	mSignalConnect();
}

void XmppClient::onDisconnect( ConnectionError e )
{
	switch (e) {
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
			throw XmppConnectionExc();
			break;
			
		case ConnOutOfMemory:
		case ConnCompressionFailed:
		default:
			throw XmppExc();
	}
	
	mSignalDisconnect();
}

void XmppClient::onTLSConnect( const CertInfo& info )
{
	mSignalTlsConnect();
}

void XmppClient::handleMessage( const Message& msg, MessageSession* sess )
{
	// process information from message
	std::string name = sess->target().username();

	std::unordered_map<std::string, XmppPeer>::const_iterator peer_itr = mRoster.find(name);
	XmppPeer peer;
	if (peer_itr != mRoster.end()) 
		peer = peer_itr->second;
	
	mSignalHandleMsg( peer, msg.subject(), msg.body() );
}

void XmppClient::handleRoster( const Roster& roster )
{
	mRoster.clear();
	
	std::list<std::string> userlist;
	Roster::const_iterator itr;
	for ( itr = roster.begin(); itr != roster.end(); ++itr ) {
		const std::string& username = (*itr).second->jidJID().username();
		mRoster.insert(std::pair<std::string, XmppPeer>(username, XmppPeer(username)));
		userlist.push_back(username);
	}
	
	mSignalRoster(userlist);
}

void XmppClient::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& message )
{	
	const std::string& name = item.jidJID().username();

	std::unordered_map<std::string, XmppPeer>::iterator peer_itr = mRoster.find(name);
	if (peer_itr != mRoster.end()) {
		peer_itr->second.setStatus(XmppPeer::presenceToStatus(presence));
	}
	
	mSignalRosterPresence( peer_itr->second, resource, XmppPeer::presenceToStatus(presence), message );
}

bool XmppClient::sendMessage(const std::string& recipient, const std::string& message, const std::string& subject)
{
	std::string r(recipient);
	r += "@";
	r += mListener.server();
	JID jid(r);
	
	return mListener.sendMessage(jid, message, subject);
}

bool operator==( const XmppPeer& lhs, const XmppPeer& rhs)
{
	return lhs.username() == rhs.username();
}

bool operator!=( const XmppPeer& lhs, const XmppPeer& rhs)
{
	return lhs.username() != rhs.username();
}
