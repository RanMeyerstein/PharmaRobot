
// PharmaRobot 1.0.h : main header file for the PROJECT_NAME application
//
#ifndef PHARMAROBOT_1_0_H
#define PHARMAROBOT_1_0_H

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPharmaRobot10App:
// See PharmaRobot 1.0.cpp for the implementation of this class
//

class CPharmaRobot10App : public CWinApp
{
public:
	CPharmaRobot10App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPharmaRobot10App theApp;
#endif //PHARMAROBOT_1_0_H
