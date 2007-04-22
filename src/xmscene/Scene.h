/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#ifndef __MOTOGAME_H__
#define __MOTOGAME_H__

#define MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION 5.0

#include "../VCommon.h"
#include "../VApp.h"
#include "../helpers/VMath.h"
#include "Level.h"
#include "../BSP.h"
#include "../DBuffer.h"
#include "../Collision.h"
#include "../ScriptDynamicObjects.h"
#include "../SomersaultCounter.h"
#include "../GameEvents.h"

#include "BasicSceneStructs.h"
#include "BikeParameters.h"
#include "BikeAnchors.h"
#include "Bike.h"

class Level;
class BikeState;
class Ghost;
class PlayerBiker;
class ReplayBiker;
class MotoGameOnBikerHooks;
class LuaLibGame;
class xmDatabase;

namespace vapp {

  class MotoGameEvent;
  class Replay;
  class GameRenderer;
  class CollisionSystem;
  class InputHandler;

  /*===========================================================================
  Serialized bike state
  ===========================================================================*/
  #define SER_BIKE_STATE_DIR_LEFT         0x01
  #define SER_BIKE_STATE_DIR_RIGHT        0x02
   
  /*===========================================================================
  Arrow pointer
  ===========================================================================*/
  struct ArrowPointer {
    ArrowPointer() {
      nArrowPointerMode = 0;
    }
  
    int nArrowPointerMode;
    Vector2f ArrowPointerPos;
    float fArrowPointerAngle;
  };

  /*===========================================================================
  Requested player state
  ===========================================================================*/
  struct GameReqPlayerPos {
    Vector2f Pos;
    bool bDriveRight;  
  };

  /*===========================================================================
  Game message
  ===========================================================================*/
  struct GameMessage {
    bool bNew;                        /* Uninitialized */
    Vector2f Pos,Vel;                 /* Screen position/velocity of message */
    float fRemoveTime;                /* The time when it should be removed */
    std::string Text;                 /* The text */
    int nAlpha;                       /* Alpha amount */
    bool bOnce;                       /* Unique message */
  };  
    
  /*===========================================================================
  Game object
  ===========================================================================*/

  class MotoGameHooks {
  public:
    virtual void OnTakeEntity() = 0;
  };

  class MotoGame {
  public:          
    MotoGame();
    ~MotoGame();
    
    void setHooks(MotoGameHooks *i_motoGameHooks);

    /* update of the structure */
    void loadLevel(xmDatabase *i_db, const std::string& i_id_level);
    void prePlayLevel(InputHandler *i_inputHandler,
		      Replay *recordingReplay,
		      bool i_playEvents);

    void playLevel();
    void updateLevel(float fTimeStep, Replay *i_recordedReplay);
    void endLevel();

    /* entities */
    void touchEntity(int i_player, Entity *pEntity, bool bHead); 
    void deleteEntity(Entity *pEntity);

    /* messages */
    void gameMessage(std::string Text,
         bool bOnce = false,
         float fDuration = MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION);
    void clearGameMessages();
    void updateGameMessages();
    std::vector<GameMessage *> &getGameMessage(void) {return m_GameMessages;}
      
    /* serialization */
    static void getSerializedBikeState(BikeState *i_bikeState, float i_fTime, SerializedBikeState *pState);
    static void unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents, bool bDisplayInformation = false);

    /* events */
    void createGameEvent(MotoGameEvent *p_event);
    void destroyGameEvent(MotoGameEvent *p_event);
    MotoGameEvent* getNextGameEvent();
    int getNumPendingGameEvents();
    void cleanEventsQueue();
    void executeEvents(Replay *p_replay);

    /* player */
    void setPlayerPosition(int i_player, float x,float y,bool bFaceRight);
    const Vector2f &getPlayerPosition(int i_player);
    bool getPlayerFaceDir(int i_player);
    void setDeathAnim(bool b) {m_bDeathAnimEnabled=b;}
    
    /* Data interface */
    Level *getLevelSrc(void) {return m_pLevelSrc;}

      float getTime(void) {return m_fTime;}
      void setTime(float f) {m_fTime=f;}
      ArrowPointer &getArrowPointer(void) {return m_Arrow;}
      CollisionSystem *getCollisionHandler(void) {return &m_Collision;}
        
      void setShowGhostTimeDiff(bool b) { m_showGhostTimeDiff = b; }

      /* action for events */
      void SetEntityPos(String pEntityID, float pX, float pY);
      void SetEntityPos(Entity* pEntity,  float pX, float pY);
      void PlaceInGameArrow(float pX, float pY, float pAngle);
      void PlaceScreenArrow(float pX, float pY, float pAngle);
      void HideArrow();
      void MoveBlock(String pBlockID, float pX, float pY);
      void SetBlockPos(String pBlockID, float pX, float pY);
      void SetBlockCenter(String pBlockID, float pX, float pY);
      void SetBlockRotation(String pBlockID, float pAngle);

