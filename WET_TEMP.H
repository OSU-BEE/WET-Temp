// WET_Temp.h: interface for the WET_Temp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WET_TEMP_H__C9203D32_B4B1_11D3_95C1_00A076B0010A__INCLUDED_)
#define AFX_WET_TEMP_H__C9203D32_B4B1_11D3_95C1_00A076B0010A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class Map;
class DataTable;

#include <maplayer.h>   // for RowColArray;
#include <reachtree.h>
#include <fdataobj.h>
//#include <datastore.h>
#include <randgen\randunif.hpp>

enum WT_NOTIFY_TYPE { WT_UPDATE, WT_STARTRUN, WT_ENDRUN };

class WET_Temp;
typedef bool (*WT_NOTIFYPROC)( WET_Temp*, WT_NOTIFY_TYPE, float, long, long );

enum 
   {
   WT_OPTION_NONE         = 1,
   WT_OPTION_USE_ALT_LULC = 2,
   WT_OPTION_USE_ALT_FLOW = 4,
   WT_OPTION_USE_AREA_COL = 8       
   };


struct SUBNODE_VEG_INFO
	{
	float vegHeight[11];
   float vegDensity[11];
   float heightAdjustment[11];
   float vegShadeAngle[11];
   int   landUse[11];
   float maxVegShadeAngle;
	float maxVegDensity;
   float averageHeight;

	
   //Restoration Scenario Information
   float vegHeightOriginal[11];
	float vegDensityOriginal[11];
	float vegShadeAngleOriginal[11];
   
   //constructor
	SUBNODE_VEG_INFO(): vegHeight(), vegDensity(),heightAdjustment(), vegShadeAngle(),
      vegHeightOriginal(), vegDensityOriginal(), vegShadeAngleOriginal(), maxVegShadeAngle( 0.0f ), 
      maxVegDensity( 0.0f ), averageHeight( 0.0f ), landUse(){}
	};




/*class VegInfoArray : public CArray<SUBNODE_VEG_INFO*, SUBNODE_VEG_INFO*>
	{
     public:
      ~VegInfoArray() { for ( int i=0; i < GetSize(); i++ ) delete GetAt( i ); }
	};*/


struct SUBNODE_INFO_TEMP
   {
   float streamTemperature;        // temperature at this subnode
   float streamFlow;
   
   //Energy balance information stored at this subnode
   float heatFlux;
   float solarFlux;
   float longwaveFlux;
   float evaporationFlux;
   float convectionFlux;
   float bedFlux;
   float bedFluxCounter;

   float topoAngle[ 8 ];
   float distToVeg[ 8 ];

   float streamAspect;
   float skyOpen;
   float rightAzimuthAngle;
   float leftAzimuthAngle;
   float vegAngleLeft;
   float vegAngleRight;
   float vegDensityLeft;
   float vegDensityRight;
   float topoAngleLeft;
   float topoAngleRight;
   float subnodeElev;
   float subnodeSlope;
   float subnodeLength;
   float resTime;
   float wdRatio;
   float subnodeVolume;
   
   //Original value (before restoration scenarios are applied)
   float streamFlowOriginal;
   float wdRatioOriginal;


   //vegetation info at this subnode
   SUBNODE_VEG_INFO vegInfo[8];

   //subnode stream parameters.  Take from WET_Hydro.
   float streamWidth;				// width of the stream subreach (m)   ASSUMES RECTANGULAR
   float streamDepth;				// depth of the stream subreach (m)
   float lateralInflow;				// flow rate into this subreach

   COORD2d location;	// x,y,z location of this subnode

   //Information stored for sensitivity analysis
   float Tmax;
   float Tmin;
   float timeOfMax;
   float timeOfMin;

   //constructor thingy
   SUBNODE_INFO_TEMP( ): streamTemperature( 14.0 ), streamAspect( 0.0f ), skyOpen( 0.0f ), 
	   streamWidth( 0.0f ), streamDepth( 0.0f ), lateralInflow( 0.0f), vegAngleLeft( 0.0f ),
	   vegAngleRight( 0.0f ), vegDensityLeft( 0.0f ), vegDensityRight( 0.0f ), topoAngleLeft( 0.0f ),
	   topoAngleRight( 0.0f ), topoAngle(), subnodeElev( 0.0f), streamFlow(0.0f),
      bedFlux ( 0.0f ), bedFluxCounter( 0.0f ), Tmax ( 14.0 ), Tmin( 14.0 ),
      timeOfMax( 0.0f ), timeOfMin( 0.0f ){}

   };


