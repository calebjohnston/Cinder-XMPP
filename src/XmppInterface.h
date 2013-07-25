#pragma once

#include <string>

#include "gloox/gloox.h"
#include "gloox/message.h"
#include "gloox/rostermanager.h"
#include "gloox/messagehandler.h"
#include "gloox/messagesession.h"
#include "gloox/rosterlistener.h"
#include "gloox/connectionlistener.h"

class XmppInterface : public gloox::ConnectionListener, gloox::MessageHandler, gloox::RosterListener {
public:
	virtual ~XmppInterface() {}
	
	// satisfy interfaces for gloox::connectionListener and gloox::MessageHandler
	virtual void onConnect() = 0;
	virtual void onDisconnect( gloox::ConnectionError e ) = 0;
	virtual bool onTLSConnect( const gloox::CertInfo& info ) = 0;
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession * /*session*/ ) = 0;

	// satisfy interfaces for gloox::RosterListener
	virtual void handleRoster( const gloox::Roster& roster ) = 0;
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& msg ) = 0;
	virtual void handleRosterError( const gloox::IQ& iq ) = 0;
	virtual void handleItemAdded( const gloox::JID& jid ) = 0;
	virtual void handleItemSubscribed( const gloox::JID& jid ) = 0;
	virtual void handleItemRemoved( const gloox::JID& jid ) = 0;
	virtual void handleItemUpdated( const gloox::JID& jid ) = 0;
	virtual void handleItemUnsubscribed( const gloox::JID& jid ) = 0;
	virtual void handleSelfPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& msg ) = 0;
	virtual bool handleSubscriptionRequest( const gloox::JID& jid, const std::string& msg ) = 0;
	virtual bool handleUnsubscriptionRequest( const gloox::JID& jid, const std::string& msg ) = 0;
	virtual void handleNonrosterPresence( const gloox::Presence& presence ) = 0;
	
	gloox::RosterListener* getRosterListenerCast() { return static_cast<gloox::RosterListener*>(this); }
	gloox::MessageHandler* getMessageHandlerCast() { return static_cast<gloox::MessageHandler*>(this); }
	
protected:
	XmppInterface() {}
	
};