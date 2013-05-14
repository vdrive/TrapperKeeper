// GWebCache.cpp

#include "stdafx.h"
#include "GWebCache.h"
#include "ConnectionManager.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "GnutellaSupplyDll.h"

//
//
//
GWebCache::GWebCache()
{
	srand((unsigned int)time(NULL));
	m_socket.InitParent(this);

	// Load url cache
	ReadWebCacheURLsFromFile();

	// If there was no file, add these 51 hard-coded ones in
	if(v_web_cache_urls.size()==0)
	{
		AddDefaultGWebCache();
		WriteWebCacheURLsToFile();
	}
}

//
//
//
GWebCache::~GWebCache()
{

}

//
//
//
void GWebCache::InitParent(ConnectionManager *manager)
{
	p_manager=manager;
}

//
//
//
bool GWebCache::ConnectToWebCache()
{
	if(v_web_cache_urls.size()==0)
		AddDefaultGWebCache();
	// Check to see if we are already connecting to a host cache
	if(m_socket.IsSocket())
	{
		return false;
	}

	// Set the connect time (for time-out check)
	m_connect_time=CTime::GetCurrentTime();

	char log[1024];
	p_manager->p_parent->m_log_window_manager.Log("Connection Manager : ",0x00800080);	// dark purple
	p_manager->p_parent->m_log_window_manager.Log("GWebCache : ",0x00008000);	// dark green

	// 1 out of ten, connect to web cache to get urls instead of hosts
	//we don't want URL reports for now

	if(rand() % 10 == 0)
	{
		string url=v_web_cache_urls[rand() % v_web_cache_urls.size()];

		p_manager->p_parent->m_log_window_manager.Log("GetURLFile",0x000000FF);		// red
		p_manager->p_parent->m_log_window_manager.Log(" from : ",0x00888888);			// grey
		sprintf(log,"%s\n",url.c_str());
		p_manager->p_parent->m_log_window_manager.Log(log);

		// Connect and get the urlfile
		m_socket.GetURLFile((char *)url.c_str());
	}
	else
	{
		string url=v_web_cache_urls[rand() % v_web_cache_urls.size()];

		p_manager->p_parent->m_log_window_manager.Log("GetHostFile",0x00FF0000);	// blue
		p_manager->p_parent->m_log_window_manager.Log(" from : ",0x00888888);			// grey
		sprintf(log,"%s\n",url.c_str());
		p_manager->p_parent->m_log_window_manager.Log(log);

		// Connect and get the hostfile
		m_socket.GetHostFile((char *)url.c_str());
	}

	return true;
}

//
//
//
void GWebCache::ReportURLs(vector<string> &urls)
{
	UINT i,j;

	// Make sure that there are no duplicates
	for(i=0;i<urls.size();i++)
	{
		bool found=false;
		
		for(j=0;j<v_web_cache_urls.size();j++)
		{
			if(stricmp(urls[i].c_str(),v_web_cache_urls[j].c_str())==0)
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			v_web_cache_urls.push_back(urls[i]);
		}
	}

	// Keep the vector size down to 200
	while(v_web_cache_urls.size()>200)
	{
		v_web_cache_urls.erase(v_web_cache_urls.begin());
	}

	// Write web cache urls to file
	WriteWebCacheURLsToFile();

	// Log it
	char log[1024];
	p_manager->p_parent->m_log_window_manager.Log("Connection Manager : ",0x00800080);	// dark purple
	p_manager->p_parent->m_log_window_manager.Log("GWebCache : ",0x00008000);	// dark green
	p_manager->p_parent->m_log_window_manager.Log("Reported URLs : ",0x000000FF);	// red
	sprintf(log,"%u\n",urls.size());
	p_manager->p_parent->m_log_window_manager.Log(log,0,true);	// black bold
}

