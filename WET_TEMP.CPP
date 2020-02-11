// WET_Temp.cpp: implementation of the WET_Temp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "WET_Temp.h"

#include <unitconv.h>
#include <math.h>
#include <geometry.hpp>
#include <tchar.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <map.h>
#include <maplayer.h>
#include <randgen\randunif.hpp>


#include "mainfrm.h"

extern CMainFrame *gpMain;

WET_Temp *gpTempModel = NULL;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WET_Temp::WET_Temp()  //Calls the constructor, defined in the header
: m_pMap( NULL ),         //initializes m_pMap and m_pSlopeLayer to a null pointer
                      //these are protected members of the WET_Temp class - available     
  m_solarAltitude( 0.0f ),
  m_azimuth( 0.0f ),
  m_etRad( 0.0f ),
  m_surfaceRad( 0.0f ),
  m_diffuseRad( 0.0f ),
  m_beamRad( 0.0f ), 
  m_declination( 0.0f ),
  m_pathLength( 0.0f ),
  m_shadeDensity( 0.0f ),
  m_waterDensity( 1000.0f ),		   // kg/m^3
  m_waterHeatCapacity ( 4.185f ),	// kJ/kg*K  (@ 20 C  Geankopolis pg856)
  m_bowenRatio ( 0.0f ),
  m_airTemp( 0.0f ),
  m_relativeHumidity( 0.0f ),
  m_dailyHighAirT( 0.0f ),
  m_dailyLowAirT( 0.0f ),
  m_latInTotal( 0.0f ),
  m_volumeTotal( 0.0f ),
  m_reachLat( 0.0f ),
  m_coverType( 1 ),
  m_noDataValue( -99 ),
  m_pCellLayer( NULL ),
  m_pStreamLayer( NULL ),
  m_pChannelLayer( NULL ),
  m_pDEMLayer ( NULL ),
  m_pSubnodeLayer ( NULL ),
  m_pObservedLayer ( NULL ),
  m_pWithdrawalsLayer ( NULL ),
  m_pFlowLayer( NULL ),  
  m_k1( NULL ),
  m_k2( NULL ),
  m_k3( NULL ),
  m_k4( NULL ),
  m_y ( NULL ),
  m_sv( NULL ),
  m_bed( NULL ),
  m_svCount ( 0 ),

  m_pClimateData( NULL ),
  m_pHabitatData( NULL ),
  m_pSensData( NULL ),
  m_pTemperatureData( NULL ),
  m_pHeatFlux( NULL ),
  m_pSolarFlux( NULL ),
  m_pEvapFlux ( NULL ),
  m_pLongwaveFlux( NULL ),
  m_pConvectionFlux( NULL ),
  m_pAdvectionFlux( NULL ),
  //m_pDataStore( NULL ),

  m_timeStep( 0.02f ),   // day;
  m_startTime( 205.0f ),  // day
  m_stopTime( 210.01f ),  // day
  m_iterations( 1 ),
  m_outputTimeStep( 0.01f ), // day
  m_maxReachDeltaX( 100.0f ), // meters
  m_preprocessingTime( 0.0f ),  //seconds??
  m_runTime( 0.0f ),
  m_bedFlux( 0.0f ),
  m_bedFluxCounter( NULL ),
  m_mouthIndex( 0 ),
  m_dayOfYear( 0 ),
  m_percentIncreaseSummerFlow( 0.03f ),
  m_maxBufferWidth( 0 ),
  m_wdDecrease( 0 ),
  m_flowIncrease( 0 ),
  m_wetlandLowFlowIncrease( 0.25 ),

  //RK integration parameters
  m_Ecurrent( 0.0f ),
  m_Eflux( 0.0f ),
  
  m_options( WT_OPTION_NONE ),
  m_useAreaCol( -1 ),
  m_colArea(-1),
  m_colSlope( -1 ),
  m_colWatershedID( -1 ),
  m_colStreamID( -1 ),
  m_colWDratio( -1 ),
  m_colTemp( -1 ),
  m_colTempDelta( -1 ),
  m_colLulc( -1 ),
  m_colAltern( -1 ),
  m_colAltLulc( -1 ),
  m_colRate( -1 ),
  m_colHeightCode( -1 ),
  m_colDensityCode( -1 ),
  m_colLanduseClass( -1 ),
  m_colSystem( -1 ),
  m_colLength( -1 ),
  m_colDepth( -1 ),
  m_colWidth( -1 ),
  m_colDataPath( -1 ),
  m_derivative( NULL ),
  //m_svTemp( NULL ),
  m_stateVar( NULL ),
  m_notifyFn( NULL ),
  m_extra( 0 ),
  m_randUnif()
   {                      //within the hydro model and any children of it.
   gpTempModel = this;
   //LoadParams( "params.wt" );

   m_stopTime -= (2.0f * m_timeStep);  // move to 11:00pm.
   }



WET_Temp::~WET_Temp()  //The destructor
   {
   //SaveParams( "params.wt");
 
   if ( m_pClimateData != NULL )
      delete m_pClimateData;

   if ( m_pHabitatData != NULL )
      delete m_pHabitatData;

   if ( m_pSensData != NULL )
      delete m_pSensData;

   if ( m_pTemperatureData != NULL )
      delete m_pTemperatureData;

   if ( m_pHeatFlux != NULL )
      delete m_pHeatFlux;

   if ( m_pSolarFlux != NULL )
      delete m_pSolarFlux;

   if ( m_pEvapFlux != NULL )
      delete m_pEvapFlux;

   if ( m_pLongwaveFlux != NULL )
      delete m_pLongwaveFlux;

   if ( m_pConvectionFlux != NULL )
      delete m_pConvectionFlux;

   if ( m_pAdvectionFlux != NULL )
      delete m_pAdvectionFlux;

   if ( m_k1 != NULL )
      delete [] m_k1;

   if ( m_k2 != NULL )
      delete [] m_k2;

   if ( m_k3 != NULL )
      delete [] m_k3;

   if ( m_k4 != NULL )
      delete [] m_k4;

   if ( m_y != NULL )
      delete [] m_y;

   if ( m_sv != NULL )
      delete [] m_sv;   
   
   if ( m_bed != NULL )
      delete [] m_bed;
   
   if (m_bedFluxCounter != NULL)
      delete [] m_bedFluxCounter;
 
   for (int i = 0; i< m_pLongitudinalDataArray.GetSize(); i++)
      delete m_pLongitudinalDataArray[i];
  
   m_cellInfoArray.Clear();

   // Note:  REACH_INFO_TEMP's and SUBNODE_INFO_TEMP's are deallocated automatically by their array containers

   }


bool WET_Temp::SaveParams( LPCSTR filename )
   {
   FILE *fp;
   fopen_s( &fp, filename, "wt");
   
   if ( fp == NULL )
      {
      CString msg( "Unable to find file " );
      msg += filename;
      ErrorMsg( msg );
      return false;
      }

   fprintf( fp, "%g %g %g %g %g %g %g %g %g", m_gwTemp, m_evapA, m_evapB, m_tempInit, m_manningN, m_cloudFrac, m_wdRatio, m_volX, m_extinct );
   
   fclose(fp);
   return true;
   }


bool WET_Temp::LoadParams( LPCSTR filename )
   {
   FILE *fp;
   fopen_s( &fp, filename, "rb" );
   
   if ( fp == NULL )
      {
      CString msg( "Unable to find file " );
      msg += filename;
      ErrorMsg( msg );
      return false;
      }

   int count = fscanf_s( fp, "%f %f %f %f %f %f %f %f %f", &m_gwTemp, &m_evapA, &m_evapB, &m_tempInit, &m_manningN, &m_cloudFrac, &m_wdRatio, &m_volX, &m_extinct );

   fclose(fp);
   return true;
   }



int WET_Temp::LoadClimateData( LPCSTR filename )
   {
   if ( m_pClimateData != NULL )  // is there already a climate data obj?
      delete m_pClimateData;

   m_pClimateData = new FDataObj;   // create a new one
   ASSERT( m_pClimateData != NULL );

   int records = (int) m_pClimateData->ReadAscii( filename, ',', TRUE );

   return records;
   }



int WET_Temp::GetReachCount( void )
   {
   if ( m_pStreamLayer == NULL )
      return -1;
   else
      return m_pStreamLayer->GetRecordCount();
   }



int WET_Temp::GetCellCount()
   {
   if ( m_pCellLayer == NULL )
      return -1;
   else
      return m_pCellLayer->GetRecordCount();
   }


int WET_Temp::GetCellReachID( int cell )
   {
   if ( m_pCellLayer == NULL )
      return -1;

   int reachID;
   bool ok = m_pCellLayer->GetData( cell, m_colWatershedID, reachID );
   ASSERT( ok );

   return reachID;
   }



float WET_Temp::GetCellArea( int cell )
   {
   if ( m_pCellLayer == NULL )
      return -1;

   float area;
   bool ok = m_pCellLayer->GetData( cell, m_colArea, area );
   ASSERT( ok );

   return area;
   }



float WET_Temp::GetCellSlope( int cell )
   {
   if ( m_pCellLayer == NULL )
      return -1;

   float slope;
   bool ok = m_pCellLayer->GetData( cell, m_colSlope, slope );
   ASSERT( ok );

   return slope;
   }



int WET_Temp::GetCellLulc( int cell, bool alt )
   {
   if ( m_pCellLayer == NULL )
      return -1;

   int lulc = -1;
   bool ok;
   
   if (alt == TRUE)
      {
      if ( m_options & WT_OPTION_USE_AREA_COL && m_useAreaCol >= 0 )
            {
            float thresholdArea;
            m_pCellLayer->GetData( cell, m_useAreaCol, thresholdArea );

            if ( thresholdArea >= 0.0f )
               ok = m_pCellLayer->GetData( cell, m_colAltLulc, lulc );
            }
      else
         ok = m_pCellLayer->GetData( cell, m_colAltLulc, lulc );
      }
   else
      ok = m_pCellLayer->GetData( cell, m_colLulc, lulc );
      
   ASSERT( ok );
   return lulc;
   }


float WET_Temp::GetRESTOREBufferWidth( int cell )
   {
   float altArea = 0.0f, streamLength = 0.0f, bufferWidth = 0.0f;
   bool ok, ok2;
   
   ok = m_pCellLayer->GetData( cell, m_colAltArea, altArea );
   ASSERT( ok );

   ok2 = m_pCellLayer->GetData( cell, m_colStreamLength, streamLength );
   ASSERT( ok2 );

   if ( streamLength <= 0.0f )
      bufferWidth = 0.0f;
   else
      bufferWidth = altArea/(2 * streamLength);
   
   return bufferWidth;
   }


int WET_Temp::GetCellHeight( int cell )
   {
   if ( m_pCellLayer == NULL )
      return -1;

   int heightCode;
   bool ok = m_pCellLayer->GetData( cell, m_colHeightCode, heightCode );
   ASSERT( ok );

   return heightCode;
   }


int WET_Temp::GetCellDensity( int cell )
   {
   if ( m_pCellLayer == NULL )
      return -1;

   int densityCode;
   bool ok = m_pCellLayer->GetData( cell, m_colDensityCode, densityCode );
   ASSERT( ok );

   return densityCode;
   }


CString WET_Temp::GetCellLanduseClass( int cell )
   {
   CString landuse;
   bool ok = m_pCellLayer->GetData( cell, m_colLanduseClass, landuse );
   ASSERT( ok ); 
   
   return landuse;
   }


CString WET_Temp::GetCellSystem( int cell )
   {
   CString system;
   bool ok = m_pCellLayer->GetData( cell, m_colSystem, system );
   ASSERT( ok ); 
   
   return system;
   }

REACH_INFO_TEMP *WET_Temp::GetReachInfoTempFromReachID(int reachID)
   {
   int reachCount = GetReachCount();   // doesn't include root node (last node) or phantom nodes
	
   for ( int i=0; i < reachCount; i++ )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
      int thisID = pNode->m_reachInfo.reachID;
      if (thisID == reachID)   //We've found it!
         {
         REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
         return pReachInfoTemp;
         }
      }
   return NULL;
   }


void WET_Temp::RestorationDischargeAdjustments()
   {
   int altern = 0, reachID = 0;
   float rate = 0.0f, wetlandArea = 0.0f;
   
   // Cycle through all cells.  If increase summer flows alternative has been placed
   // set incSummerFlow to TRUE in the corresponding reach
   for ( int i = 0; i < m_pCellLayer->GetRecordCount(); i++)
      {
      if ( m_options & WT_OPTION_USE_AREA_COL && m_useAreaCol >= 0 )
         {
         float thresholdArea;
         m_pCellLayer->GetData( i, m_useAreaCol, thresholdArea );

         if ( thresholdArea < 0.0001f )
            continue;
         }

      bool ok = m_pCellLayer->GetData( i, m_colAltern, altern );
      ASSERT( ok );

      bool ok2 = m_pCellLayer->GetData( i, m_colRate, rate );
      ASSERT( ok );

      bool ok3 = m_pCellLayer->GetData( i, m_colAltArea, wetlandArea);
      ASSERT( ok );
      
      reachID = GetCellReachID(i);

      if ( altern == 405 ) // Reallocate Water Rights
         {
         REACH_INFO_TEMP *pTemp = GetReachInfoTempFromReachID(reachID);
         pTemp->sumDiversions += rate;
         }//end if

      if ( altern == 109 ) //Increase Late Summer Flow
         {
         REACH_INFO_TEMP *pTemp = GetReachInfoTempFromReachID(reachID);
         pTemp->incSummerFlow = TRUE;
         } //end if
      if ( altern == 105 ) //Wetlands Restoration
         {
         REACH_INFO_TEMP *pTemp = GetReachInfoTempFromReachID(reachID);
         pTemp->wetlandArea += wetlandArea;
         } //end if
      }  // end i loop

   int reachCount = GetReachCount();
   float *flowIncArray   = new float[ reachCount ]; 
   float *cumFlowIncArray = new float[ reachCount ];
   
   //Initialize array values to zero
   for ( int a=0; a < reachCount; a++ )
      {
      flowIncArray[a] = 0.0f;
      cumFlowIncArray[a] = 0.0f;
      }
   
   for ( int n=0; n < reachCount; n++ )
      {
      //REACH_INFO_TEMP *pReachInfo = m_reachInfoTempArray[ n ];
      ReachNode *pNode = m_reachTree.m_nodeArray[ n ];
      REACH_INFO_TEMP *pReachInfo = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      //Cycle through each reach.  If increase summer flows is true.  Calculate the amount of this increase.
      if ( pReachInfo->incSummerFlow )
         {
         //increase calculated with respect to discharge in the first subnode
         float increase = m_percentIncreaseSummerFlow * pNode->m_reachInfo.qArray[0];  
         flowIncArray[n] += increase; 
         }
      if (pReachInfo->sumDiversions > 0.0f)
         {
         float reallocation = pReachInfo->sumDiversions / 35.29f ;  //Sum diversions converted from cfs to m^3/s and added to discharge.
         flowIncArray[n] += reallocation; 
         }
      if (pReachInfo->wetlandArea > 0.0f)
         {
         //Augment discharge based on ratio of restored wetland area to drainage area
         //Maximum discharge increase is 25% (if 100% of drainage area is restored to wetlands)
         float percentAugment = pReachInfo->wetlandArea / ( m_wetlandLowFlowIncrease * pNode->m_reachInfo.areaDrained );  //Sum diversions converted from cfs to m^3/s and added to discharge.
         float wetlandFlowAugment = pNode->m_reachInfo.qArray[0] * percentAugment;
         flowIncArray[n] += wetlandFlowAugment; 
         }
      
      }      
        
   //Now that we have marked reaches for increased later summer flow and calculated the value of those increases, 
   //we must traverse downward through the tree and apply these increases cumulatively.

   //ReachNode *pRoot = m_reachTree.m_pRoot;
      
   ComputeCumulativeFlowIncrease( m_reachTree.m_pRoot, flowIncArray, cumFlowIncArray);

   //Distribute flow increases
   for( int m=0; m < reachCount; m++ )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ m ];
      int subnodeCount =  pNode->m_reachInfo.subnodeCount;
      float cumIncrease = cumFlowIncArray[m];
      for ( int j=0; j < subnodeCount; j++ )
          pNode->m_reachInfo.qArray[j] += cumIncrease;
      }   
      
   delete [] flowIncArray;
   delete [] cumFlowIncArray;
   }


float WET_Temp::ComputeCumulativeFlowIncrease(ReachNode *pNode, float *flowIncArray, float *cumFlowIncArray)
   {
   float cumFlowInc = 0.0f;

   if ( pNode->m_pLeft != NULL )
      cumFlowInc += ComputeCumulativeFlowIncrease(pNode->m_pLeft, flowIncArray, cumFlowIncArray);

   if ( pNode->m_pRight != NULL )
      cumFlowInc += ComputeCumulativeFlowIncrease(pNode->m_pRight, flowIncArray, cumFlowIncArray);

   int index = pNode->m_reachInfo.index;
   ASSERT( index < m_reachTree.m_nodeCount-1 );

   if ( pNode->IsPhantomNode() == false && pNode->IsRootNode() == false)
      {
      cumFlowInc += flowIncArray[ index ];
      cumFlowIncArray[ index ] = cumFlowInc;
      }

     return cumFlowInc;

   }


void WET_Temp::StoreOriginalDischargeEstimates()
   {
   int reachCount = GetReachCount();   // doesn't include root node (last node) or phantom nodes
	
   for (int i=0; i < reachCount; i++)
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[i];
      REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      
      int subnodeCount = pNode->m_reachInfo.subnodeCount;
         
      for (int j=0; j <= subnodeCount-1; j++)
         {
         SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfoTemp->subnodeInfoTempArray[j];
         pSubnodeInfo->streamFlowOriginal = pNode->m_reachInfo.qArray[j];
         }   
   
      }
   }

/////////////////////////////////////////////////////////////////////////////
///////																		         ////////
///////																		         ////////
///////				** START PREPROCESSING CODE **							////////
///////																		         ////////
///////																		         ////////
/////////////////////////////////////////////////////////////////////////////

MapLayer	*WET_Temp::SetStreamLayer( MapLayer *pLayer )
   {
   m_pStreamLayer = pLayer;
   
	if ( m_pStreamLayer == NULL )
      return NULL;   // next, do stream layer column mappings

   ASSERT( m_pStreamLayer != NULL );
   DataObj *pStreamData = m_pStreamLayer->m_pData;
   ASSERT( pStreamData != NULL );

   m_colStreamID = pStreamData->GetCol("HYDRO_ID");//This gets the stream id (from
   ASSERT( m_colStreamID >= 0 );                    // the stream layer, NOT the matrix)

   m_colWDratio = pStreamData->GetCol("WD_Ratio");
/*
   m_colTemp = pLayer->GetFieldCol( "MAXTEMP" );
   if ( m_colTemp < 0 )
      m_colTemp = pLayer->m_pDbTable->AddField( "MAXTEMP", TYPE_FLOAT );

   m_colTempDelta = pLayer->GetFieldCol( "DELTA_T" );
   if ( m_colTempDelta < 0 )
      m_colTempDelta = pLayer->m_pDbTable->AddField( "DELTA_T", TYPE_FLOAT );
*/
   //Don't assert, because if the wdratio column doesn't exist, its OK.  
   //It will be populated by the m_wdRatio value 
   
   //Add z coordinate to stream layer for slope calculations
   //if ( ! m_pStreamLayer->m_3D )
    //m_pStreamLayer->Project3D(m_pDEMLayer, true);
/*
      CString msg;
      msg += "File ";
      msg += m_pStreamLayer->m_path;
      msg += " has been reprojected, so you want to save it?";

      if ( AfxMessageBox( msg, MB_YESNO ) == IDYES )
         m_pStreamLayer->SaveShapeFile( m_pStreamLayer->m_path, false );  // temporary
  */ 
   // have stream process it for run-invariant stuff

   //gpMain->SetStatus( "Building Tree..." );    
	int nodeCount = m_reachTree.BuildTree( m_pStreamLayer );  // nodes does NOT include phantoms, does include extra root node

   switch ( nodeCount )
      {
      case -1:
         ErrorMsg( "No stream reach layer has been loaded..." );
         return NULL;

      case -2: 
         ErrorMsg( "No vectors in stream coverage..." );
         return m_pStreamLayer;

      case -3:
         ErrorMsg( "Too many downstream nodes - 1 max allowed..." );
         return m_pStreamLayer;

      case -4:
         ErrorMsg( "Topology error..." );
         return m_pStreamLayer;

      case -5:
         ErrorMsg( "Too many unresolved downstream node pointers found while building reach tree..." );
         return m_pStreamLayer;

      case -6:
         ErrorMsg( "No streams upstream of the root node found while building reach tree..." );
         return m_pStreamLayer;
      }
      
   // set the size of the reachInfoTemp array, one for each reach node
   m_reachInfoTempArray.SetSize( nodeCount-1 ); // used to be nodeCount, jpb 11/12/02

   // tree is built, initialize reach nodes
   //gpMain->SetStatus( "Building reach subnodes..." );

   for ( int i=0; i < nodeCount-1; i++ )
      {
      // allocate a node
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

      // initialize its contents
      // for each REACH_INFO, (except root), make a REACH_INFO_TEMP extension
      REACH_INFO_TEMP *pTemp = new REACH_INFO_TEMP;
      pNode->m_reachInfo.pData = (void*) pTemp;      // set the REACH_INFO data to point to these
      m_reachInfoTempArray[ i ] = pTemp;             // add to our internal store

      int reachID;
      bool ok = m_pStreamLayer->GetData( i, m_colStreamID, reachID );
      ASSERT( ok );
      pNode->m_reachInfo.reachID = reachID;  // hydro_id for this reach

      float wdRatio;
      bool ok2 = false;
      if ( m_colWDratio >= 0 )  //Is WD Ratio information in the shapefile??
         ok2 = m_pStreamLayer->GetData( i, m_colWDratio, wdRatio );    //Then go get it!
      
      if ( ok2 )  //Did we find a wdRatio column?
         pTemp->wdRatio = wdRatio;  // then store wdRatio for this reach
      else
         pTemp->wdRatio = m_wdRatio;  //No wdRatio column in shapefile, so set to default value from model user
      // all done with this REACH_INFO_TEMP struct, upslope stucts are allocated in InitRun();
         
      }

   //Variables to measure the time of preprocessing
   time_t startpp, endpp;
   time (&startpp);
   
   //CreateSubnodes( shapeFileName );
   CreateSubnodePtLayer( "SubNodes" );

   //Estimate discharge for each reach in the basin.
   m_reachTree.InterpolateSubnodes(m_pCellLayer, m_pFlowLayer, m_pWithdrawalsLayer, m_pStreamLayer);

   //Store original discharge estimates
   StoreOriginalDischargeEstimates();
   
   //initialize the reach/subnode corresponding to observed values
   if (m_pObservedLayer != NULL)
      InitObsValueInfo( );

   //NSDZ width, Rosgen Stream Type, gradient, sinuosity etc... for each reach
   InitChannelParameters();

   PopulateSubnodes( m_pStreamLayer->m_path );    // use stream layer's name as a base, look for matching ".wt"
   
   //gpMain->SetStatus( "Done..." );
   
   //Preprocessing finished.  Record the time it took (m_preprocessingTime)
   time(&endpp);
   float m_preprocessingTime = (float) difftime(startpp, endpp);

	return m_pStreamLayer;
   }


MapLayer	*WET_Temp::SetCellLayer( MapLayer *pLayer )
   {
   if ( pLayer == NULL )
      return NULL;

   m_pCellLayer = pLayer;

   DataObj *pCellData = m_pCellLayer->m_pData;
   ASSERT( pCellData != NULL );

   //Right now only two types of vegetation coverages used.  More can be accommodated.
   m_colLulc = pCellData->GetCol("LULC_C");
   m_colHeightCode = pCellData->GetCol("Height");

   if ( m_colLulc >= 0 ) //Is there an LULC column??
      {
      m_coverType = 1;  //Code for RESOTRE/PNWERC Coverages
      
      m_colAltern = pCellData->GetCol("ALTERN");  //RESOTRE coverages only (alternative LULC value)
      if (m_options & WT_OPTION_USE_ALT_LULC )    //
         ASSERT( m_colAltern >= 0 );    //Did it work??

      m_colAltLulc = pCellData->GetCol("ALT_LULC");  //RESOTRE coverages only (alternative LULC value)
      if (m_options & WT_OPTION_USE_ALT_LULC )
         ASSERT( m_colAltLulc >= 0 );    //Did it work??

      m_colAltArea = pCellData->GetCol("ALT_AREA");  //RESOTRE coverages only (alternative LULC value)
      if (m_options & WT_OPTION_USE_ALT_LULC )
         ASSERT( m_colAltArea >= 0 );    //Did it work??
      
      m_colStreamLength = pCellData->GetCol("STRLENGTH");  //RESOTRE coverages only (alternative LULC value)
      if (m_options & WT_OPTION_USE_ALT_LULC )
         ASSERT( m_colStreamLength >= 0 );    //Did it work??
 
      m_colWatershedID = pCellData->GetCol("HYDRO_ID");
      ASSERT( m_colWatershedID >= 0);

      m_colRate = pCellData->GetCol( "RATE_CFS" );       //RESOTRE coverages only (alternative LULC value)
      if (m_options & WT_OPTION_USE_ALT_LULC )
         ASSERT( m_colRate >= 0 );
      }
   else if ( m_colHeightCode >= 0 )//Is there a height column??
      {
      m_coverType = 2;  //Code for ODEQ Coverages
      m_colDensityCode = pCellData->GetCol("Canopy");
      m_colLanduseClass = pCellData->GetCol("Class");
      m_colSystem = pCellData->GetCol("System");
      ASSERT( m_colDensityCode >= 0 );
      }
   else
      {
      ErrorMsg( "Unrecognized landuse information provided...." );
      }
  
   // allocate cell info structures
   m_cellInfoArray.SetSize( m_pCellLayer->GetRecordCount() );

   for ( int i=0; i < m_pCellLayer->GetRecordCount(); i++ )
      m_cellInfoArray[ i ] = new CELL_INFO;

   return m_pCellLayer;
   }


