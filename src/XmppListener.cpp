#include "Cinder/app/App.h"

#include "XmppListener.h"

using namespace gloox;

XmppListener::XmppListener()
:	j(NULL), m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ), m_isConnected(false), m_server(""), m_username("")
{
	m_xmppParentHandler = NULL;
}

XmppListener::XmppListener(XmppInterface* handler)
:	j(NULL), m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ), m_isConnected(false), m_server(""), m_username("")
{
	m_xmppParentHandler = handler;
}

XmppListener::~XmppListener()
{
	if (j) j->disconnect();
	
//	These lines cause problems??
//	delete m_session;
//	delete m_messageEventFilter;
//	delete m_chatStateFilter;
	
	delete j;
	
	m_xmppParentHandler = NULL;
}

void XmppListener::setup( const std::string& username, const std::string& server )
{
	if ((server.empty() && m_server.empty()) || (username.empty() && m_username.empty())) return;
	
	std::string address = username;
	address += "@";
	if (server.empty()){
		address += m_server;
	} 
	else {
		address += server;
	}
	
	JID jid(address);
	
	j = new Client( jid, username );
	j->registerPresenceHandler( this );
	j->registerConnectionListener( this );
	j->registerMessageSessionHandler( this, 0 );
	gloox::RosterListener* rl = this->getRosterListenerCast();
	j->rosterManager()->registerRosterListener(rl);
	j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
	j->disco()->setIdentity( "client", "bot" );
	j->disco()->addFeature( XMLNS_CHAT_STATES );

	//j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
}

void XmppListener::openConnection(bool toggle)
{
	if (!m_isConnected && j) {
		if ( j->connect( toggle ) ) {
			ConnectionError ce = ConnNoError;
			while ( ce == ConnNoError && j ) {
				ce = j->recv();
			}
		}
	}
}

void XmppListener::onConnect()
{
	m_xmppParentHandler->onConnect();
	
	m_isConnected = true;
}

void XmppListener::onDisconnect( ConnectionError e )
{
	if ( e == ConnAuthenticationFailed ) {
		ci::app::console() << "Auth failed. reason: " << j->authError() << std::endl;
	}
	
	m_xmppParentHandler->onDisconnect(e);
	
	m_isConnected = false;
}

bool XmppListener::onTLSConnect( const CertInfo& info )
{
	time_t from( info.date_from );
	time_t to( info.date_to );
	
	// info.status
	// info.issuer
	// info.server
	// info.protocol
	// info.mac
	// info.cipher
	// info.compression
	// ctime( &from )
	// ctime( &to )
		
	m_xmppParentHandler->onTLSConnect(info);
		
	return true;
}

void XmppListener::handleMessage( const Message& msg, MessageSession * sess )
{
	// msg.subtype()
	// msg.subject()
	// msg.body()
	// msg.thread()

	m_xmppParentHandler->handleMessage(msg, sess);
}

void XmppListener::handleMessageEvent( const JID& from, MessageEventType event ) {}

void XmppListener::handleChatState( const JID& from, ChatStateType state ) {}

void XmppListener::handleMessageSession( MessageSession *session )
{
	// this example can handle only one session. so we get rid of the old session
	j->disposeMessageSession( m_session );
	m_session = session;
	gloox::MessageHandler* mh = this->getMessageHandlerCast();
	m_session->registerMessageHandler( mh );
	m_messageEventFilter = new MessageEventFilter( m_session );
	m_messageEventFilter->registerMessageEventHandler( this );
	m_chatStateFilter = new ChatStateFilter( m_session );
	m_chatStateFilter->registerChatStateHandler( this );
}

void XmppListener::handleRoster( const Roster& roster )
{	
	m_xmppParentHandler->handleRoster(roster);
}

void XmppListener::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& msg )
{
	//item.jid()
	//presence
	
	m_xmppParentHandler->handleRosterPresence(item, resource, presence, msg);
}

void XmppListener::handlePresence( const Presence& presence ) {}

void XmppListener::handleLog( LogLevel level, LogArea area, const std::string& message )
{
	ci::app::console() << "XMPP Log Level: " << level << ", area: " << area << " : " << message.c_str() << std::endl;
}

bool XmppListener::sendMessage( const JID& recipient, const std::string& msg, const std::string& subject )
{
	if(!m_isConnected || msg.empty() || j == NULL) return false;
	
	Message m(Message::Chat, recipient, msg, subject);
	
	j->send(m);
	
	return true;
}

gloox::Client& XmppListener::getClient() const
{
	return *j;
}