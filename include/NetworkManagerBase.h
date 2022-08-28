#ifndef NETWORKMANAGERBASE_H
#define NETWORKMANAGERBASE_H

#include <mutex>
#include <list>
#include <thread>
#include <algorithm>
#include <iomanip>
#include "ClientHandler.h"

typedef std::list<std::tuple<uint32_t,const time_t> > pairListIpTime;
typedef std::list<std::tuple<uint32_t,const time_t> >::iterator pairListIpTimeIterator;

class NetworkManagerBase
{
public:
	NetworkManagerBase();
	virtual ~NetworkManagerBase();
	
	static void ClearTimer(int arg);
	
	static void CloseAllClientSockets();
	static void CloseSocket(int & sock,SSL **sslConnection);
	
	static void AddClientHandler(ClientHandler *pHandler);
	static std::list<ClientHandler*>& GetHandlerList();
	static void UnlockHandlerList();
	static bool IsHandlerListEmpty();
	static size_t GetHandlerListSize();
	static void RemoveFromHandlerList(ClientHandler * targetHandle);
	static bool IsBlacklisted(uint32_t ip_addr); 
	static void AddToBlacklist(uint32_t ip_addr);
private:
	static std::list<ClientHandler*> mHandler;
	static std::mutex mHandlerMutex;
	
	static pairListIpTime mBlacklist;
	static std::mutex mBlacklistMutex;
	
	static const double mBlacklistResetTime;	// time in sceonds
	static const unsigned int mResetDelay;
	static const double mConnectionTimeout;
	static bool mRunClearTimer;

};

#endif // NETWORKMANAGERBASE_H
