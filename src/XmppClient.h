#pragma once

#include <string>
#include <list>
#include <unordered_map>

#include "Cinder/Thread.h"

#include "gloox/gloox.h"
#include "gloox/jid.h"

#include "XmppListener.h"

typedef std::shared_ptr<class std::thread> ThreadRef;

class XmppPeer {
public:
	enum status_t {
		Available,    //!< online
		Chat,         //!< available for chat
		Away,         //!< away
		DND,          //!< DND (Do Not Disturb)
		XA,           //!< eXtended Away
		Unavailable,  //!< offline
		Invalid       //!< invalid
	};
	
public:
	XmppPeer(const std::string& user = "unknown", bool conn = false, status_t status = Invalid) : mUsername(user), mConnected(conn), mStatus(status) {}
	~XmppPeer() {}
	
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
	
	static status_t presenceToStatus(const gloox::Presence::PresenceType value)
	{
		if ((int) value <= 6) {
			return XmppPeer::status_t((int)value);
		}
		return XmppPeer::Invalid;
	}
	
	static std::string statusString(const status_t value)
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
	
private:
	bool mConnected;
	std::string mUsername;
	status_t mStatus;
};


typedef ci::signals::signal<void()> SignalCallback;
typedef ci::signals::signal<void(const std::list<std::string>&)> SignalCallbackRoster;
typedef ci::signals::signal<void(const XmppPeer&, const std::string&, const std::string&)> SignalCallbackMsg;
typedef ci::signals::signal<void(const XmppPeer&, const std::string&, XmppPeer::status_t, const std::string&)> SignalCallbackPresence;

class XmppClient {
public:
	XmppClient();
	~XmppClient();
	
	bool connect(const std::string& xmppUser, const std::string& xmppServer);
	
	bool sendMessage( const std::string& recipient, const std::string& message, const std::string& subject );
	
	bool isConnected() const { return mListener.isConnected(); }
	
	SignalCallback&				getSignalConnect() { return mSignalConnect; }
	SignalCallback&				getSignalTlsConnect() { return mSignalTlsConnect; }
	SignalCallback&				getSignalDisconnect() { return mSignalDisconnect; }
	SignalCallbackRoster&		getSignalRoster() { return mSignalRoster; }
	SignalCallbackMsg&			getSignalMessage() { return mSignalHandleMsg; }
	SignalCallbackPresence&		getSignalRosterPresence() { return mSignalRosterPresence; }
	
protected:
	friend class XmppListener;
	
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual void onTLSConnect( const gloox::CertInfo& info );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession *sess );
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& message );
	
private:
	XmppListener mListener;
	ThreadRef mListenerThread;
	std::unordered_map<std::string, XmppPeer> mRoster;
	
	SignalCallback mSignalConnect, mSignalDisconnect, mSignalTlsConnect;
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

class XmppFailedConnectionExc : public XmppExc {
};

#pragma mark comparison operators

bool operator==( const XmppPeer& lhs, const XmppPeer& rhs);
bool operator!=( const XmppPeer& lhs, const XmppPeer& rhs);
