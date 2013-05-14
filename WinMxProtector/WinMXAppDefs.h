/*-----------------------------------------------------------------------------
Purpose: This is a list of common defines for the winmx procjest
Created: 3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/

/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef WINMX_APP_DEFS_H
#define WINMX_APP_DEFS_H
/*-------------------------------INCLUDE---------------------------------------*/
#define kWINMX_APP_STRING                   "WinMX v"
#define kWINMX_NETWORKS_COMMAND_BUTTON      "Networks"
#define kWINMX_CHANGE_ID_STRING             "Change"
#define kWINMX_NETWORK_DISCONNECT           "Disconnect"
#define kWINMX_NETWORK_CONNECT              "Connect" //Connect/Refresh Button
#define kWINMX_PEER_NETWORK_ID_EDIT_DIALOG  "WinMX Peer Network ID"
#define kWINMX_PEER_NETWORK_ID_EDIT_OK      "OK"
#define kWINMX_PEER_NETWORK_ID_EDIT_CANCEL  "Cancel" 
#define kWINMX_PEER_NETWORK_ID_EDIT_BOX     "Edit"
#define kWINMX_SEARCH_COMMAND_BUTTON        "Search"
#define kWINMX_SEARCH_STOP_BUTTON           "Stop"
#define kWINMX_SEARCH_LIST_BOX              "Search List"
#define kWINMX_SEARCH_HEADER_LIST_BOX       "Search Header"

//used for setting up share folder
#define kWINMX_SETUP_SHARE_COMPONENT        "Shared Files"
#define kWINMX_SETUP_SHARE_DIALOG           "Browse for Folder"

#define kWINMX_ARTIST_EDIT_BOX              "ArtistEdit"//inits to nothing        
#define kWINMX_TITLE_EDIT_BOX               "TitleEdit"//inits to nothing
#define kWINMX_USERID_EDIT_BOX              " "//inits to nothing
#define kWINMX_NETWORK_REFRESH              "Refresh"

#define kWINMX_NUM_MAIN_COMPONENTS 19
#define kMAXWINDOWNAME 100

enum WINMX_APP_STATES 
{
  NETWORK_WINDOW_STATE = 0,
  CHANGE_USERID_STATE,
  SONG_SEARCH_STATE,
  NUM_OF_WINMX_STATES 
};

#endif