MapLayer	*WET_Temp::SetDEMLayer( MapLayer *pLayer )
   {
   if ( pLayer == NULL )
		return NULL;

   m_pDEMLayer = pLayer;	
   m_pDEMLayer->Hide();
   return m_pDEMLayer;
   }

MapLayer	*WET_Temp::SetFlowLayer( MapLayer *pLayer )
   {
   return m_pFlowLayer = pLayer;
   }


MapLayer *WET_Temp::SetObservedLayer( MapLayer *pLayer )
   {
   m_pObservedLayer = pLayer;

   if ( pLayer == NULL )
      return NULL;

   DataObj *pObservedData = m_pObservedLayer->m_pData;
   ASSERT( pObservedData != NULL );

   m_colDataPath = pObservedData->GetCol("Path");  //Gets Path column from observed layer
   
   return m_pObservedLayer;
   }


MapLayer   *WET_Temp::SetWithdrawalsLayer( MapLayer *pLayer )
   {
   return m_pWithdrawalsLayer = pLayer;
   }


MapLayer   *WET_Temp::SetChannelLayer( MapLayer *pLayer )
   {
   m_pChannelLayer = pLayer;

   if (m_pChannelLayer == NULL )
      return NULL;

   // create a spatial index if needed (for GetNearbyPolys())
   //m_pChannelLayer->CreateSpatialIndex( NULL, 32, 120.f, SIM_NEAREST );

   DataObj *pHabitatData = m_pChannelLayer->m_pData;
   ASSERT( pHabitatData != NULL );

   m_colWidth = pHabitatData->GetCol("cor_width");
   ASSERT( m_colWidth >= 0 );

   m_colDepth = pHabitatData->GetCol("depth");
   ASSERT( m_colDepth >= 0 );

   m_colLength = pHabitatData->GetCol("cor_length");
   ASSERT( m_colLength >= 0 );

   return m_pChannelLayer;
   }


MapLayer *WET_Temp::SetCellLayer( LPCSTR shapeFileName )
   {
   bool temp = VData::m_useWideChar;
   VData::m_useWideChar = false;
   MapLayer *pLayer = m_pMap->AddShapeLayer( shapeFileName, true );  // DO load data as well...
   VData::m_useWideChar = temp;
   return SetCellLayer( pLayer );
   }


MapLayer *WET_Temp::SetDEMLayer ( LPCSTR gridFile )
	{
   MapLayer *pLayer = m_pMap->AddGridLayer( gridFile, DOT_FLOAT);
   return SetDEMLayer( pLayer );
	}


MapLayer *WET_Temp::SetObservedLayer( LPCSTR shapeFileName )
   {
   bool temp = VData::m_useWideChar;
   VData::m_useWideChar = false;
   MapLayer *pLayer = m_pMap->AddShapeLayer(shapeFileName, true);
   VData::m_useWideChar = temp;
   return SetObservedLayer( pLayer );
   }


MapLayer *WET_Temp::SetFlowLayer( LPCSTR shapeFileName )
   {
   MapLayer *pLayer = m_pMap->AddShapeLayer(shapeFileName, true);
   return SetFlowLayer( pLayer );
   }


MapLayer *WET_Temp::SetWithdrawalsLayer( LPCSTR shapeFileName )
   {
   MapLayer *pLayer = m_pMap->AddShapeLayer(shapeFileName, true);
   return SetWithdrawalsLayer( pLayer );
   }


MapLayer *WET_Temp::SetChannelLayer( LPCSTR shapeFileName )
   {
   MapLayer *pLayer = m_pMap->AddShapeLayer(shapeFileName, true);
   return SetChannelLayer( pLayer );
   }


MapLayer *WET_Temp::SetStreamLayer( LPCSTR shapeFileName )
   {
   MapLayer *pLayer = m_pMap->AddShapeLayer( shapeFileName, true );  // DO load data as well...
   return SetStreamLayer( pLayer );
   }


bool WET_Temp::CreateSubnodePtLayer( LPCSTR shapeFileName )
   {
   m_reachTree.CreateSubnodes( m_pStreamLayer, shapeFileName, m_maxReachDeltaX );
   
   InitSubnodeStructures();
   
   ASSERT( m_pMap != NULL );

   int totalSubnodeCount = 0;
	int reachCount = GetReachCount();   // doesn't include root node (last node) or phantom nodes
	for ( int i=0; i < reachCount; i++ )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
      totalSubnodeCount += pNode->m_reachInfo.subnodeCount;  
      }

   if ( m_pSubnodeLayer == NULL )      // has the subnode layer been allocated yet?
      {
      m_pSubnodeLayer = m_pMap->AddLayer( LT_POINT );  //add a point coverage
      ASSERT( m_pSubnodeLayer != NULL );

      m_pSubnodeLayer->CreateDataTable( totalSubnodeCount, 7 );        // reachIndex, subnodeIndex
      m_pSubnodeLayer->SetFieldLabel( 0, "Reach Index" );
      m_pSubnodeLayer->SetFieldLabel( 1, "Subnode Index" );
      m_pSubnodeLayer->SetFieldLabel( 2, "ReachID" );
      m_pSubnodeLayer->SetFieldLabel( 3, "MaxTemp" );
      m_pSubnodeLayer->SetFieldLabel( 4, "Temp Delta" );
      m_pSubnodeLayer->SetFieldLabel( 5, "Flow" );
      m_pSubnodeLayer->SetFieldLabel( 6, "Flow Delta" );

      m_pSubnodeLayer->SetFieldType( 0, TYPE_INT );
      m_pSubnodeLayer->SetFieldType( 1, TYPE_INT );
      m_pSubnodeLayer->SetFieldType( 2, TYPE_INT );
      m_pSubnodeLayer->SetFieldType( 3, TYPE_FLOAT );
      m_pSubnodeLayer->SetFieldType( 4, TYPE_FLOAT );
      m_pSubnodeLayer->SetFieldType( 5, TYPE_FLOAT );
      m_pSubnodeLayer->SetFieldType( 6, TYPE_FLOAT );

      m_pSubnodeLayer->m_name = "Subnodes";
      }

   // initialize layer
   int currentSubnodeIndex = 0;
	for ( int i=0; i < reachCount; i++ )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

      int subnodeCount = pNode->m_reachInfo.subnodeCount;
      int reachID = pNode->m_reachInfo.reachID;
      REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      ASSERT( pReachInfoTemp != NULL );

	  // cycle through each subnode.  Add a point to the shape layer at each subnode
	  for ( int j=0; j < subnodeCount; j++ )
		   {
         SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfoTemp->subnodeInfoTempArray[ j ];

         m_pSubnodeLayer->AddPoint( pNode->m_reachInfo.subnodeInfoArray[j]->location.x, pNode->m_reachInfo.subnodeInfoArray[j]->location.y );	
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 0, i );  // reach index
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 1, j );  // subnode index
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 2, reachID);  //reachID
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 3, 0 );  // max temp    
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 4, 0 );  // temp delta
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 5, 0 );  // flow    
         m_pSubnodeLayer->SetData( currentSubnodeIndex, 6, 0 );  // flow delta

         currentSubnodeIndex++;
		   }  //close subnode loop
      }  //close reach loop
   
   return true;
   }


void WET_Temp::InitSubnodeStructures()
   {
   // cycle through each reach
   int reachCount = GetReachCount();

   for ( int i=0; i < reachCount; ++i )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
      int subnodeCount = pNode->m_reachInfo.CreateSubnodeArray( m_maxReachDeltaX );
      
      REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;


      pReachInfoTemp->subnodeInfoTempArray.SetSize( subnodeCount );

      // cycle through each subnode.  Allocate a new SUBNODE_INFO_TEMP structure to hold data. 
	   for ( int j=0; j < subnodeCount; j++ )
	      {
         SUBNODE_INFO_TEMP *pSubnodeInfo = new SUBNODE_INFO_TEMP; // note: Deallocated automatically once added to array
         
         pReachInfoTemp->subnodeInfoTempArray[ j ] = pSubnodeInfo;
         
         //Store the subnode location in the SUBNODE_INFO_TEMP structure
         pSubnodeInfo->location.x = m_reachTree.m_nodeArray[i]->m_reachInfo.subnodeInfoArray[j]->location.x;
         pSubnodeInfo->location.y = m_reachTree.m_nodeArray[i]->m_reachInfo.subnodeInfoArray[j]->location.y;
         }//close subnode loop
      }//close reach loop
   }


void WET_Temp::PopulateSubnodes( LPCSTR shapeFileName )
	{
   ASSERT( m_pDEMLayer != NULL );
	MapLayer *pGrid = m_pDEMLayer;

   //ASSERT( m_pChannelLayer != NULL );  //OK for habitat layer to be NULL
   MapLayer *pHabitat = m_pChannelLayer;

   FILE *fp;
   char _filename[ 256 ];

   if ( shapeFileName == NULL )
      fp = NULL;
   else
      {
      lstrcpy( _filename, shapeFileName );
      char *dot = strrchr( _filename, '.' );

      ASSERT ( dot != NULL );
      lstrcpy( dot+1, "wt" );
      fopen_s( &fp, _filename, "rb");  // does the file containing preprocessed data already exist?
      }

   if ( fp == NULL )  // no preprocessed file
      {
	   // for each reach node, create a subnode array for numerical purposes
	   int reachCount = GetReachCount();   // doesn't include root node (last node) or phantom nodes
	   for ( int i=0; i < reachCount; ++i )
         {
         ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

         int subnodeCount = pNode->m_reachInfo.subnodeCount;

         REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

         if ( i % 20 == 0 )
            {
            //CString msg;
            //msg.Format( "Initializing Reach %i", i );
            //gpMain->SetStatus( msg );           
            }
           
         // SUBNODE_INFO_TEMP structs have been previouly allocated (InitSubnodeStructures)
         // Now, cycle through and populate the structure with the appropriate subnode-level information
         for( int j=0; j < subnodeCount; j++ )
           {
           SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfoTemp->subnodeInfoTempArray[j];
           InitSubnode( pSubnodeInfo, pGrid, i, j );
	        }
         }

      if ( shapeFileName != NULL )
		   SavePreprocessingData( _filename );
      }  // end of:  if (fp == NULL )

	else  // fp != NULL                 // preprocessing data exists, use it!
      {
      bool ok = ReadPreprocessingData( fp );

      if ( ! ok )
         ErrorMsg( "Error reading preprocessing file" );

      fclose( fp );
	   }
   }


void WET_Temp::ApplyReachByReachRestoration( int reachIndex )
   {
   ReachNode *pNode = m_reachTree.m_nodeArray[ reachIndex ];
   REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      
   int subnodeCount = pNode->m_reachInfo.subnodeCount;
   float wdNew = 0.0f, flowNew = 0.0f;
   float wd;

   for (int j=0; j <= subnodeCount-1; j++)
      {
      SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfoTemp->subnodeInfoTempArray[j];      
      
      wd = pSubnodeInfo->wdRatio;
      
      wdNew = wd * (1.0f - ( m_wdDecrease/100.0f ) );
      
      pSubnodeInfo->wdRatio = wdNew;
      float directStepLength = 0; 
      float diagonalStepLength = 0;
      
      //Potential better way to do this **CODECHANGE**
         //Following four lines compare given restoration buffer widths to find the maximum
         
         for (int d=0; d < 8; d++)  //Iterate through the 8 directions
            {
            float cumBufferWidth = 0.0f;
            for (int n=1; n <= 11; n++ )  //Now across the 11 distance steps in each direction
               {
               if (n <= 2)  //The first two steps are 5m long
                  {
                  directStepLength = 5.0f;
                  diagonalStepLength = (float)sqrt(12.5);
                  }
               else    //The remaining 9 steps are 10m long
                  {
                  directStepLength = 10.0f;
                  diagonalStepLength = 5 * (float)sqrt(2.0);
                  }

               //First two steps are 5m, the remaining 9 steps are 10m
               if (n == 1) 
                  cumBufferWidth = 5.0f;
               else if (n == 2)
                  cumBufferWidth = 10.0f;
               else
                  cumBufferWidth = (10.0f * n) - 10.0f;

            if (m_iterations == 1)  //Then do all the calculating of restoration normally
               {
               int m1 = max(m_agWidth, m_huWidth);
               int m2 = max(m1, m_ssWidth);
               int mfinal = max(m2, m_emWidth);
               m_maxBufferWidth = mfinal;
               }

            if (cumBufferWidth <= m_agWidth && pSubnodeInfo->vegInfo[d].landUse[n-1] == 101)  
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_agHeight;
               pSubnodeInfo->vegInfo[d].vegDensity[n-1] = (float)m_agDen;
               }
                  
            if (cumBufferWidth <= m_emWidth && pSubnodeInfo->vegInfo[d].landUse[n-1] == 102)  
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_emHeight;
               pSubnodeInfo->vegInfo[d].vegDensity[n-1] = (float)m_emDen;
               }
                  
            if (cumBufferWidth <= m_ssWidth && pSubnodeInfo->vegInfo[d].landUse[n-1] == 103)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_ssHeight;
               pSubnodeInfo->vegInfo[d].vegDensity[n-1] = (float)m_ssDen;
               }
            
            if (cumBufferWidth <= m_huWidth && pSubnodeInfo->vegInfo[d].landUse[n-1] == 104)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_huHeight;
               pSubnodeInfo->vegInfo[d].vegDensity[n-1] = (float)m_huDen;
               }
                  
               
            //Change forest heights throughout
            if (pSubnodeInfo->vegInfo[d].landUse[n-1] == 111)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_fo1Height;
               }
            else if (pSubnodeInfo->vegInfo[d].landUse[n-1] == 112)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_fo2Height;
               }
            else if (pSubnodeInfo->vegInfo[d].landUse[n-1] == 113)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_fo3Height;
               }
            else if (pSubnodeInfo->vegInfo[d].landUse[n-1] == 114)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_fo4Height;
               }
            else if (pSubnodeInfo->vegInfo[d].landUse[n-1] == 115)
               {
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = m_fo5Height;
               }
            else
               {
               //Do nothing.  Heights of other landuse classes remain unchanged
               }
          
                     
            //Vegetation shading angles must now be recalculated, due to change in buffer height      
            float shadeAngle;
            float overshade  = pSubnodeInfo->vegInfo[d].vegHeight[n-1]*0.1f;
      
            // If overshade is greater than distToVeg, make overshade just .01 less, so the shade angle function
            // will work and return a value of 89.99 degrees (because the shade angle is truly 90 is this case)
            if (overshade >= pSubnodeInfo->distToVeg[d])
               overshade = pSubnodeInfo->distToVeg[d] - 0.01f;

            if( n == 1 )   // First step.  Must take distance to veg into account
               {
               shadeAngle = (float)atan( (pSubnodeInfo->vegInfo[d].vegHeight[n-1]
                          + pSubnodeInfo->vegInfo[d].heightAdjustment[n-1])
                           / (pSubnodeInfo->distToVeg[d] - overshade) );
               }
            else   
               {
               shadeAngle = (float)atan( (pSubnodeInfo->vegInfo[d].vegHeight[n-1] 
                             + pSubnodeInfo->vegInfo[d].heightAdjustment[n-1]) 
                             /( (pSubnodeInfo->distToVeg[d] - overshade) + directStepLength * (n-1) ) );
               }
            
            //Change any slightly negative values for shade angle to slightly positive
            if (shadeAngle < 0.0f)
               shadeAngle = 0.0001f;   
               
            // Load Vegetation Shading Angle into our structure
		      pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1] = shadeAngle;


            // Is this angle the maximum encountered so far?
		      // if so, it takes the place of old maximum shading angle value
		      if (shadeAngle > pSubnodeInfo->vegInfo[d].maxVegShadeAngle)		
			      pSubnodeInfo->vegInfo[d].maxVegShadeAngle = shadeAngle;

		      // Is this the maximum vegetation density encountered so far?
		      // If so, it takes the place of old maximum vegetation density value
	         if (pSubnodeInfo->vegInfo[d].vegDensity[n-1] > pSubnodeInfo->vegInfo[d].maxVegDensity)
               pSubnodeInfo->vegInfo[d].maxVegDensity = pSubnodeInfo->vegInfo[d].vegDensity[n-1];
            
            //if ( m_maxBufferWidth <= cumBufferWidth )  // quit altering vegHeight if we are past the maximum buffer width
              // break;
            }
         
         //Recalculate average height incorporating applied restoration
         
         //Reset old value first
         pSubnodeInfo->vegInfo[d].averageHeight = 0.0f;
                  
         //Now recalculate average height
         for ( int z=1; z <= 11; z++ )
            {
            float avght = (pSubnodeInfo->vegInfo[d].vegHeight[z-1])/11;
            pSubnodeInfo->vegInfo[d].averageHeight += avght;
            }
         }//end of d loop
      } //end of j loop     
   }//end of function




void WET_Temp::RemoveReachbyReachRestoration()
   {
    //loop thorugh each reach that has had restoration applied
   int restoredReaches = m_restoreReachArray.GetSize();
   
   for( int i=0; i < restoredReaches; i++ )
      {

      int reachIndex = m_restoreReachArray[ i ];

      ReachNode *pNode = m_reachTree.GetReachNode( reachIndex );
      
      REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      
      int subnodeCount = pNode->m_reachInfo.subnodeCount;
      
      for (int j=0; j <= subnodeCount-1; j++)
         {
         SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfoTemp->subnodeInfoTempArray[j];      
     
         pSubnodeInfo->wdRatio = pSubnodeInfo->wdRatioOriginal;
         pSubnodeInfo->streamFlow = pSubnodeInfo->streamFlowOriginal;

         
         for (int d=0; d < 8; d++)  //Iterate through the 8 directions
            {
            for (int n=1; n <= 11; n++ )  //Now across the 11 distance steps in each direction
               {
               
               pSubnodeInfo->vegInfo[d].vegHeight[n-1] = pSubnodeInfo->vegInfo[d].vegHeightOriginal[n-1];
               pSubnodeInfo->vegInfo[d].vegDensity[n-1] = pSubnodeInfo->vegInfo[d].vegDensityOriginal[n-1];
               pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1] = pSubnodeInfo->vegInfo[d].vegShadeAngleOriginal[n-1]; 
               float shadeAngle = pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1];
               
               // Is this angle the maximum encountered so far?
		         // if so, it takes the place of old maximum shading angle value
		         if (shadeAngle > pSubnodeInfo->vegInfo[d].maxVegShadeAngle)		
			         pSubnodeInfo->vegInfo[d].maxVegShadeAngle = pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1];

		         // Is this the maximum vegetation density encountered so far?
		         // If so, it takes the place of old maximum vegetation density value
	            if (pSubnodeInfo->vegInfo[d].vegDensity[n-1] > pSubnodeInfo->vegInfo[d].maxVegDensity)
                  pSubnodeInfo->vegInfo[d].maxVegDensity = pSubnodeInfo->vegInfo[d].vegDensity[n-1];
               }
         
            //Recalculate average height incorporating applied restoration
            for ( int z=1; z <= 11; z++ )
               {
               float avght = (pSubnodeInfo->vegInfo[d].vegHeight[z-1])/11;
               pSubnodeInfo->vegInfo[d].averageHeight += avght;
               }
            }
         }      
      }
   
   m_restoreReachArray.RemoveAll();

   //Temporary check to find shifting output problem (mmc 2/27/03)
   FILE *fp;
   
   fopen_s( &fp, "/wet_data/uppergranderonde/stream.wt", "rb");  // does the file containing preprocessed data already exist?
     
 
   bool ok = ReadPreprocessingData( fp );

   if ( ! ok )
      ErrorMsg( "Error reading preprocessing file" );

   fclose( fp );
	}


         
 
void WET_Temp::InitChannelParameters()
   {
   int reachCount = GetReachCount();
   for ( int i=0; i < reachCount; ++i )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

      REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

      //Calculate bankfull width using drainage area
      float areaDrained = pNode->m_reachInfo.areaDrained;
      
      //change units to km^2. Origninally stated in m^2
      areaDrained = areaDrained/(1000*1000); //km^2
      
      //Calculate and store bank full width    //CODECHANGE this regression is watershed specific
      float bfW = 0.0816f*areaDrained + 6.5f;
      pTemp->bfW = bfW;

      //Determine the gradient of the reach
      float gradient = m_pStreamLayer->m_polyArray[ i ]->CalcSlope();
      pTemp->gradient = gradient;

      //Determine the sinuosity of the reach
      float distance = m_pStreamLayer->m_polyArray[ i ]->CalcDistance();
      float reachLength = pNode->m_reachInfo.length;
      float sinuosity = reachLength/distance;
      pTemp->sinuosity = sinuosity;
      
      //Assign Rosgen Level 1 stream type to each reach
      CString rosgen;
      rosgen = AssignRosgenType(sinuosity, gradient);
      pTemp->rosgen = rosgen;
      
      //Now, using Rosgen stream class, estimate the near stream disturbance zone (NSDZ) width
      float nsdz;
      
      if (rosgen == "A")
         nsdz = bfW;  //m
      else if (rosgen == "B")
         nsdz = bfW + 0.05f*bfW;  //m
      else
         nsdz = bfW + 0.25f*bfW; //m

      pTemp->nsdz = nsdz;
      }  
   }



CString WET_Temp::AssignRosgenType(float sinuosity, float gradient)
   {
   
   //CODECHANGE - Perhaps have these changeable by the model user??
   CString rosgen;
   
   if ( gradient <= 1.0 && sinuosity <= 1.1 )
      rosgen = "F";
   
   else if ( gradient <= 1.0 && 1.1 < sinuosity && sinuosity <= 1.2 )
      rosgen = "F";

   else if ( gradient <= 1.0 && 1.2 < sinuosity && sinuosity <= 1.4 )
      rosgen = "F";

   else if ( gradient <= 1.0 && 1.4 < sinuosity && sinuosity <= 1.5 )
      rosgen = "C";
   
   else if ( gradient <= 1.0 && 1.5 < sinuosity && sinuosity <= 999 )
      rosgen = "C";
   
   else if ( 1.0 < gradient && gradient <= 1.8 && sinuosity && sinuosity <= 1.1 )
      rosgen = "B";

   else if ( 1.0 < gradient && gradient <= 1.8 && 1.1 < sinuosity && sinuosity <= 1.2 )
      rosgen = "G";

   else if ( 1.0 < gradient && gradient <= 1.8 && 1.2 < sinuosity && sinuosity <= 1.4 )
      rosgen = "F";

   else if ( 1.0 < gradient && gradient <= 1.8 && 1.4 < sinuosity && sinuosity <= 1.5 )
      rosgen = "C";

   else if ( 1.0 < gradient && gradient <= 1.8 && 1.5 < sinuosity && sinuosity <= 999 )
      rosgen = "C";

   else if ( 1.8 < gradient && gradient <= 4.5 && sinuosity <= 1.1 )
      rosgen = "A";

   else if ( 1.8 < gradient && gradient <= 4.5 && 1.1 < sinuosity && sinuosity <= 1.2 )
      rosgen = "B";

   else if ( 1.8 < gradient && gradient <= 4.5 && 1.2 < sinuosity && sinuosity <= 1.4 )
      rosgen = "F";

   else if ( 1.8 < gradient && gradient <= 4.5 && 1.4 < sinuosity && sinuosity <= 1.5 )
      rosgen = "E";

   else if ( 1.8 < gradient && gradient <= 4.5 && 1.5 < sinuosity && sinuosity <= 999 )
      rosgen = "E";

   else if ( 4.5 < gradient && gradient <= 8.0 && sinuosity <= 1.1 )
      rosgen = "A";

   else if ( 4.5 < gradient && gradient <= 8.0 && 1.1 < sinuosity && sinuosity <= 999 )
      rosgen = "B";

   else if ( 8.0 < gradient && gradient <= 999 && sinuosity <= 1.1 )
      rosgen = "A";

   else if ( 8.0 < gradient && gradient <= 999 && 1.1 < sinuosity && sinuosity <= 1.2 )
      rosgen = "A";

   else if ( 8.0 < gradient && gradient <= 999 && 1.2 < sinuosity && sinuosity <= 999 )
      rosgen = "B";

   return rosgen;
   }


void WET_Temp::InitSubnode( SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid, int i, int j )
	{
   ReachNode *pNode = m_reachTree.m_nodeArray[i];
  
   pSubnodeInfo->streamFlow =  pNode->m_reachInfo.qArray[j];
   
   pSubnodeInfo->streamAspect		  = StreamAspect( i, j );
   pSubnodeInfo->rightAzimuthAngle = RightAzimuthAngle( pSubnodeInfo );
	pSubnodeInfo->leftAzimuthAngle  = LeftAzimuthAngle( pSubnodeInfo );
	
   CalculateSubnodeSlope( i, j );
   CalcChannelParameters( pSubnodeInfo, i, j );
   
   CalculateTopographicShading( pSubnodeInfo, pGrid );
   CalculateDistanceToVeg( pSubnodeInfo, i );
	InitVegetationInfo( pSubnodeInfo, pGrid );
   
	pSubnodeInfo->topoAngleLeft   = TopoAngleLeft( pSubnodeInfo );
	pSubnodeInfo->topoAngleRight	= TopoAngleRight( pSubnodeInfo );
	VegInfoLeft ( pSubnodeInfo );
	VegInfoRight( pSubnodeInfo );

	pSubnodeInfo->skyOpen			= SkyOpen( pSubnodeInfo );
   }


