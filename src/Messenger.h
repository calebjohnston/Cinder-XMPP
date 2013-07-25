#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include "Cinder/Thread.h"

#include "gloox/gloox.h"
#include "gloox/jid.h"

#include "XmppListener.h"

typedef std::shared_ptr<class std::thread> ThreadRef;

class XmppUser {
public:
	XmppUser() : mConnected(false) {}
	~XmppUser() {}
	
private:
	bool mConnected;
};

class Messenger {
public:
	Messenger();
	~Messenger();
	
	// member methods for setup
	bool connect(const std::string& xmppUser, const std::string& xmppServer);
	
	bool sendMessage( const std::string& recipient, const std::string& message, const std::string& subject );
	
	bool isConnected() const { return mXmppListener->isConnected(); }
	
protected:
	/** methods to comply with MessengerInterface */
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual bool onTLSConnect( const gloox::CertInfo& info );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession *sess );
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& message );
	
private:
	/** Communication objects */
	XmppListener mListener;
	ThreadRef mListenerThread;
	std::map<std::string, unsigned int> mUserMap;
	std::set<XmppUser> mUserSet;
	
	std::string mServer;
	std::string mUser;
	
};
