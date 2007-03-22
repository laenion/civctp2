//----------------------------------------------------------------------------
//
// Project      : Call To Power 2
// File type    : C++ source
// Description  : Single player new game rules screen
// Id           : $Id$
//
//----------------------------------------------------------------------------
//
// Disclaimer
//
// THIS FILE IS NOT GENERATED OR SUPPORTED BY ACTIVISION.
//
// This material has been developed at apolyton.net by the Apolyton CtP2 
// Source Code Project. Contact the authors at ctp2source@apolyton.net.
//
//----------------------------------------------------------------------------
//
// Compiler flags
//
// - None
//
//----------------------------------------------------------------------------
//
// THIS FILE MAPS TO SPNEWGAMEPOPUPS.LDL
// Modifications from the original Activision code:
//
// - 7 options total needed to implement a new rule
//
// - Memory leaks repaired.
// - Initialized local variables. (Sep 9th 2005 Martin G�hmann)
// 
// - removed new rules attempt - E 12.27.2006
// - added citycapture as an option and mapped it to gameplayoptions
//   but appears to have no affect
// - changed whole file to look more like gameplayoptions 3.21.2007
// - added revoltinsurgents option 3.22.2007
//
//----------------------------------------------------------------------------

#include "c3.h"
#include "c3window.h"
#include "c3_popupwindow.h"
#include "c3_button.h"
#include "c3_listitem.h"
#include "c3_dropdown.h"
#include "c3_static.h"
#include "c3slider.h"
#include "c3ui.h"
#include "aui_switch.h"
#include "aui_uniqueid.h"

#include "profileDB.h"

#include "spnewgamewindow.h"
#include "spnewgamerulesscreen.h"

#include "keypress.h"
//missing?
#include "GameSettings.h" 
#include "screenutils.h"
#include "network.h"

extern C3UI			*g_c3ui;
extern ProfileDB	*g_theProfileDB;
extern Network				g_network;
static c3_PopupWindow	*s_spNewGameRulesScreen	= NULL;
static aui_Switch		*s_genocide			= NULL,
						*s_pollution		= NULL,
						*s_citycapture		= NULL, //emod1
						*s_onecity			= NULL, //emod1
						*s_revoltinsurgent  = NULL, //emod1
						*s_revoltcasualty   = NULL,
						*s_barbspawn		= NULL,
						*s_secthappy		= NULL,

						*s_NULL				= NULL;

enum
{
	R_GENOCIDE,
	R_POLLUTION,
	GP_CITYCAPTURE, //emod2
	R_ONECITY, //emod2
	R_INSURGENT,
	R_CASUALTY,
	R_BARBSPAWN,
	R_SECTHAPPY,
	GP_TOTAL
};

static uint32 check[] =
{
	R_GENOCIDE,
	R_POLLUTION,
	GP_CITYCAPTURE, //emod3
	R_ONECITY, //emod3
	R_INSURGENT,
	R_CASUALTY,
	R_BARBSPAWN,
	R_SECTHAPPY,
	GP_TOTAL
};

sint32 spnewgamerulesscreen_updateData()
{
	if ( !g_theProfileDB ) return -1;

	s_genocide->SetState( g_theProfileDB->IsGenocideRule() ); 
	s_pollution->SetState( g_theProfileDB->IsPollutionRule() ); 

	s_citycapture->SetState( g_theProfileDB->IsCityCaptureOptions() ); //emod4
	s_onecity->SetState( g_theProfileDB->IsOneCityChallenge() ); //emod4
	s_revoltinsurgent->SetState( g_theProfileDB->IsRevoltInsurgents() ); //emod4
	s_revoltcasualty->SetState( g_theProfileDB->IsRevoltCasualties() ); //emod4
	s_barbspawn->SetState( g_theProfileDB->IsBarbarianSpawnsBarbarian() );
	s_secthappy->SetState( g_theProfileDB->IsSectarianHappiness() );
	return 1;
}


sint32	spnewgamerulesscreen_displayMyWindow()
{
	sint32 retval=0;
	if(!s_spNewGameRulesScreen) { retval = spnewgamerulesscreen_Initialize(); }

	spnewgamerulesscreen_updateData();

	AUI_ERRCODE auiErr;

	auiErr = g_c3ui->AddWindow(s_spNewGameRulesScreen);
	keypress_RegisterHandler(s_spNewGameRulesScreen);

	Assert( auiErr == AUI_ERRCODE_OK );

	return retval;
}
sint32 spnewgamerulesscreen_removeMyWindow(uint32 action)
{
	if ( action != (uint32)AUI_BUTTON_ACTION_EXECUTE ) return 0;

	AUI_ERRCODE auiErr;

	auiErr = g_c3ui->RemoveWindow( s_spNewGameRulesScreen->Id() );
	keypress_RemoveHandler(s_spNewGameRulesScreen);

	Assert( auiErr == AUI_ERRCODE_OK );

	spnewgamescreen_update();

	return 1;
}



