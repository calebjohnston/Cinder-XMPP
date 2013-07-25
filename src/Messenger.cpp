#include <sstream>

#include "Cinder/app/App.h"
#include "Cinder/System.h"

#include "Messenger.h"

using namespace ci;
using namespace ci::app;
using namespace gloox;

Messenger::Messenger()
{
}

Messenger::~Messenger()
{
}

void Messenger::connect(const std::string& xmppUser, const std::string& xmppServer)
{
	mUser = xmppUser;
	mServer = xmppServer;
	
	mListener.setup(mUser, mServer);
	
	mListenerThread = ThreadRef( new std::thread(&XmppListener::openConnection, &mListener) );
}

void Messenger::onConnect()
{	
}

void Messenger::onDisconnect( ConnectionError e )
{
}

bool Messenger::onTLSConnect( const CertInfo& info )
{
	return false;
}

void Messenger::handleMessage( const Message& msg, MessageSession* sess )
{
	// process information from message
	std::string name = sess->target().username();
    
	int message_value;
	std::string message_str, message_tag;
	message_str = msg.body();
	size_t index = message_str.find(":");
	message_tag = message_str.substr(0,index);

	// OR process regular iOS registration.
	std::map<std::string, unsigned int>::iterator usr_itr = mUserMap.find(name);
	if(usr_itr != mUserMap.end() && usr_itr->second < mUserSet.size()){
		std::shared_ptr<IosClient> client = mUserSet.at(usr_itr->second);
		
		if(message_tag == "ready"){
            client->setIpAddress(message_str.substr(index+1));
			client->setReady(true);
		}
		else if(message_tag == "hint"){
			MessageEvent* e = new MessageEvent(MessageEvent::HINT);
			client->addEvent(e);
			// mAppMessenger->onXmppMessageReceived(client, MessengerInterface::HINT);
		}
	}
}

void Messenger::handleRoster( const Roster& roster )
{
	mUserMap.clear();
	
	Roster::const_iterator it = roster.begin();
	for ( ; it != roster.end(); ++it ) {
		std::string username = (*it).second->jid();
		size_t index = username.find("@");
		username = username.substr(0,index);

		// TODO: make less crazy
		mUserMap.insert(std::pair<std::string, unsigned int>(username, (unsigned int) mUserSet.size()));
		std::shared_ptr<IosClient> client = std::shared_ptr<IosClient>(new IosClient(username, /* ip address */ "", Presence::Unavailable));
		mUserSet.push_back(client);
	}
	
	// mAppMessenger->onXmppRosterLoaded();
}

void Messenger::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& message )
{	
	std::string name = item.jid();
	size_t index = name.find("@");
	name = name.substr(0,index);
	
	int i;
	
	// add/update clients
	std::map<std::string, unsigned int>::iterator usr_itr;
	std::shared_ptr<IosClient> client;
	if(presence == Presence::Available){
		usr_itr = mUserMap.find(name);
		
		if(usr_itr != mUserMap.end() && usr_itr->second < mUserSet.size()){
			client = mUserSet.at(usr_itr->second);
			if(client->isOnline()){
				ci::app::console() << "Client is already online, but this shouldn't happen!" << std::endl;
				return;
			}
			
			// send client index for OSC messages
			for(i=0; i<mUserSet.size(); i++){
				if(mUserSet[i]->username() == client->username()){
					mUserSet[i]->setPresence(presence);
					break;
				}
			}
			
			std::stringstream message(std::stringstream::in | std::stringstream::out);
			message << "registered:";
			message << mOscHost;
			message << ",";
			message << i;
			this->sendMessage(name, message.str(), "");
			
			// mAppMessenger->onXmppRosterUpdate(client);
		}
	}
	else if(presence==Presence::Invalid || presence==Presence::Unavailable){
		usr_itr = mUserMap.find(name);
		if(usr_itr != mUserMap.end() && usr_itr->second < mUserSet.size()){
			client = mUserSet.at(usr_itr->second);
			client->setPresence(presence);
		}
		
        // 2nd param signals to remove the client instead of adding it
		// mAppMessenger->onXmppRosterUpdate(client);
	}
}

bool Messenger::sendMessage(const std::string& recipient, const std::string& message, const std::string& subject)
{
	std::string r(recipient);
	r += "@";
	r += mServer;
	JID jid(r);
	
	mListener.sendMessage(jid, message, subject);
	
	return true;
}
