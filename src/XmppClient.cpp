#include <sstream>

#include "Cinder/app/App.h"
#include "Cinder/System.h"

#include "XmppClient.h"

using namespace ci;
using namespace ci::app;
using namespace gloox;

namespace xmpp {

status_t presenceToStatus(const gloox::Presence::PresenceType value)
{
	if ((int) value <= 6) {
		return xmpp::status_t((int)value);
	}
	return xmpp::Invalid;
}

std::string statusString(const status_t value)
{
	switch (value) {
		case Available: return "available";
			break;
		case Chat: return "chat";
			break;
		case Away: return "away";
			break;
		case DND: return "do not disturb";
			break;
		case XA: return "extended away";
			break;
		case Unavailable: return "unavailable";
			break;
			
		case Invalid: return "invalid";
		default:
			break;
	}
}

Client::Client()
{
	mListener = xmpp::Listener(this);
}

Client::~Client()
{
}

bool Client::connect(const std::string& xmppUser, const std::string& xmppServer)
{
	if (xmppUser.empty() || xmppServer.empty()) return false;
	
	mListener.setup(xmppUser, xmppServer);
	
	mListenerThread = ThreadRef( new std::thread(&xmpp::Listener::openConnection, &mListener, this) );
	
	return true;
}

void Client::onConnect()
{	
	mSignalConnect();
}

void Client::onDisconnect( gloox::ConnectionError e )
{
	error_t err;
	switch (e) {
		case ConnNoError:
			err = NoError;
			break;
		
		case ConnStreamError:
		case ConnStreamClosed:
		case ConnIoError:
			err = StreamError;
			break;
			
		case ConnProxyAuthRequired:
		case ConnProxyAuthFailed:
		case ConnProxyNoSupportedAuth:
		case ConnTlsFailed:
		case ConnTlsNotAvailable:
		case ConnNoSupportedAuth:
		case ConnAuthenticationFailed:
			err = AuthenticationError;
			break;
			
		case ConnParseError:
		case ConnStreamVersionError:
			err = ProtocolError;
			break;
			
		case ConnUserDisconnected:
		case ConnNotConnected:
		case ConnDnsError:
		case ConnConnectionRefused:
			err = ConnectionError;
			break;
			
		case ConnOutOfMemory:
		case ConnCompressionFailed:
		default:
			err = GeneralError;
	}
	
	mSignalDisconnect(err);
}

void Client::onTLSConnect( const CertInfo& info )
{
	mSignalTlsConnect();
}

void Client::handleMessage( const Message& msg, MessageSession* sess )
{
	// process information from message
	std::string name = sess->target().username();

	std::unordered_map<std::string, xmpp::Peer>::const_iterator peer_itr = mRoster.find(name);
	xmpp::Peer peer;
	if (peer_itr != mRoster.end()) 
		peer = peer_itr->second;
	
	mSignalHandleMsg( peer, msg.subject(), msg.body() );
}

void Client::handleRoster( const Roster& roster )
{
	mRoster.clear();
	
	std::list<std::string> userlist;
	Roster::const_iterator itr;
	for ( itr = roster.begin(); itr != roster.end(); ++itr ) {
		const std::string& username = (*itr).second->jidJID().username();
		mRoster.insert(std::pair<std::string, xmpp::Peer>(username, xmpp::Peer(username)));
		userlist.push_back(username);
	}
	
	mSignalRoster(userlist);
}

void Client::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& message )
{	
	const std::string& name = item.jidJID().username();

	std::unordered_map<std::string, xmpp::Peer>::iterator peer_itr = mRoster.find(name);
	if (peer_itr != mRoster.end()) {
		peer_itr->second.setStatus(xmpp::presenceToStatus(presence));
	}
	
	mSignalRosterPresence( peer_itr->second, resource, xmpp::presenceToStatus(presence), message );
}

bool Client::sendMessage(const std::string& recipient, const std::string& message, const std::string& subject)
{
	std::string r(recipient);
	r += "@";
	r += mListener.server();
	JID jid(r);
	
	return mListener.sendMessage(jid, message, subject);
}

xmpp::status_t Client::getStatusForPeer(const std::string& username) const
{
	std::unordered_map<std::string, xmpp::Peer>::const_iterator iter = mRoster.find(username);
	if (iter != mRoster.end())
		return iter->second.status();
	
	return xmpp::Invalid;
}

bool Client::isPeerConnected(const std::string& username) const
{
	std::unordered_map<std::string, xmpp::Peer>::const_iterator iter = mRoster.find(username);
	if (iter != mRoster.end())
		return iter->second.isConnected();
	
	return false;
}

bool operator==( const xmpp::Peer& lhs, const xmpp::Peer& rhs)
{
	return lhs.username() == rhs.username();
}

bool operator!=( const xmpp::Peer& lhs, const xmpp::Peer& rhs)
{
	return lhs.username() != rhs.username();
}

}