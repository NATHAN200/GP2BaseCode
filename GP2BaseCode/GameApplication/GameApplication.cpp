#include "GameApplication.h"

//should rellly check to see if we are on a windows platform
#include "../Window/Win32Window.h"

//Constructor
CGameApplication::CGameApplication(void)
{
	//set to NULL
	m_pWindow=NULL;
	//Set the window name to GP2
	m_GameOptionDesc.gameName=TEXT("GP2");
	//Window Hieght and Width
	m_GameOptionDesc.width=640;
	m_GameOptionDesc.height=480;
	//FullScreen
	m_GameOptionDesc.fullscreen=false;
	//config options
	m_ConfigFileName=TEXT("game.cfg");
}

//Deconstructor
CGameApplication::~CGameAppliction(void)
{
	if(m_pWindow)
	{
		delete m_pWindow;
		m_pWindow=NULL;
	}
}