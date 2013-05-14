#pragma once

struct DCHeader
{
	enum op_code
	{
		//from source
		Init = 1,
		/* each collector should response with their Init Response message when they see the Init message */
		Gnutella_DC_Init_Response,
		Emule_DC_Init_Response,
		Kazaa_DC_Init_Response,
		Piolet_DC_Init_Response,
		Piolet_Spoofer_Init_Response,
		Gnutella_Protector_Init_Response,
		Direct_Connect_Init_Response,
		Direct_Connect_Spoofer_Init_Response,
		FastTrack_DC_Init_Response,
		FastTrack_Spoofer_Init_Response, //spoofer
		FastTrack_Swarmer_Init_Response, //swarmer
		Gnutella_Spoofer_Spited_Init_Response,
		WinMx_DC_Init_Response,
		WinMx_Spoofer_Init_Response,
		WinMx_Decoyer_Init_Response,
		WinMx_Swarmer_Init_Response,
		SoulSeek_Spoofer_Init_Response,
		BitTorrent_Init_Response,
		Overnet_DC_Init_Response,
		Overnet_Spoofer_Init_Response,
		Piolet_Spoofer_Splitted_Init_Response,
		BearShare_Protector_Init_Response,
		FastTrack_Decoyer_Init_Response,
		Generic_Init_Response=99,
		//op code 5-99 reserved

		/* project keywords data */
		ProjectChecksums = 100,
		UpdateProjectsRequest,
		//ProjectKeywordsStart,
		ProjectKeywords,
		//ProjectKeywordsEnd,
		//op code 105-199 reserved
		RequestProjectChecksums,


		DatabaseInfo = 200,		//database's host,user,password
		KazaaControllerIP,		//IP of Kazaa Controller that Kazaa Supply Taker requests the supernodes list from
		PioletRawDatabaseInfo,
		GnutellaRawDatabaseInfo,

		/* opcodes for gift clients */
		GiftSupplyRetrievalStarted = 300,
		GiftSupplyRetrievalEnded,
		GiftSupplyRetrievalProgress
	};
	op_code op;
	unsigned int size;
};