class SubnodeInfoTempArray : public CArray< SUBNODE_INFO_TEMP*, SUBNODE_INFO_TEMP* >
   {
   public:
      ~SubnodeInfoTempArray() { for ( int i=0; i < GetSize(); i++ ) delete GetAt( i ); }
   };

   
struct REACH_INFO_TEMP  // extends the REACH_INFO structure
   {
   int reachID;     // grid code (unique id) for this reach

	//Reach scale channel parameters
   float nsdz;
   float bfW;
   float sinuosity;
   float gradient;
   float wdRatio;

   float reachFlow;  // stil being used???

   bool incSummerFlow;
   float sumDiversions;
   float wetlandArea;
   
   CString rosgen;
   CString dataPath;
   
   SubnodeInfoTempArray subnodeInfoTempArray;   // Note: Deallocates automatically

   REACH_INFO_TEMP( void ): nsdz( 0.0f ), bfW( 0.0f ), sinuosity( 0.0f),
                  gradient( 0.0f ), wdRatio( 0.0f ), rosgen(), dataPath(),
                  incSummerFlow(FALSE),  reachFlow( 0.0f ) { }

   };


class ReachInfoTempArray : public CArray< REACH_INFO_TEMP*, REACH_INFO_TEMP* >
   {
   public:
      ~ReachInfoTempArray( void ) { for ( int i=0; i < GetSize(); i++) delete GetAt( i ); }

   };



struct CELL_INFO
   {
   CELL_INFO( void ) { }
   };


class CellInfoArray : public CArray< CELL_INFO*, CELL_INFO* >
   {
   public:
      ~CellInfoArray( void ) { Clear(); }

      void Clear( void ) {for ( int i=0; i < GetSize(); i++) delete GetAt( i ); RemoveAll(); }
   };


/*
struct SVINFO
   {
   int east;
   int north;
   int south;
   int west;

   int row;
   int col;

   float xSlope;     // decimal percent slope across this cell in the x direction (across cols)
   float ySlope;     // decimal percent slope across this cell in the y direction (across rows)

   SVINFO() :  row( -1 ), col( -1 ), east( -1 ), north( -1 ), south( -1 ), west( -1 ), xSlope( 0.0f ), ySlope( 0.0f ) { }

   SVINFO( int _row, int _col ) : row( _row ), col( _col ), east( -1 ), north( -1 ), south( -1 ), west( -1 ),
            xSlope( 0.0f ), ySlope( 0.0f )  { }
   };


class SVInfoArray : public CArray< SVINFO, SVINFO& > { };
*/

struct OBSINFO
   {
   FDataObj *pData;
   int reachIndex;
   int subnodeIndex;
   FDataObj *pMeasured;

   // constructor
   OBSINFO( ) : reachIndex( -1 ), subnodeIndex( -1 ), pData( NULL), pMeasured( NULL ) { }
   ~OBSINFO() { if ( pData != NULL ) delete pData; if ( pMeasured != NULL ) delete pMeasured; }
   };

class ObsInfoArray : public CArray< OBSINFO, OBSINFO& > { };



class WET_Temp
{
public:
	WET_Temp();
	virtual ~WET_Temp();

	void  SetMapPtr( Map *pMap ) { m_pMap = pMap; }
   bool  SaveParams( LPCSTR filename );
   bool  LoadParams( LPCSTR filename );
	int   LoadClimateData( LPCSTR filename );
   
	void  SetTimeStep( float timeStep ) { m_timeStep = timeStep; }
	float GetTimeStep( void ) { return m_timeStep; }

   void  SetStartTime( float startTime ) { m_startTime = startTime; }
	float GetStartTime( void ) { return m_startTime; }
   
   void  SetStopTime( float stopTime ) { m_stopTime = stopTime; }
	float GetStopTime( void ) { return m_stopTime; }
   
