// WetTemp.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include "EnvContext.h"
#include "maplayer.h"
#include "EnvModel.h"



#ifdef _MANAGED
#error Please read instructions in WetTemp.cpp to compile with /clr
// If you want to add /clr to your project you must do the following:
//	1. Remove the above include for afxdllx.h
//	2. Add a .cpp file to your project that does not have /clr thrown and has
//	   Precompiled headers disabled, with the following text:
//			#include <afxwin.h>
//			#include <afxdllx.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static AFX_EXTENSION_MODULE WetTempDLL = { NULL, NULL };

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// The autonomous process API
//extern "C" BOOL PASCAL EXPORT APRun(EnvContext *pExtraData );
//extern "C" BOOL PASCAL EXPORT APInitRun(EnvContext*, bool useInitialSeed );
//extern "C" BOOL PASCAL EXPORT APInit( EnvContext *pExtraData, CString * );
//extern "C" BOOL PASCAL EXPORT APShowProperties( CWnd *pParent, int id );
//extern "C" int  PASCAL EXPORT APInputVar( int, MODEL_VAR** );




// The evaluative model API
extern "C" BOOL PASCAL EMRun( EnvContext *pExtraData );
extern "C" BOOL PASCAL EMShowProperties( CWnd *pParent );
extern "C" BOOL PASCAL EMInitRun( EnvContext *pExtraData, bool useInitialSeed );
extern "C" BOOL PASCAL EMInit(EnvContext*, CString * );
extern "C" int  PASCAL EMInputVar( int, MODEL_VAR** );



extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("WetTemp.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(WetTempDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(WetTempDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("WetTemp.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(WetTempDLL);
	}
	return 1;   // ok
}

#ifdef _MANAGED
#pragma managed(pop)
#endif



//-------------------
int  PASCAL EMInputVar( int modelID, MODEL_VAR** modelVar )
{
/*   if (modelID == 0)
   {
      *modelVar = (MODEL_VAR*)&gResModelVar;
      return MV_N_RESCAP;
   }
   *modelVar = NULL; */
   return 0;
}

//----------------------------------------------------------------
BOOL PASCAL EMInit(EnvContext* pEnv, CString * iStr)  
   {  
   //EM_AP_state |= EM_INIT;

   //MAPLAYER_NODATA_VALUE = pEnv->pMapLayer->GetNoDataValue();

   // LULC and UGB
   ASSERT(0 == pEnv->year && 0 == pEnv->runID);

   MODEL_INFO * pInfo = static_cast<MODEL_INFO*>(pEnv->pProcessInfo);
   ASSERT(pInfo->id == pEnv->id && pInfo->col == pEnv->col);  

   if (0 == pInfo->id)
      {
     
	   /*
      // Get a new EnvResidentialCapacity::EnvResidentialCapacity(const vector<EP_resState> & cap)
      ASSERT(NULL == gpEnvCap);

      if (NULL == (gpEnvCap = new EnvResidentialCapacity(pEnv) ))
         return false;
      
      if (0 != initDbCols( pEnv) )
         return false;
*/
      }

	return true;   
	}


//----------------------------------------------------------------
BOOL PASCAL EMInitRun( EnvContext *pEnv, bool useInitialSeed ) 
{ 
   //EM_AP_state |= EM_INITRUN;
   //EM_AP_state &= ~EM_RUN;

   //assert(! (EM_AP_state & AP_INITRUN));

   MODEL_INFO * pInfo = static_cast<MODEL_INFO*>(pEnv->pProcessInfo);
   if (pInfo->id == 0)
   {
      return true; 
   }

   // We have yet to balance the metagoals...to be done next in APInitRun
   // We have yet to initialize the ResStateVector counters ... also APInitRun

   return true;
}

//---------------------------------------------
BOOL PASCAL EMRun( EnvContext *pEnv )
{
   //EM_AP_state |= EM_RUN;
   //EM_AP_state &= ~EM_INITRUN;

   // Calculate the overall residential capacity score as an avg of the 
   // component res cap models.  Thus modelId=0 calculate all.
   MODEL_INFO * pInfo = static_cast<MODEL_INFO*>(pEnv->pProcessInfo);
/*
   map<int, EP_resState*>::iterator it;
   if (0 == pInfo->id)
   {
      assert(0 == pEnv->id);
      double overallResCapScr = 0.0;
      gpEnvCap->checkExpansionCondition(gpEnvPop->obj->current(), &overallResCapScr);
      pEnv->score = static_cast<float>(overallResCapScr);
      //pEnv->score = static_cast<float>(gpEnvCap->meanScarcity());
      return true;
   }
   
   it = gpEnvCap->modelIdToResState.find(pInfo->id);
   assert(it != gpEnvCap->modelIdToResState.end());
   pEnv->score = static_cast<float>(it->second->scarcity());
 */
   return true;
}
//=======================================================================================
BOOL PASCAL EXPORT EMShowProperties( CWnd *pParent, int id )
{
   //LogHrt();
   return true;
}