//
//
//
void GWebCache::ReportHosts(vector<string> &hosts)
{
	UINT i;
	
	// Convert the ip:port strings to gnutella hosts
	vector<GnutellaHost> gnutella_hosts;
	for(i=0;i<hosts.size();i++)
	{
		char *buf=new char[hosts[i].size()+1];
		strcpy(buf,hosts[i].c_str());

		if(strchr(buf,':')!=NULL)
		{
			GnutellaHost gnutella_host;

			char *port=strchr(buf,':');
			*port='\0';
			port++;
			gnutella_host.Host(buf);
			gnutella_host.Port(atoi(port));

			gnutella_hosts.push_back(gnutella_host);
		}

		delete [] buf;
	}

	// Report these gnutella hosts to the connection manager
	p_manager->ReportHosts(gnutella_hosts);

	// Log it
	char log[1024];
	p_manager->p_parent->m_log_window_manager.Log("Connection Manager : ",0x00800080);	// dark purple
	p_manager->p_parent->m_log_window_manager.Log("GWebCache : ",0x00008000);	// dark green
	p_manager->p_parent->m_log_window_manager.Log("Reported Hosts : ",0x00FF0000);	// blue
	sprintf(log,"%u\n",hosts.size());
	p_manager->p_parent->m_log_window_manager.Log(log,0,true);	// black bold
}

//
//
//
void GWebCache::ReadWebCacheURLsFromFile()
{
	v_web_cache_urls.clear();

	// If there is a web_cache_urls.txt file, read it in, on each line is a web cache url
	CStdioFile file;
	if(file.Open("Host Cache\\web_cache_urls.txt",CFile::modeRead|CFile::typeText|CFile::shareDenyNone)==TRUE)
	{
		char buf[4096+1];
		while(file.ReadString(buf,4096))
		{
			CString cstring=buf;
			cstring.TrimLeft();
			cstring.TrimRight();
			string str;
			if(cstring.Find("<")==-1 && cstring.Find("|")==-1 && cstring.Find("\"")==-1 && cstring.Find(",")==-1
				&& cstring.Find(",")==-1 && cstring.Find("%")==-1 && cstring.Find("?")==-1)
			{
				int index = cstring.ReverseFind('.');
				if(index != -1 && index != cstring.GetLength()-1)
					str=cstring;
			}

			if(str.size()>0)
			{
				v_web_cache_urls.push_back(str);
			}
		}

		file.Close();
	}
	WriteWebCacheURLsToFile();
}