      void setRenderer(GameRenderer *p_renderer);
      void CameraZoom(float pZoom);
      void CameraMove(float p_x, float p_y);

      void killPlayer(int i_player);
      void playerEntersZone(int i_player, Zone *pZone);
      void playerLeavesZone(int i_player, Zone *pZone);
      void playerTouchesEntity(int i_player, std::string p_entityID, bool p_bTouchedWithHead);
      void entityDestroyed(const std::string& i_entityId);
      void addDynamicObject(SDynamicObject* p_obj);
      void removeSDynamicOfObject(std::string pObject);
      void addPenalityTime(float fTime);

      void createKillEntityEvent(std::string p_entityID);

      unsigned int getNbRemainingStrawberries();
      void makePlayerWin(int i_player);

      void setGravity(float x,float y);
      const Vector2f &getGravity(void);

      ReplayBiker* addReplayFromFile(std::string i_ghostFile,
				     Theme *i_theme, BikerTheme* i_bikerTheme);
      Ghost* addGhostFromFile(std::string i_ghostFile, std::string i_info,
			      Theme *i_theme, BikerTheme* i_bikerTheme);
      PlayerBiker* addPlayerBiker(Vector2f i_position, DriveDir i_direction,
				  Theme *i_theme, BikerTheme* i_bikerTheme,
				  const TColor& i_filterColor,
				  const TColor& i_filterUglyColor);

      std::vector<Ghost *> &Ghosts();
      std::vector<Biker*> &Players();

      bool doesPlayEvents() const;

      void fastforward(float fSeconds);
      void fastrewind(float fSeconds);
      void pause();
      void faster();
      void slower();
      float getSpeed() const;
      void setSpeed(float f);
      bool isPaused();

      void handleEvent(MotoGameEvent *pEvent);

      void setInfos(std::string i_infos);
      std::string getInfos() const;

      LuaLibGame* getLuaLibGame();

  private:
       
      /* Data */
      std::queue<MotoGameEvent*> m_GameEventQueue;
      
      float m_fTime;
      
      float m_fLastStateSerializationTime; 

      float m_speed_factor; /* nb hundreadths to increment each time ;
			       is a float so that manage slow */
      bool  m_is_paused;

      MotoGameHooks *m_motoGameHooks;

      std::string m_infos;

      bool m_bDeathAnimEnabled;

      Vector2f m_PhysGravity; /* gravity */

      ArrowPointer m_Arrow;               /* Arrow */  
      
      CollisionSystem m_Collision;        /* Collision system */
            
      Level *m_pLevelSrc;              /* Source of level */            

      LuaLibGame *m_luaGame;

      std::vector<Entity *> m_DestroyedEntities; /* destroyed entities */
      
      std::vector<Entity *> m_DelSchedule;/* Entities scheduled for deletion */
      std::vector<GameMessage *> m_GameMessages;
      
      std::vector<Biker*> m_players;

      bool m_showGhostTimeDiff;

    std::vector<Ghost*> m_ghosts;
    std::vector<float> m_myLastStrawberries;

      GameRenderer *m_renderer;

      std::vector<SDynamicObject*> m_SDynamicObjects;
      
      int m_nLastEventSeq;
      
      /* for EveryHundreath function */
      float m_lastCallToEveryHundreath;
            
      bool m_playEvents;

      void cleanGhosts();
      void cleanPlayers();

      /* Helpers */
      void _GenerateLevel(void);          /* Called by playLevel() to 
                                             prepare the level */
      bool _DoCircleTouchZone(const Vector2f &Cp,float Cr,Zone *pZone);
      Entity *_SpawnEntity(std::string ID,EntitySpeciality Type,Vector2f Pos, Entity *pSrc);
      void _KillEntity(Entity *pEnt);
      void _UpdateEntities(void);
      void _UpdateZones(void);
      bool touchEntityBodyExceptHead(const BikeState &pBike, const Entity &p_entity);

      void DisplayDiffFromGhost();

      void cleanScriptDynamicObjects();
      void nextStateScriptDynamicObjects(int i_nbCents);

      void _SerializeGameEventQueue(DBuffer &Buffer,MotoGameEvent *pEvent);      
      void _UpdateDynamicCollisionLines(void);
      
    };

}

class MotoGameOnBikerHooks : public OnBikerHooks {
 public:
  MotoGameOnBikerHooks(vapp::MotoGame* i_motoGame, int i_playerNumber);
  virtual ~MotoGameOnBikerHooks();
  void onSomersaultDone(bool i_counterclock);
  void onWheelTouches(int i_wheel, bool i_touch);
  void onHeadTouches();

 private:
  vapp::MotoGame* m_motoGame;
  int m_playerNumber;
};

#endif