   void  SetIterations( int iterations ) { m_iterations = iterations; }
   int   GetIterations( void ) { return m_iterations; }


   int   GetReachCount();
   
   
   // Solar Calculation functions
	void  SolarCalculations( float time );
	float JulianDay( float time );
	float SetDeclination ( float julianDay );
	float SolarTime ( float julianDay );
	float HourAngle ( float lat );
	float SetSolarAltitude( float hourAngle );
	float GetAzimuth( float hourAngle );
	float SetETRad( float julianDay );
	float SetSurfaceRad( SUBNODE_INFO_TEMP *pSubnodeInfo, float julianDay );
	void  SetRadSplit( float julianDay );

	//Shade Calculation functions
	float StreamAspect (int i, int j);
	float RightAzimuthAngle(SUBNODE_INFO_TEMP *pSubnodeInfo);
	float LeftAzimuthAngle(SUBNODE_INFO_TEMP *pSubnodeInfo);
	float TopoAngleRight(SUBNODE_INFO_TEMP *pSubnodeInfo);
	float TopoAngleLeft(SUBNODE_INFO_TEMP *pSubnodeInfo);
	void  VegInfoRight(SUBNODE_INFO_TEMP *pSubnodeInfo);
	void  VegInfoLeft (SUBNODE_INFO_TEMP *pSubnodeInfo);
	float SkyOpen (SUBNODE_INFO_TEMP *pSubnodeInfo);
	void  GetTopographicShading(MapLayer *pGrid);
	void  CalculateTopographicShading(SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid );
   void  CalculateSubnodeSlope(int i, int j);
   void  CalculateDistanceToVeg(SUBNODE_INFO_TEMP *pSubnodeInfo, int i);
	float GetElevationAdjustment(SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid, float x, float y);
	float TopoShadeAngle(SUBNODE_INFO_TEMP *pSubnodeInfo);
	float StreamAlbedo ();
	float StreamWidth(int streamOrder);
	float SunwardBank (SUBNODE_INFO_TEMP *pSubnodeInfo );
	float VegShade( ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, int j);
	float Overshade (SUBNODE_INFO_TEMP *pSubnodeInfo);
   
	
	//Energy Balance functions
	float GetSolarFlux (ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, float time, int j, int dtStartIndex, float surfaceArea);
   float GetHeatFlux(ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, float time, float streamWidth, float dx, int j, float temp, int dtStartIndex);
	float GetAirTemp( float time );
	float GetRH( float time );
   float GetWindSpeed(float time);
	float GetLongwaveFlux(ReachNode *pNode, SUBNODE_INFO_TEMP *pSubnodeInfo, float time, int j, float temp);
   float GetEvaporationFlux(SUBNODE_INFO_TEMP *pSubnodeInfo, float time, float temp);
	float GetConvectionFlux(float evaporationFlux);

   float StreamTemperature(SUBNODE_INFO_TEMP *pSubnodeInfo, REACH_INFO *pData /*qArray*/);

   void  InitSubnode( SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid, int i, int j);
   bool  InitReachSubnode( REACH_INFO_TEMP *pReachInfo, ReachNode *pNode, int i );
   REACH_INFO_TEMP *GetReachInfoTempFromReachID(int reachID);


   //Channel Parameters
   void  InitChannelParameters();
   void  CalcChannelParameters( SUBNODE_INFO_TEMP *pSubnodeInfo, int i, int j );
   void CalcReachWDRatio( int i );
   CString AssignRosgenType(float sinuosity, float gradient);

   
   SUBNODE_INFO_TEMP *GetReachSubnodeInfo( int reachIndex, int subnodeIndex )
       {
       if ( reachIndex > GetReachCount()-1 ) return NULL;
       if ( subnodeIndex > m_reachInfoTempArray[ reachIndex ]->subnodeInfoTempArray.GetSize() -1 ) return NULL;

       return m_reachInfoTempArray[ reachIndex ]->subnodeInfoTempArray[ subnodeIndex ];
       }
   
    void  Run( void );

