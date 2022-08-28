#include "NetworkManagerBase.h"

std::list<ClientHandler*> NetworkManagerBase::mHandler;
std::mutex NetworkManagerBase::mHandlerMutex;
pairListIpTime NetworkManagerBase::mBlacklist;
std::mutex NetworkManagerBase::mBlacklistMutex;
const double NetworkManagerBase::mBlacklistResetTime = 604800.0;	// blacklist for one week
const unsigned int NetworkManagerBase::mResetDelay = 20;
const double NetworkManagerBase::mConnectionTimeout = 3600;
bool NetworkManagerBase::mRunClearTimer = true;

NetworkManagerBase::NetworkManagerBase()
{
	
}

NetworkManagerBase::~NetworkManagerBase()
{
}

std::list<ClientHandler*>& NetworkManagerBase::GetHandlerList()
{
	mHandlerMutex.lock();
	return mHandler;
}

void NetworkManagerBase::UnlockHandlerList()
{
	mHandlerMutex.unlock();
}

bool NetworkManagerBase::IsHandlerListEmpty()
{
	return mHandler.empty();
}

size_t NetworkManagerBase::GetHandlerListSize()
{
	return mHandler.size();
}

void NetworkManagerBase::RemoveFromHandlerList(ClientHandler * targetHandle)
{
	bool found = false;
	std::list<ClientHandler*> & handler = GetHandlerList();
	for(std::list<ClientHandler*>::iterator it = handler.begin();it != handler.end();++it)
	{ 
		ClientHandler* pHandler = (*it);
		if(pHandler==targetHandle)
		{
			handler.erase(it);
			found = true;
			break;
		}
	}
	UnlockHandlerList();
	if(found==false)
		std::cout << "WARNING client handler not found !!!" << std::endl;
}

void NetworkManagerBase::AddClientHandler(ClientHandler *pHandler)
{
	bool found = false;
	std::list<ClientHandler*> & handler = GetHandlerList();
	for(std::list<ClientHandler*>::iterator it = handler.begin();it != handler.end();++it)
	{ 
		ClientHandler* cHandle = (*it);
		if(pHandler==cHandle)
		{
			found = true;
			std::cout << "WARNING client handler allready on list !!!" << std::endl;
			break;
		}
	}
	if(found==false)
		handler.push_back(pHandler);
	UnlockHandlerList();
}

bool NetworkManagerBase::IsBlacklisted(uint32_t ip_addr)
{
	bool res = false;
	mBlacklistMutex.lock();
	for(pairListIpTimeIterator it = mBlacklist.begin();it != mBlacklist.end();++it)
	{
		if( std::get<0>(*it)== ip_addr)
		{
			res = true;
			break;
		}
	}
	mBlacklistMutex.unlock();
	return res;
}

void NetworkManagerBase::AddToBlacklist(uint32_t ip_addr)
{
	const time_t t = time(nullptr);
		
	mBlacklistMutex.lock();
	mBlacklist.push_back(std::make_tuple(ip_addr,t));
	mBlacklistMutex.unlock();
	
	std::cout << "add ip to blacklist, " << mBlacklist.size() << " IP's now on the list"<< std::endl;
}

void NetworkManagerBase::CloseAllClientSockets()
{
	std::list<ClientHandler*> & handler = GetHandlerList();
	for(std::list<ClientHandler*>::iterator it = handler.begin();it != handler.end();++it)
	{
		(*it)->CloseSocket();
	}
	UnlockHandlerList();
	mRunClearTimer = false;
}

void NetworkManagerBase::CloseSocket(int & sock,SSL **sslConnection)
{
	if((sslConnection != nullptr) and (*sslConnection != nullptr ))
	{
		int res = SSL_shutdown(*sslConnection);
		if( res == 0) // shutdown is not yet finished
		{
			usleep(10);
			if(SSL_pending(*sslConnection)>0)
				SSL_read(*sslConnection,NULL,0); // Call SSL_read() to do a bidirectional shutdown
		}
        SSL_free(*sslConnection);
		
		*sslConnection = nullptr;
	}
	if(sock != -1)
	{
		shutdown(sock, SHUT_RDWR);
		usleep(10);
		if(close(sock)== -1)
		{
			std::cout << "error in closing socket... " << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
		sock = -1;
	}
}

void NetworkManagerBase::ClearTimer(int arg)
{
	while(mRunClearTimer)
	{
		if(mBlacklist.empty() == false)
		{
			const time_t now = time(nullptr);
			mBlacklistMutex.lock();
			for(pairListIpTimeIterator it = mBlacklist.begin();it != mBlacklist.end();)
			{
				if( difftime(now,std::get<1>(*it)) >mBlacklistResetTime)
				{
					it = mBlacklist.erase(it);
				}
				else
				{
					++it;
				}
			}
			mBlacklistMutex.unlock();
		}
		
		if(IsHandlerListEmpty() == false)
		{
			const time_t now = time(nullptr);
			std::list<ClientHandler*> & handler = GetHandlerList();
			for(std::list<ClientHandler*>::iterator it = handler.begin();it != handler.end();++it)
			{
				if(difftime(now,(*it)->GetStartupTime())>mConnectionTimeout)
				{
					std::cout << "\033[0;31m" << "client handler timeout" << "\033[0m" << std::endl;
					(*it)->CloseSocket();
				}
			}
			UnlockHandlerList();
		}
		
		std::this_thread::sleep_for(std::chrono::seconds(mResetDelay));
	}
	std::cout << "\033[0;31m" << "thread3 ClearTimer closed\n" << "\033[0m" << std::endl;
}