#pragma once
#include "GameTechRenderer.h"
#include "StateGameObject.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/GameStateMachine.h"

namespace NCL {
	namespace CSC8503 {
		enum ObjectMask {
			PLAYER = 1,
			ENEMY = 2,
			BONUS = 4,
			FLOOR = 8,
			CUBE = 16,
			SPHERE = 32,
			CAPSULE = 64,
			STATEOBJ = 128,
			LAVA = 256,
		};

		class MainGame		{
		public:
			MainGame();
			~MainGame();

			virtual void UpdateGame(float dt);
			bool IsEnd();

		protected:
			void InitTileTypes();
			void InitGameState();
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor(string file);
			GameObject* AddTileToWorldFloor(Vector3 pos, float angle, Vector3 axis, Vector2 floorSize, float friction, float elasticity, OGLTexture* tex);
			void InitLavaFloor();
			void InitBuildings();
			GameObject* BuildBuildings(int num,Transform t);

			void BridgeConstraintTest();
	
			bool SelectObject();
			void MoveSelectedObject();
			void CheckIfObjectSee();
			void WorldFloorMovement(float dt);
			void DebugObjectMovement();
			void UpdateTransForms();
			void UpdateErasables();

			void LockedObjectMovement();
			void LockCameraMovment(float dt);

			GameObject* CreateLavaFloor(const Vector3& position, const Vector3& floorSize);
			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& floorSize, float friction, float elasticity, OGLTexture* tex);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool hollow = false, float innerRadius = 0.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			
			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			StateGameObject* testStateObject;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLTexture* lavaTex = nullptr;
			OGLTexture* brickTex = nullptr;
			OGLTexture* brick2Tex = nullptr;
			OGLTexture* metalTex = nullptr;
			OGLTexture* iceTex = nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Game Feature
			GameObject* worldFloor = nullptr;
			GameObject* worldFloorGimbal = nullptr;
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
			float rotX;
			float rotY;
			struct TileType {
				OGLTexture* tex;
				float friction;
				float elasticity;
			} tileTypes[4];
			vector<GameObject*> Buildings;
			
			//System
			bool isEnd;
			bool pause;
			GameStateMachine* gameState;
		};
	}
}