float WET_Temp::StreamAspect ( int i, int j)
	{
   //This function determines the stream aspect, North being 0 degrees

	//Which reach are we in??
	Poly *pLine = m_pStreamLayer->m_polyArray[ i ];
	
	//Get last vertex in the reach (the first subnode in the next reach).
	//Vertex &vNode = m_reachTree.GetUpstreamVertex( pLine );
	ReachNode *pNode = m_reachTree.m_nodeArray[i];
	float orientAngle;
	int subnodeCount = pNode->m_reachInfo.subnodeCount;

	//Define pointers to this subnode, the upstream subnode and the downstream subnode
	//Points to current subnode information
	SUBNODE_INFO_TEMP *pSubnodeInfo	= m_reachInfoTempArray[ i ]->subnodeInfoTempArray[ j ];
	
   //Points to downstream subnode unless the downstream subnode is the next node
	SUBNODE_INFO_TEMP *pSubnodeInfoDown = NULL;
   if (j != (subnodeCount-1) && subnodeCount != 1)
      pSubnodeInfoDown	= m_reachInfoTempArray[ i ]->subnodeInfoTempArray[ j + 1 ];

   //Location of downstream NODE
   Vertex vDownstreamNode = m_reachTree.GetDownstreamVertex(pLine);

   //Points to upstream subnode unless there is no ustream subnode (i.e. headwaters)
   SUBNODE_INFO_TEMP *pSubnodeInfoUp = NULL;
   if ( j != 0 )
      pSubnodeInfoUp = m_reachInfoTempArray[ i ]->subnodeInfoTempArray[ j - 1 ];

   float upMidx, upMidy, downMidx, downMidy, upMidxLeft, upMidyLeft, upMidxRight, upMidyRight;

   if ( j == 0)
      {
      if ( m_reachTree.IsLeaf(pNode) == true)  //We are at a headwater.  Look only downstream. 
		   {
		   if (pSubnodeInfoDown != NULL) // the headwater reach has more than 1 subnode
            {
            //Find midpoint of downstream reach
            downMidx = GetMidpoint(pSubnodeInfo->location.x, pSubnodeInfoDown->location.x);
            downMidy = GetMidpoint(pSubnodeInfo->location.y, pSubnodeInfoDown->location.y);
            upMidx   = pSubnodeInfo->location.x;
            upMidy   = pSubnodeInfo->location.y;
      
            //Determine the angle between the headwater node and the midpoint
		      float xabs = (float) fabs(pSubnodeInfo->location.x - downMidx);
		      float yabs = (float) fabs(pSubnodeInfo->location.y - downMidy);
		      orientAngle = (float)atan(xabs/yabs);
            }
         if (pSubnodeInfoDown == NULL) //the headwater reach has ONLY 1 subnode (at the node)
            {
            downMidx = GetMidpoint(pSubnodeInfo->location.x, vDownstreamNode.x);
            downMidy = GetMidpoint(pSubnodeInfo->location.y, vDownstreamNode.y);
            upMidx   = pSubnodeInfo->location.x;
            upMidy   = pSubnodeInfo->location.y;
      
            //Determine the angle between the headwater node and the midpoint
		      float xabs = (float) fabs(pSubnodeInfo->location.x - downMidx);
		      float yabs = (float) fabs(pSubnodeInfo->location.y - downMidy);
		      orientAngle = (float)atan(xabs/yabs);
            }
         }
	   else // We are at non-headwater NODE
		   {
		   if (pSubnodeInfoDown != NULL && pNode->m_pRight == NULL) //Node is not at a junction
            {
            int subnodeCountLeft = pNode->m_pLeft->m_reachInfo.subnodeCount;
            
            //Special case!!  Node that is not at a junction
            if (pSubnodeInfoDown != NULL) // the reach has more than 1 subnode
               {
               //Find midpoint of downstream  and upstream reaches
               
               downMidx = GetMidpoint(pSubnodeInfo->location.x, pSubnodeInfoDown->location.x);
               downMidy = GetMidpoint(pSubnodeInfo->location.y, pSubnodeInfoDown->location.y);
               upMidx   = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pLeft->m_reachInfo.subnodeInfoArray[subnodeCountLeft - 1]->location.x);
               upMidy   = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pLeft->m_reachInfo.subnodeInfoArray[subnodeCountLeft - 1]->location.y);
      
               //Determine the angle between the headwater node and the midpoint
		         float xabs = (float) fabs(pSubnodeInfo->location.x - downMidx);
		         float yabs = (float) fabs(pSubnodeInfo->location.y - downMidy);
		         orientAngle = (float)atan(xabs/yabs);
               }
            if (pSubnodeInfoDown == NULL) //the headwater reach has ONLY 1 subnode (at the node)
               {
               downMidx = GetMidpoint(pSubnodeInfo->location.x, vDownstreamNode.x);
               downMidy = GetMidpoint(pSubnodeInfo->location.y, vDownstreamNode.y);
               upMidx   = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pLeft->m_reachInfo.subnodeInfoArray[subnodeCountLeft - 1]->location.x);
               upMidy   = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pLeft->m_reachInfo.subnodeInfoArray[subnodeCountLeft - 1]->location.y);
      
               //Determine the angle between the headwater node and the midpoint
		         float xabs = (float) fabs(pSubnodeInfo->location.x - downMidx);
		         float yabs = (float) fabs(pSubnodeInfo->location.y - downMidy);
		         orientAngle = (float)atan(xabs/yabs);
               }
            }
         if (pSubnodeInfoDown != NULL && pNode->m_pRight != NULL)//Node is at a junction, and the reach has more than 1 subnode
            {
            //Find midpoint of downstream reach
            downMidx = GetMidpoint(pSubnodeInfo->location.x, pSubnodeInfoDown->location.x);
            downMidy = GetMidpoint(pSubnodeInfo->location.y, pSubnodeInfoDown->location.y);
      
            //There are TWO upstream reaches(right and left), therefore we need both upstream midpoitns
            int subnodeCountLeft = pNode->m_pLeft->m_reachInfo.subnodeCount;
            upMidxLeft = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pLeft->m_reachInfo.subnodeInfoArray[subnodeCountLeft - 1]->location.x);
            upMidyLeft = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pLeft->m_reachInfo.subnodeInfoArray[subnodeCountLeft - 1]->location.y);
      
            
            //If m_pRight is a phantom Node, average the two reaches upstream of the 
            //phantom node to determine m_pRight
            if ( pNode->m_pRight->IsPhantomNode() )
               {
               //Make sure there isn't ANOTHER phantom node left or right of the phantom node
               //ASSERT( pNode->m_pRight->m_pLeft->IsPhantomNode() );
               //ASSERT( pNode->m_pRight->m_pRight->IsPhantomNode() );
               
               int phantomSubnodeCountLeft = pNode->m_pRight->m_pLeft->m_reachInfo.subnodeCount;
               float phantomxLeft = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pRight->m_pLeft->m_reachInfo.subnodeInfoArray[phantomSubnodeCountLeft - 1]->location.x);
               float phantomyLeft = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pRight->m_pLeft->m_reachInfo.subnodeInfoArray[phantomSubnodeCountLeft - 1]->location.y);
               
               int phantomSubnodeCountRight = pNode->m_pRight->m_pRight->m_reachInfo.subnodeCount;
               float phantomxRight = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pRight->m_pRight->m_reachInfo.subnodeInfoArray[phantomSubnodeCountRight - 1]->location.x);
               float phantomyRight = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pRight->m_pRight->m_reachInfo.subnodeInfoArray[phantomSubnodeCountRight - 1]->location.y);
                 
               
               upMidxRight = (phantomxRight + phantomxLeft)/2;     // jpb - assume they are the same as the left reach, although
               upMidyRight = (phantomyRight + phantomyLeft)/2;     //  this isn't really correct.
               }
            else
               {
               int subnodeCountRight = pNode->m_pRight->m_reachInfo.subnodeCount;
            
               upMidxRight = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pRight->m_reachInfo.subnodeInfoArray[subnodeCountRight - 1]->location.x);
               upMidyRight = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pRight->m_reachInfo.subnodeInfoArray[subnodeCountRight - 1]->location.y);
               }

            //Determine the angle between the headwater node and the midpoint
		      float xabsLeft = (float) fabs(downMidx - upMidxLeft);
		      float yabsLeft = (float) fabs(downMidy - upMidyLeft);
		      float orientAngleLeft = (float)atan(xabsLeft/yabsLeft);

   	      float xabsRight = (float) fabs(downMidx - upMidxRight);
		      float yabsRight = (float) fabs(downMidy - upMidyRight);
		      float orientAngleRight = (float)atan(xabsRight/yabsRight);

            //Calculated for use below in returning a specific orient angle
            upMidx   = (upMidxRight + upMidxLeft)/2;
            upMidy   = (upMidyRight + upMidyLeft)/2;
      
            //Average the left and right orient angles to determine final angle
            if (orientAngleRight > orientAngleLeft)
               orientAngle = (orientAngleRight - orientAngleLeft)/2; 
            else
               orientAngle = (orientAngleLeft - orientAngleRight)/2;
            }
         if (pSubnodeInfoDown == NULL) //the reach has ONLY one subnode
            {
            //Find the midpoint of the downstream reach
            downMidx = GetMidpoint(pSubnodeInfo->location.x, vDownstreamNode.x);
            downMidy = GetMidpoint(pSubnodeInfo->location.y, vDownstreamNode.y);
            
            //There are either ONE or TWO upstream reaches(right and left), 
            //therefore we need both upstream midpoints if they exist.  If only one exists
            //it will be the the left
            upMidxLeft = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pLeft->m_reachInfo.nodeLocation.x);
            upMidyLeft = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pLeft->m_reachInfo.nodeLocation.y);
      
            //Determine the angle between the downstream and upstream midpoints
		      float xabsLeft = (float) fabs(downMidx - upMidxLeft);
		      float yabsLeft = (float) fabs(downMidy - upMidyLeft);
		      float orientAngleLeft = (float)atan(xabsLeft/yabsLeft);

            //Must check if m_pRight exists.  If it does not, then the node is not located
            //at a junction.  In that case, set the right equal to the left and go on
            if ( pNode->m_pRight != NULL ) //Node IS at a junction.  Calculate m_pRight orient angle 
				   {
               upMidxRight = GetMidpoint(pSubnodeInfo->location.x, pNode->m_pRight->m_reachInfo.nodeLocation.x);
		         upMidyRight = GetMidpoint(pSubnodeInfo->location.y, pNode->m_pRight->m_reachInfo.nodeLocation.y);
				  
               float xabsRight = (float) fabs(downMidx - upMidxRight);
		         float yabsRight = (float) fabs(downMidy - upMidyRight);
		         float orientAngleRight = (float)atan(xabsRight/yabsRight);

               //Average the left and right orient angles to determine final angle
               if (orientAngleRight > orientAngleLeft)
                  orientAngle = (orientAngleRight - orientAngleLeft)/2; 
               else
                  orientAngle = (orientAngleLeft - orientAngleRight)/2;
               }
			   else //Node NOT at a junction.  Only left orient angle exists.
			      {
               orientAngle = orientAngleLeft;
               }
				
            //Calculated for use below in returning a specific orient angle
            upMidx   = (upMidxRight + upMidxLeft)/2;
            upMidy   = (upMidyRight + upMidyLeft)/2;
      
            }

         }
      }

   //all j != 0 cases

   if (j!= 0)
      {
      if (j == (subnodeCount-1))  //We are at the last subnode in a reach (that is NOT ALSO the first subnode). Looking at next node
         {
         downMidx = GetMidpoint(pSubnodeInfo->location.x, vDownstreamNode.x);
         downMidy = GetMidpoint(pSubnodeInfo->location.y, vDownstreamNode.y);
         upMidx   = GetMidpoint(pSubnodeInfo->location.x, pSubnodeInfoUp->location.x);
         upMidy   = GetMidpoint(pSubnodeInfo->location.y, pSubnodeInfoUp->location.y); 

         float xabs = (float) fabs(upMidx - downMidx);
		   float yabs = (float) fabs(upMidy - downMidy);
		   orientAngle = (float)atan(xabs/yabs);
		   }
      else  //We are at a non-node subnode
         {
         downMidx = GetMidpoint(pSubnodeInfo->location.x, pSubnodeInfoDown->location.x);
         downMidy = GetMidpoint(pSubnodeInfo->location.y, pSubnodeInfoDown->location.y);
         upMidx   = GetMidpoint(pSubnodeInfo->location.x, pSubnodeInfoUp->location.x);
         upMidy   = GetMidpoint(pSubnodeInfo->location.y, pSubnodeInfoUp->location.y); 

         float xabs = (float) fabs(upMidx - downMidx);
		   float yabs = (float) fabs(upMidy - downMidy);
		   orientAngle = (float)atan(xabs/yabs);
		   }
      }


	if      (upMidx < downMidx &&  upMidy < downMidy)
	  return orientAngle;

	else if (upMidx <= downMidx && upMidy > downMidy )
      return PI - orientAngle;

	else if (upMidx > downMidx && upMidy > downMidy)
      return PI + orientAngle;

    else if (upMidx >= downMidx && upMidy < downMidy)
	  return 2*PI - orientAngle;
	  
	else if (upMidx < downMidx && upMidy == downMidy)
	  return PI/2;

	else if (upMidx > downMidx && upMidy == downMidy)
	  return 3*PI/2;

	//else
      //ASSERT( 0 );
	else
		return 0.0f;
	}	


float WET_Temp::GetMidpoint(float x1,float x2)
   {
      return (x1+x2)/2;
   }

   
///The purpose of the next series of functions is to read in vegetation data and determine 
///vegetation shading angles. This allows for the calculation of sky openness															//////
float WET_Temp::RightAzimuthAngle(SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	if (pSubnodeInfo->streamAspect <= 4.7125f)
      return pSubnodeInfo->streamAspect + 1.5708f;   
	else
		return pSubnodeInfo->streamAspect - 4.7125f;
   }


float WET_Temp::LeftAzimuthAngle(SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	  if (pSubnodeInfo->streamAspect >= 1.5708f)
			return pSubnodeInfo->streamAspect - 1.5708f;
	  else
			return pSubnodeInfo->streamAspect + 4.7125f;
	}

void WET_Temp::CalcReachWDRatio( int i )
   {
   //ASSERT( m_pChannelLayer != NULL );  //OK for Habitat Layer to be NULL
   //MapLayer *pHabitat = m_pChannelLayer;
   
   ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
   REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
   //Get poly line representing this reach
   Poly *pLine = m_pStreamLayer->m_polyArray[ i ];
	int neighbors[ 200 ];   //array of 200 polygons, within the bounding rectangle
   
   float width = 0.0f, depth = 0.0f, length = 0.0f, subnodeVolume = 0.0f;
   float lengthFraction = 0.0f, lengthTotal = 0.0f, wdRatio = 0.0f;
   float Qout = 0.0f, dx = 0.0f, averageWidth =0.0f, averageDepth = 0.0f;
   
   //Get habitat segments that are near the poly line representing this reach
   int habSegments = 0;
   //only go check for nearby polys if Channel Data exists
   if ( m_pChannelLayer != NULL )
      {
      habSegments = m_pStreamLayer->GetNearbyPolys(pLine, neighbors, NULL, 200, 200, SIM_CENTROID, NULL);
      }

   if (habSegments > 0)  //ODFW Channel Data Available!!  Use it!
      {
      //Cycle through and find the total length of all the segments;
      for (int h = 1; h <= habSegments; h++)
         {
         int z = neighbors[h - 1];
         m_pChannelLayer->GetData(z, m_colLength, length);
         lengthTotal += length;
         }
      
      //Cycle through and add up cumulative widths and depths, normalized by length
      for (int m = 1; m <= habSegments; m++)
         {
         int reach = neighbors[m - 1];
         m_pChannelLayer->GetData(reach, m_colLength, length);
         lengthFraction = length/lengthTotal;

         m_pChannelLayer->GetData(reach, m_colWidth, width);
         width = width*lengthFraction;
         averageWidth += width;

         m_pChannelLayer->GetData(reach, m_colDepth, depth);
         depth = depth*lengthFraction;
         averageDepth += depth;
         }
      
      pTemp->wdRatio = averageWidth/averageDepth;  //Store calculated wdRatio in pTemp structure
      }
   else   //No ODFW data available.  Use wdRatio set by user.
      pTemp->wdRatio = m_wdRatio;

   }   

void WET_Temp::CalcChannelParameters( SUBNODE_INFO_TEMP *pSubnodeInfo, int i, int j )
   {
   float x = pSubnodeInfo->location.x;
	float y = pSubnodeInfo->location.y;

   ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
   REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

   float width = 0.0f, depth = 0.0f, length = 0.0f, subnodeVolume = 0.0f;
   float lengthFraction = 0.0f, lengthTotal = 0.0f, wdRatio = 0.0f;
   float Qout = 0.0f, dx = 0.0f, averageWidth =0.0f, averageDepth = 0.0f;
    
   dx = pNode->m_reachInfo.deltaX;

   Qout  = pNode->m_reachInfo.qArray[ j ];
   depth = GetDepthFromQ( pNode, Qout, j);
   
   if (pTemp->wdRatio == m_wdRatio)
      depth = depth * m_volX;  //subnode volume is increased by increasing depth, only if channel data not available
   
   width = pTemp->wdRatio*depth;  
   subnodeVolume = (width * depth * dx);
      
   //store values in subnode_info structure
   pSubnodeInfo->wdRatio = pTemp->wdRatio;
   pSubnodeInfo->streamDepth = depth;
   pSubnodeInfo->streamWidth = width;
   pSubnodeInfo->subnodeVolume = subnodeVolume;
   pSubnodeInfo->wdRatioOriginal = pSubnodeInfo->wdRatio;
      
   //This prevents zero values for width, depth or volume
   if ( pSubnodeInfo->streamWidth <= 0.0f )
      pSubnodeInfo->streamWidth = 5.0f;

   if ( pSubnodeInfo->streamDepth <= 0.0f )
      pSubnodeInfo->streamDepth = 0.5f;

   if (pSubnodeInfo->wdRatio <= 0.0f )
      pSubnodeInfo->wdRatio = pSubnodeInfo->streamWidth/pSubnodeInfo->streamDepth;

   if (pSubnodeInfo->subnodeVolume <= 0.0f )
      pSubnodeInfo->subnodeVolume = pSubnodeInfo->streamWidth * pSubnodeInfo->streamDepth * dx;
   }


   
void WET_Temp::CalculateTopographicShading(SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid )
   {	
	float x = pSubnodeInfo->location.x;
	float y = pSubnodeInfo->location.y;

   float gridSize = m_pDEMLayer->GetGridCellWidth();
   float noDataValue = m_pDEMLayer->GetNoDataValue();
   int maxcount = (int)(2000/gridSize);  //Farthest amount we can step in any one direciton (2km)

	//Get grid cell which corresponds to the stream subnode
	float subnodeElev = 0;
   int startRow, startCol;
	pGrid->GetGridCellFromCoord( x, y, startRow, startCol );
	int row = startRow;
   int col = startCol;


   
   pGrid->GetData( row, col, subnodeElev );   //Gets elevation at this subnode

	//Load subnode elevation into data structure
	pSubnodeInfo->subnodeElev = subnodeElev;
   
   int rowCount = pGrid->GetRowCount();			//Total # of rows in the grid
				
	int colCount = pGrid->GetColCount();			//Total # of columns in the grid

	 //Allocate array to hold topographic shading values in 8 directions
			  
	//float topoAngle[8]; 
    for (int d = 0; d < 8; d++)
		{ 
		pSubnodeInfo->topoAngle[d] = 0;
		}  

	float newElev = 0;
	

	//Now, cycle through each of the 8 directions and calculate maximum topographic shading angle
	for ( int d=0; d < 8; d++ )
		{
      row = startRow;
      col = startCol;
      float distance = 0;
      

      //Topographic shading after 200 steps (2 km) is considered negligible
      for(int count = 0; count<maxcount; count++)
         {
		   switch( d )
				{
				case 0:  //NORTH
					row = row - 1;		//Step to the north 10m
				break;

				case 1:  //NORTHEAST
					row = row - 1;		//Step to the north 10m
					col = col + 1;		//Step to the east 10m
				break;

				case 2:  //EAST
					col = col + 1;		//Step to the east 10m
				break;

				case 3:  //SOUTHEAST
					row = row + 1;		//Step to the south 10m
					col = col + 1;		//Step to the east 10m
				break;

				case 4:  //SOUTH
					row = row + 1;		//Step to the south 10m
				break;

				case 5:  //SOUTHWEST
					row = row + 1;		//Step to the south 10m
					col = col - 1;		//Step to the west 10m
				break;

				case 6:  //WEST
					col = col - 1;		//Step to the west 10m
				break;

				case 7:  //NORTHWEST
					row = row - 1;		//Step to the north 10m
					col = col - 1;		//Step to the west 10m
				break;
         }

         // out of bounds?
        if ( row < 0 || row >= rowCount || col < 0 || col >= colCount )
            break;
		
		pGrid->GetData( row, col, newElev );   //gets elevation at that cell
            	
		if (newElev == noDataValue)
			break;	 
		   

	 	//How far from stream are we?
		if (d % 2 == 0)
			distance += gridSize;  //step size is gridsize for N,S,E and W
		else
			distance += gridSize * 1.414f; //step size is gridsize * sqrt(2) m for NE, SE, SW and NW
				    
		//Calculation of topograhpic shading.  topoAngle[d] is set to max value
		float newTopoAngle = (float)atan((newElev - subnodeElev)/distance);
		if (newTopoAngle > pSubnodeInfo->topoAngle[d])
			pSubnodeInfo->topoAngle[d] = newTopoAngle;
         }

	  }
		
}


void WET_Temp::CalculateDistanceToVeg(SUBNODE_INFO_TEMP *pSubnodeInfo, int i)
   {
   ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

   REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

   //Get distance to vegetation on the right and left banks
   float dnvPerp = pTemp->nsdz*0.5f;   //Same for either bank

   float raa = pSubnodeInfo->rightAzimuthAngle;
   float laa = pSubnodeInfo->leftAzimuthAngle;

   
   for (int d=0; d<8; d++)
      {
      float distToVeg = 0.0f;
      float angleIncrement = (2.0f*PI)/8.0f;  //45 degrees.  The increment of the direction angles

      float directionAngle = d*angleIncrement;  //i.e. 0 for north, 45 for northeast etc...

      float theta;  //This represent the angle BETWEEN the azimuth and the direction angle

      if (directionAngle - laa < PI/2)
         {
         theta = (float)fabs( directionAngle - laa );
         distToVeg = (float)fabs(dnvPerp/(float)cos(theta));
         }
      else if (directionAngle - laa == PI/2)
         distToVeg = 30.0f;   //Distance to next subnode (max value of distToVeg)
      else
         {
         theta = (float)fabs(directionAngle - raa);
         distToVeg = (float)fabs(dnvPerp/(float)cos(theta));
         }
      
      //Distance to veg can't exceed 30m
      if (distToVeg > 30.0f)
         distToVeg = 30.0f;
      
      pSubnodeInfo->distToVeg[d] = distToVeg;
      }
   
   }


void WET_Temp::InitVegetationInfo(SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid)
	{
	 //Set x and y as the location of the subnode
	 float x = pSubnodeInfo->location.x;
	 float y = pSubnodeInfo->location.y;

	 //Cycle through each of the 8 directions (north defined as 0).
	 for (int d=0; d < 8; d++)
		{
	   //Now, populate each element of the arrays contained in SUBNODE_VEG_INFO
      PopulateVegInfo(d, x, y,pGrid, pSubnodeInfo);
		}
	}



	
