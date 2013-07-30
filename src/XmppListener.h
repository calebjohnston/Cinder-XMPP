#pragma once

#include "gloox/gloox.h"
#include "gloox/client.h"
#include "gloox/message.h"
#include "gloox/rostermanager.h"
#include "gloox/messagehandler.h"
#include "gloox/messagesession.h"
#include "gloox/rosterlistener.h"
#include "gloox/connectionlistener.h"
#include "gloox/presencehandler.h"
#include "gloox/messagesessionhandler.h"
#include "gloox/messageeventhandler.h"
#include "gloox/messageeventfilter.h"
#include "gloox/chatstatehandler.h"
#include "gloox/chatstatefilter.h"
#include "gloox/disco.h"
#include "gloox/lastactivity.h"
#include "gloox/loghandler.h"
#include "gloox/logsink.h"
#include "gloox/connectiontcpclient.h"
#include "gloox/connectionsocks5proxy.h"
#include "gloox/connectionhttpproxy.h"

#ifndef _WIN32
# include <unistd.h>
#endif

#include <iostream>
#include <string>

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class XmppClient;

class XmppListener : public gloox::ConnectionListener, gloox::MessageHandler,
	gloox::RosterListener, gloox::MessageSessionHandler, gloox::LogHandler,
	gloox::MessageEventHandler, gloox::ChatStateHandler, gloox::PresenceHandler
{
public:
	XmppListener(XmppClient* delegate = 0);
	virtual ~XmppListener();

	void setup( const std::string& username = "", const std::string& server = "", const bool enableLogging = false );

	void openConnection(bool toggle = false);

	const gloox::Client& getClient() const;
		
	bool isConnected() const { return mIsConnected; }
	std::string server() const { return mServer; }
	std::string username() const { return mUsername; }
	
	virtual bool sendMessage( const gloox::JID& recipient, const std::string& message, const std::string& subject );
	
protected:
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual bool onTLSConnect( const gloox::CertInfo& info );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession* session );
	virtual void handleMessageEvent( const gloox::JID& from, gloox::MessageEventType event );
	virtual void handleChatState( const gloox::JID& from, gloox::ChatStateType state );
	virtual void handleMessageSession( gloox::MessageSession* session );
	virtual void handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message );
	virtual void handlePresence( const gloox::Presence& presence );

	// satisfy interfaces for gloox::RosterListener
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource,
									  gloox::Presence::PresenceType presence, const std::string& msg );
	virtual void handleRosterError( const gloox::IQ& iq ) {}
	virtual void handleItemAdded( const gloox::JID& jid ) {}
	virtual void handleItemSubscribed( const gloox::JID& jid ) {}
	virtual void handleItemRemoved( const gloox::JID& jid ) {}
	virtual void handleItemUpdated( const gloox::JID& jid ) {}
	virtual void handleItemUnsubscribed( const gloox::JID& jid ) {}
	virtual void handleSelfPresence( const gloox::RosterItem& item, const std::string& resource,
									gloox::Presence::PresenceType presence, const std::string& msg ) {}
	virtual bool handleSubscriptionRequest( const gloox::JID& jid, const std::string& msg ) { return false; }
	virtual bool handleUnsubscriptionRequest( const gloox::JID& jid, const std::string& msg ) { return false; }
	virtual void handleNonrosterPresence( const gloox::Presence& presence ) {}

private:	
	bool mIsConnected;
	std::string mServer;
	std::string mUsername;
	
	gloox::Client* mClient;
	gloox::MessageSession* mSession;
	gloox::MessageEventFilter* mMessageEventFilter;
	gloox::ChatStateFilter* mChatStateFilter;
	
	XmppClient* mDelegate;
};