//
//
//
void GWebCache::WriteWebCacheURLsToFile()
{
	UINT i;

	// Open the hosts.dat file for writing...if the open fails, then who cares
	CStdioFile file;
	MakeSureDirectoryPathExists("Host Cache\\");
	if(file.Open("Host Cache\\web_cache_urls.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeText|CFile::shareDenyNone)==FALSE)
	{
		return;
	}

	for(i=0;i<v_web_cache_urls.size();i++)
	{
		file.WriteString(v_web_cache_urls[i].c_str());
		file.WriteString("\n");
	}

	file.Close();
}

//
//
//
unsigned int GWebCache::NumURLs()
{
	return (UINT)v_web_cache_urls.size();
}

//
//
//
void GWebCache::CheckForTimeout()
{
	if(m_socket.IsSocket()==false)
	{
		return;
	}

	// Check for time-out
	CTime now=CTime::GetCurrentTime();
	if((now-m_connect_time).GetTotalSeconds()>10)
	{
		// This connection has timed out. Close the socket and try another connection.
		ReportBadURL(m_socket.m_url.c_str());
		m_socket.Close();

		// Log it
		p_manager->p_parent->m_log_window_manager.Log("Connection Manager : ",0x00800080);	// dark purple
		p_manager->p_parent->m_log_window_manager.Log("GWebCache : ",0x00008000);	// dark green
		p_manager->p_parent->m_log_window_manager.Log("CONNECTION TIMED OUT\n",0x000000FF,true);	// bold red
	}
}

//
//
//
void GWebCache::ReportBadURL(const char* url)
{
	char* bad_url = new char[strlen(url)+1];
	strcpy(bad_url,url);
	char* ptr = strchr(bad_url,'?');
	*ptr='\0';

	bool found=false;

	vector<string>::iterator iter = v_web_cache_urls.begin();
	while(iter!=v_web_cache_urls.end())
	{
		if(strcmp(bad_url,iter->c_str())==0)
		{
			v_web_cache_urls.erase(iter);
			break;
		}
		iter++;
	}
	delete [] bad_url;
}

//
//
//
void GWebCache::AddDefaultGWebCache()
{
		v_web_cache_urls.push_back("http://bbs.robertwoolley.co.uk/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://bobsmith.is-a-geek.org/");
		v_web_cache_urls.push_back("http://bobsmith-gwc.kicks-ass.org/");
		v_web_cache_urls.push_back("http://cache.kicks-ass.net:8000/");
		v_web_cache_urls.push_back("http://cache.mynapster.com/index.php");
		v_web_cache_urls.push_back("http://cache.warrink.ath.cx:8000/");
		v_web_cache_urls.push_back("http://cgi.edu.lahti.fi/~staili/gwebcache/index.php");
		v_web_cache_urls.push_back("http://corky.net/gcache/gcache.php");
		v_web_cache_urls.push_back("http://crab.ghostwhitecrab.com/gwc/");
		v_web_cache_urls.push_back("http://dlaikar.de/gcache/gcache.php");
		v_web_cache_urls.push_back("http://dlaikar.de/gcache2/gcache.php");
		v_web_cache_urls.push_back("http://dlaikar.de/gcache-mac/gcache.php");
		v_web_cache_urls.push_back("http://g2cache.theg2.net/gwcache/lynnx.asp");
		v_web_cache_urls.push_back("http://galvatron.dyndns.org:59009/gwcache");
		v_web_cache_urls.push_back("http://gcache.cloppy.net/");
		v_web_cache_urls.push_back("http://gcache.kccricket.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://gcache.shacknet.nu:8088/gwc");
		v_web_cache_urls.push_back("http://gcachw.xolox.nl/gwebcache");
		v_web_cache_urls.push_back("http://goeg.dk/Gnutella/gcache.php");
		v_web_cache_urls.push_back("http://grupaprojektor.host.sk/gcache.php");
		v_web_cache_urls.push_back("http://gwc.ionichost.com/gwc.php");
		v_web_cache_urls.push_back("http://gwc.itbl.net:3000/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://gwc.nonamer.ath.cx:8080/");
		v_web_cache_urls.push_back("http://gwc.nonexiste.net/");
		v_web_cache_urls.push_back("http://gwc1.mager.org:8081/GWebCache/req");
		v_web_cache_urls.push_back("http://gwc1.nouiz.org/servlet/GWebCache");
		v_web_cache_urls.push_back("http://gwc1.nouiz.org/servlet/GWebCache/re");
		v_web_cache_urls.push_back("http://gwc1.nouiz.org/servlet/GWebCache/req");
		v_web_cache_urls.push_back("http://gwebcache.bearshare.net/gcache.php");
		v_web_cache_urls.push_back("http://gwebcache1.limewire.com/gwebcache/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://gwebcache3.jonatkins.org.uk/perlgcache.cgi");
		v_web_cache_urls.push_back("http://home.1asphost.com/kunkie/lynn.asp");
		v_web_cache_urls.push_back("http://intense.homelinux.coww.xolox.nl/gwebcache");
		v_web_cache_urls.push_back("http://jason.ionichost.com/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://jum.dyndns.org:9006/GWebCache/req");
		v_web_cache_urls.push_back("http://laplata.dnsalias.org/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://loot.alumnigroup.org/");
		v_web_cache_urls.push_back("http://mac.gc.dlaikar.de/");
		v_web_cache_urls.push_back("http://members.lycos.co.uk/malaycyber/gwcii.php");
		v_web_cache_urls.push_back("http://mitglied.lycos.de/monster954/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://mute-cache.2y.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://r3dk0w.dyndns.org/cgi-bin/gwebcache/gcache.cgi");
		v_web_cache_urls.push_back("http://terrorist.host.sk/gnutella-cache/gwcii.php");
		v_web_cache_urls.push_back("http://uui.upce.cz/perlgcache.cgi");
		v_web_cache_urls.push_back("http://www.commontology.de/andreas/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://www.easwww.xolox.nl/gwebcache");
		v_web_cache_urls.push_back("http://www.gbwebpro.com/guo/lynn/cache/lynn.asp");
		v_web_cache_urls.push_back("http://www.goeg.dk/Gnutella/gcache.php");
		v_web_cache_urls.push_back("http://www.gwc.freeola.com/cgi-bin/gwc/gcache.cgi");
		v_web_cache_urls.push_back("http://www.gwc2.ip3.com/cgi-bin/gwc2/gcache.cgi");
		v_web_cache_urls.push_back("http://www.kuwaitcontact.com/g2/bazooka.php");
		v_web_cache_urls.push_back("http://www.la-forza.com/gnucache/gcache.php");
		v_web_cache_urls.push_back("http://www.p2pchat.net/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://www.rodage.net/gnetcache/gcache.php");
		v_web_cache_urls.push_back("http://www.thany.org/gcache/gcache.php");
		v_web_cache_urls.push_back("http://www.theholt.net/cache.php");
		v_web_cache_urls.push_back("http://www.theholt.net/gcache.php");
		v_web_cache_urls.push_back("http://www.xolox.nl/gwebcache");
		v_web_cache_urls.push_back("http://www27.brinkster.com/snok7/lynn04/lynn.asp");
		v_web_cache_urls.push_back("http://www.hottdeals.com/postNuke7/gWebCache");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/cpwc4.cgi");
		v_web_cache_urls.push_back("http://nodecache.dyndns.org/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://crab.bishopston.net:3558/");
		v_web_cache_urls.push_back("http://cache.warrink.ath.cx:8000/");
		v_web_cache_urls.push_back("http://krill.shacknet.nu:20095/gwc");
		v_web_cache_urls.push_back("http://cache.mynapster.com/");
		v_web_cache_urls.push_back("http://node00.hewson.cns.ufl.edu:8080/pwc.cgi");
		v_web_cache_urls.push_back("http://ds.tranzision.com/cache/gcache.php");
		v_web_cache_urls.push_back("http://www.xolox.nl/gwebcache/");
		v_web_cache_urls.push_back("http://mute.d2lod.org/gwcii.php");
		v_web_cache_urls.push_back("http://gwebcache2.limewire.com:9000/gwc");
		v_web_cache_urls.push_back("http://gwebcache.bearshare.net/gcache.php");
		v_web_cache_urls.push_back("http://mute-net.sourceforge.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://unknown.pingtime.com/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://cache.kicks-ass.net:8000/");
		v_web_cache_urls.push_back("http://members.lycos.co.uk/itgwc/webcache/gcache.php");
		v_web_cache_urls.push_back("http://loot.alumnigroup.org/");
		v_web_cache_urls.push_back("http://crab1.dyndns.org:8002/gwc/");
		v_web_cache_urls.push_back("http://www.goeg.dk/Gnutella/gcache.php");
		v_web_cache_urls.push_back("http://www.xolox.nl:80/gwebcache/");
		v_web_cache_urls.push_back("http://crabcake.dynalias.net:9627/");
		v_web_cache_urls.push_back("http://gcache.cloppy.net/");
		v_web_cache_urls.push_back("http://gwc1.mager.org:8081/GWebCache/req");
		v_web_cache_urls.push_back("http://loot.alumnigroup.org");
		v_web_cache_urls.push_back("http://gwc.eigengang.nl/venus.php");
		v_web_cache_urls.push_back("http://abilgisayarcs.host.sk/gnutella");
		v_web_cache_urls.push_back("http://goeg.dk/Gnutella/gcache.php");
		v_web_cache_urls.push_back("http://pokerface.bishopston.net:3558/");
		v_web_cache_urls.push_back("http://gazth-sonika.ath.cx:8080/");
		v_web_cache_urls.push_back("http://www.easwww.xolox.nl/GWEBcache/");
		v_web_cache_urls.push_back("http://terrorist.host.sk/gnutella-cache/gwcii.php");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/cpwc5.cgi");
		v_web_cache_urls.push_back("http://www.roundbox.ws/gwebcache/gcache.php");
		v_web_cache_urls.push_back("http://www.schraders.us/gwebcache-0.7.4/gcache.php");
		v_web_cache_urls.push_back("http://gwc.lame.net/gwcii.php");
		v_web_cache_urls.push_back("http://bobsmith.is-a-geek.org/index.cgi");
		v_web_cache_urls.push_back("http://loot.alumnigroup.org/");
		v_web_cache_urls.push_back("http://cache.mynapster.com/index.php");
		v_web_cache_urls.push_back("http://g2cache.theg2.net/gwcache/lynnx.asp");
		v_web_cache_urls.push_back("http://www.xolox.nl/gwebcache/default.asp");
		v_web_cache_urls.push_back("http://www.gwc.freeola.com/cgi-bin/gwc/gcache.cgi");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/cpwc2.cgi");
		v_web_cache_urls.push_back("http://g1.blacknex.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://gwcrab.sarcastro.com:8001/");
		v_web_cache_urls.push_back("http://gcache.kccricket.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/cpwc1.cgi");
		v_web_cache_urls.push_back("http://crab2.dyndns.org:30002/gwc/");
		v_web_cache_urls.push_back("http://www.mute.dds.nl/Mcache1/mcache.php");
		v_web_cache_urls.push_back("http://gwc.ionichost.com/gwc.php");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/ccpwcc.cgi");
		v_web_cache_urls.push_back("http://www.testwebserver.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://g2.k4.cn:8080/");
		v_web_cache_urls.push_back("http://www.vittavi.net/cgi-bin/perlgcache.cgi");
		v_web_cache_urls.push_back("http://members.lycos.co.uk/coolebra/gwebcache-0.7.4/gcache.php");
		v_web_cache_urls.push_back("http://kisama.ath.cx:8080/");
		v_web_cache_urls.push_back("http://www.justthefaqs.org/gnu/gcache.php");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/ccpwc.cgi");
		v_web_cache_urls.push_back("http://gwebcache3.jonatkins.org.uk/perlgcache.cgi");
		v_web_cache_urls.push_back("http://bobsmith.is-a-geek.org/");
		v_web_cache_urls.push_back("http://www.easwww.xolox.nl/gwebcache/default.asp");
		v_web_cache_urls.push_back("http://www.arach.net.au/~holmesd6/gcache.php");
		v_web_cache_urls.push_back("http://godfella.host.sk/gwcii/gwcii.php");
		v_web_cache_urls.push_back("http://loot.alumnigroup.org:80/");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/pwc4.cgi");
		v_web_cache_urls.push_back("http://gwebcache.daems.org/GWebCache/req");
		v_web_cache_urls.push_back("http://loot.alumnigroup.org:80/");
		v_web_cache_urls.push_back("http://galvatron.dyndns.org:59009/gwcache");
		v_web_cache_urls.push_back("http://gwc.jooz.net:8010/gwc/");
		v_web_cache_urls.push_back("http://ygwc.y-0.net/ygwc.php");
		v_web_cache_urls.push_back("http://starscream.dynalias.com/");
		v_web_cache_urls.push_back("http://intense.homelinux.coww.xolox.nl/gwebcache/");
		v_web_cache_urls.push_back("http://node01.hewson.cns.ufl.edu:8080/cpwc3.cgi");
		v_web_cache_urls.push_back("http://dlaikar.de/gcache2/gcache.php");

}