void WET_Temp::PopulateVegInfo(int d, float x, float y,MapLayer *pGrid, SUBNODE_INFO_TEMP *pSubnodeInfo)	 
	{		 
	//Looks 100m in each of the eight directions.
	//Then populates the SUBNODE_VEG_INFO with values for height, density and adjusted height and vegShadingAngle
	
   float noDataValue = m_pCellLayer->GetNoDataValue();
	
   //New vegInfo to contain vegetation data
   //SUBNODE_VEG_INFO *pVeg = pSubnodeInfo->vegInfo[d];
   pSubnodeInfo->vegInfo[d].maxVegDensity    = 0.0f;  //Reinitialize maxVegDensity and maxShadeAngle for each direction
   pSubnodeInfo->vegInfo[d].maxVegShadeAngle = 0.0f;
   pSubnodeInfo->vegInfo[d].averageHeight    = 0.0f;

	Poly *pPoly = NULL;
   float directStepLength = 0; 
   float diagonalStepLength = 0;
   float cumBufferWidth = 0.0f;

   for ( int n=1; n <= 11; n++ )
		{
      // In order to give the user the ability to analyze potential buffer widths less than 10m, the
      // first two steps in each direction are ~5m (slightly longer on the diagonals)
      if ( n <= 2 )
         {
         directStepLength   = 5.0f;
         diagonalStepLength = (float)sqrt(12.5);
         }
      else
         {
         directStepLength   = 10.0f;
         diagonalStepLength = 5 * (float)sqrt(2.0);
         }

		switch( d )
			{
			case 0:  //NORTH
				y = y + directStepLength;
				break;							//Step to the north

			case 1:  //NORTHEAST
				y = y + diagonalStepLength;		
				x = x + diagonalStepLength;		//Step to the northeast 
				break;

			case 2:  //EAST
				x = x + directStepLength;				//Step to the east 
				break;

			case 3:  //SOUTHEAST
				y = y - diagonalStepLength;		
				x = x + diagonalStepLength;		//Step to the southeast 
				break;

			case 4:  //SOUTH
				y = y - directStepLength;				//Step to the south 
				break;

			case 5:  //SOUTHWEST
				y = y - diagonalStepLength;		
				x = x - diagonalStepLength;		//Steps to the southwest
				break;

			case 6:  //WEST
				x = x - directStepLength;				//Step to the west 
				break;

			case 7:  //NORTHWEST
				y = y + diagonalStepLength;		
				x = x - diagonalStepLength;		//Step to the northwest 10m
				break;	
			}
	
      
      if ( n == 1 )  ///Get polygon on first step
         pPoly = m_pCellLayer->GetPolygonFromCoord( x, y );  //Get Polygon at that point 
      else  //on each step thereafter, check first to see if you are still in the same polygon
         {
         //Convert new x/y coordinates into a vertex
         Vertex newvert;
         newvert.x = x;
         newvert.y = y;
      
         bool ok = pPoly->IsPointInPoly( newvert ); // Is the vertex in the same polygon??
      
          //If not, find a new polygon.
         if (ok == false)
            {
            pPoly = m_pCellLayer->GetPolygonFromCoord( x, y );  //Get Polygon at that point 
            }
         }
      
      //If there is no polygon (i.e we are off the cell coverage) set veg parameters to nodatavalue
      if ( pPoly == NULL )
         {
         pSubnodeInfo->vegInfo[d].heightAdjustment[n-1] = noDataValue;
         pSubnodeInfo->vegInfo[d].vegDensity[n-1]       = noDataValue;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1]        = noDataValue;
         pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1]    = noDataValue;
         pSubnodeInfo->vegInfo[d].landUse[n-1]          = (int) noDataValue;
         break;  //CODECHANGE???
         }
      
      int cell = pPoly->m_id;

		if ( cell == noDataValue || cell == -1 )
         {
         pSubnodeInfo->vegInfo[d].heightAdjustment[n-1] = noDataValue;
         pSubnodeInfo->vegInfo[d].vegDensity[n-1]       = noDataValue;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1]        = noDataValue;
         pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1]    = noDataValue;
         pSubnodeInfo->vegInfo[d].landUse[n-1]          = (int)noDataValue;
         break; //CODECHANGE???
         }

      CString system;

      //First two steps are 5m, the remaining 9 steps are 10m
      if (n == 1) 
         cumBufferWidth = 5.0f;
      else if (n == 2)
         cumBufferWidth = 10.0f;
      else
         cumBufferWidth = (10.0f * n) - 10.0f;
      
      int lulc_c = 0;
      float RESTORE_BufferWidth = 0.0f;
      
      if ( m_coverType == 1 )
         {
         if (m_options & WT_OPTION_USE_ALT_LULC )
            {
            RESTORE_BufferWidth = GetRESTOREBufferWidth( cell );
               
            if ( cumBufferWidth <= RESTORE_BufferWidth )
               lulc_c = GetCellLulc( cell, TRUE );  //Get Alt_Lulc Data from the polygon
            else
               lulc_c = GetCellLulc( cell, FALSE );  //Get Regular Lulc Data from the polygon, as we are outside modeled buffer width
            }
         else
            lulc_c = GetCellLulc( cell, FALSE );  //Get Regular Lulc Data from the polygon, as we are not using alt_lulc

         DecodeLulc(lulc_c, d, n, pSubnodeInfo);   //Gets vegetation Height and Density based on lulc_c code
         }
      else if ( m_coverType == 2 )
         {
         int heightCode = GetCellHeight( cell );
         int densityCode = GetCellDensity( cell );
         CString landuseClass = GetCellLanduseClass( cell );
         system = GetCellSystem( cell );
         DecodeHeight(heightCode, d, n, pSubnodeInfo);
         DecodeDensity(densityCode, d, n, pSubnodeInfo);
         DecodeLanduseClass(landuseClass, d, n, pSubnodeInfo, heightCode);
         //DecodeSystem(system, d, n, pSubnodeInfo);
         //pSubnodeInfo->vegInfo[d].landUse[n-1] = landuseClass;
         }
          
      
      //Calculation of average vegetation height in direction d
      float avght = (pSubnodeInfo->vegInfo[d].vegHeight[n-1])/11;
      pSubnodeInfo->vegInfo[d].averageHeight += avght;
      
      pSubnodeInfo->vegInfo[d].heightAdjustment[n-1] = GetElevationAdjustment(pSubnodeInfo, pGrid, x, y );

		//Calculation of vegetation shading angle in this direction, at each sampling step
      //Assume overhang is 10% of vegetation height
      float shadeAngle;
      float overshade  = pSubnodeInfo->vegInfo[d].vegHeight[n-1]*0.1f;
      
      //If overshade is greater than distToVeg, make overshade just .01 less, so the shade angle function
      //will work and return a value of 89.99 degrees (because the shade angle is truly 90 is this case)
      if (overshade >= pSubnodeInfo->distToVeg[d])
         overshade = pSubnodeInfo->distToVeg[d] - 0.01f;

      //First step.  Must take distance to veg into account
      if( n == 1 )  
         {
         shadeAngle = (float)atan((pSubnodeInfo->vegInfo[d].vegHeight[n-1]
                    + pSubnodeInfo->vegInfo[d].heightAdjustment[n-1])
                     / (pSubnodeInfo->distToVeg[d] - overshade));
         }
      else  
         {
         shadeAngle = (float)atan( ( pSubnodeInfo->vegInfo[d].vegHeight[n-1] 
                       + pSubnodeInfo->vegInfo[d].heightAdjustment[n-1] ) 
                       /( (pSubnodeInfo->distToVeg[d] - overshade) + directStepLength * (n-1) ) );
         }
      
      
      //Change any slightly negative values for shade angle to slightly positive
      if (shadeAngle < 0.0f)
         shadeAngle = 0.0001f;
      
      //Load Vegetation Shading Angle into our structure
		pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1] = shadeAngle;

		
      //Is this angle the maximum encountered so far?
		//if so, it takes the place of old maximum shading angle value
		if (shadeAngle > pSubnodeInfo->vegInfo[d].maxVegShadeAngle)		
			pSubnodeInfo->vegInfo[d].maxVegShadeAngle = shadeAngle;

		//Is this the maximum vegetation density encountered so far?
		//If so, it takes the place of old maximum vegetation density value
	   if (pSubnodeInfo->vegInfo[d].vegDensity[n-1] > pSubnodeInfo->vegInfo[d].maxVegDensity)
         pSubnodeInfo->vegInfo[d].maxVegDensity = pSubnodeInfo->vegInfo[d].vegDensity[n-1];

      //Store this vegetation data as the "original" (i.e. unrestored) information
      pSubnodeInfo->vegInfo[d].vegHeightOriginal[n-1] = pSubnodeInfo->vegInfo[d].vegHeight[n-1];
      pSubnodeInfo->vegInfo[d].vegDensityOriginal[n-1] = pSubnodeInfo->vegInfo[d].vegDensity[n-1];
      pSubnodeInfo->vegInfo[d].vegShadeAngleOriginal[n-1] = pSubnodeInfo->vegInfo[d].vegShadeAngle[n-1];

      //if the vegetation poly is of the riverine system, we are going to skip it and calculate the
      //next vegetation polygon as if it were the first step.  To do this we will alter the value of
      //n so that the step is recalculated.  R values should only occur in the main river channel.
      // NOTE:  This could cause unforseen problems in larger, braided river systems with vegetated
      //islands. WET_Temp is not currently set up to consider this type of vegetation pattern.
      
      if (system == "R")
         n = n - 1;

		}
	}




void WET_Temp::DecodeHeight(int heightCode, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo)
   {
   float h;

   switch (heightCode)
      {
      case 0:
         h = (float) m_randUnif.GetUnif01();
         //h = 0.5f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         
         break;
      
      case 1:
         h = (float) m_randUnif.RandValue( 6.0, 12.0 );
         //h = 9.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      case 2:
         h = (float) m_randUnif.RandValue( 12.0, 18.0 );
         //h = 15.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      case 3:
         h = (float) m_randUnif.RandValue( 18.0, 24.0 );
         //h = 21.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      case 4:
         h = (float) m_randUnif.RandValue( 24.0, 30.0 );
         //h = 27.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      case 5:
         h = (float) m_randUnif.RandValue( 30.0, 36.0 );
         //h = 33.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      case 6:
         h = (float) m_randUnif.RandValue( 0.5, 1.5 );
         //h = 1.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      //Emergent Vegetation
      case 7:
         h = (float) m_randUnif.RandValue( 0.5, 1.5 );
         //h = 1.0f;
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = h; 
         break;
      
      
      default:
         pSubnodeInfo->vegInfo[d].vegHeight[n-1] = 0;
   
      }
   }



void WET_Temp::DecodeDensity(int densityCode, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo)
   {
   float den;

   switch (densityCode)
      {
      case 6:
         pSubnodeInfo->vegInfo[d].vegDensity[n-1] = 30; 
         break;
      
      case 7:
         den = (float) m_randUnif.RandValue( 31.0, 50.0 );
         pSubnodeInfo->vegInfo[d].vegDensity[n-1] = den;
         break;
      
      case 8:
         den = (float) m_randUnif.RandValue( 51.0, 70.0 );
         pSubnodeInfo->vegInfo[d].vegDensity[n-1] = den;
         break;
      
      case 9:
         den = (float) m_randUnif.RandValue( 71.0, 90.0 );
         pSubnodeInfo->vegInfo[d].vegDensity[n-1] = den;
         break;
      
      case 0:
         den = (float) m_randUnif.RandValue( 91.0, 100.0 );
         pSubnodeInfo->vegInfo[d].vegDensity[n-1] = den;
         break;
      
      default:
         pSubnodeInfo->vegInfo[d].vegDensity[n-1] = 1.0f;
      }
   }


void WET_Temp::DecodeLanduseClass(CString landuseClass, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo, int heightCode)
   {
   if (landuseClass == "AG")               //Agricultural land
      pSubnodeInfo->vegInfo[d].landUse[n-1] = 101;
   else if (landuseClass == "EM")               //Emergent Vegetation
      pSubnodeInfo->vegInfo[d].landUse[n-1] = 102;
   else if (landuseClass == "SS")               //Scrub-Shrub
      pSubnodeInfo->vegInfo[d].landUse[n-1] = 103;
   else if (landuseClass == "HU")               //Herbaceous Uplands 
      pSubnodeInfo->vegInfo[d].landUse[n-1] = 104;
   else if (landuseClass == "FO")               //Forests  (5 different height classes of forest)
      {
      if (heightCode == 1)
         pSubnodeInfo->vegInfo[d].landUse[n-1] = 111;
      else if (heightCode == 2)
         pSubnodeInfo->vegInfo[d].landUse[n-1] = 112;
      else if (heightCode == 3)
         pSubnodeInfo->vegInfo[d].landUse[n-1] = 113;
      else if (heightCode == 4)
         pSubnodeInfo->vegInfo[d].landUse[n-1] = 114;
      else
         pSubnodeInfo->vegInfo[d].landUse[n-1] = 115;
      }
   else
      pSubnodeInfo->vegInfo[d].landUse[n-1] = 999;

   }




void WET_Temp::DecodeLulc(int lulc_c, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	switch ( lulc_c )
		{
		case 49:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 15;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 30;
			break;

		case 51:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 20;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 15;
			break;

		case 52:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 20;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 55;
			break;

		case 53:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 15;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 80;
			break;

		case 54:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 20;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 80;
			break;

		case 55:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 35;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 55;
			break;

		case 56:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 4;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

		case 57:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 14;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

		case 58:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 22;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

		case 59:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 29;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

		case 60:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 43;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

		case 61:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 60;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 75;
			break;

		case 62:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 15;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 55;
			break;

		case 66:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 10;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 80;
			break;

      case 86:
         pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 0.5;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 80;
			break;

      case 87:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 4;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 60;
			break;

		case 95:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 15;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 60;
			break;

		case 97:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 3;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

		case 98:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 15;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 10;
			break;

      case 99:
         pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 0.5;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 90;
			break;

      //wet-shrub
      case 101:
         pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 8.0;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 85;
			break;

      default:
			pSubnodeInfo->vegInfo[d].vegHeight[n-1]	= 0;
			pSubnodeInfo->vegInfo[d].vegDensity[n-1]	= 0;
			break;
   	}	

	}





void WET_Temp::CalculateSubnodeSlope(int i, int j)
   {
   ReachNode *pNode = m_reachTree.m_nodeArray[i];
         
   REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

   float maxz, minz, distance, slope;

   distance = 2 * m_maxReachDeltaX;  //twice the subnode interval normally
   
   if (j == 0) //We are at the first reach, look only downstream
      {
      maxz = pTemp->subnodeInfoTempArray[j]->subnodeElev;
      distance = m_maxReachDeltaX;  
      }
   else
      maxz = pTemp->subnodeInfoTempArray[j-1]->subnodeElev;

   int subnodeCount = pNode->m_reachInfo.subnodeCount;

   if (j == subnodeCount - 1) //We are at the last reach, look only upstream
      {
      minz = pTemp->subnodeInfoTempArray[j]->subnodeElev;
      distance = m_maxReachDeltaX;
      }
   else
      minz = pTemp->subnodeInfoTempArray[j + 1]->subnodeElev;
      

   slope = ( maxz - minz ) / distance;
	
   if (slope <= 0.0f)
      slope = 0.001f;

   pTemp->subnodeInfoTempArray[j]->subnodeSlope = slope;

   }

   
float WET_Temp::GetElevationAdjustment(SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid, float x, float y)
	{//This function finds the elevation adjustment at each vegetation sample point and adds
	 //this to the VEG_SUBNODE_INFO structure
	
	//First, find elevation of stream at the subnode
	float streamElev = 0.0f;
	float xStream = pSubnodeInfo->location.x;
	float yStream = pSubnodeInfo->location.y;
	int rowStream, colStream;
	pGrid->GetGridCellFromCoord(xStream, yStream, rowStream, colStream);
	pGrid->GetData(rowStream, colStream, streamElev);

	//Now, find elevation at the vegetation sample point in question
	float elev = 0.0f;
	int row, col;
	pGrid->GetGridCellFromCoord(x, y, row, col);
	pGrid->GetData(row, col, elev);


	return elev - streamElev;
	}



float WET_Temp::TopoAngleLeft(SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	float azdeg			= pSubnodeInfo->leftAzimuthAngle * (180.0f/PI);
	
	int nearAzimuthLeft; 
   if (azdeg >= 360.0f)
      nearAzimuthLeft = 315;
   else
      nearAzimuthLeft = int(azdeg/45) * 45;

   int nearAzimuthRight = nearAzimuthLeft + 45;

	int a = nearAzimuthLeft/45;

    float nearTopoLeft = pSubnodeInfo->topoAngle[a];

	float nearTopoRight;

	if (a==7)
		nearTopoRight = pSubnodeInfo->topoAngle[0];
	else
		nearTopoRight = pSubnodeInfo->topoAngle[a+1];

	float deltaAzimuthRight = azdeg - (float)nearAzimuthLeft;
	
	float deltaAzimuthLeft = 45.0f - deltaAzimuthRight;

	float topoAngleLeft = ((deltaAzimuthLeft * nearTopoLeft) + (deltaAzimuthRight * nearTopoRight))/45.0f;

	return topoAngleLeft;

	}



float WET_Temp::TopoAngleRight(SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	float azdeg			= pSubnodeInfo->rightAzimuthAngle * (180.0f/PI);
	
	int nearAzimuthLeft; 
   if (azdeg >= 360.0f)
      nearAzimuthLeft = 315;
   else
      nearAzimuthLeft = int(azdeg/45) * 45;

	int nearAzimuthRight = nearAzimuthLeft + 45;

	int a = nearAzimuthLeft/45;

   float nearTopoLeft = pSubnodeInfo->topoAngle[a];

	float nearTopoRight;

	if (a==7)
		nearTopoRight = pSubnodeInfo->topoAngle[0];
	else
		nearTopoRight = pSubnodeInfo->topoAngle[a+1];

	float deltaAzimuthRight = azdeg - (float)nearAzimuthLeft;
	
	float deltaAzimuthLeft = 45.0f - deltaAzimuthRight;

	float topoAngleLeft = ((deltaAzimuthLeft * nearTopoLeft) + (deltaAzimuthRight * nearTopoRight))/45.0f;

	return topoAngleLeft;

	}



void WET_Temp::VegInfoLeft(SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	//This function returns the vegetation shading angle and shade density to the left 
	//of the subnode (perpendicular 90 degrees to the stream aspect)
	float azdeg				= pSubnodeInfo->leftAzimuthAngle * (180.0f/PI); //convert rightAzimuthAngle to degrees
		
	int nearAzimuthLeft; 
   if (azdeg >= 360.0f)
      nearAzimuthLeft = 315;
   else
      nearAzimuthLeft = int(azdeg/45) * 45;

	int nearAzimuthRight = nearAzimuthLeft + 45;

	int a = nearAzimuthLeft/45;

	SUBNODE_VEG_INFO pVegL = pSubnodeInfo->vegInfo[ a ];

	SUBNODE_VEG_INFO pVegR;
	
	if ( a == 7 )
		pVegR = pSubnodeInfo->vegInfo[0];
	else
		pVegR = pSubnodeInfo->vegInfo[a + 1];

	float deltaAzimuthRight = azdeg - (float)nearAzimuthLeft;
	
	float deltaAzimuthLeft = 45.0f - deltaAzimuthRight;

   pSubnodeInfo->vegAngleLeft = ((deltaAzimuthLeft * pVegL.maxVegShadeAngle) + (deltaAzimuthRight * pVegR.maxVegShadeAngle))/45.0f;

	pSubnodeInfo->vegDensityLeft = ((deltaAzimuthLeft * pVegL.maxVegDensity) + (deltaAzimuthRight * pVegR.maxVegDensity))/45.0f;
	}



void WET_Temp::VegInfoRight(SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
	//This function returns the vegetation shading angle and shade density to the right 
	//of the subnode (perpendicular 90 degrees to the stream aspect)
	float azdeg				= pSubnodeInfo->rightAzimuthAngle * (180.0f/PI); //convert rightAzimuthAngle to degrees

	int nearAzimuthLeft; 
   if (azdeg >= 360.0f)
      nearAzimuthLeft = 315;
   else
      nearAzimuthLeft = int(azdeg/45) * 45;

	int nearAzimuthRight = nearAzimuthLeft + 45;

	int a = nearAzimuthLeft/45;

   SUBNODE_VEG_INFO pVegL = pSubnodeInfo->vegInfo[a];

	SUBNODE_VEG_INFO pVegR;

	if ( a == 7 )
	   pVegR = pSubnodeInfo->vegInfo[0];
	else
		pVegR = pSubnodeInfo->vegInfo[a + 1];

	float deltaAzimuthRight = azdeg - (float)nearAzimuthLeft;
	
	float deltaAzimuthLeft = 45.0f - deltaAzimuthRight;
	
	pSubnodeInfo->vegAngleRight = ((deltaAzimuthLeft * pVegL.maxVegShadeAngle) + (deltaAzimuthRight * pVegR.maxVegShadeAngle))/45.0f;

	pSubnodeInfo->vegDensityRight = ((deltaAzimuthLeft * pVegL.maxVegDensity) + (deltaAzimuthRight * pVegR.maxVegDensity))/45.0f;
	}



float WET_Temp::SkyOpen (SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
		float sky_open = ((PI - max(pSubnodeInfo->topoAngleRight,pSubnodeInfo->vegAngleRight) - max(pSubnodeInfo->topoAngleLeft,pSubnodeInfo->vegAngleLeft) ))/PI;
		//expressed in percentage of the total possible open sky (3.14157 radiians)

		return sky_open;
	}



float WET_Temp::StreamWidth(int streamOrder)
	{	
   //Calculation of stream width based on stream order
	//streamWidth expressed in m and based on 23 field sites in the LongTom basin
      if (streamOrder == 1)
			return 0.7f;
		else if (streamOrder == 2)
				return  1.75f;
		else if (streamOrder == 3)
			return 2.75f;
		else if (streamOrder == 4)
			return 2.85f;
		else
			return 4.0f;
	}


////////////////////////////////////////////////////////////////////////////////////////
///////																			////////
///////																			////////
///////					** END PREPROCESSING CODE  **							////////
///////																			////////
///////																			////////
////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////
///////																			////////
///////																			////////
///////					**  MODEL RUN CODE  **									////////
///////																			////////
///////																			////////
////////////////////////////////////////////////////////////////////////////////////////



void WET_Temp::Run( void )
   {
   //gpMain->SetStatus( "Initializing simulation..." );
   Notify( WT_STARTRUN, 0.0f, 0 );

   time_t startRun, endRun;
   time( &startRun );
   
   InitRun();         // do any necessary initialization, set up state variable matrices

   
   //Create new DataStore and open the file containing hydrologic information
   //if ( m_pDataStore == NULL )
	//	m_pDataStore = new DataStore;
	
   //FILE *fp = fopen("j:/wt/wettemptest/temptest.hyd", "rb");
   //ASSERT(fp != NULL);

   //m_pDataStore->InputFromFile("j:/wt/wettemptest/temptest.hyd");  //Now the hydrologic information is stored and accessible by the model

   m_svCount = m_reachTree.GetSubnodeCount();

   if ( m_k1 == NULL )
      {
      int count = m_reachTree.GetSubnodeCount();
      m_k1 = new float[ count ];
      m_k2 = new float[ count ];
      m_k3 = new float[ count ];
      m_k4 = new float[ count ];
      m_y  = new float[ count ];
      m_sv = new float[ count ];
      m_bed= new float[ count ];
      m_bedFluxCounter = new float [ count ];
      }

   //set m_mouthIndex
   //m_mouthIndex = m_reachTree.m_pRoot->m_pLeft->m_reachInfo.index;
   
   //m_mouthIndex = 165;   //McDowell Creek only
   m_mouthIndex = 1;   //UGR only

   // initialize, starting with left leaf
   ReachNode *pLeftLeaf = m_reachTree.FindLeftLeaf();
   int start = 0;
   InitBranch( pLeftLeaf, start );
  
  
   m_dayOfYear = int( m_startTime );   //initialize dayOfYear (julianDay)
   float simTime = m_startTime;          //set simTime to startTime
   int step = 0;

   while ( simTime <= m_stopTime )
		{
		//Solar calculations. Time-variant but constant across reaches.
		SolarCalculations( simTime );

      // solve reaches
      m_latInTotal = 0.0f;  // mass balance
      m_volumeTotal = 0.0f;  

      //IntRK4( simTime );  //explicit solution, solve entire network at once
      
      m_airTemp = GetAirTemp(simTime);
      m_relativeHumidity = GetRH(simTime);

      RunTemperatureModel( m_k1, simTime );  //implicit solution.  Solve subnode by subnode

      float reachFlowCheck = m_latInTotal;  //mass balance.  should equal flow at root
      //Note, residence Time will only be correct if mass balance is working
      float residenceTime = m_volumeTotal/(m_latInTotal*3600); // hours

      int index = 0;
      
      //EnergyTemp(pLeftLeaf, index);   //converts energy values into temperature for display
		
      
      if ( fmod( simTime, m_outputTimeStep ) < m_timeStep*.75 )
         {
		   CollectData( simTime );
	      
         if (m_pObservedLayer != NULL)   //Make sure Observed layer exists before collecting calibration data
            CollectCalibrationData( simTime );
         }

      //Sensitivity Data (Tmax, Tmin, time of max/min) written once a day
      int timeint = (int)simTime;
      float simDuration = m_stopTime - m_startTime;
      float timeRemaining = m_stopTime - simTime;

      
      //LONGITUDINAL DATA COLLECTION
      //This case we will collect longitudinal data for six of the last 7 days if a 7-day average is to be used
      //if (simDuration >= 10.0f && timeRemaining < 8.0f && timeint > m_dayOfYear)
        // {
         //CollectLongitudinalData( simDuration );
         //}
      
      //This case will collect longitudinal data on the last day
      if ( timeRemaining <= m_timeStep )   
         {
         CollectLongitudinalData( simDuration );
         }      
      
      
      if (timeint > m_dayOfYear)  //We have just crossed over a day boundary
         {
         WriteSensData();
         ResetDailyInfo();
         
         //after daily info is reset, change m_dayOfYear
         m_dayOfYear = timeint;
         }
      
      step++;
		simTime = m_startTime + step*m_timeStep;

      if ( Notify( WT_UPDATE, simTime, 0 ) == false )
         break;

      //float hour = (float)fmod(mtime,timeint)*24;
		}

   //EndRun();
   Notify( WT_ENDRUN, simTime, 0 );

   // store max temps, deltas in layer
   for ( int i=0; i < m_pSubnodeLayer->GetRecordCount(); i++ )
      {
      int reachIndex, subnodeIndex;
      m_pSubnodeLayer->GetData( i, 0, reachIndex );
      m_pSubnodeLayer->GetData( i, 1, subnodeIndex );

      SUBNODE_INFO_TEMP *pTemp = GetReachSubnodeInfo( reachIndex, subnodeIndex );

      float lastTemp;
      m_pSubnodeLayer->GetData( i, 3, lastTemp );  

      if ( lastTemp < 0.0001 )  //Is there no Tmax??  If so this is the first run.  Set Tmax.
         m_pSubnodeLayer->SetData( i, 3, pTemp->Tmax );
      else    //Tmax already exists.  This is the second run.  Set delta T.
         {
         float tempDelta = pTemp->Tmax - lastTemp;
         float tempDeltaF = tempDelta * 1.8f;  //Convert tempDelta from degree C to degrees F 
         
         if ( tempDelta <= -3.0 )
            {
            CString msg;
            msg.Format( "Temperature Increase > 1 degree: Tmax %g, lastTemp %g", pTemp->Tmax, lastTemp);
            //ErrorMsg( msg );
            TRACE( msg );
            }
         if (pTemp->Tmax == m_tempInit)
            m_pSubnodeLayer->SetNoData( i, 4 );
         else
            m_pSubnodeLayer->SetData( i, 4, tempDeltaF );
         }
      float lastFlow;
      m_pSubnodeLayer->GetData(i, 5, lastFlow );

      if ( lastFlow < 0.0001 )  //Is there no flow value??  If so this is the first run.  Set flow.
         m_pSubnodeLayer->SetData( i, 5, pTemp->streamFlow );
      else    //Flow already exists.  This is the second run.  Set delta Flow.
         m_pSubnodeLayer->SetData( i, 6, pTemp->streamFlow - lastFlow );
      }


   //Write calibration data to files for analysis
   WriteObsValueInfo();

   time( &endRun );
   float m_runTime = (float) difftime( endRun, startRun );
   
   CString msg;
   msg.Format( "Finished!    Run Time:  %g    seconds", m_runTime );
   //gpMain->SetStatus( msg );
   }


   void WET_Temp::ResetDailyInfo()
      {
      int reachCount = GetReachCount();

      for (int i=0; i < reachCount; i++)
         {
         ReachNode *pNode = m_reachTree.m_nodeArray[i];
         REACH_INFO_TEMP *pReachInfoTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      
         int subnodeCount = pNode->m_reachInfo.subnodeCount;
         
         for (int j=0; j <= subnodeCount-1; j++)
            {
            SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfoTemp->subnodeInfoTempArray[j];
            //reset daily maximum and minimum temperature values at each subnode
         
            pSubnodeInfo->Tmax = -99.0;
            pSubnodeInfo->Tmin = 99.0;
            }
         }      
      }
   
