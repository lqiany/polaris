//*********************************************************
//	Time_Panel.h - Time Navigation Control
//*********************************************************

#pragma once
#include "Antares_Includes.h"

//---------------------------------------------------------
//	Time_Panel - time panel class definition
//---------------------------------------------------------

prototype struct Time_Panel
{
	tag_as_prototype;

	feature_accessor(play,none,none);
	feature_accessor(play_button,none,none);
	feature_accessor(pause_button,none,none);
	feature_accessor(sizer,none,none);
	feature_accessor(time_display,none,none);
};