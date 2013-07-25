#pragma once

#include "XmppInterface.h"

#include "gloox/gloox.h"
#include "gloox/client.h"
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

class XmppListener : public cg::esi::XmppInterface, gloox::MessageSessionHandler, gloox::LogHandler,
	gloox::MessageEventHandler, gloox::ChatStateHandler, gloox::PresenceHandler {
public:
	XmppListener();
	XmppListener(XmppInterface* handler);
	virtual ~XmppListener();
	void setup( const std::string& username = "", const std::string& server = "" );
	void openConnection(bool toggle = false);
	gloox::Client& getClient() const;
	
	virtual bool sendMessage( const gloox::JID& recipient, const std::string& msg, const std::string& subject );
	
protected:
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual bool onTLSConnect( const gloox::CertInfo& info );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession * /*session*/ );
	virtual void handleMessageEvent( const gloox::JID& from, gloox::MessageEventType event );
	virtual void handleChatState( const gloox::JID& from, gloox::ChatStateType state );
	virtual void handleMessageSession( gloox::MessageSession *session );
	virtual void handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message );
	virtual void handlePresence( const gloox::Presence& presence );

	// satisfy interfaces for gloox::RosterListener
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& msg );
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
	bool m_isConnected;
	
	gloox::Client *j;
	gloox::MessageSession *m_session;
	gloox::MessageEventFilter *m_messageEventFilter;
	gloox::ChatStateFilter *m_chatStateFilter;
	std::string m_server;
	cg::esi::XmppInterface* m_xmppParentHandler;
	std::string m_username;
};