//--------------------------------------------------------------------------------------
// WET_Temp::InitRun
//
//  
// This method sets the model to collect output data at user specified points.
//
//--------------------------------------------------------------------------------------

void WET_Temp::InitRun( void ) 
   {
   //////////////////////////////////////////////////////////////////////////////////////// 
   // set layer pointers
   ASSERT( m_pCellLayer != NULL );

   ////////////////////////////////////////////////////////////
   // Calculate the slope of each polygon and add the value to the matrix layer data
   DataObj *pCellData = m_pCellLayer->m_pData;
   ASSERT( pCellData != NULL );
	
   //gpMain->SetStatus( "Creating Upslope Information Structures..." );
   // create upslope information structures for each reach
   int reachCount = GetReachCount();
   int cellCount  = GetCellCount();

   
   //If channel data exists, create spatial index from stream layer to channel layer
   if (m_pChannelLayer != NULL)
      m_pStreamLayer->CreateSpatialIndex( NULL, 32, 120.f, SIM_CENTROID, m_pChannelLayer );

   ResetDailyInfo();  //Resets daily parameters (Tmax, Tmin) left over from other runs

   for ( int i=0; i < reachCount; i++ )
      {
      REACH_INFO_TEMP *pReachInfo = m_reachInfoTempArray[ i ];
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
        
      InitReachSubnode( pReachInfo, pNode, i );// initial any necessary subreach-level information (including subnodes) prior to making a new running
      
	   //m_reachDataArray.Add( i );   //Temporary for small data only ???
      }  // end of: for (i < reachCount );

   //If restored landscape exists, apply increased late summer flows
   //---RESTORE generated landscapes---
   if ( m_options & WT_OPTION_USE_ALT_FLOW )
      {
      RestorationDischargeAdjustments();
      }
   //-----------
   
   for ( int i=0; i < m_pSubnodeLayer->GetRecordCount(); i++ )
      {
      int reachIndex, subnodeIndex;
      m_pSubnodeLayer->GetData( i, 0, reachIndex );
      m_pSubnodeLayer->GetData( i, 1, subnodeIndex );

      SUBNODE_INFO_TEMP *pTemp = GetReachSubnodeInfo( reachIndex, subnodeIndex );

      ReachNode *pNode = m_reachTree.m_nodeArray[ reachIndex ];
      
      if (m_options & WT_OPTION_USE_ALT_FLOW )
         {
         m_pSubnodeLayer->SetData( i, 5, pNode->m_reachInfo.qArray[subnodeIndex] );
         m_pSubnodeLayer->SetData( i, 6, pNode->m_reachInfo.qArray[subnodeIndex] - pTemp->streamFlowOriginal );
         }
      }
   
      
      
   
   // iterate through the upslope info structures, adding weighted values and initializing
   if ( m_pTemperatureData != NULL )  // is there already a general data obj?
      delete m_pTemperatureData;

   if ( m_pHeatFlux != NULL )
      delete m_pHeatFlux;

   if ( m_pSolarFlux != NULL )
      delete m_pSolarFlux;

   if ( m_pEvapFlux != NULL )
      delete m_pEvapFlux;

   if ( m_pLongwaveFlux != NULL )
      delete m_pLongwaveFlux;

   if ( m_pConvectionFlux != NULL )
      delete m_pConvectionFlux;

   if ( m_pAdvectionFlux != NULL )
      delete m_pAdvectionFlux;
      
   //////////////////////////////////////////////////////////////
   // set up data collection for temperature
   int dataToCollect = m_reachDataArray.GetSize()+1;
   
   m_pTemperatureData = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pTemperatureData != NULL );
   m_pTemperatureData->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Reach: %i ", m_reachDataArray[ i-1 ]);
      m_pTemperatureData->SetLabel( i, (char*)(LPCSTR) label );
      }
   
   ///////////////////////////////////////////////////////////////
   //allocate data object for the collection of daily max and min temperatures
   int longDataToCollect = m_reachDataArray.GetSize();
   
   for (int i=0; i< m_pLongitudinalDataArray.GetSize(); i++)
      delete m_pLongitudinalDataArray[i];
   
   m_pLongitudinalDataArray.RemoveAll();
   
   //Create a data object for each reach where longitudinal data will be collected
   for (int i = 0; i < longDataToCollect; i++)
      {
      float simDuration = m_stopTime - m_startTime;
      int reachIndex = m_reachDataArray[ i ];
      
      /*
      if ( simDuration >= 10.0f )  //Simulation is longer than 10 day, collect 7 day max values
         {
         m_pLongitudinalDataArray.Add( new FDataObj( 17,0 ) );

         ASSERT( m_pLongitudinalDataArray[i] != NULL );
         m_pLongitudinalDataArray[i]->SetLabel( 0, "Distance From Mouth" );
         for (int m = 1; m <= 14; m = m + 2)
            {
            m_pLongitudinalDataArray[i]->SetLabel( m, "Tmax" );
            m_pLongitudinalDataArray[i]->SetLabel( m +1 , "Tmin" );
            }
         m_pLongitudinalDataArray[i]->SetLabel( 15, "7-day Average Maximum" );
         m_pLongitudinalDataArray[i]->SetLabel( 16, "7-day Average Minimum" );
         }
      else   //simulation is shorter than 10 days, so max/min data will only be collected on last day
         {*/
         m_pLongitudinalDataArray.Add( new FDataObj( 4,0 ) );

         ASSERT( m_pLongitudinalDataArray[i] != NULL );
         m_pLongitudinalDataArray[i]->SetLabel( 0, "Distance From Mouth" );
         m_pLongitudinalDataArray[i]->SetLabel( 1, "TMax" );
         m_pLongitudinalDataArray[i]->SetLabel( 2, "Tmin" );
         m_pLongitudinalDataArray[i]->SetLabel( 3, "Discharge" );
         
         //}
      }
   
   
   ////////////////////////////////////////////////////////////////
   //allocate data objects for energy balance components
   m_pHeatFlux = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pHeatFlux != NULL );
   m_pHeatFlux->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Heat Flux kJ/m2");
      m_pHeatFlux->SetLabel( i, (char*)(LPCSTR) label );
      }

   m_pSolarFlux = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pSolarFlux != NULL );
   m_pSolarFlux->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Solar Flux kJ/m2");
      m_pSolarFlux->SetLabel( i, (char*)(LPCSTR) label );
      }

   m_pEvapFlux = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pEvapFlux != NULL );
   m_pEvapFlux->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Evap Flux kJ/m2");
      m_pEvapFlux->SetLabel( i, (char*)(LPCSTR) label );
      }

   m_pLongwaveFlux = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pLongwaveFlux != NULL );
   m_pLongwaveFlux->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Longwave Flux kJ/m2");
      m_pLongwaveFlux->SetLabel( i, (char*)(LPCSTR) label );
      }
   
   m_pConvectionFlux = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pConvectionFlux != NULL );
   m_pConvectionFlux->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Convection Flux kJ/m2");
      m_pConvectionFlux->SetLabel( i, (char*)(LPCSTR) label );
      }

   m_pAdvectionFlux = new FDataObj( dataToCollect, 0 );
   ASSERT( m_pAdvectionFlux != NULL );
   m_pAdvectionFlux->SetLabel( 0, "Time" );
   for ( int i=1; i < dataToCollect; i++ )
      {
      CString label;
      label.Format( "Advection Flux kJ/m2");
      m_pAdvectionFlux->SetLabel( i, (char*)(LPCSTR) label );
      }

   // allocate data object for observed flow locations
   for ( int k=0; k < m_obsInfoArray.GetSize(); k++ )
      {
      if ( m_obsInfoArray[ k ].pData != NULL )
         delete m_obsInfoArray[ k ].pData;

      m_obsInfoArray[ k ].pData = new FDataObj( 6, 0 );
      m_obsInfoArray[ k ].pData->SetLabel(0, "Time");
      m_obsInfoArray[ k ].pData->SetLabel(1, "Modeled Temp");
      m_obsInfoArray[ k ].pData->SetLabel(2, "Measured Temp");
      m_obsInfoArray[ k ].pData->SetLabel(3, "deltaT/km - modeled");
      m_obsInfoArray[ k ].pData->SetLabel(4, "deltaT/km - measured");
      m_obsInfoArray[ k ].pData->SetLabel(5, "SolarFlux/100");      
      }

   }

// initialize any-level necessary reach-level information (including subnodes) prior to making a new run

bool WET_Temp::InitReachSubnode( REACH_INFO_TEMP *pReachInfo, ReachNode *pNode, int i )
   {
    int subnodeCount = pNode->m_reachInfo.subnodeCount;
    int reachCount = m_reachTree.m_nodeCount - 1;

    //reset parameters that may have changed in a restoration scenario
    pReachInfo->incSummerFlow = false;
    pReachInfo->sumDiversions = 0.0f;
    pReachInfo->wetlandArea = 0.0f;
    
  
   CalcReachWDRatio( i );

    for (int j=0; j <= subnodeCount-1; j++)
      {
       SUBNODE_INFO_TEMP *pSubnodeInfo = pReachInfo->subnodeInfoTempArray[j];
    
      //reset streamFlow to Original value if alt_flow is not being used
      if ( m_options & WT_OPTION_NONE )
         {
         pNode->m_reachInfo.qArray[j] = pSubnodeInfo->streamFlowOriginal;
         }

      //Not used???
      //Apply basin wide change in stream discharge as dictated by model user
      /*float newDischarge = 0.0f;
      float discharge = pSubnodeInfo->streamFlow;
      float multiplier = 1.0f + (m_flowIncrease/100.0f);
      newDischarge = discharge * multiplier;
      pSubnodeInfo->streamFlow = newDischarge;
 */
      //reinitialize channel params each run in case things have changes (wdratio)
      CalculateSubnodeSlope(i, j);
      CalcChannelParameters( pSubnodeInfo, i, j);  
      pSubnodeInfo->streamTemperature = m_tempInit;
      }

	return 0;
   }



//////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------//
//  SolarCalculations																	//
//																						//
//  Series of functions for Calculation of sun position and radiation at each timestep	//
//--------------------------------------------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////

void WET_Temp::SolarCalculations( float time )
	{
	float julianDay = JulianDay(time);
	
   m_declination = SetDeclination( julianDay );
	float solarTime = SolarTime(julianDay);
	float hourAngle	= HourAngle(solarTime);
	m_solarAltitude = SetSolarAltitude( hourAngle );
	m_azimuth = GetAzimuth( hourAngle );
	m_etRad = SetETRad( julianDay );
	}


float WET_Temp::JulianDay(float time)
	{
	//Calculates Julian Day (1 to 365 plus a fraction of each day) at each timestep
	float julianDay;

	if (fmod(time,365) <= 1.0)
		julianDay = time +1;
	else
		julianDay = (float)fmod(time,365);       //Assume: time=0 == Jan 1

	return julianDay;
	}

	

float WET_Temp::SetDeclination(float julianDay)
	{
	return (23.45f * (float)sin( (360.0f/365) * (PI/180) * (284 + julianDay)))*PI/180;  //Hsieh pg 12 , converted to radians
	}
	

float WET_Temp::SolarTime(float julianDay)
	{
	//Solar time is based on the apparent angular motion of the sun across the sky
	//It can be calculated by adjusting the local standard time

	float longcorrect = (4*(120 - 123))/60.0f;   //longitude correction for time. 4 minutes for each degree
		                                          //away from the standard meridian (120 degrees for PST)
									          //NOTE:  longitude is not defined.  
								              //This needs to come from data!!!  123 is just filler.
	
	//Empirical relationship for equation of time.  Eqtime describes the slight variations
	//in the path of the sun relative to a point on earth.
	float b = (360/364.0f) * (julianDay - 81);
	
	float eqtime = (9.87f*(float)sin(2*b) - 7.53f*(float)cos(b) - 1.5f*(float)sin(b))/60;  //Hsieh pg 17 

	//the integer portion of julianDay
	int dayint = (int)julianDay;			
	
	//A decimal less than 1 remains after subtracting the integer portion of julianDay
	//from the total value.  This decimal is converted into time of day (local standard time).
	float lst;	
   if (fmod(julianDay,(float)dayint) == 0.0f)
	   lst = 24 + ((julianDay - dayint)*24);
   else
	   lst = ((julianDay - dayint)*24);   //Local Standard Time -- Calculate time of day from timestep
	
	float solarTime=lst+eqtime+longcorrect;   //Local standard time + all correction values

	return solarTime;
	}


float WET_Temp::HourAngle (float solarTime)
	{
    float hourAngle = (12-solarTime)*(180/12)*(PI/180);   ///Units are radians
    ////NOTE:  In Iqbal, this value oscillates between pi and -pi.
	////This is why the solarTime is modified by 12 hours.  It is only a shift so that trig functions will have the proper signs

	return hourAngle;
	}


float WET_Temp::SetSolarAltitude(float hourAngle)
	{
	//////Calculation of the hour angle at sunrise (Iqbal pg 16)
	float sunriseAngle =(float)acos( -tan( 44*PI/180 )*tan( m_declination ) ); 
	
	//////Calculation of Sunset Angle below
	float sunriseDegree = (sunriseAngle*180/PI);
	float dayLength = (0.133333f)*(sunriseDegree);
	float sunriseTime = (float)fabs(sunriseDegree*(0.06666667)-12.0f);
	float sunsetTime = sunriseTime + dayLength;
	float sunsetAngle = (12-sunsetTime)*(180.0f/12)*(PI/180);

    //////Calculation of solar altuitude (Iqbal pg 15)///////////
	if (hourAngle > sunriseAngle)		//pre dawn.  solar altitude zero
		m_solarAltitude = 0;
	else if (hourAngle < sunsetAngle)	//after sunset.  solar altitude zero
		m_solarAltitude = 0;
	else 
		m_solarAltitude = ((float)asin(((float)sin(m_declination)*(float)sin(44*PI/180))+((float)cos(m_declination)*(float)cos(44*PI/180)*(float)cos(hourAngle))));
		//  Again, 44 is just latitute filler. Actual lat/long should come from watershed data
	
	return m_solarAltitude;
	}


float WET_Temp::GetAzimuth ( float hourAngle )
	{
    ////////Calculation of Azimuth (Hsieh pg 13, modified for proper angle measures)//////////////
	//if (abs( (int) hourAngle) >= 1.5708)
	//	m_azimuth = 3.14157f - (float)asin((float)cos(m_declination)*((float)sin(hourAngle)/(float)cos(m_solarAltitude)));
    //else
		m_azimuth = (float)asin((float)cos(m_declination)*((float)sin(hourAngle)/(float)cos(m_solarAltitude)));
	
	return m_azimuth;
	}



float WET_Temp::SetETRad(float julianDay)
	{
	///Calcuation of extra-terrestrial solar radiation on a horizontal section
	///above a certain piece of the earth

	int solarConstant = 4871;  //Solar Constant in kJ/(m^2 * h)

	float eccentricityFactor = 1+(0.033f*(float)cos((2*PI*julianDay)/365));
	    //Eccentricity factor.  Correction for irregularity of earth's orbit.

	float etNormal = solarConstant*eccentricityFactor;  
		//Extra terrestrial Solar Radiation on surface normal to the sun

	float m_etRad = etNormal*(float)sin(m_solarAltitude); 
		//Extra terrestrial Solar radiation on a horizontal surface per hour {kJ/(m^2 * h)}
		
	return m_etRad;    //Units --  kJ/m^2 * h 
	}

float WET_Temp::SetSurfaceRad( SUBNODE_INFO_TEMP *pSubnodeInfo, float julianDay )
	{

	///Calculation of global solar radiation at earth's surface
	
	float mopt = 35/(1224*(float)pow(((float)sin(m_solarAltitude)+ 1.0),0.5));  
	//optical air mass thickness (basically path length)  -- Iqbal pg 103
	
	float m = mopt*(float) exp(-0.0001184 * pSubnodeInfo->subnodeElev);
	//optical air mass corrected for elevation  -- Iqbal pg  100

	float atmosphericTransmissivity = 0.0685f*(float)cos(((2*PI)/365)*(julianDay + 10))+0.80f;
	//atmospheric transmissivity    --  Fritz 1980 paper
	//NOTE:  This is an empirical fit over Lake Eire.  If possible on down the line, it
	//would be prudent to fit this to western Oregon data???.

	m_surfaceRad = m_etRad*(float)pow(atmosphericTransmissivity,m)*(0.8f);
	//radiation at the surface under CLEAR SKY conditions
	//need cloudiness factor.  Using 0.2 for now (0.8 = 1 - 0.2)???
	
	return m_surfaceRad;  // kJ/m^2 * h 
	}

/////////////////////////////////////////////////////////////////////////////
///																		   //
///   The following three functions takes solar radiation and			   //
///   disaggreate it into direct beam normal and diffuse radiation		   //
/////////////////////////////////////////////////////////////////////////////
   
void WET_Temp::SetRadSplit(float julianDay )
	{	
	float kt = m_surfaceRad/m_etRad;      

	//From Vignola and McDaniels 1984  --- Get reference???
	//float diffuseFraction = 0.938f + 1.071f*kt - 5.146f*kt*kt + 2.982f*kt*kt*kt - (0.009f - 0.078f*kt)*(float)(sin((2*PI*(julianDay - 40))/365));

      /*if (diffuseFraction > 1)
		  diffuseFraction=1;

	  if (diffuseFraction <= 0)			//Only a temporary fix until I can get Iqbal to check units????
		  diffuseFraction=0.15f;
		*/

	//Orgill and Hollands Correlation (from Iqbal pg 269)
	float diffuseFraction = 0.0f;
	if (kt >= 0.0f && kt <= 0.35f)
		diffuseFraction = 1.0f - 0.249f*kt;
	else if (kt > 0.35f && kt <= 0.75f)
		diffuseFraction = 1.577f - 1.84f*kt;
	else
		diffuseFraction = 0.177f;


	m_diffuseRad = diffuseFraction*m_surfaceRad;
	m_beamRad = m_surfaceRad - m_diffuseRad;      // Both m_beamRad and m_diffuseRad in kJ/m^2 * h	
	}


////////END SOLAR CALCULATIONS///////////////////////////////////////////////////////////


//--------------------------------------------------------------------------------------
// WET_Temp::RunInstreamModel
//  
// Runs the instream temperature model.  Assumes that BuildReachTree() has been previously
//  called to build up the reach tree data structure, and that RunUpslopeModel() has been
//  called to provide upslope drainage information to the model.  This method iterates
//  from the leaves of the reach tree downstream, using an implicit solution procedure
//  to efficiently compute instream termperature.
//--------------------------------------------------------------------------------------
/*
int WET_Temp::RunTemperatureModel( float time )
   {
   // get leftmost leaf
   ReachNode *pNode = m_reachTree.FindLeftLeaf();  // find leftmost leaf of entire tree

  	int solvedReachCount = 0;

   //GetHydroData(pNode,time);
   
   while ( '( pNode, time) )  // solves the KW equations for the reach downstream from pNode
      {
      solvedReachCount++;

      // next, move to the next downstream node
      ReachNode *pDown = pNode->m_pDown;

      if ( pDown == NULL ) // root of tree reached?
         break;

      // if we're coming from the left and a right branch exists, solve the right branch first

      // are we coming from the left?  and    right branch exists?
      if ( ( pDown->m_pLeft == pNode ) && ( pDown->m_pRight != NULL ) )
         pNode = m_reachTree.FindLeftLeaf( pDown->m_pRight );

      else
         pNode = pDown;

      //GetHydroData(pNode,time);
      
      }

   return solvedReachCount;
   }

*/

/*
int WET_Temp::RunTemperatureModel( float time )
   {
   // get leftmost leaf
   ReachNode *pNode = m_reachTree.m_pRoot;

   int dtStartIndex = 0;

  	int solvedReachCount = SolveBranch( pNode, time, dtStartIndex );

   return solvedReachCount;
   }
*/


void WET_Temp::InitBranch( ReachNode *pNode, int &dtStartIndex  )
   {
    //Calculate flow and channel parameters that are time invariant
    //Only need to do this once. (This will change in a dynamic flow scenario)
    
    float dx = 0.0f;
    
    //Calculation of flow leaving a particular reach (simply the flow value at that subnode)
    if ( pNode->m_reachInfo.index >= 0 )
      {
      REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

      for ( int j=0; j < pNode->m_reachInfo.subnodeCount; j++ )
         {
   	   SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[ j ];

         dx = pNode->m_reachInfo.deltaX;

         pSubnodeInfo->bedFlux = 0.0f;
             
         m_sv[ dtStartIndex + j ] = m_tempInit * pSubnodeInfo->subnodeVolume * m_waterDensity * m_waterHeatCapacity;
         
         m_bed[ dtStartIndex + j ] = 0.0;

         m_bedFluxCounter[ dtStartIndex +j ] = 0.0f;
         //are we at the last subnode in the reach??  If so, store m_sv array offset
         if ( j == pNode->m_reachInfo.subnodeCount - 1 )
            pNode->m_reachInfo.energyOffset = ( dtStartIndex + j );

         }
      dtStartIndex += pNode->m_reachInfo.subnodeCount;
      }


      ReachNode *pDown = pNode->m_pDown;

      if ( pDown == NULL ) // root of tree reached?
         return;

      // if we're coming from the left and a right branch exists, solve the right branch first

      // are we coming from the left?  and    right branch exists?
      if ( ( pDown->m_pLeft == pNode ) && ( pDown->m_pRight != NULL ) )
         pNode = m_reachTree.FindLeftLeaf( pDown->m_pRight );

      else
         pNode = pDown;
      

      InitBranch( pNode, dtStartIndex );
   
   // Used in explicit solution procedure
   /*if ( pNode->m_pLeft != NULL )
      InitBranch( pNode->m_pLeft, dtStartIndex );

   if ( pNode->m_pRight != NULL )
      InitBranch( pNode->m_pRight, dtStartIndex );
   */

   return;
   }


void WET_Temp::EnergyTemp( ReachNode *pNode, int &index  )
   {
    if ( pNode->m_reachInfo.index >= 0 )
      {
      REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

      for ( int i=0; i < pNode->m_reachInfo.subnodeCount; i++ )
         {
   	   SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[ i ];
         
         float subnodeVolume = pSubnodeInfo->subnodeVolume;

         
         //Calculate temperature based on energy value stored in m_sv Array
         pSubnodeInfo->streamTemperature = m_sv[ index+i ]/ (subnodeVolume * m_waterDensity * m_waterHeatCapacity); 
         
         }
      index += pNode->m_reachInfo.subnodeCount;
      }


   if ( pNode->m_pLeft != NULL )
      EnergyTemp( pNode->m_pLeft, index );

   if ( pNode->m_pRight != NULL )
      EnergyTemp( pNode->m_pRight, index );

   return;
   }




int WET_Temp::RunTemperatureModel( float *derive, float time )
   {
   // get root
   //ReachNode *pNode = m_reachTree.m_pRoot;
  
   //get Left Leaf
   ReachNode *pNode = m_reachTree.FindLeftLeaf();

   int dtStartIndex = 0;  

  	int solvedReachCount = SolveBranch( pNode, time, dtStartIndex, derive  );

   return solvedReachCount;
   }