    // set layers from a file
    MapLayer	*SetStreamLayer( LPCSTR shapeFileName );
    MapLayer	*SetCellLayer( LPCSTR shapeFileName );
    MapLayer	*SetDEMLayer( LPCSTR gridFile );
	 MapLayer	*SetFlowLayer( LPCSTR shapeFileName );
    MapLayer   *SetObservedLayer( LPCSTR shapeFileName );
    MapLayer   *SetWithdrawalsLayer( LPCSTR shapeFileName );
    MapLayer   *SetChannelLayer( LPCSTR shapeFileName );
    //DataStore  *m_pDataStore;

    // set layer directly
    MapLayer	*SetStreamLayer( MapLayer * );
    MapLayer	*SetCellLayer( MapLayer * );
    MapLayer	*SetDEMLayer( MapLayer * );
	 MapLayer	*SetFlowLayer( MapLayer * );
    MapLayer   *SetObservedLayer( MapLayer * );
    MapLayer   *SetWithdrawalsLayer( MapLayer * );
    MapLayer   *SetChannelLayer( MapLayer * );    
    	
   void  InitSubnodeStructures();

   void  PopulateSubnodes( LPCSTR shapeFileName );
   void  ApplyReachByReachRestoration( int reachIndex );
	void  RemoveReachbyReachRestoration();
	void  SetSubnodeLocation(SUBNODE_INFO_TEMP *pSubnodeInfo, int i, int j);
	void  InitVegetationInfo(SUBNODE_INFO_TEMP *pSubnodeInfo, MapLayer *pGrid);
	void  DecodeLulc(int lulc_c, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo);
	void  DecodeHeight(int heightCode, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo);
   void  DecodeDensity(int densityCode, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo);
   void  DecodeLanduseClass(CString landuseClass, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo, int heightCode);
   void  DecodeLanduse(CString landuseClass, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo);
   void  DecodeSystem(CString system, int d, int n, SUBNODE_INFO_TEMP *pSubnodeInfo);
   void  PopulateVegInfo(int d, float x, float y, MapLayer *pGrid, SUBNODE_INFO_TEMP *pSubnodeInfo);
	bool  SavePreprocessingData( LPCSTR );
	bool  ReadPreprocessingData( FILE* );
	bool  SaveFormattedData(float time, ReachNode *pNode, int j, float flow, float lateralInflow);
	void  GetHydro( ReachNode *pNode, float time, int i, int j, float &flow, float &lateralInflow );
   float GetMidpoint(float x1, float x2);
   void  GetHydroData(ReachNode *pNode, float time);
   float GetDepthFromQ( ReachNode *pNode, float Q, int j ); 
   void  SetReachGeometry( ReachNode *pNode, float Q);
   void  RestorationDischargeAdjustments();
   void  StoreOriginalDischargeEstimates();
   
   float  ComputeCumulativeFlowIncrease(ReachNode *pNode, float *flowIncArray, float *cumFlowIncArray);
   
protected:
   void  InitObsValueInfo();
   void  CollectCalibrationData(float time);
   void  WriteObsValueInfo();
   void  WriteSensData();
   void  WriteLongitudinalData(int reachIndex, int i );
   float GetRESTOREBufferWidth( int cell );
   
protected:
	Map *m_pMap;

	// basin-scale, time variant data
	float m_solarAltitude;
   int   m_dayOfYear;
	float m_azimuth;
	float m_etRad;
	float m_surfaceRad;
	float m_diffuseRad;
	float m_beamRad;
	float m_declination;
   float m_airTemp;
   float m_relativeHumidity;
   float m_dailyHighAirT;
   float m_dailyLowAirT;
	
	//subnode scale time variant data
	float m_heatFlux;
   float m_bowenRatio;
   float m_latInTotal;
   float m_volumeTotal;
   float m_reachLat;
   float m_pathLength;
	float m_shadeDensity;
	
   float m_Ecurrent;
   float m_Eflux;
  
   //time invariant
   int m_mouthIndex;
   
   //physical constants
	float m_waterHeatCapacity;
	float m_waterDensity;

