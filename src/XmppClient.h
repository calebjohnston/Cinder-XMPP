#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Cinder/Thread.h"

#include "gloox/gloox.h"
#include "gloox/jid.h"

#include "XmppListener.h"

typedef std::shared_ptr<class std::thread> ThreadRef;

typedef enum {
	Available = 0,  /**< The entity is online. */
    Chat,           /**< The entity is 'available for chat'. */
    Away,           /**< The entity is away. */
    DND,            /**< The entity is DND (Do Not Disturb). */
    XA,             /**< The entity is XA (eXtended Away). */
    Unavailable,    /**< The entity is offline. */
    Probe,          /**< This is a presence probe. */
    Error,          /**< This is a presence error. */
    Invalid         /**< The stanza is invalid. */	
} PresenceType;

class XmppPeer {
public:
	XmppPeer(const std::string& user = "unknown", bool conn = false, PresenceType status) : mUsername(user), mConnected(conn), mPresence(status) {}
	~XmppPeer() {}
	
	void setConnected(const bool value) { mConnected = value; }
	void setUsername(const std::string& value) { mUsername = value; }
	void setStatus(const PresenceType value) { mPresence = value; }
	
private:
	bool mConnected;
	std::string mUsername;
	PresenceType mPresence;
};

typedef ci::signals::signal<void()> SignalCallback;
typedef ci::signals::signal<void(const XmppPeer&, const std::string&, const std::string&)> SignalCallbackMsg;
typedef ci::signals::signal<void(const std::string&, const std::string&, PresenceType, const std::string&)> SignalCallbackPresence;

class XmppClient {
public:
	XmppClient();
	~XmppClient();
	
	bool connect(const std::string& xmppUser, const std::string& xmppServer);
	
	bool sendMessage( const std::string& recipient, const std::string& message, const std::string& subject );
	
	bool isConnected() const { return mXmppListener->isConnected(); }
	
	SignalCallback&				getSignalConnect() { return mSignalConnect; }
	SignalCallback&				getSignalTlsConnect() { return mSignalTlsConnect; }
	SignalCallback&				getSignalDisconnect() { return mSignalDisconnect; }
	SignalCallback&				getSignalRoster() { return mSignalRoster; }
	SignalCallbackMsg&			getSignalMessage() { return mSignalHandleMsg; }
	SignalCallbackPresence&		getSignalRosterPresence() { return mSignalRosterPresence; }
	
protected:
	friend class XmppListener;
	
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual bool onTLSConnect( const gloox::CertInfo& info );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession *sess );
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& message );
	
private:
	XmppListener mListener;
	ThreadRef mListenerThread;
	std::unordered_map<std::string, XmppUser> mUserMap;
	
	SignalCallback mSignalConnect, mSignalDisconnect, mSignalTlsConnect, mSignalRoster;
	SignalCallbackMsg mSignalHandleMsg;
	SignalCallbackPresence mSignalRosterPresence;
	
};

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

class XmppFailedConnectionExc : public XmppExc {
};