int WET_Temp::SolveBranch( ReachNode *pNode, float time, int &dtStartIndex, float *derive  )
   {
   int count = 1;
   
   float Qout = 0.0f;
   if (pNode->m_reachInfo.index >= 0 )  //root node and phantom nodes have no output flow
      Qout = pNode->GetOutputFlow();
      
   if ( pNode->m_reachInfo.index < 0 )  //root or phantom node
      {
      //simply move downstream to the next node
      
      ReachNode *pDown = pNode->m_pDown;

      if (pDown == NULL)   //root node quit calculating
         {
         return count;
         }
      else     //phantom node. No calculation, just continue downstream
         {
         // if we're coming from the left and a right branch exists, solve the right branch first
         // are we coming from the left?  and    right branch exists?
      
         if ( ( pDown->m_pLeft == pNode ) && ( pDown->m_pRight != NULL ) )
            pNode = m_reachTree.FindLeftLeaf( pDown->m_pRight );
         else
            pNode = pDown;
      
         count += SolveBranch( pNode, time, dtStartIndex, derive );
         }
      }
  /* else if ( Qout <= 0.014f )   // "Low Flow" special case.  Do not solve reach.  temp = 16.0??
      {
      if (pNode->m_reachInfo.index >= 0)
         {//Set the derivative values (ie. change in temperature) to 0
         int subCount = pNode->m_reachInfo.subnodeCount;
         ASSERT( pNode->m_reachInfo.energyOffset == dtStartIndex + (subCount - 1) );
         for (int j=0; j <= subCount - 1; j++)
            {
            m_k1[ dtStartIndex ] = 0.0;
            dtStartIndex++;
            }
         }
         
      ReachNode *pDown = pNode->m_pDown;

      //Assumption:  in this "Low Flow" special case, we will not be encountering the root node
      //if ( pDown == NULL ) // root of tree reached?
         //break;

      // if we're coming from the left and a right branch exists, solve the right branch first
      // are we coming from the left?  and    right branch exists?
      if ( ( pDown->m_pLeft == pNode ) && ( pDown->m_pRight != NULL ) )
         pNode = m_reachTree.FindLeftLeaf( pDown->m_pRight );
      else
         pNode = pDown;
      
      count += SolveBranch( pNode, time, dtStartIndex, derive );
      }*/
   else
      {
      int subCount = pNode->m_reachInfo.subnodeCount;
      ASSERT( pNode->m_reachInfo.energyOffset == dtStartIndex + (subCount - 1) );
        
      SolveReach( pNode, time, dtStartIndex, derive );
         
      dtStartIndex += pNode->m_reachInfo.subnodeCount;
         
      ReachNode *pDown = pNode->m_pDown;

      // if we're coming from the left and a right branch exists, solve the right branch first

      // are we coming from the left?  and    right branch exists?
      if ( ( pDown->m_pLeft == pNode ) && ( pDown->m_pRight != NULL ) )
         pNode = m_reachTree.FindLeftLeaf( pDown->m_pRight );
      else
         pNode = pDown;

      count += SolveBranch( pNode, time, dtStartIndex, derive );
      }
   
   //mass balance across the reach
   /*if (pNode->m_reachInfo.index >= 0)
      {
      int subCount = pNode->m_reachInfo.subnodeCount;
      float reachFlow = m_reachLat;
      float reachFlowDos = pNode->m_reachInfo.qArray[subCount - 1] - pNode->m_reachInfo.qArray[0];
 
      float massError = reachFlow - reachFlowDos;
      m_reachLat = 0.0f;
      }*/

  
   //Used in root-up solution sequence
   /*if ( pNode->m_pLeft != NULL )
      count += SolveBranch( pNode->m_pLeft, time, dtStartIndex, derive );
      
   if ( pNode->m_pRight != NULL )
      count += SolveBranch( pNode->m_pRight, time,  dtStartIndex, derive );
   */
   
   return count;
   }




// WET_Temp::SolveReach() ----------------------------------------------
//
// solves the KW equations for the reach downstream from pNode.
// Assumes all basin scale information (e.g. m_solarAltitude) have been set
//------------------------------------------------------------------------

bool WET_Temp::SolveReach( ReachNode *pNode, float time, int dtStartIndex, float *derive )
   {
   if ( pNode->m_reachInfo.index < 0 )  // phantom node or the root node?
      return false;
   
   //pointers to this reach, left and right upstream reaches
   REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
   
   REACH_INFO_TEMP *pLeft, *pRight;
   
   if (pNode->m_pLeft != NULL)
      pLeft = (REACH_INFO_TEMP*) pNode->m_pLeft->m_reachInfo.pData;
   
   if (pNode->m_pRight != NULL)
      pRight = (REACH_INFO_TEMP*) pNode->m_pRight->m_reachInfo.pData;

   
	// compute Qin for this reach
   float dx;
   float Qin = 0.0;
   float Qout = 0.0;
   float Tin = 0.0;

   int subnodeCount = pNode->m_reachInfo.subnodeCount;
   
   // cycle through EACH SUBNODE in the finite difference grid
   //for ( int j=subnodeCount-1; j >= 0; j-- )   //<--- use this for explicit
	
   for ( int j=0; j < pNode->m_reachInfo.subnodeCount; j++ )  //<--- use this for implicit
   	{
		//Define pointer to the SUBNODE_INFO_TEMP at this subnode
		SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[ j ];
      SUBNODE_INFO_TEMP *pSubnodeInfoUp = NULL;
      
      //Calculate flow and channel parameters that are time invariant
      //Only need to do this once. (This will change in a dynamic flow scenario)
      float streamWidth = pSubnodeInfo->streamWidth;
      float streamDepth = pSubnodeInfo->streamDepth;
     
      //Calculation of flow leaving a particular reach (simply the flow value at that subnode)
       Qout = pNode->m_reachInfo.qArray[j];
     
      /*if( time == m_startTime )
          {
          //Get stream parameters used to calculate water volume
          streamDepth = GetDepthFromQ( pNode, Qout);
          streamWidth = m_wdRatio*streamDepth;  //using width/depth ration input by user
          //check velocity for reasonable number
          velocity = Qout/(streamWidth*streamDepth);
          pSubnodeInfo->streamDepth = streamDepth;  //hold depth value in subnode info
	       pSubnodeInfo->streamWidth = streamWidth;  //hold depth value in subnode info
          }
    
       //If not the first time through, just use previously calculated stream width and depth
       streamWidth = pSubnodeInfo->streamWidth;
       streamDepth = pSubnodeInfo->streamDepth;
      
       */
       //Get subreach volume
      float subVolume = pSubnodeInfo->subnodeVolume; //subreach volume  m^3
	   float Eupstream = 0.0f;

      
       
      // handle upstream boundary condition
      if ( j == 0 )  // are we computing the most upstream subnode? (remember there are multiple subnodes per reach)
         {           // then get the upstream reach exit temperature
         // are we at a reach other than a headwater?  If so, we have discharge flowing into us from
         // an upstream reach.
         if ( pNode->m_pLeft != NULL )
            {
            if ( pNode->m_pRight != NULL ) //Node is at a junction
               {
               //energy balance actually encompasses two upstream reach segments
               //and one downstream segment
               dx =  pNode->m_pLeft->m_reachInfo.deltaX*0.5f + 
                     pNode->m_pRight->m_reachInfo.deltaX*0.5f +
                     pNode->m_reachInfo.deltaX*0.5f;

               /*
               int subleft = 0, subright = 0, offsetLeft = 0, offsetRight = 0;
               float leftVol =0.0, rightVol = 0.0, Eleft = 0.0, Eright = 0.0, Qleft = 0.0, Qright = 0.0;
               //Calculate the combined volume of the upstream reaches
               if (pNode->m_pLeft->m_reachInfo.index > 0)  //make sure this is not a phantom node
                  {
                  subleft = pNode->m_pLeft->m_reachInfo.subnodeCount;
                  leftVol = pLeft->subnodeInfoTempArray[ subleft - 1 ]->subnodeVolume;
                  Qleft   = pNode->m_pLeft->m_reachInfo.qArray[ subleft - 1 ]; 

                  offsetLeft = pNode->m_pLeft->m_reachInfo.energyOffset;
                  Eleft    = m_sv[offsetLeft];
                  }
               if (pNode->m_pRight->m_reachInfo.index > 0)  //make sure this is not a phantom node
                  {
                  int subright = pNode->m_pRight->m_reachInfo.subnodeCount;
                  rightVol = pRight->subnodeInfoTempArray[ subright - 1 ]->subnodeVolume;
                  Qright   = pNode->m_pRight->m_reachInfo.qArray[ subright - 1];

                  offsetRight= pNode->m_pRight->m_reachInfo.energyOffset;
                  Eright   = m_sv[offsetRight];
                  }
                  
               float combinedVolume = (leftVol + rightVol);
               Qin = Qleft + Qright;

               Eupstream = Eleft + Eright;

               Tin = Eupstream / ( combinedVolume*m_waterDensity*m_waterHeatCapacity );
               */
               
               // compute a flow-weigthed average from the upstream reaches
               float Qleft =     GetOutputFlow(pNode->m_pLeft);
               float Qright =    GetOutputFlow(pNode->m_pRight);
               Qin = Qleft + Qright;

               float Tinleft  = GetOutputFlow(pNode->m_pLeft)  * GetOutputTemp( pNode->m_pLeft ); 
               float Tinright = GetOutputFlow(pNode->m_pRight) * GetOutputTemp( pNode->m_pRight );
               
               Tin = Tinleft + Tinright;
               
               Tin /= Qin;

               }
            else //Node is NOT at a junction.  Use only left pointer (right does not exist)
               {
               dx = pNode->m_reachInfo.deltaX;
               /*int subleft = pNode->m_pLeft->m_reachInfo.subnodeCount;
               float leftVol = pLeft->subnodeInfoTempArray[subleft - 1]->subnodeVolume;
               
               
               int offsetLeft = pNode->m_pLeft->m_reachInfo.energyOffset;
               float Eleft    = m_sv[offsetLeft];
               
               Tin = Eleft / ( leftVol*m_waterDensity*m_waterHeatCapacity);
               */
               Qin = GetOutputFlow(pNode->m_pLeft);
               Tin = GetOutputFlow(pNode->m_pLeft) * GetOutputTemp( pNode->m_pLeft );
               Tin /= Qin;
               }
            }
		   else
			   {//Headwater stream, first subnode (leaf)			 
			   //Qin = 0.0f;
            Qin = pNode->m_reachInfo.qArray[ 0 ];  // ??? should this be zero???
			  
			   Tin = m_tempInit;

            //subreach is only half as long at a headwater
            dx = pNode->m_reachInfo.deltaX*0.5f;
            }
         }  // end of: if ( j == 0 )   // upstream boundary condition

      else  // j != 0, not at boundary - at a subnode in the reach other than the first subnode
         {
         dx = pNode->m_reachInfo.deltaX;
         pSubnodeInfoUp = pTemp->subnodeInfoTempArray[ j - 1 ];

         // get the node above us' flow
         Qin = pNode->m_reachInfo.qArray[j-1];         

         /*
         Eupstream = m_sv[ dtStartIndex + ( j-1 ) ];
         Tin = Eupstream / ( pSubnodeInfoUp->subnodeVolume*m_waterDensity*m_waterHeatCapacity );
         */
		   Tin = pSubnodeInfoUp->streamTemperature;			// get the node above us' temperature
         }
      
      //Calcualte subreach surface Area
	   float surfaceArea = dx*streamWidth;
      //Store subnodeLength
      pSubnodeInfo->subnodeLength = (float)dx;
     
      //Calculate and store subreach residence time
      float resTime = (subVolume/Qout)/60;  //minutes
      pSubnodeInfo->resTime = (float)resTime;
      
      
      //Solar radiation at surface.  Varies with time and between subnodes due to changes in elevation.
	   float julianDay = JulianDay( time );
	   SetSurfaceRad( pSubnodeInfo, julianDay );
	   SetRadSplit( julianDay );

      //??????????????????????? this will fail on implicit solution!~~!~
      //if ( GetOutputFlow( pNode ) < 1/32.0 )  // ~<1 cfs
      //if ( Qout < .025f )  
         //continue;
     
     //Calculate lateral Inflow (as defined by flow estimation code in ReachTree)
     float latIn;
     if (Qin == 0.0)   //This will happen in the case that a "reach" consists of only 1 subnode
        latIn = 0.0;
     else
        latIn = Qout - Qin;
     
     pSubnodeInfo->lateralInflow = (float)latIn;  //store for display

     float latErr = (float)latIn - pNode->m_reachInfo.subnodeInfoArray[ j ]->lateralInflow; //Mass balance on each subnode     
     m_latInTotal += (float)latIn;  //used in mass balance on entire system
            
     //Initialize energy balance terms
    pSubnodeInfo->solarFlux = 0.0f;    
    pSubnodeInfo->longwaveFlux = 0.0f;
    pSubnodeInfo->evaporationFlux = 0.0f;
    pSubnodeInfo->convectionFlux = 0.0f;
    pSubnodeInfo->heatFlux = 0.0f;

    //Get current temperature at this subnode. (This was set last timestep).
      
    //float temp = m_sv[ dtStartIndex+j ] / (subVolume*m_waterDensity*m_waterHeatCapacity );
    float temp = pSubnodeInfo->streamTemperature;


    float heatFlux	= GetHeatFlux(pNode, pSubnodeInfo, time, streamWidth, dx, j, temp, dtStartIndex);  // kJ/m^2*h
    //if (heatFlux <= 0.0)
      // heatFlux = 0.0;
    //float heatFlux = 1000.0f;

     
    float deltaT = Tin - temp;
    //Is lateral inflow negative???  
    //If so energy leaves the system at the current reach temperature
    //If not, energy is input into the system at the groundwater temperature
    float Elateral = 0.0;
     if (latIn > 0.0f)
        Elateral  = latIn * m_gwTemp * m_waterDensity * m_waterHeatCapacity * 3600; //kJ/h
     else
        Elateral = latIn * temp * m_waterDensity * m_waterHeatCapacity * 3600; //kJ/h
     
     //Advected energy input from upstream
	  float Ein = 3600 * Qin * Tin * m_waterDensity * m_waterHeatCapacity;    // kJ/h

     //Energy lost due to advection downstream
	  float Eout = 3600 * Qout * temp * m_waterDensity * m_waterHeatCapacity;    //kJ/h 

     //Convert heat flux into proper units using surface area
     float hFlux = heatFlux * surfaceArea;       //kJ/h
     
     //Store advection flux and total heat flux for energy balance graph
     float adIn = (Ein+Elateral);      //kJ/h
     float adOut = Eout;    //kJ/h
     float adFlux = (adIn - adOut);    //kJ/h
     pNode->m_reachInfo.subnodeInfoArray[j]->advectionFlux = adFlux/surfaceArea; //kJ/m^2*h
     //pSubnodeInfo->heatFlux = (float)heatFlux; //+ (float)adFlux/(float)surfaceArea;  //combined for viewing
     //pSubnodeInfo->heatFlux = hFlux*m_timeStep*24;
          
     m_volumeTotal += (float)subVolume; // System volume. Used to calculate residence time.     
     
     //Implicit Euler solution using E terms
     //float Ecurrent = temp*subVolume*m_waterDensity*m_waterHeatCapacity;
     //float E = Ecurrent + (Ein + Elateral - Eout + hFlux)*m_timeStep*24;
     
     
     // + value = heat input from stream to bedrock.  - value, heat input from bedrock to stream
     //float bedFlux = (m_sv[ dtStartIndex + j ] - m_bed[ dtStartIndex + j ]) * m_heatTransferK; 
     //float bedFlux = 0.0;
     

     ///// Derivative calculation for explicit RK or Euler solution
     /*float dE = Ein + Elateral - Eout + hFlux + m_bedFlux;  // kJ/h
     derive[ dtStartIndex + j ] = dE*24;  //now kJ/day
     */
           
     ///Convert new E to temperature using volume of the subreach
     //float T = E/(subVolume*m_waterHeatCapacity*m_waterDensity);   // degrees C  - jpb 2/3/02
     
     //If lateral inflow is negative, it must be removed at the temperature of the current reach
     //This is for the implicit solution procedure only
     float latTerm = 0.0f;
     
     if (latIn > 0.0f)
        latTerm  = ((latIn * m_gwTemp * m_timeStep * 3600 * 24)/subVolume);
     else
         latTerm  = ((latIn * temp * m_timeStep * 3600 * 24)/subVolume);
     


     
     //Calculation of T for implicit solution
	  float T = (((Qin*Tin*m_timeStep*3600*24)/subVolume) + (latTerm + ((heatFlux*m_timeStep*24*surfaceArea) + m_bedFlux)
               /(m_waterDensity*m_waterHeatCapacity*subVolume)) + temp)
                    /(1.0f + ((Qout * m_timeStep * 3600*24)/subVolume));
	  
	  
     //low flow special case.  Keep subnode temperature constant at 13 for flows < 0.2 cfs
     if (pSubnodeInfo->streamFlowOriginal < 0.004)  //mmc 03/28/03  changed from .006 to .0001
        T = 13.0;
     
     //if (T <= 0)
       // T = 0;
     
     //store new T
     pSubnodeInfo->streamTemperature = T;   //- jpb 2/3/02
     
     //store sensitivity data
     int timeint = (int)time;
	
	
     if ( T >= pSubnodeInfo->Tmax )
         {
         pSubnodeInfo->Tmax = T;         
         pSubnodeInfo->timeOfMax = (float)fmod(time,(float)timeint)*24;
         }

     if ( T <= pSubnodeInfo->Tmin )
         {
         pSubnodeInfo->Tmin = T;
         pSubnodeInfo->timeOfMin = (float)fmod(time,(float)timeint)*24;
         }
      }  // end of:  for ( j < subnodeCount )
   
   return true;
   }
 


//This function calculates the topographic shading angle for each subnode

float WET_Temp::TopoShadeAngle(SUBNODE_INFO_TEMP *pSubnodeInfo)
   {
	m_azimuth = 6.28314f - (m_azimuth + 3.14159f);
	// This correction changes azimuth, which has south = 0,
	// into streamAspect system, where north = 0

	float azdeg = m_azimuth*180/PI;
	//conversion of azimuth into degrees

	int nearAzimuthLeft = int(azdeg/45) * 45;

	int nearAzimuthRight = nearAzimuthLeft + 45;

	int a = nearAzimuthLeft/45;

    float nearTopoLeft = pSubnodeInfo->topoAngle[a];

	float nearTopoRight;

	if (a==7)
		nearTopoRight = pSubnodeInfo->topoAngle[0];
	else
		nearTopoRight = pSubnodeInfo->topoAngle[a+1];

	float deltaAzimuthLeft = azdeg - (float)nearAzimuthLeft;
	
	float deltaAzimuthRight = 45.0f - deltaAzimuthLeft;

	float topoShadeAngle = ((deltaAzimuthLeft * nearTopoLeft) + (deltaAzimuthRight * nearTopoRight))/45.0f;

	return topoShadeAngle;
	}



float WET_Temp::StreamAlbedo ()
	{
	//Albedo function from Chen (1996) pg 79.  With appropriate unit conversions.
	float saDegrees = m_solarAltitude*(180/PI);
   float streamAlbedo;
	
   if (saDegrees >= 10.0f)
		streamAlbedo = (0.091f/(float)cos(PI/2.0f - m_solarAltitude) - 0.0386f);
	else
		streamAlbedo = (0.0515f*(90-saDegrees) - 3.635f);

   return streamAlbedo;
	}


/*  This function not used as of now!!!
float WET_Temp::SunwardBank (SUBNODE_INFO_TEMP *pSubnodeInfo)
	{
		m_azimuth = 6.28314f - (m_azimuth + 3.14159f);
	  // This correction changes azimuth, which has south = 0,
	  // into streamAspect system, where north = 0

		if (pSubnodeInfo->streamAspect <= PI)
			{
			if (m_azimuth <= pSubnodeInfo->streamAspect || m_azimuth >= (pSubnodeInfo->streamAspect + PI) )
				return 'L';
			else 
				return 'R';
			}

		else 
			{
			if (m_azimuth <= pSubnodeInfo->streamAspect && m_azimuth >= (pSubnodeInfo->streamAspect - PI) )
				return 'L';
			else 
				return 'R';
			}
	
	}
*/



float WET_Temp::VegShade( ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, int j)
	{
	if ( m_azimuth == pSubnodeInfo->streamAspect || m_azimuth == (pSubnodeInfo->streamAspect +  PI)  )
	   return 0.0f;
	
   // This correction changes azimuth, which has south = 0,
	// into streamAspect system, where north = 0
   m_azimuth	= PI - m_azimuth;
	
	
	float azdeg = m_azimuth*180/PI;
	//conversion of azimuth into degrees

	int nearAzimuthLeft = int(azdeg/45) * 45;		//What is the nearest direction with data to the left?
	int nearAzimuthRight = nearAzimuthLeft + 45;	//What is the nearest direction with data to the right?

	int a = nearAzimuthLeft/45;

   float distToVegR = 0.0f, distToVegL = 0.0f;
   //Get vegetation ( and distance to vegetation ) info for the nearest left direction
	SUBNODE_VEG_INFO *pVegL = &pSubnodeInfo->vegInfo[ a ];
   distToVegL = pSubnodeInfo->distToVeg[a];   

	//Get vegetation (and distance to vegetation) info for the nearest right direction
	SUBNODE_VEG_INFO *pVegR;
	if (a==7)
      {
      pVegR = &pSubnodeInfo->vegInfo[0];
      distToVegR = pSubnodeInfo->distToVeg[0];
      }
	else
      {
      pVegR = &pSubnodeInfo->vegInfo[a+1]; 
	   distToVegR = pSubnodeInfo->distToVeg[a+1];
      }

	//Initialize variables used within the following if statement;
   float pathLengthLeft = 0.0,  pathLengthRight = 0.0, shadeDensityRight = 0.0, shadeDensityLeft = 0.0;
	float heightAdjRight = 0.0, heightAdjLeft = 0.0, sdRight = 0.0, sdLeft = 0.0; 
   int countLeft =0, countRight = 0;
   float stepLength = 0.0f;
   
   //Calculate cumulative path lengths and shade densities along the sun's path
	for(int n=1; n < 11; n++)
		{ 
      if (n <= 2)
         stepLength = 5.0f;
      else
         stepLength = 10.0f;
         

		if (pVegR->vegShadeAngle[ n - 1 ] > m_solarAltitude)
			{
			//calculation of path length.
         float plVegR = ( distToVegR + stepLength * (n-1) ) / (float)cos( m_solarAltitude );
         float h1R = plVegR*(float)sin(m_solarAltitude);
         float h2R = (pVegR->vegHeight[n-1] + pVegR->heightAdjustment[n-1]) - h1R;
         float zR = (PI/2) - m_solarAltitude;
         float x1R = h2R*(float)tan(zR);
         
         if (x1R >= stepLength || x1R <= -stepLength)
            pathLengthRight += stepLength/(float)cos(m_solarAltitude);
         else
            pathLengthRight += h2R/(float)cos(zR);

         shadeDensityRight += pVegR->vegDensity[ n - 1 ];  //average density
         countRight = countRight + 1;  //# of contributing veg strips
         }
		
		if (pVegL->vegShadeAngle[ n - 1 ] > m_solarAltitude)
			{
			//calculation of path length.
         float plVegL = ( distToVegL + stepLength * (n-1) ) / (float)cos( m_solarAltitude );
         float h1L = plVegL*(float)sin(m_solarAltitude);
         float h2L = (pVegL->vegHeight[n-1] + pVegL->heightAdjustment[n-1]) - h1L;
         float zL = (PI/2) - m_solarAltitude;
         float x1L = h2L*(float)tan(zL);
         
         if (x1L >= stepLength || x1L <= -stepLength)
            pathLengthLeft += stepLength/(float)cos(m_solarAltitude);
         else
            pathLengthLeft += h2L/(float)cos(zL);

         shadeDensityLeft += pVegL->vegDensity[ n - 1 ];  //average density
         countLeft = countLeft + 1; //# of contributing veg strips
         }
		
      //These will be used to calculate average height adjustment and
      //then average total height for use in the extinction coefficient equation
      heightAdjRight = heightAdjRight + pVegR->heightAdjustment[n - 1];
      heightAdjLeft = heightAdjLeft + pVegL->heightAdjustment[n - 1];   
      }
   //Average total height calculation (for extinction coefficient calculation)
   float avgHeightAdjust = (heightAdjRight + heightAdjLeft)/22;
   float avgTotalHeight = (pVegR->averageHeight+pVegL->averageHeight)/2 + avgHeightAdjust;

	//Average density calcuations
   if (countRight != 0)
      sdRight  = shadeDensityRight/countRight; //average density
   else
      sdRight = 0.0f;  //No contributing vegetation.  Shade is zero.
   
   if (countLeft != 0)
      sdLeft   = shadeDensityLeft/countLeft; //average density
   else
      sdLeft = 0.0f;    //No contributing vegetation.  Shade is zero.
   
   //Interpolate between the two directions to find the average path length through the vegetation
	float deltaAzimuthLeft = azdeg - (float)nearAzimuthLeft;
	float deltaAzimuthRight = 45.0f - deltaAzimuthLeft;

   m_pathLength		= ((deltaAzimuthRight * pathLengthLeft) + (deltaAzimuthLeft * pathLengthRight))/45.0f;
	m_shadeDensity		= ((deltaAzimuthRight * sdLeft) + (deltaAzimuthLeft * sdRight))/45.0f; 
	
   //If path length is less than zero, set it equal to zero
   if (m_pathLength < 0.0f)
      m_pathLength = 0.0f;
   
   float maxVegDensity = ((deltaAzimuthRight * pVegL->maxVegDensity) + (deltaAzimuthLeft * pVegR->maxVegDensity))/45.0f; 

	//Make sure that shadeDensity does not exceed maxVegDensity
	if (m_shadeDensity > maxVegDensity)
		m_shadeDensity = maxVegDensity;
	
	
   //Beer's law extinction coefficient
   float kExtinct = 0.0f;
   float vegShade = 0.0f;
   
   kExtinct = m_extinct;
   //New formulation as of 3/8/2003 mmc.  kExtinct is a calibratable model parameter
   if (m_pathLength == 0.0f)
      vegShade = 0.0f;
   else
      vegShade = (float)exp( -kExtinct *(1 / ((m_shadeDensity/100) * m_pathLength) ));
   
   /*if (avgTotalHeight > 0.0f)
      {
      kExtinct = (float) -(log(1-(m_shadeDensity/100))/avgTotalHeight);
      vegShade = 1- (float)exp(-kExtinct*m_pathLength);
      }*/
   
   return vegShade;

	if (vegShade > 1)
		return 1.0f;
	}