   // various map layers
public:
   MapLayer *m_pStreamLayer;
   MapLayer *m_pCellLayer;
   MapLayer *m_pDEMLayer;
   MapLayer *m_pSubnodeLayer;
   MapLayer *m_pObservedLayer;
   MapLayer *m_pFlowLayer;
   MapLayer *m_pWithdrawalsLayer;
   MapLayer *m_pChannelLayer;
   ReachTree m_reachTree;
   RandUniform m_randUnif;

protected:
   // tree to contain reach information
   ReachInfoTempArray m_reachInfoTempArray;
   CellInfoArray      m_cellInfoArray;
   ObsInfoArray       m_obsInfoArray;

   float  *m_k1;
   float  *m_k2;
   float  *m_k3;
   float  *m_k4;
   float  *m_y;
   float  *m_sv;
   float  *m_bed;
   int     m_svCount;

   // simulation control
   float m_timeStep;
   float m_stopTime;
   float m_startTime;
   int   m_iterations;
   float m_outputTimeStep;
   float m_cellSize;
   float m_maxReachDeltaX;
   float m_preprocessingTime;
   float m_runTime;

   float m_derivative;
   float m_svTemp;
   float m_stateVar;
   
public:   
   int   m_options;
   int   m_useAreaCol;  // -1=ignore, otherwise only consider restoration alternatives if this col > 0

   //Input Parameters
   float m_gwTemp;
   float m_evapA;
   float m_evapB;
   float m_tempInit;
   float m_manningN;
   float m_cloudFrac;
   float m_wdRatio;
   float m_volX;   //volume multiplier
   //float m_percentAbsorb;
   float m_extinct;
   float m_bedFlux;
   float *m_bedFluxCounter;
   float m_percentIncreaseSummerFlow;

   // restoration parameters
   float m_agHeight;
   int m_agWidth;
   int m_agDen;
   float m_emHeight;
   int m_emWidth;
   int m_emDen;
   float m_huHeight;
   int m_huWidth;
   int m_huDen;
   float m_ssHeight;
   int m_ssWidth;
   int m_ssDen;
   float m_fo1Height;
   float m_fo2Height;
   float m_fo3Height;
   float m_fo4Height;
   float m_fo5Height;
   int m_maxBufferWidth;
   
   
   int m_wdDecrease;
   int m_flowIncrease;
   float m_wetlandLowFlowIncrease;

   void SetGwTemp   ( float gwTemp    ){ m_gwTemp    = gwTemp; }
   void SetEvapA    ( float evapA     ){ m_evapA     = evapA; }
   void SetEvapB    ( float evapB     ){ m_evapB     = evapB; }
   void SetTempInit ( float tempInit  ){ m_tempInit  = tempInit; }
   void SetManningN ( float manningN  ){ m_manningN  = manningN; }
   void SetCloudFrac( float cloudFrac ){ m_cloudFrac = cloudFrac; }
   void SetWdRatio  ( float wdRatio   ){ m_wdRatio   = wdRatio; }
   void SetVolX     ( float volX      ){ m_volX  = volX; }
   void SetExtinct   ( float extinct    ){ m_extinct = extinct; }
   //Set parameters for reach restoration application (ODEQ coverages)
   void SetAgHeight    (  float agHt  ){ m_agHeight  = agHt; }
   void SetAgDensity   (  int agDen ){ m_agDen = agDen; }
   void SetAgWidth     (  int agWid ){ m_agWidth   = agWid; }
   void SetEmHeight    (  float emHt  ){ m_emHeight  = emHt; }
   void SetEmDensity   (  int emDen ){ m_emDen = emDen; }
   void SetEmWidth     (  int emWid ){ m_emWidth   = emWid; }
   void SetHuHeight    (  float huHt  ){ m_huHeight  = huHt; }
   void SetHuDensity   (  int huDen ){ m_huDen = huDen; }
   void SetHuWidth     (  int huWid ){ m_huWidth   = huWid; }
   void SetSsHeight    (  float ssHt  ){ m_ssHeight  = ssHt; }
   void SetSsDensity   (  int ssDen ){ m_ssDen = ssDen; }
   void SetSsWidth     (  int ssWid ){ m_ssWidth   = ssWid; }
   void SetFo1Height    (  float fo1Ht  ){ m_fo1Height  = fo1Ht; }
   void SetFo2Height    (  float fo2Ht  ){ m_fo2Height  = fo2Ht; }
   void SetFo3Height    (  float fo3Ht  ){ m_fo3Height  = fo3Ht; }
   void SetFo4Height    (  float fo4Ht  ){ m_fo4Height  = fo4Ht; }
   void SetFo5Height    (  float fo5Ht  ){ m_fo5Height  = fo5Ht; }
      
