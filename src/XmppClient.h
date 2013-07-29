#pragma once

#include <string>
#include <list>
#include <unordered_map>

#include "Cinder/Thread.h"

#include "gloox/gloox.h"
#include "gloox/jid.h"

#include "XmppListener.h"

namespace xmpp {

typedef std::shared_ptr<class std::thread> ThreadRef;

enum error_t {
	NoError,				//!< everything was fine, no error
	GeneralError,			//!< non-descript
	AuthenticationError,	//!< problem with auth info
	ProtocolError,			//!< problem with following protocol
	StreamError,			//!< problem stream writing or reading
	ConnectionError			//!< connection dropped
};
	
enum status_t {
	Available,    //!< online
	Chat,         //!< available for chat
	Away,         //!< away
	DND,          //!< DND (Do Not Disturb)
	XA,           //!< eXtended Away
	Unavailable,  //!< offline
	Invalid       //!< invalid
};

status_t presenceToStatus(const gloox::Presence::PresenceType value);

std::string statusString(const status_t value);
	
class Peer {
public:
	Peer(const std::string& user = "unknown", bool conn = false, status_t status = Invalid) : mUsername(user), mConnected(conn), mStatus(status) {}
	~Peer() {}
	
	bool isConnected() const { return mConnected; }
	void setConnected(const bool value) { mConnected = value; }
	
	const std::string& username() const { return mUsername; }
	void setUsername(const std::string& value) { mUsername = value; }
	
	status_t status() const { return mStatus; }
	void setStatus(const status_t value)
	{
		mStatus = value;
		mConnected = mStatus < 5;
	}
	
private:
	bool mConnected;
	std::string mUsername;
	status_t mStatus;
};


typedef ci::signals::signal<void()> SignalCallback;
typedef ci::signals::signal<void(const xmpp::error_t&)> SignalCallbackDisconnect;
typedef ci::signals::signal<void(const std::list<std::string>&)> SignalCallbackRoster;
typedef ci::signals::signal<void(const xmpp::Peer&, const std::string&, const std::string&)> SignalCallbackMsg;
typedef ci::signals::signal<void(const xmpp::Peer&, const std::string&, xmpp::status_t, const std::string&)> SignalCallbackPresence;

class Client {
public:
	Client();
	~Client();
	
	bool connect(const std::string& xmppUser, const std::string& xmppServer);
	
	bool sendMessage( const std::string& recipient, const std::string& message, const std::string& subject );
	
	xmpp::status_t getStatusForPeer(const std::string& username) const;
	bool isPeerConnected(const std::string& username) const;
	
	bool isConnected() const { return mListener.isConnected(); }
	
	const std::unordered_map<std::string, xmpp::Peer>& getRoster() const { return mRoster; }
	
	SignalCallback&				getSignalConnect() { return mSignalConnect; }
	SignalCallback&				getSignalTlsConnect() { return mSignalTlsConnect; }
	SignalCallbackDisconnect&	getSignalDisconnect() { return mSignalDisconnect; }
	SignalCallbackRoster&		getSignalRoster() { return mSignalRoster; }
	SignalCallbackMsg&			getSignalMessage() { return mSignalHandleMsg; }
	SignalCallbackPresence&		getSignalRosterPresence() { return mSignalRosterPresence; }
	
protected:
	friend class xmpp::Listener;
	
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual void onTLSConnect( const gloox::CertInfo& info );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession *sess );
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& message );
	
private:
	xmpp::Listener mListener;
	ThreadRef mListenerThread;
	std::unordered_map<std::string, xmpp::Peer> mRoster;
	
	SignalCallback mSignalConnect, mSignalTlsConnect;
	SignalCallbackDisconnect mSignalDisconnect;
	SignalCallbackRoster mSignalRoster;
	SignalCallbackMsg mSignalHandleMsg;
	SignalCallbackPresence mSignalRosterPresence;
	
};

#pragma mark exception types

class XmppExc : public std::exception {
};

class XmppAuthExc : public XmppExc {
};

class XmppProtocolExc : public XmppExc {
};

class XmppStreamExc : public XmppExc {
};

class XmppConnectionExc : public XmppExc {
};

#pragma mark comparison operators

bool operator==( const xmpp::Peer& lhs, const xmpp::Peer& rhs);
bool operator!=( const xmpp::Peer& lhs, const xmpp::Peer& rhs);

}	/* end namespace xmpp */