float WET_Temp::GetHeatFlux(ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, float time, float streamWidth, float dx, int j, float temp, int dtStartIndex)
	{
	 float surfaceArea = streamWidth * dx; 

    float solarFlux			= GetSolarFlux(pNode, pSubnodeInfo, time, j, dtStartIndex, surfaceArea); 
	 //float solarFlux = 2000.0f;
    pSubnodeInfo->solarFlux = (float)solarFlux;       

    float longwaveFlux		= GetLongwaveFlux(pNode, pSubnodeInfo, time, j, temp);	      
	 pSubnodeInfo->longwaveFlux = (float)longwaveFlux;

    float evaporationFlux	= GetEvaporationFlux(pSubnodeInfo, time, temp);   
    //float evaporationFlux = -(0.30 * solarFlux);
    //float evaporationFlux = 0.0;
    pSubnodeInfo->evaporationFlux = (float)evaporationFlux;

    float convectionFlux   =  GetConvectionFlux(evaporationFlux); 
    //pSubnodeInfo->convectionFlux = (float)convectionFlux;

	 float heatFlux = solarFlux + longwaveFlux + convectionFlux + evaporationFlux;
    //pSubnodeInfo->heatFlux = heatFlux;  //kJ/m^2*h
   
    return heatFlux;
    }	  


float WET_Temp::GetSolarFlux(ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, float time, int j, int dtStartIndex, float surfaceArea)
	{
	float streamBeamRad;
	float streamAlbedo	 = StreamAlbedo();
	//float streamAlbedo = 0.05f;
   float vegShade		 = VegShade(pNode, pSubnodeInfo, j);
   pSubnodeInfo->convectionFlux = vegShade;  //Temporary!! to check out vegShade ???
 	//float vegShade = 0.7f;
   float topoShadeAngle  = TopoShadeAngle( pSubnodeInfo );
   //float topoShadeAngle = 0.3;
   //pSubnodeInfo->convectionFlux = vegShade*100;
   
	if ( topoShadeAngle > m_solarAltitude )
		streamBeamRad	= 0.0;
	else
		streamBeamRad	=  ((1 - vegShade) * m_beamRad);

	float shadedAngleLeft	=  (pSubnodeInfo->vegAngleLeft - pSubnodeInfo->topoAngleLeft)/PI;  //Fraction of 180 degree field from bank to bank

	float shadedAngleRight	=  (pSubnodeInfo->vegAngleRight - pSubnodeInfo->topoAngleRight)/PI; //Fraction of 180 degree field from bank to bank

	float diffuseLeft		=  ((1 - (pSubnodeInfo->vegDensityLeft/100)) * shadedAngleLeft)*m_diffuseRad; //Diffuse rad coming through trees on left bank

	float diffuseRight		=  ((1 - (pSubnodeInfo->vegDensityRight/100)) * shadedAngleRight)*m_diffuseRad; //Diffuse rad coming through trees on right
	
	//Diffuse Radiation through sky opening + Diffuse Radiation through vegetation//
	float streamDiffuseRad = pSubnodeInfo->skyOpen * m_diffuseRad + diffuseLeft + diffuseRight; 
	//float streamDiffuseRad = 200.0f;
   //streamBeamRad = 2800.0f;

   float solarFlux = (streamBeamRad + streamDiffuseRad)*(1-streamAlbedo);   //kJ/m^2 * h

   pSubnodeInfo->heatFlux = streamBeamRad;
   /*//Absorb a portion of the solar flux into the bedrock
   float bedAbsorb = solarFlux * m_percentAbsorb/100; 
   pSubnodeInfo->bedFlux += (float)bedAbsorb*(float)surfaceArea*m_timeStep*24;


   
   if (solarFlux <= 0.0f)
      {
      m_bedFlux = 0.0;
      float increment = (1/(24*m_timeStep))*8.0f;
      m_bedFlux = pSubnodeInfo->bedFlux/increment; //releases stored solar energy evenly over 8 hours 
      //pNode->m_reachInfo.subnodeInfoArray[j]->advectionFlux = m_bedFlux;
      
      if (m_bedFlux == 0.0f)
         pSubnodeInfo->bedFluxCounter = 0.0f;
      else
         pSubnodeInfo->bedFluxCounter += (1.0f/increment);
      
      if (pSubnodeInfo->bedFluxCounter >= 1.0f)
         {
         pSubnodeInfo->bedFlux = 0.0f;
         pSubnodeInfo->bedFluxCounter = 0.0f;
         }
      }
   */
   solarFlux = ((solarFlux*(1.0f - m_cloudFrac))); // - (float)bedAbsorb);

   //return value of solar flux
	return solarFlux;	
   }

     
float WET_Temp::GetAirTemp(float time)
	{
	
   float tempF = m_pClimateData->IGet(time, 1, IM_LINEAR);
   
   /*Generation of Sinusoidal Air Temperature Distribution from daily high and low temps
   float low, high, amplitude, median, tempF;

   //int timeint = (int)time;
	
	//float hour = (float)fmod(time,timeint)*24;
  
   //each day, estimate an sinusoidal air temperature distribution based on the daily high and low temp
   if (fmod(time, timeint) <= m_timeStep)
      {
      high = m_pAirTempData->IGet(timeint, 1, IM_LINEAR);
      m_dailyHighAirT = high;

      low = m_pAirTempData->IGet(timeint, 2, IM_LINEAR);
      m_dailyLowAirT = low;
      }

   median = (m_dailyHighAirT + m_dailyLowAirT)/2.0f;

   amplitude = (m_dailyHighAirT - m_dailyLowAirT)/2.0f;
      
   tempF = median + amplitude*sin((hour*(2*PI)/24.0f) + 9.7f);
   */
   
   
   //Old hard code way
   //float tempF = (float) ( 61.68488f + 8.812435f*sin(( hour*((2.0f*PI)/24.0f) ) + 9.637652f) );

	return (tempF - 32.0f)*(5.0f/9.0f);   //Return temp in degrees C
	}



float WET_Temp::GetRH(float time)
	{
	//int timeint = (int)time;
	
	//float hour = (float)fmod(time,timeint)*24;

	float dewPointF = m_pClimateData->IGet(time, 2, IM_LINEAR);
   float dewPointC = (dewPointF - 32.0f)*(5.0f/9.0f);

   float tempF = m_pClimateData->IGet(time, 1, IM_LINEAR);
   float tempC = (tempF - 32.0f)*(5.0f/9.0f);

   
   //Relative Humidity is the ratio between saturation vapor pressure at the dew point and
   //saturation vapor pressure at the current air temperature

   float eSat_dew, eSat_air;

   eSat_dew = 6.11f*(float)exp((17.3f*dewPointC)/(dewPointC + 237.3f));
   
   eSat_air = 6.11f*(float)exp((17.3f*tempC)/(tempC + 237.3f));
   
   float RH = 100*(eSat_dew / eSat_air);

   return RH;
   
   //Old hard code way
   //return (float) ( 75.14586f + 16.41609f*sin(( hour*((2.0f*PI)/24.0f) ) + 0.207626f)  ); 
	}

float WET_Temp::GetWindSpeed(float time)
   {
   float mphWindSpeed = m_pClimateData->IGet(time, 3, IM_LINEAR);
   
   float windSpeed = mphWindSpeed*(5280.0f/(3600.0f*3.28f));  // coverted to m/s

   return windSpeed;
   }


float WET_Temp::GetLongwaveFlux(ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, float time, int j, float temp)
	{
	 //Also need to estimate value of cloudFraction.  Weather data on this????
	//float cloudFraction = 0.1f;	//??? Temporary.
	//float airTemp = GetAirTemp( time );
	
	//float sigma = 1.355e-08f;  //   cal/m^2*s*K^4    Stefan-Boltzman Contant
   float sigma = 2.0412e-07f; //kJ/m^2*h*K^4        Stefan-Boltzman Contant

   //apparent emissivity  Lee page 67
	//float emissivity = 1.0f - 0.261f * (float)exp(-7.77e-4f * (float)pow((273.0f - (airTemp+273)),2));

	//float emissivity = clearSkyEmissivity + (1 - clearSkyEmissivity) * m_cloudFrac; 
	
	//float atmosphericLongwave = sigma*emissivity*(float)pow((airTemp + 273.0f),4)*(1.0f+(0.17f*.01f));  //.01 is cloudiness coefficient (.1 ) squared
   //float atmosphericLongwave = sigma*(float)pow(((airTemp + 273.0f) - (20.0f * (1.0f - m_cloudFrac))), 4);

	//Alternate atmopsheric longwave calculation  uses RH rather than cloud frac
   //float relativeHumidity = GetRH( time );
   //page 193 in McCutcheon
	float satVaporPressure = (float)exp(2.3026f*(((7.5f * m_airTemp)/(m_airTemp + 237.3f))+ 0.7858f));  //  units-> mbar
	//Dingman correlation
	float atmVaporPressure = (m_relativeHumidity * satVaporPressure)/100.0f;
   //float atmEmissivity = 0.53+0.065*(pow(atmVaporPressure, 0.5));  //Dingman pg 189
   //float atmEmissivity = 0.95f;
   float atmosphericLongwave = sigma*(float)pow( ((m_airTemp +273.0f) - ( 20.0f - (0.9f*(m_relativeHumidity/5.0f)) ) ),4);
   
   
   //Longwave coming from open gap in vegetation
   float atmosphericLongwaveOpen = atmosphericLongwave * pSubnodeInfo->skyOpen;
     //Richard Lee - Forest Microclimatology pg. 67

	float shadedAngleLeft	=  (pSubnodeInfo->vegAngleLeft - pSubnodeInfo->topoAngleLeft)/PI;  //Fraction of 180 degree field from bank to bank
   float shadedAngleRight	=  (pSubnodeInfo->vegAngleRight - pSubnodeInfo->topoAngleRight)/PI; //Fraction of 180 degree field from bank to bank

	//Next two functions account for longwave coming through the gaps in the canopy
   float atmosphericLongwaveRight = (1 - (pSubnodeInfo->vegDensityRight/100.0f)) * shadedAngleRight * atmosphericLongwave;
   float atmosphericLongwaveLeft  = (1 - (pSubnodeInfo->vegDensityLeft/100.0f)) * shadedAngleLeft * atmosphericLongwave;
	
   //float netAtmosphericLongwave = atmosphericLongwaveLeft + atmosphericLongwaveRight + atmosphericLongwaveOpen;
	float netAtmosphericLongwave = atmosphericLongwaveOpen;


   //longwave emitted from vegetation as a function of temperature
   float vegLongwave = 0.95f*sigma*(float)pow(((m_airTemp) +273),4.0f);
	
   //Next two function account from longwave radiation coming from the vegetation on both sides based on amount of vegetation
	//float radVegLeft = (pSubnodeInfo->vegDensityLeft/100) * shadedAngleLeft * vegLongwave;
   //float radVegRight = (pSubnodeInfo->vegDensityRight/100) * shadedAngleRight * vegLongwave;
   
   //float netVegLongwave = radVegLeft + radVegRight;
   float netVegLongwave = vegLongwave * (1.0f - pSubnodeInfo->skyOpen);
   
	float streamLongwave = 0.97f*sigma*(float)pow((temp + 273.0f),4.0f);
	//???Need upstream temp input into this function.  Figure this out later
	//0.97 is estimate of stream emmissivity

	//float vegRadiatingArea = (2/PI)*(((pSubnodeInfo->vegDensityLeft/100) * pSubnodeInfo->vegAngleLeft) + ((pSubnodeInfo->vegDensityRight/100) * pSubnodeInfo->vegAngleRight));//???
	
	//???Assumption that airTemp = Tvegetation
	//0.97 is estimate of vegetation emissivity
	
	float longwaveFlux = netAtmosphericLongwave + netVegLongwave - streamLongwave;
	
   return longwaveFlux;  //kJ/m^2*h
   }


float WET_Temp::GetEvaporationFlux(SUBNODE_INFO_TEMP *pSubnodeInfo, float time, float temp)
	{
	//Need to obtain estimates of air temp, RH and wind speed.
   
   //wind speed data from file
   int timeint = (int)time;
	float hour = (float)fmod(time,(float)timeint);
   float windSpeed = GetWindSpeed(time);
   
	//Wind speed from Corvallis agrimet
   //float mphWindSpeed			= 4.5f;			// mph  from Corvallis agrimet station
   //float windSpeed = mphWindSpeed*(5280.0f/(3600.0f*3.28f));  // coverted to m/s

	//float airTemp = GetAirTemp( time );

	//float relativeHumidity = GetRH( time );

	//page 193 in McCutcheon
	float satVaporPressure = (float)exp(2.3026f*(((7.5f * m_airTemp)/(m_airTemp + 237.3f))+ 0.7858f));  //  units-> mbar
	satVaporPressure = satVaporPressure/10;  //convert into kPa for Bowie correlation
	
	//Dingman correlation
	float atmVaporPressure = (m_relativeHumidity * satVaporPressure)/100.0f;

   float deltaE = satVaporPressure - atmVaporPressure;

	///////////////////////////
   //Calculation of Bowen ratio (used in determination of convective flux
   float pressureCorrect = (float) pow(((288.0f - 0.0065f*pSubnodeInfo->subnodeElev) / 288.0f) , 5.256f);
   
   float totalPressure  = (1000*pressureCorrect)/10;  //kPa

   m_bowenRatio = 0.01f * (((float)temp - m_airTemp)/(satVaporPressure - atmVaporPressure))*(totalPressure / 29.92f);

   
   //Evaporation parameters input by model user as one method of calibration.
   //Coefficients from Bowie et al for San Diego aqueduct are a=3.11, b=1.13
   float evaporationRate = (m_evapA + m_evapB*windSpeed)*(satVaporPressure - atmVaporPressure);

	//Now we need to determine evaporation flux, which = pWater * LatentHeatVapor * evaporationRate
	float latentHeat = 4.187f*(597.3f - 0.564f*temp);  //pg 258 Dingman  (units adjusted to KJ/kg)

	float evaporationFlux = -(m_waterDensity * latentHeat * evaporationRate)/ 24000;

	return evaporationFlux;  // kJ/m^2*h 
	}


float WET_Temp::GetConvectionFlux( float evaporationFlux )
   {
   //Calculate BOWEN ratio
   float convectionFlux = m_bowenRatio * evaporationFlux;

   return convectionFlux;

   }








// this function returns the temperature of water flowing out of the specified reach
float WET_Temp::GetOutputTemp( ReachNode *pNode)
   {
   // if phantom  node, look upstream 
   if ( pNode->IsPhantomNode() )
      {
      ASSERT( pNode->m_pRight != NULL );
      ASSERT( pNode->m_pLeft  != NULL );

      float leftFlow  = pNode->GetOutputFlow();
      float rightFlow = pNode->GetOutputFlow();
      float leftTemp  = GetOutputTemp( pNode->m_pLeft );
      float rightTemp = GetOutputTemp( pNode->m_pRight );

      return ( leftFlow * leftTemp + rightFlow *rightTemp ) / ( leftFlow + rightFlow );
      }

   else
      {
      
      // get the temperature info assocaited with this reach
      REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      int lastNode = pNode->m_reachInfo.subnodeCount-1;

      SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[ lastNode ];
      return pSubnodeInfo->streamTemperature;        // temperature at this subnode
      
      }
   }



float WET_Temp::GetOutputFlow( ReachNode *pNode )
   {
   return pNode->GetOutputFlow();
   }


   
   
void WET_Temp::CollectData( float time )
   {
   static int counter = 0;
    
   ////////////////////////////////////////////////////////
   // Collect temperature data (reach-level)
   int dataToCollect = m_reachDataArray.GetSize() + 1;  // include time

   float *data = new float[ dataToCollect ];
   //float *data = new float[ 2 ];
   data[ 0 ] = time;
   
   float *heatFlux = new float[dataToCollect ];
   heatFlux[ 0 ] = time;

   float *solarFlux = new float[dataToCollect ];
   solarFlux[ 0 ] = time;

   float *longwaveFlux = new float[dataToCollect ];
   longwaveFlux[ 0 ] = time;

   float *evaporationFlux = new float[dataToCollect ];
   evaporationFlux[ 0 ] = time;

   float *convectionFlux = new float[dataToCollect ];
   convectionFlux[ 0 ] = time;

   float *advectionFlux = new float[dataToCollect ];
   advectionFlux[ 0 ] = time;


   ASSERT( m_pStreamLayer != NULL );
   DataObj *pStreamData = m_pStreamLayer->m_pData;


   // iterate through each polygon to collect data for temperature and energy components
   for ( int i=1; i < dataToCollect; i++ )
   {
   //if ( i == dataToCollect-1 )
     
      int reachIndex = m_reachDataArray[ i-1 ];

	   int lastNode = m_reachTree.m_nodeArray[reachIndex]->m_reachInfo.subnodeCount-1;

      float streamTemperature = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->streamTemperature;

      float streamFlow = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->streamFlow;

      data[ i ] = (float)streamTemperature;
      
      heatFlux[ i ] = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->heatFlux;
      
      solarFlux[ i ] = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->solarFlux;

      longwaveFlux[ i ] = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->longwaveFlux;

      evaporationFlux[ i ] = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->evaporationFlux;

      convectionFlux[ i ] = m_reachInfoTempArray[reachIndex]->subnodeInfoTempArray[lastNode]->convectionFlux;

      advectionFlux[ i ] = m_reachTree.m_nodeArray[reachIndex]->m_reachInfo.subnodeInfoArray[lastNode]->advectionFlux; 
         
    }
    //}
   
   m_pTemperatureData->AppendRow( data );
   
   m_pHeatFlux->AppendRow( heatFlux );
   m_pSolarFlux->AppendRow( solarFlux );
   m_pLongwaveFlux->AppendRow( longwaveFlux );
   m_pEvapFlux->AppendRow( evaporationFlux );
   m_pConvectionFlux->AppendRow( convectionFlux );
   m_pAdvectionFlux->AppendRow( advectionFlux );
  
   
   delete data;
   delete heatFlux;
   delete solarFlux;
   delete longwaveFlux;
   delete evaporationFlux;
   delete convectionFlux;
   delete advectionFlux;
   
   //////////////////////////////////////////////////////////
   // notify interface
   if ( counter % 10 == 0 )
      {
      //CString msg;
      //msg.Format( "Completed %g", time );
      //gpMain->SetStatus( msg );
      }

   counter++;
   }



void WET_Temp::InitObsValueInfo( )
   {
   int obsCount   = m_pObservedLayer->GetRecordCount();   

   int reachCount = m_reachTree.m_nodeCount-1;
   // allocate an array of pointers to DataObjs
   m_obsInfoArray.RemoveAll();

   //Create new Data objects at each point where observed temperature data is available
   for ( int k=0; k<obsCount; k++ )
      {
      // add it
      m_obsInfoArray.Add( OBSINFO() );
      
      // allocate data object for measured data
      m_obsInfoArray[ k ].pMeasured = new FDataObj( 2, 0 );   //2 cols, 0 rows
      
      // find its location in the reachtree
      REAL xObs, yObs;
      bool ok = m_pObservedLayer->GetPointCoords( k, xObs, yObs );
      ASSERT( ok );

      // iterate through tree, looking for closest node/subnode
      int reachIndex   = -1;
      int subnodeIndex = -1;
      REAL distance = 1.0e10;

      for ( int i=0; i < reachCount; i++ )
         {
         ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
         int subnodeCount = pNode->m_reachInfo.subnodeCount;

         for ( int j=0; j < subnodeCount; j++ )
            {
            REAL x = pNode->m_reachInfo.subnodeInfoArray[ j ]->location.x;
            REAL y = pNode->m_reachInfo.subnodeInfoArray[ j ]->location.y;

            REAL _distance = DistancePtToPt( x, y, xObs, yObs );
            
            if ( _distance < distance )
               {
               distance = _distance;
               reachIndex = i;
               subnodeIndex = j;
               }
            }  // end of:  for ( j < subnodeCount )
         }  // end of:  for ( i < reachCount )
            
      
      // Get reach_info_temp structure for the reach in which the observation is located
      ReachNode *pObsNode = m_reachTree.m_nodeArray[reachIndex];
      REACH_INFO_TEMP *pObsTemp = (REACH_INFO_TEMP*) pObsNode->m_reachInfo.pData;

      CString dataPath;
      bool ok_c = m_pObservedLayer->GetData( k, m_colDataPath, dataPath );
      // ASSERT( ok_c );
      pObsTemp->dataPath = dataPath;
      
      if ( dataPath != "NULL" )
         int rec = (int) m_obsInfoArray[k].pMeasured->ReadAscii(dataPath, ',', TRUE);
      else
          m_obsInfoArray[ k ].pMeasured = NULL;

      ASSERT( reachIndex != -1 );
      ASSERT( subnodeIndex != -1 );

      m_obsInfoArray[ k ].reachIndex   = reachIndex;
      m_obsInfoArray[ k ].subnodeIndex = subnodeIndex;
      }  
   }


void  WET_Temp::WriteObsValueInfo()
   {
   ASSERT( m_pCellLayer != NULL );
   char filename[ 256 ];
   lstrcpy( filename, m_pCellLayer->m_path );       // full path name for vector file

   char *start = strrchr( filename, '\\' );

   if ( start == NULL )  // no slash
      {
      filename[ 0 ] = NULL;
      start = filename;
      }
   else
      {
      start++;
      *start = NULL;
      }

   int obsCount = m_obsInfoArray.GetSize();

   //Create new Data objects at each point where observed temperature data is available
   for ( int k=0; k < obsCount; k++ )
      {
      // ready to append to string
      char temp[ 32 ];
      sprintf_s( temp, 31, "obs_%i_%i_%i", k, m_obsInfoArray[ k ].reachIndex, m_obsInfoArray[ k ].subnodeIndex );

      lstrcpy( start, temp );

      ASSERT( m_obsInfoArray[ k ].pData != NULL );
      m_obsInfoArray[ k ].pData->WriteAscii( filename );

      delete m_obsInfoArray[ k ].pData;
      m_obsInfoArray[ k ].pData = NULL;
      }
   }

//This function populates data objects with longitudinal temperature information
void WET_Temp::CollectLongitudinalData( float simDuration )
   {
   //3 columns, minTemp, maxTemp and distance from Sample Point
   float *data = new float[ 4 ];
   
   int longDataToCollect = m_reachDataArray.GetSize();  
   
   for ( int i=0; i < longDataToCollect; i++ )
      {
      int reachIndex = m_reachDataArray[ i ];
      data[ 0 ] = 0.0f;
   
      ReachNode *pNode = m_reachTree.GetReachNode( reachIndex );
      ASSERT( pNode != NULL );
      while ( pNode != NULL )
         {
         float deltaX = pNode->m_reachInfo.deltaX;
         int subnodeCount = pNode->m_reachInfo.subnodeCount;
         for ( int j=0; j < subnodeCount; j++ )
            {
            REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
            float maxTemp = pTemp->subnodeInfoTempArray[ j ]->Tmax;
            float minTemp = pTemp->subnodeInfoTempArray[ j ]->Tmin;
                  
            //This keeps graph from breaking when undefined values exists
            if (maxTemp == NULL) 
               data[ 1 ] = 0.0f;
            else
               data[ 1 ]= maxTemp;
                     
            if (minTemp == NULL) 
               data[ 2 ] = 0.0f;
            else
               data[ 2 ]= minTemp;

            data[3] = pNode->m_reachInfo.qArray[j];  //Discharge
                  
            m_pLongitudinalDataArray[ i ]->AppendRow( data );
            data[ 0 ] += deltaX;
            }
         

         pNode = pNode->m_pDown;

         while ( pNode != NULL && pNode->IsPhantomNode() ) // skip phantom nodes
            pNode = pNode->m_pDown;
         }
      
      WriteLongitudinalData( reachIndex, i );
   
      }
    
   delete [] data;
   }