   void SetIncreaseFlow    (  int flowinc ){ m_flowIncrease  = flowinc; }
   

   // miscellaneous
   float m_noDataValue;  // for all constructed layers   

public:
   // data storage
   FDataObj *m_pClimateData;
   FDataObj *m_pHabitatData;
   FDataObj *m_pSensData;
   
   // data storage resulting from a simulation
   FDataObj *m_pTemperatureData;
   FDataObj *m_pHeatFlux;
   FDataObj *m_pSolarFlux;
   FDataObj *m_pEvapFlux;
   FDataObj *m_pLongwaveFlux;
   FDataObj *m_pConvectionFlux;
   FDataObj *m_pAdvectionFlux;

   CArray<FDataObj*,FDataObj*> m_pLongitudinalDataArray;
   
   
   bool  m_useVariableStep;

public:
	CWordArray m_reachDataArray;        // reach offsets to collect data on
   CWordArray m_restoreReachArray;     //reaches to which hypothetical restoration has been applied

public:
   int m_colArea;
   int m_colSlope;
   int m_colWatershedID;   //hydro_ID in the cell layer
   int m_colStreamID;
   int m_colWDratio;
   int m_colTemp;
   int m_colTempDelta;
   int m_colLulc;
   int m_colAltern;
   int m_colAltLulc;
   int m_colAltArea;
   int m_colStreamLength;
   int m_colRate;
   int m_colHeightCode;
   int m_colDensityCode;
   int m_colLanduseClass;
   int m_colSystem;
   int m_colLength;
   int m_colWidth;
   int m_colDepth;
   int m_colRosgen;
   int m_colGradient;
   int m_colSinuosity;
   int m_colDataPath;
   int m_coverType;     // 1 = Restore ;  2 = ODEQ

protected:
   WT_NOTIFYPROC m_notifyFn;
   long m_extra;
   bool Notify( WT_NOTIFY_TYPE t, float f, long l ) { if ( m_notifyFn != NULL ) return m_notifyFn( this, t, f, l, m_extra ); else return true; }

public:
   WT_NOTIFYPROC InstallNotifyHandler( WT_NOTIFYPROC p, long extra=0L ) { WT_NOTIFYPROC temp = m_notifyFn; m_notifyFn = p; m_extra = extra; return temp; }
   
   
protected:
   int   GetCellCount();
   int   GetCellReachID( int cell );
   float GetCellArea( int cell );
   float GetCellSlope( int cell );
   float GetCellDepth( int cell );
   int   GetCellLulc( int cell, bool alt );
   int   GetCellHeight( int cell );
   int   GetCellDensity( int cell );
   CString GetCellLanduseClass( int cell );
   CString GetCellSystem( int cell );
   bool  CreateSubnodePtLayer( LPCSTR shapeFileName);


   // model setup, etc.
   void  InitRun( void ) ;
   void  CollectData( float time );
   void  CollectSolarData( float time );
   void  CollectLongitudinalData( float simDuration );
   void  ResetDailyInfo();
   void  EndRun( void );
   int   RunTemperatureModel( float *derive, float time );
   void  InitBranch( ReachNode *pNode, int &dtStartIndex  );
   void  EnergyTemp( ReachNode *pNode, int &dtStartIndex  );
   bool  SolveReach( ReachNode *pNode, float time, int dtStartIndex, float *derive );
   int   SolveBranch( ReachNode *pNode, float time, int &dtStartIndex, float *derive  );

   int   IntRK4( float time );

   float  GetOutputTemp( ReachNode *pNode );
   float GetOutputFlow( ReachNode *pNode );
};





#endif // !defined(AFX_WET_Hydro_H__C9203D32_B4B1_11D3_95C1_00A076B0010A__INCLUDED_)

