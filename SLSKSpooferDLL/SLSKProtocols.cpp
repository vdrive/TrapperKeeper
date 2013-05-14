

#include "stdafx.h"
#include "SLSKProtocols.h"
#include "SLSKSpooferDlg.h"


SLSKProtocols::SLSKProtocols()
{
	dip = "38.119.66.38";
	dun = "root";
	dpass = "ebertsux37";
	ddb = "slsk";
}

SLSKProtocols::~SLSKProtocols()
{
}

void SLSKProtocols::reply(buffer b)
{
	int message = 0;

	switch(message)
	{
	case 1:
		//no reply needed back for login 
		//make sure login successful only
		break;
	case 5:
		//there is no reply for this
		break;
	case 36:
		//reply
		break;
	case 64:
		//put info after room list
		//send 5 and mess
		break;
	case 83:
		//no reply required
		break;
	case 84:
		//no reply required
		break;
	case 86:
		//no reply required
		break;
	case 87:
		//no reply required
		break;
	case 90:
		//no reply required
		break;
	case 102:
		myDB.dbConnection(dip, dun, dpass, ddb);
		//add parents list info to database
		myDB.dbClose();
		break;
	case 104:
		//no reply required
		break;
	default:
		break;
	}

	return b;
}

void SLSKProtocols::login(void)
{
}