//This function writes longitudinal data to a file
void WET_Temp::WriteLongitudinalData( int reachIndex, int i )
   {
   char filename[ 512 ];

   lstrcpy( filename, (LPCSTR) m_pCellLayer->m_path );       // full path name for vector file
   char *p = strrchr( filename, '\\' );

   if ( p == NULL )
      lstrcpy( filename, "\\output\\" );
   else
      lstrcpy( p+1, "output\\" );

   char _filename[ 256 ];
   
   if ( m_options & WT_OPTION_USE_ALT_LULC )
      sprintf_s( _filename, 255, "LongitudinalDataRestored_reach%i.csv", reachIndex );
   else
      sprintf_s( _filename, 255, "LongitudinalData_reach%i.csv", reachIndex );
   
   lstrcat( filename, _filename );
   m_pLongitudinalDataArray[ i ]->WriteAscii( filename );
   }


//This function write data used in sensitivity analysis to a file
void WET_Temp::WriteSensData()
   {
   //Open a file in datasets directory named "sens_dayOfYear" 
   ASSERT( m_pCellLayer != NULL );
   char filename[ 256 ];
   lstrcpy( filename, m_pCellLayer->m_path );       // full path name for vector file

   char *start = strrchr( filename, '\\' );

   if ( start == NULL )  // no slash
      {
      filename[ 0 ] = NULL;
      start = filename;
      }
   else
      {
      start++;
      *start = NULL;
      }

   char temp[ 32 ];
   sprintf_s(temp, 32, "sens_%i", m_dayOfYear );
   lstrcpy( start, temp );

   
   //create a data object containing the information to be written to the file 
   if ( m_pSensData != NULL )
      delete m_pSensData;

   m_pSensData = new FDataObj( 9, 0 );
   
   float data[ 9 ];
   m_pSensData->SetLabel( 0, "reachID" );
   m_pSensData->SetLabel( 1, "index" );
   m_pSensData->SetLabel( 2, "distToMouth(km)" );
   m_pSensData->SetLabel( 3, "areaDrained(km^2)" );
   m_pSensData->SetLabel( 4, "Flow(m^3/s)" );
   m_pSensData->SetLabel( 5, "Tmax" );
   m_pSensData->SetLabel( 6, "Tmin" );
   m_pSensData->SetLabel( 7, "Time of Max" );
   m_pSensData->SetLabel( 8, "Time of Min" );
   
   int reachCount = GetReachCount();

   for (int i=0; i < reachCount; i++)
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[i];
   
      REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

      int subnodeCount = pNode->m_reachInfo.subnodeCount;
      SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[ subnodeCount - 1 ];
      
      data[0] = (float)pNode->m_reachInfo.reachID;
      data[1] = (float)pNode->m_reachInfo.index;
      data[2] = pNode->m_reachInfo.distToMouth/1000;
      data[3] = pNode->m_reachInfo.areaDrained/(1000*1000);
      data[4] = pSubnodeInfo->streamFlow;
      data[5] = (float)pSubnodeInfo->Tmax;
      data[6] = (float)pSubnodeInfo->Tmin;
      data[7] = pSubnodeInfo->timeOfMax;
      data[8] = pSubnodeInfo->timeOfMin;
   
      m_pSensData->AppendRow(data);
      }

   ASSERT( m_pSensData != NULL );
   m_pSensData->WriteAscii( filename );
   
   //delete data object
   delete m_pSensData;
   m_pSensData = NULL;
   }
	      

void WET_Temp::CollectCalibrationData(float time)
   {
   ASSERT (m_pObservedLayer != NULL);
   int obsCount   = m_pObservedLayer->GetRecordCount();

   ASSERT( m_obsInfoArray.GetSize() == obsCount );
   
   //Locate the points that correspond with the observed data,
   //then store time and stream temperature
   float data[ 6 ];

   for ( int k=0; k < obsCount; k++ )
      {
      int reachIndex   = m_obsInfoArray[ k ].reachIndex;
      int subnodeIndex = m_obsInfoArray[ k ].subnodeIndex;

      ASSERT( reachIndex   != -1 );
      ASSERT( subnodeIndex != -1 );

      //Stream Temperature at observation point
      ReachNode *pNode = m_reachTree.m_nodeArray[reachIndex];
      REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
      float streamTemp = (float)pTemp->subnodeInfoTempArray[subnodeIndex]->streamTemperature;
      
      //Stream Temperature at mouth
      ReachNode *pMouth = m_reachTree.m_nodeArray[m_mouthIndex];
      REACH_INFO_TEMP *pTempMouth = (REACH_INFO_TEMP*) pMouth->m_reachInfo.pData;
      float mouthTemp = (float)pTempMouth->subnodeInfoTempArray[ 1 ]->streamTemperature;
      
      data[ 0 ] = time;
      data[ 1 ] = streamTemp;
      
      //Get measured temp value from data object, if temp data exists at that point
      if (m_obsInfoArray[ k ].pMeasured == NULL)
         data[ 2 ] = 0.0f;   
      else
         {
         float measuredTemp = m_obsInfoArray[ k ].pMeasured->IGet(time, 1, IM_LINEAR);
			data[ 2 ] = measuredTemp;
         }   

      //slope metric for temperature profile
      data[ 3 ] = ( mouthTemp - streamTemp )/(pNode->m_reachInfo.distToMouth/1000);  //degrees C per km
      
      //Get measured temperature at mouth (observation zero in shapefile, by convention)
      //obs 2 for McDowell Creek
      
      //float measuredTempAtMouth = m_obsInfoArray[ 2 ].pMeasured->IGet(time, 1, IM_LINEAR);
      //data[ 4 ] = ( measuredTempAtMouth - data[ 2 ] )/(pNode->m_reachInfo.distToMouth/1000);  //degrees C per km
      data[ 4 ] = 0.0f;   //Temporary until UGR measured data is input  9/5/02 mmc


      data[ 5 ] = pTemp->subnodeInfoTempArray[ subnodeIndex ]->solarFlux/100.0f;

      ASSERT( m_obsInfoArray[ k ].pData != NULL );
      m_obsInfoArray[ k ].pData->AppendRow( data );

      }//end of : for (k < obsCount)
   }



void WET_Temp::EndRun( void )
   {
   // do mass balance check on each stream reach
   int reachCount = m_pStreamLayer->GetRecordCount();

   int colOutputFlow = m_pStreamLayer->GetFieldCol( "SLOPE" );  // use slope for now
   ASSERT( colOutputFlow >= 0 );

   float tempMax = 0.0f;

   for ( int i=0; i < reachCount; i++ )
      {
      ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

      // put final q values for each reach into the Stream layer
      //float temperature = pNode->GetTemperature();

      //if ( temperature > tempMax )
      //   tempMax = temperature;
      }

   m_pStreamLayer->SetActiveField( colOutputFlow );
   m_pStreamLayer->SetBinColorFlag( BCF_BLUE );
   m_pStreamLayer->SetBins( 0.0f, tempMax, 20 );
   m_pStreamLayer->ClassifyData();
   m_pMap->RedrawWindow();
   }






bool WET_Temp::SavePreprocessingData( LPCSTR filename )
{
	int nodeCount = m_reachTree.m_nodeCount;
	int currentSubnode = 0;

	//Open a file where preprocessing data will be stored
	
	//Make sure the file exists
   FILE *fp;
   fopen_s( &fp, filename, "wb" );

	 if ( fp == NULL )
      {
      CString msg( "Unable to find file " );
      msg += filename;
      ErrorMsg( msg );
      return false;
      }


	//Cycle through reaches
	int nodecheck = fwrite((void*)&nodeCount, sizeof(int), 1, fp);
	for ( int i=0; i < nodeCount; i++ )
		{  
		ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
	  
		int reachID = pNode->m_reachInfo.reachID;
		int subnodeCount = pNode->m_reachInfo.subnodeCount;
		
		int ID  = fwrite((void*)&reachID, sizeof(int), 1, fp);
		int snc = fwrite((void*)&subnodeCount, sizeof(int), 1, fp);

		REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
	
		//Cycle through subnodes in each reach
		for ( int j=0;  j< subnodeCount; j++ )
			{
			SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[j];
			fwrite((void*)pSubnodeInfo, sizeof(SUBNODE_INFO_TEMP), 1, fp);
			}

		}
   fclose(fp);
   return true;
   }


bool WET_Temp::ReadPreprocessingData( FILE *fp )
	{
	int nodeCount;
	int count = fread((void*)&nodeCount, sizeof(int), 1, fp);
	if ( count != 1 )
		return false;
	int subnodeCount, reachID;

	for ( int i=0; i < nodeCount; i++ )
		{
		ReachNode *pNode = m_reachTree.m_nodeArray[ i ];

		count = fread((void*)&reachID, sizeof(int), 1, fp);
		if ( count != 1 )
		return false;

		count = fread((void*)&subnodeCount, sizeof(int), 1, fp);
		if ( count != 1 )
		return false;
		
		ASSERT(subnodeCount == pNode->m_reachInfo.subnodeCount);

		REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
	
		for ( int j=0; j < subnodeCount; j++ )
			{
			SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[j];
			fread((void*)pSubnodeInfo, sizeof(SUBNODE_INFO_TEMP), 1, fp);
			if ( count != 1 )
			return false;
			}

		}

	return true;
	}

///Output file for examing model variables of choice
/*
bool WET_Temp::SaveFormattedData(float time, ReachNode *pNode, int j, float flow, float lateralInflow)
{
	int nodeCount = m_reachTree.m_nodeCount;
	int currentSubnode = 0;
   int reachID = pNode->m_reachInfo.reachID;
	
	
	//Open a file where formatted data will be stored
	FILE *fp = fopen( "c:\\formdata.wt", "a+" );

    //Make sure the file exists
	 if ( fp == NULL )
      {
      CString msg( "Unable to find file " );
      msg += "formdata.wt";
      ErrorMsg( msg );
      return false;
      }

	fprintf(fp, "ID: %5d time: %5f, subnode: %5d, flow: %5f  laIn: %5f\n", reachID, time, j, flow, lateralInflow);
         

    //Cycle through reaches
	//int nodecheck = fwrite((void*)&nodeCount, sizeof(int), 1, fp);
	//for ( int i=0; i < nodeCount; i++ )
		//{  
		//ReachNode *pNode = m_reachTree.m_nodeArray[ i ];
	  
		//int reachID = pNode->m_reachInfo.reachID;
		//int subnodeCount = pNode->m_reachInfo.subnodeCount;
      //int index = pNode->m_reachInfo.index;
		
		//int ID  = fprintf(fp, "\n\nReachID  %10d\n", reachID);
		//int snc = fprintf(fp, "SubnodeCount  %5d\n", subnodeCount);
      //int dex = fprintf(fp, "Index  %5d\n\n", index);

		//REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;
	
		//Cycle through states to view hydro Data
   
         //int stateCount = m_pDataStore->GetStateCount();

       //for (int t=0;  t< 1; t++)
         //{
         //SAVED_STATE_INFO *pGetState = m_pDataStore->m_savedDataPtrArray[t];
   
         //float tim = pGetState->time;

         //
         
         
         //Cycle through subnodes in each reach
		   //for ( int j=0;  j< subnodeCount; j++ )
			   //{
		   	//SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[j];
			      
            //float flw = pSubnodeInfo->streamFlow;
            //float temp = pSubnodeInfo->streamTemperature;
            //float depth = pSubnodeInfo->streamDepth;
            //float lat = pSubnodeInfo->lateralInflow;
            //if (j == 0)
               //fprintf(fp, "Subnode %5d time %5f ---> TEMP || Qin:Qout:LatIn   %5f ||   %5f : %5f : %5f\n", j, time, T, Qin, Qout, latIn);
            //}
		   //}
      
      	
		//}
fclose(fp);
return true;
}
*/


//Used to get flow data from Wet_Hydro
/*
void WET_Temp::GetHydro( ReachNode *pNode, float time, int i, int j, float &flow, float &lateralInflow )
   {
	
   int stateCount = m_pDataStore->GetStateCount();

   int timeStatic = (int) time;

   float timeUp = 0.0f, timeDown= 0.0f;
   
   for (int t=0;  t<= stateCount-1; t++)
      {
      SAVED_STATE_INFO *pGetState = m_pDataStore->m_savedDataPtrArray[t];
      
      SAVED_STATE_INFO *pGetLastState = NULL;
      if (t !=0)
         pGetLastState = m_pDataStore->m_savedDataPtrArray[t-1];
 
      if (t == 0)
         {
         bool ok = m_pDataStore->GetSubnodeInfo(t, i, j, flow);
         ASSERT(ok);
         bool okr= m_pDataStore->GetReachInfo(t,i, lateralInflow);
         ASSERT( okr );
         }
      else
         {
         int year = (int)pGetState->time/1000;
         timeUp   = pGetState->time;
         timeDown = pGetLastState->time;

         //This converts my time convention into yyddd  i.e. 99357 etc..  used by Wet-hydro
         time = time+year*1000;

         if (time >= timeUp)
            {
            time = (float) timeStatic;            
            continue;
            }
         else
            break;
         
      
         //This is an interpolation to determine which flow data to use
         float gapBack = time - timeDown;
         float gapForward = timeUp - time;
      

         if (gapBack > gapForward)
            {
            bool ok = m_pDataStore->GetSubnodeInfo(t, i, j, flow);
            ASSERT( ok );
            bool okr= m_pDataStore->GetReachInfo(t,i,lateralInflow);
            ASSERT( okr );
            }
         else
            {
            bool ok =  m_pDataStore->GetSubnodeInfo(t-1, i, j, flow);
            ASSERT( ok );
            bool okr= m_pDataStore->GetReachInfo(t,i,lateralInflow);
            ASSERT( okr );
         
            }
         }
      }
   }
*/

///Used to get Wet_Hydro flow inputs
/*
void WET_Temp::GetHydroData(ReachNode *pNode, float time)
   {
   
    // cycle through EACH SUBNODE in the finite difference grid
    for ( int j=0; j < pNode->m_reachInfo.subnodeCount; j++ )
		 {
       float flow = 0.0f, lateralInflow = 0.0f;
       int reachIndex = pNode->m_reachInfo.index;

       GetHydro(pNode,time,reachIndex,j, flow, lateralInflow);
     
       //Define pointer to the SUBNODE_INFO_TEMP at this subnode
       REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

       SUBNODE_INFO_TEMP *pSubnodeInfo = pTemp->subnodeInfoTempArray[ j ];
      
       //Place hydro information into subnode structure for access at this timestep 
       pSubnodeInfo->lateralInflow= lateralInflow;
       pSubnodeInfo->streamFlow   = flow;
       //if(time <= 220.1)
         //SaveFormattedData(time, pNode, j, flow, lateralInflow);
       }
     
   }
*/

float WET_Temp::GetDepthFromQ( ReachNode *pNode, float Q, int j)
   {
   REACH_INFO_TEMP *pTemp = (REACH_INFO_TEMP*) pNode->m_reachInfo.pData;

   float n = m_manningN;  // Manning's roughness. Input by model user.
   float wdr = pTemp->wdRatio; // Width/Depth ratio.  Input by model user.
   float slope = pTemp->subnodeInfoTempArray[j]->subnodeSlope;  //rise/run
   
            
   //Temporary, testing constant slope scenarios
   //float slope = 0.01;
   
   //Assuming trapezoidal geometry, not quite correct as of 11/21/01.  Potential to be used later/
   /*float ssangle = 0.916;   //side slope angle in radians  (average from 23 field sites)

   float K = pow((Q/(n*sqrt(pNode->m_reachInfo.slope))), 1.5);  //Constant terms in depth equation(grouped for clarity)

   //Assuming a TRAPEZOIDAL channel, depth is represented by a quadratic equation in the 
   //for ad^2 + bd +c.  The constants are as follows:

   float a = 1.0f/tan(ssangle);
   float b = width - (2*K/sin(ssangle));
   float c = -width*K;

   float depth = (-b - sqrt((b*b)-4*a*c))/2*a;*/
   
   //float depth = (float) pow( ( 3.0f/25.0f * Q * n * (pNode->m_reachInfo.slope * pNode->m_reachInfo.slope) ),  3.0f/8.0f );
   
   //depth equation broken into two pieces for clarity
   float wdterm = (float) pow( (wdr/( 2 + wdr )), 2.0f/3.0f)*wdr;
   float depth  = (float) pow(((Q*n)/((float)sqrt(slope)*wdterm)), 3.0f/8.0f);
   /*
   CString msg;
   msg.Format( "n %g, wdr %g, slope %g, flow %g, depth %g", n, wdr, slope, Q, depth);
   ErrorMsg( msg );
   */
   return (float) depth;
   }


/*void WET_Temp::SetReachGeometry( ReachNode *pNode, float Q)
   {
   //REACH_INFO_HYDRO *pHydro = (REACH_INFO_HYDRO*) pNode->m_reachInfo.pData;

   pHydro->depth = GetDepthFromQ( pNode, Q );
   pHydro->width = 10 * pHydro->depth;
   }
*/



//---------------------- Integration Methods ----------------------//

//-- IntRK4() -------------------------------------------------------
//
//--- general purpose Runge-Kutta integrator (fourth order)
//    code is modified from "Numerical Recipes"
//
//--INPUTS
//   int     svCount    = state variable count
//   float  *stateVar   = state variable value array
//   double *derivative = derivative value array
//   float  *time      = time variable address
//   float   dt         = timestep
//-------------------------------------------------------------------

int WET_Temp::IntRK4( float time )
   {
   int  i;   // loop counter

   
   //////////////////Euler solution/////////////////////////

   RunTemperatureModel( m_k1, time );  //get derivative
   
   for (i=0; i < m_svCount; i++)
      m_sv[ i ] = m_sv[ i ] + m_k1[ i ] * m_timeStep;  //Add derivative * timestep to sv[]
     
   /////////////// END Euler solution ////////////////////////
   
   
   /*
   ////////////START RK4 code/////////////////////////////////
   for ( i=0; i < m_svCount; i++ )       // remember initial values
      m_y[ i ] = m_sv[ i ];
   
   //-- pass 1 --//
   RunTemperatureModel( m_k1, time );     // get derivatives

   for ( i=0; i < m_svCount; i++ )       // update state variables
      m_sv[ i ] = m_y[ i ] + m_k1[ i ] * m_timeStep/2;

   //-- pass 2 --//
   time += m_timeStep/2;
   RunTemperatureModel( m_k2, time );     // get derivatives

   for ( i=0; i < m_svCount; i++ )       // update state variables
      m_sv[ i ] = m_y[ i ] + m_k2[ i ] * m_timeStep/2;

   //- pass 3 --//
   RunTemperatureModel( m_k3, time );     // get derivatives

   for ( i=0; i < m_svCount; i++ )
      m_sv[ i ] = m_y[ i ] + m_k3[ i ] * m_timeStep;    // update state variables

   //-- pass 4 --//
   time += m_timeStep/2;
   RunTemperatureModel( m_k4, time );     // get derivatives

   for ( i=0; i < m_svCount; i++ )
      m_sv[ i ] = m_y[ i ] + ( m_k1[ i ] + 2*m_k2[ i ] + 2*m_k3[ i ] + m_k4[ i ] ) * m_timeStep/6;
   ////////////////////END RK4 code///////////////////////////////////
   */

   return 1;
   }



/*bool WET_Temp::IntRKF( ReachNode *pNode, float time )
   {
   
   
   int reachCount = GetReachCount();
   int svCount = 0;

   for ( int i=0; i < reachCount; i++ )
		{
      REACH_INFO_HYDRO *pHydro = m_reachInfoHydroArray[ i ];
      svCount += pHydro->upslopeInfoArray.GetSize();
      }

   //TODO:  WONT WORK FOR MULTIRUNS WITH DIFFERENT UPSLOPE DEFINITIONS - NEED TO REALLOCATE ARRAYS BELOW
   if ( m_k1 == NULL )
      {
      m_k1 = new double [ svCount ] ;
      m_k2 = new double [ svCount ] ;
      m_k3 = new double [ svCount ] ;
      m_k4 = new double [ svCount ] ;
      m_k5 = new double [ svCount ] ;
      m_k6 = new double [ svCount ] ;

      m_derivative = new double[ svCount ];
      m_svTemp     = new double[ svCount ] ;
      m_stateVar   = new double[ svCount ] ;
      }

   // initialize (remember) state variables
   int count = 0;
   for ( i=0; i < reachCount; i++ )
		{
      REACH_INFO_HYDRO *pHydro = m_reachInfoHydroArray[ i ];

      for ( int j=0; j < pHydro->upslopeInfoArray.GetSize(); j++ )
         {
         UPSLOPE_INFO *pUpslope = pHydro->upslopeInfoArray[ j ];

         m_stateVar[ count ] = pUpslope->volume;
         count++;
         }
      }


   double error;
   double sum;

   double delta    = 0;
   //double curTime  = sysCurrentTime;
   double timeStep = m_timeStep;

   
   
   //-- get derivative values from object at current time --//   
   GetDerivatives( m_derivative, curTime, a );
   

   for ( i=0; i < svCount; i++ )
      {
      m_k1[ i ] = timeStep * m_derivative[i];
      m_svTemp[i] = m_stateVar[i] + m_k1[i] / 4;
      }

   //-- get derivative values from object at current time + 1/4--//
   GetDerivatives( m_derivative, (float)( curTime+timeStep/4 ), a );

   for (i=0; i < svCount; i++)
      {
      m_k2[i] = timeStep * m_derivative[i];
      m_svTemp[i] = m_stateVar[i] + ( m_k1[i]*3 + m_k2[i]*9 ) / 32;
      }

   //-- get derivative values from object at current time + -//
   GetDerivatives( m_derivative, (float)( curTime+timeStep*3/8 ), a );

   for (i=0; i < svCount; i++)
      {
      m_k3[i] = timeStep * m_derivative[i];
      m_svTemp[i] = m_stateVar[i]
                  + ( m_k1[i]*1932 - m_k2[i]*7200 + m_k3[i]*7296) / 2197;
      }

   //-- get derivative values from object at current time + -//
   GetDerivatives( m_derivative, (float)(curTime+timeStep*12/13), a );

   for (i=0; i < svCount; i++)
      {
      m_k4[i] = timeStep * m_derivative[i];
      m_svTemp[i] = m_stateVar[i]
            - (m_k1[i]*439/216 - 8*m_k2[i] + m_k3[i]*3680/513 - m_k4[i]*845/4104);
      }

   //-- get derivative values from object at current time + -//
   GetDerivatives( m_derivative, (float) ( curTime+timeStep), a );

   for (i=0; i < svCount; i++)
      {
      m_k5[i] = timeStep * m_derivative[i];
      m_svTemp[i] = m_stateVar[i]
        - m_k1[i]*8/27 + 2*m_k2[i]+m_k3[i]*3544/2565-m_k4[i]*1859/4104-m_k5[i]*11/40 ;
      }

   //-- get derivative values from object at current time + -//
   GetDerivatives( m_derivative, (float)(curTime+timeStep/2), a );

   for ( i=0; i< svCount; i++ )
      m_k6[i] = timeStep * m_derivative[i];


   //--------------- done with k values ------------//
   sum = 0.0;
   error = 0.0;

   //-- all k values calculated, calculate 4th and 5th degree terms --//
   for ( i=0; i < svCount; i++ )
      {
      sum = fabs ( m_k1[i]/360 - m_k3[i]*128/4275 - m_k4[i]*2197/75204 +
                   m_k5[i]/50 + m_k6[i]*2/55) ;

      if ( m_stateVar[ i ] != 0.0 )
         error = max( error, fabs( sum / m_stateVar[ i ] ) );
      }

   //-- update state variables --//
   for ( i=0; i < reachCount; i++ )
		{
      REACH_INFO_HYDRO *pHydro = m_reachInfoHydroArray[ i ];

      for ( int j=0; j < pHydro->upslopeInfoArray.GetSize(); j++ )
         {
         UPSLOPE_INFO *pUpslope = pHydro->upslopeInfoArray[ j ];

         float volume = (float)( m_stateVar[ i ] +  ( m_k1[i]*16/135 + m_k3[i]*6656/12825 + m_k4[i]* 
               28561/56430 - m_k5[i]*9/50 + m_k6[i]*2/55) );

         ASSERT( volume < 1.0e20 );
//         ASSERT( volume >= 0 );

         pUpslope->volume = volume;
         }
      }

   if ( m_useVariableStep )
      {
      //-- increment time  --//
      curTime = (float) (curTime+timeStep);

      if ( error == 0 )
         delta = 5;
      else
         delta = pow( m_rkvTolerance / ( 2*fabs( error ) ),  0.25 );  /// todo

      //-- don't change the timeStep dramatically because it makes a
      //-- difference by factor of 16

      //-- never increase delta more than 5 --//
      if ( delta > 5 )        // get time step multiplier value...
         delta = 5;
      else if ( delta < 0.1 )
         delta = 0.1;

      //-- calculate new timeStep --//  
      timeStep *= delta;

      //ASSERT( timeStep > 0 );

      //-- check the sign --//
      if (  timeStep > m_rkvMaxTimeStep )
        timeStep = m_rkvMaxTimeStep;

      else if ( timeStep < m_rkvMinTimeStep )
        timeStep = m_rkvMinTimeStep;

      //-- check singular differential equation case --//
      //ASSERT( fabs( timeStep ) >= m_rkvMinTimeStep );


      if ( ( timeStep > 0 ) && ((curTime+timeStep) > m_stopTime ) )         
         {
         //-- step just past end of simulation --//
   //      timeStep = m_stopTime - curTime;
           curTime = m_stopTime; 
         }

      ASSERT( timeStep > 0.0f );
   
      m_timeStep = (float) timeStep;
      }  // end of: if ( m_variableStep == true )


  return TRUE;
  }
*/