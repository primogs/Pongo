#include "NetworkManagerBase.h"

extern volatile bool keepRunning;

std::list<ClientHandler*> NetworkManagerBase::mHandler;
std::mutex NetworkManagerBase::mHandlerMutex;
pairListIpTime NetworkManagerBase::mBlacklist;
std::mutex NetworkManagerBase::mBlacklistMutex;
const double NetworkManagerBase::mBlacklistResetTime = 604800.0;	// blacklist for one week
const unsigned int NetworkManagerBase::mResetDelay = 3600; // check every hour
const double NetworkManagerBase::mConnectionTimeout = 10800;

NetworkManagerBase::NetworkManagerBase()
{
	
}

NetworkManagerBase::~NetworkManagerBase()
{
}

void NetworkManagerBase::Init()
{
	std::thread thread(NetworkManagerBase::ClearTimer,0);
	thread.detach();
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

void NetworkManagerBase::ClearTimer(int arg)
{
	while(keepRunning==true)
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
					std::cout << "client handler timeout" << std::endl;
					(*it)->ShutdownSocket();
				}
			}
			UnlockHandlerList();
		}
		
		
		std::this_thread::sleep_for(std::chrono::seconds(mResetDelay));
	}
}