#include "Cinder/app/App.h"

#include "XmppListener.h"

using namespace gloox;

XmppListener::XmppListener()
:	mClient(NULL), mSession( 0 ), mMessageEventFilter( 0 ), mChatStateFilter( 0 ),
	mIsConnected(false), mServer(""), mUsername("")
{
}

XmppListener::~XmppListener()
{
	if (mClient) mClient->disconnect();
	
//	These lines cause problems??
//	delete mSession;
//	delete mMessageEventFilter;
//	delete mChatStateFilter;
	
	delete mClient;
}

void XmppListener::setup( const std::string& username, const std::string& server )
{
	if ((server.empty() && mServer.empty()) || (username.empty() && mUsername.empty())) return;
	
	std::string address = username;
	address += "@";
	if (server.empty()){
		address += mServer;
	} 
	else {
		address += server;
	}
	
	JID jid(address);
	
	mClient = new Client( jid, username );
	mClient->registerPresenceHandler( this );
	mClient->registerConnectionListener( this );
	mClient->registerMessageSessionHandler( this, 0 );
	mClient->rosterManager()->registerRosterListener(this);
	mClient->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
	mClient->disco()->setIdentity( "client", "bot" );
	mClient->disco()->addFeature( XMLNS_CHAT_STATES );

	//j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
}

void XmppListener::openConnection(bool toggle)
{
	if (!mIsConnected && mClient) {
		if ( mClient->connect( toggle ) ) {
			ConnectionError ce = ConnNoError;
			while ( ce == ConnNoError && mClient ) {
				ce = mClient->recv();
			}
		}
	}
}

void XmppListener::onConnect()
{
	mSignalConnect();
	
	mIsConnected = true;
}

void XmppListener::onDisconnect( ConnectionError e )
{
	if ( e == ConnAuthenticationFailed ) {
		ci::app::console() << "Auth failed. reason: " << mClient->authError() << std::endl;
	}
	
	mIsConnected = false;
	
	mSignalDisconnect();
}

bool XmppListener::onTLSConnect( const CertInfo& info )
{
	// info.status
	// info.issuer
	// info.server
	// info.protocol
	// info.mac
	// info.cipher
	// info.compression
	// time_t from( info.date_from );
	// time_t to( info.date_to );
	// ctime( &from )
	// ctime( &to )
		
	mSignalTlsConnect(info);
		
	return true;
}

void XmppListener::handleMessage( const Message& msg, MessageSession* sess )
{
	// msg.subtype()
	// msg.subject()
	// msg.body()
	// msg.thread()

	mSignalHandleMsg(msg, sess);
}

void XmppListener::handleMessageEvent( const JID& from, MessageEventType event ) {}

void XmppListener::handleChatState( const JID& from, ChatStateType state ) {}

void XmppListener::handleMessageSession( MessageSession* session )
{
	// this example can handle only one session. so we get rid of the old session
	mClient->disposeMessageSession( mSession );
	mSession = session;
	mSession->registerMessageHandler( this );
	mMessageEventFilter = new MessageEventFilter( mSession );
	mMessageEventFilter->registerMessageEventHandler( this );
	mChatStateFilter = new ChatStateFilter( mSession );
	mChatStateFilter->registerChatStateHandler( this );
}

void XmppListener::handleRoster( const Roster& roster )
{	
	mSignalRoster(roster);
}

void XmppListener::handleRosterPresence(const RosterItem& item, const std::string& resource,
										Presence::PresenceType presence, const std::string& msg )
{
	//item.jid()
	//presence
	
	mSignalRosterPresence(item, resource, presence, msg);
}

void XmppListener::handlePresence( const Presence& presence ) {}

void XmppListener::handleLog( LogLevel level, LogArea area, const std::string& message )
{
	//ci::app::console() << "XMPP Log Level: " << level << ", area: " << area << " : " << message.c_str() << std::endl;
}

bool XmppListener::sendMessage( const JID& recipient, const std::string& msg, const std::string& subject )
{
	if(!mIsConnected || msg.empty() || mClient == NULL) return false;
	
	Message m(Message::Chat, recipient, msg, subject);
	
	mClient->send(m);
	
	return true;
}

gloox::Client& XmppListener::getClient() const
{
	return *mClient;
}