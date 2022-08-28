/////////////////////////////////////////////////////////////////////////////////////////
//    This file is part of Pongo.
//
//    Copyright (C) 2020 Matthias Hund
//    
//    This program is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 2
//    of the License, or (at your option) any later version.
//    
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//    
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
/////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <signal.h>
#include <thread>
#include "NetworkManagement.h"
#include "NetworkManagementSsl.h"
#include "ElmtCanvas.h"

void intHandler(int dummy) 
{
	std::cout << "\033[0;31m" << "\nstarting application shutdown sequence\n" << "\033[0m" << std::endl;
	NetworkManagerBase::CloseAllClientSockets();
	NetworkManagement::CloseServerSocket();
	NetworkManagementSsl::CloseServerSocket();
}

void BlockUntilAllClientThreadsClosed(int timeout)
{
	while(NetworkManagerBase::IsHandlerListEmpty() == false and timeout>0)
	{
		sleep(1);
		timeout--;
	}
	if(NetworkManagerBase::IsHandlerListEmpty() == false)
	{
		std::cout << "\033[0;31m" << "not all connection closed!!!" << "\033[0m" << std::endl;
	}
}

int main(int argc,char * argv[]) 
{ 
	signal(SIGINT, intHandler);
	if(argc>1)
	{
		ResourceManager::SetBaseFolder(argv[1]);
		NetworkManagementSsl::SetBaseFolder(argv[1]);
	}
	NetworkManagementSsl::SetCertName("dummy");
	
	std::thread thread1(NetworkManagement::StartWebServer, 8080);
	std::thread thread2(NetworkManagementSsl::StartWebServer, 8081);
	
	std::thread thread3(NetworkManagerBase::ClearTimer,0);
	
	thread1.join();
	thread2.join();
	thread3.join();
	
	BlockUntilAllClientThreadsClosed(30);
	
	ResourceManager::Clear();
	
	std::cout << "main finished! bye" << std::endl;
	return 0;
} 