AUI_ERRCODE spnewgamerulesscreen_Initialize( void )
{
	AUI_ERRCODE errcode = AUI_ERRCODE_OK;
	MBCHAR		windowBlock[ k_AUI_LDL_MAXBLOCK + 1 ];

	if ( s_spNewGameRulesScreen ) return AUI_ERRCODE_OK; 

	strcpy(windowBlock, "SPNewGameRulesScreen");

	s_spNewGameRulesScreen = new c3_PopupWindow( &errcode, aui_UniqueId(), windowBlock, 16, AUI_WINDOW_TYPE_FLOATING, false);
		Assert( AUI_NEWOK(s_spNewGameRulesScreen, errcode) );
		if ( !AUI_NEWOK(s_spNewGameRulesScreen, errcode) ) errcode;

		
		s_spNewGameRulesScreen->Resize(s_spNewGameRulesScreen->Width(),s_spNewGameRulesScreen->Height());
		s_spNewGameRulesScreen->GrabRegion()->Resize(s_spNewGameRulesScreen->Width(),s_spNewGameRulesScreen->Height());
		s_spNewGameRulesScreen->SetStronglyModal(TRUE);
	
	

	s_genocide			= spNew_aui_Switch(&errcode, windowBlock, "RuleOne", spnewgamerulesscreen_checkPress, &check[R_GENOCIDE]);
	s_pollution			= spNew_aui_Switch(&errcode, windowBlock, "RuleTwo", spnewgamerulesscreen_checkPress, &check[R_POLLUTION]);
	s_citycapture		= spNew_aui_Switch(&errcode, windowBlock, "CityCapture", spnewgamerulesscreen_checkPress, &check[GP_CITYCAPTURE]); //emod5
	s_onecity			= spNew_aui_Switch(&errcode, windowBlock, "OneCity", spnewgamerulesscreen_checkPress, &check[R_ONECITY]); //emod5
	s_revoltinsurgent	= spNew_aui_Switch(&errcode, windowBlock, "RevoltInsurgents", spnewgamerulesscreen_checkPress, &check[R_INSURGENT]); //emod5
	s_revoltcasualty	= spNew_aui_Switch(&errcode, windowBlock, "RevoltCasualties", spnewgamerulesscreen_checkPress, &check[R_CASUALTY]); //emod5
	s_barbspawn			= spNew_aui_Switch(&errcode, windowBlock, "BarbSpawn", spnewgamerulesscreen_checkPress, &check[R_BARBSPAWN]); //emod5
	s_secthappy			= spNew_aui_Switch(&errcode, windowBlock, "SectHappy", spnewgamerulesscreen_checkPress, &check[R_SECTHAPPY]); //emod5


	spnewgamerulesscreen_updateData();

	MBCHAR block[ k_AUI_LDL_MAXBLOCK + 1 ];
	sprintf( block, "%s.%s", windowBlock, "Name" );
	s_spNewGameRulesScreen->AddTitle( block );
	s_spNewGameRulesScreen->AddClose( spnewgamerulesscreen_exitPress );

	
	errcode = aui_Ldl::SetupHeirarchyFromRoot( windowBlock );
	Assert( AUI_SUCCESS(errcode) );

	return AUI_ERRCODE_OK;
}

AUI_ERRCODE spnewgamerulesscreen_Cleanup()
{
#define mycleanup(mypointer) if(mypointer) { delete mypointer; mypointer = NULL; };

	if ( !s_spNewGameRulesScreen  ) return AUI_ERRCODE_OK; 

	g_c3ui->RemoveWindow( s_spNewGameRulesScreen->Id() );
	keypress_RemoveHandler(s_spNewGameRulesScreen);

	mycleanup(s_genocide);
	mycleanup(s_pollution);
	mycleanup(s_citycapture); //emod6
	mycleanup(s_onecity); //emod 6
	mycleanup(s_revoltinsurgent); //emod6
	mycleanup(s_revoltcasualty); //emod6
	mycleanup(s_barbspawn); //emod6
	mycleanup(s_secthappy); //emod6
	delete s_spNewGameRulesScreen;
	s_spNewGameRulesScreen = NULL;

	return AUI_ERRCODE_OK;
#undef mycleanup
}

void spnewgamerulesscreen_checkPress(aui_Control *control, uint32 action, uint32 data, void *cookie )
{
	
	if ( action != (uint32)AUI_SWITCH_ACTION_PRESS ) return;

	uint32 checkbox = *((uint32*)cookie);
	void (ProfileDB::*func)(BOOL) = 0;
	uint32 state = data; 

	switch(checkbox) {
	case R_GENOCIDE: func = &ProfileDB::SetGenocideRule; break; 
	case R_POLLUTION: func = &ProfileDB::SetPollutionRule; break; 
	case GP_CITYCAPTURE: func = &ProfileDB::SetCityCaptureOptions; break; //emod7
	case R_ONECITY: func = &ProfileDB::SetOneCity; break; //emod7
	case R_INSURGENT: func = &ProfileDB::SetRevoltInsurgents; break; //emod7
	case R_CASUALTY: func = &ProfileDB::SetRevoltCasualties; break; //emod7
	case R_BARBSPAWN: func = &ProfileDB::SetBarbarianSpawnsBarbarian; break; //emod7
	case R_SECTHAPPY: func = &ProfileDB::SetSectarianHappiness; break; //emod7
	default:  Assert(0); break;
	};

	if(func)
		(g_theProfileDB->*func)(state ? FALSE : TRUE); 
}


void spnewgamerulesscreen_exitPress(aui_Control *control, uint32 action, uint32 data, void *cookie )
{
	g_theProfileDB->Save();

	spnewgamerulesscreen_removeMyWindow(action);
}