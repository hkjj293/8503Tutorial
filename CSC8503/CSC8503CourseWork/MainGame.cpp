#include "MainGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "../../Common/Maths.h"
#include "../../Common/Assets.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/GameStateMachine.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

#include <fstream>
#include <exception>

using namespace NCL;
using namespace CSC8503;

MainGame::MainGame()	{
	isEnd		= false;
	pause		= false;

	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;
	selectionObject = nullptr;
	lockedObject = nullptr;

	rotX = 0;
	rotY = 0;

	Debug::SetRenderer(renderer);
	InitGameState();
	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void  MainGame::InitTileTypes() {
	tileTypes[0].tex = lavaTex;
	tileTypes[0].elasticity = 0.0f;
	tileTypes[0].friction = 1.0f;

	tileTypes[1].tex = brickTex;
	tileTypes[1].elasticity = 0.25f;
	tileTypes[1].friction = 0.75f;

	tileTypes[2].tex = metalTex;
	tileTypes[2].elasticity = 0.75f;
	tileTypes[2].friction = 0.25f;

	tileTypes[3].tex = iceTex;
	tileTypes[3].elasticity = 1.0f;
	tileTypes[3].friction = 0.0f;
}

void MainGame::InitGameState() {
	gameState = new GameStateMachine();
	State* mainMenu = new State([&](float dt)-> void
		{
			UpdateKeys();
			//SelectObject();

			gameState->SetTimeLapse(gameState->GetTimeLapse() + dt);
;			int t = ((int)gameState->GetTimeLapse()) % 2;
			renderer->DrawString("CSC 8503 ScrewBall!", Vector2(3 + t, 10 - t),Vector4(t,1-t,t,1),50.0f);
			renderer->DrawString("CSC 8503 ScrewBall!", Vector2(4 - t, 11 + t), Vector4(1-t, 1-t, 1-t, 1), 50.0f);

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN))  gameState->SetMainChoice((gameState->GetMainChoice() + 1) % 3);
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP))	gameState->SetMainChoice((gameState->GetMainChoice() - 1) % 3);

			Vector4 white = Vector4(1, 1, 1, 1);
			Vector4 red = Vector4(1, 0 , 0, 1);
			renderer->DrawString("Start Part A", Vector2(32, 50), gameState->GetMainChoice() == 0 ? red : white, 30.0f);
			renderer->DrawString("Start Part B", Vector2(32, 60), gameState->GetMainChoice() == 1 ? red : white, 30.0f);
			renderer->DrawString("Exit", Vector2(42, 70), gameState->GetMainChoice() == 2 ? red : white, 30.0f);

			UpdateTransForms();
		});

	State* pauseMenu = new State([&](float dt)-> void
		{
			UpdateKeys();
			SelectObject();

			renderer->DrawString("Paused.", Vector2(1, 5));
			renderer->DrawString("Menu", Vector2(5, 14), Vector4(0.5,0.5,0.5,1), 50.0f);
			renderer->DrawString("Menu", Vector2(6, 15), Vector4(0.1, 0.1, 0.1, 1), 50.0f);

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN))  gameState->SetPauseChoice((gameState->GetPauseChoice() + 1) % 3);
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP))	gameState->SetPauseChoice((gameState->GetPauseChoice() - 1) % 3);

			Vector4 white = Vector4(1, 1, 1, 1);
			Vector4 red = Vector4(1, 0, 0, 1);
			renderer->DrawString("Continue", Vector2(4, 50), gameState->GetPauseChoice() == 0 ? red : white, 30.0f);
			renderer->DrawString("Restart", Vector2(4, 60), gameState->GetPauseChoice() == 1 ? red : white, 30.0f);
			renderer->DrawString("Exit", Vector2(4, 70), gameState->GetPauseChoice() == 2 ? red : white, 30.0f);

			UpdateTransForms();
		});

	State* level1 = new State([&](float dt)-> void
		{
			gameState->SetTimeLapse(gameState->GetTimeLapse() + dt);
			int time = gameState->GetTimeLapse() * 100;
			string timeText = "Time Passed: " + std::to_string(time/100) + "." + std::to_string(time % 100) + "s";
			renderer->DrawString(timeText, Vector2(1, 5));
			UpdateKeys();
			SelectObject();

			gameState->SetLevel(1);
			renderer->DrawString("Press Esc to Pause.", Vector2(1, 70));

			WorldFloorMovement(dt);
			physics->Update(dt);
			LockCameraMovment(dt);

			world->UpdateWorld(dt);
			UpdateTransForms();
		});

	State* level2 = new State([&](float dt)-> void
		{
			UpdateKeys();
			SelectObject();

			gameState->SetLevel(2);
			renderer->DrawString("Press Esc to Pause.", Vector2(1, 5));

			WorldFloorMovement(dt);
			physics->Update(dt);

			LockCameraMovment(dt);

			world->UpdateWorld(dt);
			UpdateTransForms();
		});

	State* lose = new State([&](float dt)-> void
		{
			UpdateKeys();
			SelectObject();
			//Print Message
			Vector4 white = Vector4(1, 1, 1, 1);
			Vector4 red = Vector4(1, 0, 0, 1);
			renderer->DrawString("You Lose!!", Vector2(32, 50),white, 30.0f);
			renderer->DrawString("Back to Main Menu", Vector2(42, 70), red , 30.0f);
		});

	State* win = new State([&](float dt)-> void
		{
			UpdateKeys();
			SelectObject();
			//Print Message
			Vector4 white = Vector4(1, 1, 1, 1);
			Vector4 red = Vector4(1, 0, 0, 1);
			int time = gameState->GetTimeLapse() * 100;
			string timeText = "You Win!! Your time is: " + std::to_string(time / 100) + "." + std::to_string(time % 100) + "s";
			renderer->DrawString(timeText, Vector2(32, 50), white, 30.0f);
			renderer->DrawString("Back to Main Menu", Vector2(42, 70), red, 30.0f);
		});

	State* exit = new State([&](float dt)-> void
		{
			// Say goodbye?
			isEnd = true;
		});

	gameState->AddState(mainMenu);
	gameState->AddState(pauseMenu);
	gameState->AddState(level1);
	gameState->AddState(level2);
	gameState->AddState(lose);
	gameState->AddState(win);
	gameState->AddState(exit);

	gameState->AddTransition(new StateTransition(mainMenu, level1,
		[&]()-> bool
		{
			// Choose the option in menu
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
			bool b = gameState->GetMainChoice() == 0;
			if (a && b) {
				InitWorld();
				selectionObject = nullptr;
				if (!lockedObject) {
					lockedObject = world->AddGameObject(AddSphereToWorld(Vector3(0, 20, 0), 1, 10.0f, true, 0.9));
					lockedObject->SetName("player");
				}
			}
			gameState->SetTimeLapse(0);
			return a && b;
		}
	));

	gameState->AddTransition(new StateTransition(mainMenu, level2,
		[&]()-> bool
		{
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
			bool b = gameState->GetMainChoice() == 1;
			return a && b;
		}
	));


	gameState->AddTransition(new StateTransition(level1, win,
		[&]()-> bool
		{
			if (!lockedObject) {
				return false;
			}
			return lockedObject->GetName() == "Win";
		}
	));
	gameState->AddTransition(new StateTransition(level2, win,
		[&]()-> bool
		{
			return false;
		}
	));
	gameState->AddTransition(new StateTransition(level2, lose,
		[&]()-> bool
		{
			return false;
		}
	));
	gameState->AddTransition(new StateTransition(level1, lose,
		[&]()-> bool
		{
			return lockedObject == nullptr;
		}
	));
	gameState->AddTransition(new StateTransition(level2, pauseMenu,
		[&]()-> bool
		{
			// a pause
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE);
			if (a) {
				gameState->SetPauseChoice(0);
			}
			return a;
		}
	));
	gameState->AddTransition(new StateTransition(level1, pauseMenu,
		[&]()-> bool
		{
			// a pause
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE);
			if (a) {
				gameState->SetPauseChoice(0);
			}
			return a;
		}
	));
	gameState->AddTransition(new StateTransition(pauseMenu, level1,
		[&]()-> bool
		{
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE);
			bool b = gameState->GetLevel() == 1;
			bool c = Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
			bool d = gameState->GetPauseChoice() == 0;
			bool e = gameState->GetPauseChoice() == 1;
			if (c && e && !a) {
				InitWorld();
				selectionObject = nullptr;
				lockedObject = nullptr;
			}
			return b && (a || (c && (d || e)));
		}
	));

	gameState->AddTransition(new StateTransition(pauseMenu, level2,
		[&]()-> bool
		{
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE);
			bool b = gameState->GetLevel() == 2;
			bool c = Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
			bool d = gameState->GetPauseChoice() == 0;
			bool e = gameState->GetPauseChoice() == 1;
			if (c && e && !a) {
				InitWorld();
				selectionObject = nullptr;
				lockedObject = nullptr;
			}
			return b && (a || (c && (d || e)));
		}
	));
	gameState->AddTransition(new StateTransition(pauseMenu, mainMenu,
		[&]()-> bool
		{
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
			bool b = gameState->GetPauseChoice() == 2;
			if (a && b) {
				gameState->SetPauseChoice(0);
			}
			return (a && b);
		}
	));
	gameState->AddTransition(new StateTransition(win, mainMenu,
		[&]()-> bool
		{
			//Pressed a button?
			return Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
		}
	));
	gameState->AddTransition(new StateTransition(lose, mainMenu,
		[&]()-> bool
		{
			//Pressed a button?
			return Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
		}
	));
	gameState->AddTransition(new StateTransition(mainMenu, exit,
		[&]()-> bool
		{
			bool a = Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE); 
			bool b = Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN);
			bool c = gameState->GetMainChoice() == 2;
			return a || (b && c);
		}
	));
	
}

void MainGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	lavaTex = (OGLTexture*)TextureLoader::LoadAPITexture("lava2.png");   // Source from https://www.moddb.com/mods/hoover1979-ultrahd-doom-texture-pack/images/lava-texture
	brickTex = (OGLTexture*)TextureLoader::LoadAPITexture("brick.png");//https://www.cleanpng.com/png-wall-brick-icon-drawn-cartoon-of-a-wall-93864/
	brick2Tex = (OGLTexture*)TextureLoader::LoadAPITexture("brick2.png");//https://www.cleanpng.com/png-stone-wall-brick-material-texture-vintage-black-br-135307/download-png.html
	metalTex = (OGLTexture*)TextureLoader::LoadAPITexture("metal.png"); // https://www.cleanpng.com/png-grunge-heavy-metal-texture-photography-wallpaper-p-615575/download-png.html
	iceTex = (OGLTexture*)TextureLoader::LoadAPITexture("ice.png"); //https://pngtree.com/element/down?id=NTkyMzU3Mw==&type=1&time=1639712093&token=YmRiYzcyMmIxZWRmMzJhNDhkNGFiNjYxZTlmNWFjOGY=
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitTileTypes();

	InitCamera();
	InitWorld();
}

MainGame::~MainGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete worldFloor;
}

void MainGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	UpdateErasables();

	gameState->Update(dt);
	
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

bool MainGame::IsEnd() {
	return isEnd;
}

void MainGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Y)) {
		Vector3 offset = worldFloor->GetTransform().GetPosition();
		world->AddGameObject(AddSphereToWorld(Vector3(4, 20, 0) + offset, 1, 10.0f));
		world->AddGameObject(AddCapsuleToWorld(Vector3(8, 20, 0) + offset, 1, 0.5f, 10.0f));
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void MainGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 10.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = lockedObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void MainGame::LockCameraMovment(float dt) {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 v = lockedObject->GetPhysicsObject()->GetLinearVelocity();
		Vector3 camTargetPos = objPos - Vector3(v.x, 0, v.z) + Vector3(0, 40, 0);
		float r = 0.2;
		Vector3 delta = (world->GetMainCamera()->GetPosition() - objPos);
		float sqrLength = (delta.x * delta.x + delta.z * delta.z);
		if (sqrLength < 400.0f) {
			r = 0;
		}
		else if (sqrLength > 1600.0f) {
			r = Vector3(v.x, 0, v.z).Length() * (sqrLength - 1600) * 0.000625f + r;
		}
		Vector3 camPos = (camTargetPos - world->GetMainCamera()->GetPosition()) * dt * r + world->GetMainCamera()->GetPosition();
		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}
}

void MainGame::WorldFloorMovement(float dt) {
	if (!lockedObject) {
		worldFloor->SetLocalOffset(Vector3(0, 0, 0));
		worldFloor->changeOrigin(Vector3(0, 0, 0));
		worldFloor->GetTransform().SetPosition(Vector3(0, 0, 0));
		worldFloor->GetTransform().SetOrientation(Quaternion());
		return;
	}
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0));

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();
	Vector2 movement = Window::GetMouse()->GetRelativePosition();
	//worldFloor->GetTransform().SetPosition(lockedObject.GetPosition);
	rotX += Window::GetMouse()->GetRelativePosition().x;
	rotY += Window::GetMouse()->GetRelativePosition().y;
	float maxN = 15.0f;
	rotX = min(maxN, max(-maxN, rotX));
	rotY = min(maxN, max(-maxN, rotY));
	Matrix4 rotation = Matrix4(Quaternion(Matrix4::Rotation(rotX, fwdAxis)) * Quaternion(Matrix4::Rotation(rotY, rightAxis)));
	if (lockedObject) {
		Vector3 pos = Vector3(lockedObject->GetTransform().GetPosition().x, 0, lockedObject->GetTransform().GetPosition().z);
		//worldFloorGimbal->SetLocalOffset(pos);
		//worldFloorGimbal->changeOrigin(pos);
		worldFloorGimbal->GetTransform().SetPosition(pos);
		//worldFloor->SetLocalOffset(-pos);
		//worldFloor->changeOrigin(-pos);
		worldFloor->GetLocalTransform().SetPosition(-pos);
	}
	else {
		worldFloorGimbal->GetTransform().SetPosition(world->GetMainCamera()->GetPosition());
		worldFloor->GetLocalTransform().SetPosition(-world->GetMainCamera()->GetPosition());
	}
	worldFloorGimbal->GetTransform().SetOrientation(rotation);
	//worldFloor->GetTransform().SetPosition(Vector3(Window::GetMouse()->GetRelativePosition().x/100.0f,0,0));
	//worldFloor->UpdateGlobalTransform();
}

void MainGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void MainGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(2000.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void MainGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(5, 5, 3.5f, 3.5f);
	//InitGameExamples();
	InitDefaultFloor("FloorPlan.txt");
	InitLavaFloor();
	InitCamera();
	//BridgeConstraintTest();
	//testStateObject = AddStateObjectToWorld(Vector3(10, 1, 0));
}

void MainGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(2, 1, 8);

	float invCubeMass = 50;
	int numLinks = 100;
	float maxDistance = 4;
	float cubeDistance = 4;

	Vector3 startPos = Vector3(100, 100, 100);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* MainGame::AddFloorToWorld(const Vector3& position, const Vector3& floorSize, float friction, float elasticity, OGLTexture* tex) {
	GameObject* floor = new GameObject("floor");
	floor->SetLayer(ObjectMask::FLOOR);
	floor->SetWorldID(rand());
	OBBVolume* volume	= new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetLocalTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, tex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetMask(0xFFFFFFFF ^ ObjectMask::FLOOR);
	floor->GetPhysicsObject()->SetFriction(friction);
	floor->GetPhysicsObject()->SetElasticity(elasticity);
	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	//world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* MainGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, bool hollow, float innerRadius) {
	GameObject* sphere = new GameObject("sphere");
	sphere->SetLayer(ObjectMask::SPHERE);
	sphere->SetWorldID(rand());
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius,hollow,innerRadius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetLocalTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, brick2Tex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));
	sphere->GetPhysicsObject()->SetLinearResistance(Vector3(0.2, 0.2, 0.2));
	sphere->GetPhysicsObject()->SetAngularResistance(Vector3(0.1, 0.1, 0.1));
	sphere->GetPhysicsObject()->SetInverseMass(inverseMass); 
	sphere->GetPhysicsObject()->InitSphereInertia(volume->GetHollow(),volume->GetInnerRadius());

	if (hollow) {
		sphere->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	}

	//world->AddGameObject(sphere);

	return sphere;
}

GameObject* MainGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject("capsule");
	capsule->SetLayer(ObjectMask::CAPSULE);
	capsule->SetWorldID(rand());
	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	float HRRatio = halfHeight / radius;
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetLocalTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetFriction(0.3);
	float LinRes = 0.01 * HRRatio;
	if (LinRes > 0.8) LinRes = 0.8;
	float AngRes = 0.01 * HRRatio;
	if (AngRes > 0.8) AngRes = 0.8;
	capsule->GetPhysicsObject()->SetLinearResistance(Vector3(0.3 + LinRes, 0.3, 0.3 + LinRes));
	capsule->GetPhysicsObject()->SetAngularResistance(Vector3(0.1 + AngRes, 0.1, 0.1 + AngRes));
	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	//world->AddGameObject(capsule);

	return capsule;

}

GameObject* MainGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("Cube");
	cube->SetLayer(ObjectMask::CUBE);
	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetWorldID(rand());
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetLocalTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetLinearResistance(Vector3(0.7, 0.7, 0.7));
	cube->GetPhysicsObject()->SetAngularResistance(Vector3(0.5, 0.5, 0.5));
	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	//world->AddGameObject(cube);

	return cube;
}

void MainGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100),1.0,1.0,basicTex);
}

void MainGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void MainGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void MainGame::InitDefaultFloor(string file) {
	float floorSize = 20;
	worldFloorGimbal = new GameObject("WorldFloorGimbal");
	world->AddGameObject(worldFloorGimbal)->GetTransform().SetPosition(Vector3(0, 0, 0));
	worldFloor = new GameObject("WorldFloor");
	worldFloorGimbal->AddChild(worldFloor)->GetTransform().SetPosition(Vector3(0, 0, 0));

	std::ifstream input(Assets::DATADIR + file);
	std::vector<Vector3> pointList;
	std::vector<Vector2> sizeList;
	//std::vector<Vector3> pointList;
	char inst = 's';
	float degree = 0;
	float angle = 0;
	float elavation = 0;
	float eleErrorX = 0;
	float eleErrorZ = 0;
	float CurrX = 0;
	float CurrZ = 0;
	float sizeX = 1;
	float sizeZ = 1;
	int type = 0;
	Vector3 axis(0, 0, 0);
	float offsetY = 2;
	char dir = '+';
	int PointNum = 0;
	GameObject* last = nullptr;
	Matrix4 rotation;
	GameObject* b = nullptr;
	Transform t;
	bool final = false;

	try {
		if (input.is_open()) {
			while (inst != 'e')
			{
				input >> inst;
				switch (inst) {
				case 's':
					input >> CurrX;
					input >> elavation;
					input >> CurrZ;
					break;
				case 'm':
					input >> PointNum;
					if (PointNum < pointList.size() && PointNum >= 0) {
						CurrX = pointList[pointList.size() - PointNum - 1].x;
						CurrZ = pointList[pointList.size() - PointNum - 1].z;
						elavation = pointList[pointList.size() - PointNum - 1].y;
						sizeX = sizeList[pointList.size() - PointNum - 1].x;
						sizeZ = sizeList[pointList.size() - PointNum - 1].y;
					}
					break;
				case 'b':
					int building;
					float bX, bY, bZ;
					float yaw, pitch, roll;
					float sX, sY, sZ;
					input >> building;
					input >> bX;
					input >> bY;
					input >> bZ;
					input >> pitch;
					input >> yaw;
					input >> roll;
					input >> sX;
					input >> sY;
					input >> sZ;
					t.SetPosition(last->GetLocalTransform().GetPosition() + Vector3(bX, bY, bZ));
					t.SetScale(Vector3(sX, sY, sZ));
					b = last->AddChild(BuildBuildings(building,t));
					if (b) {
						rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0))* Matrix4::Rotation(pitch, Vector3(1, 0, 0))* Matrix4::Rotation(roll, Vector3(0, 0, -1));
						b->GetLocalTransform().SetOrientation(Quaternion(rotation));
					}
					break;
				case 'x':
					input >> dir;
					if (dir == '+') {
						axis += Vector3(0, 0, 1);
					}
					else {
						axis += Vector3(0, 0, -1);
					}
					break;

				case 'z':
					input >> dir;
					if (dir == '+') {
						axis += Vector3(1, 0, 0);
					}
					else {
						axis += Vector3(-1, 0, 0);
					}
					break;
				case 'f':
					final = true;
				case 'p':
					CurrX += floorSize * sizeX * axis.z;
					CurrZ += floorSize * sizeZ * axis.x;

					input >> degree;
					input >> sizeX;
					input >> sizeZ;
					input >> type;

					CurrX += floorSize * sizeX * axis.z;
					CurrZ += floorSize * sizeZ * axis.x;
					//rotate
					angle = degree * 3.1415f / 180;
					float eleX = (floorSize * sizeX * sin(angle)) * abs(axis.z);
					float eleZ = (floorSize * sizeZ * sin(angle)) * abs(axis.x);
					elavation += eleX + eleZ;
					eleErrorX = floorSize * sizeX - sqrt((floorSize * sizeX) * (floorSize * sizeX) - eleX * eleX);
					eleErrorZ = floorSize * sizeZ - sqrt((floorSize * sizeZ) * (floorSize * sizeZ) - eleZ * eleZ);
					CurrX -= eleErrorX * axis.z;
					CurrZ -= eleErrorZ * axis.x;
					//std::cout << Vector3(CurrX, elavation - offsetY, CurrZ) << angle << axis << Vector2(sizeX * floorSize, sizeZ * floorSize) << std::endl;
					last = AddTileToWorldFloor(Vector3(CurrX, elavation - offsetY, CurrZ), degree, axis.Abs(), Vector2(sizeX * floorSize, sizeZ * floorSize), tileTypes[type].friction, tileTypes[type].elasticity, tileTypes[type].tex);
					if (final) {
						last->SetName("finalPlate");
					}
					elavation += eleX + eleZ;
					CurrX -= eleErrorX * axis.z;
					CurrZ -= eleErrorZ * axis.x;
					axis = Vector3();
					pointList.push_back(Vector3(CurrX, elavation, CurrZ));
					sizeList.push_back(Vector2(sizeX, sizeZ));
					break;
				}
			}
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	input.close();
}

GameObject* MainGame::AddTileToWorldFloor(Vector3 pos, float angle, Vector3 axis, Vector2 floorSize, float friction, float elasticity, OGLTexture* tex) {
	GameObject* obj = worldFloor->AddChild(AddFloorToWorld(pos, Vector3(floorSize.x, 2, floorSize.y), friction, elasticity, tex));
	obj->GetLocalTransform().SetOrientation(Quaternion(Matrix4::Rotation(angle, axis)));
	return obj;
}

void MainGame::InitLavaFloor() {
	for (int i = -5; i < 5; i++) {
		for (int j = -5; j < 5; j++) {
			world->AddGameObject(CreateLavaFloor(Vector3(i*800, -100, j* 800), Vector3(400, 1, 400)));
		}
	}
}

void MainGame::InitBuildings() {
	Buildings.clear();
	//GameObject* obstacleBar = AddCubeToWorld(Vector3())

	//Buildings.push_back(obstacleBar);
}

GameObject* MainGame::BuildBuildings(int num, Transform t) {
	if (num == 0) {
		GameObject* a = AddFloorToWorld(t.GetPosition(), t.GetScale(),1.0f,1.0f,basicTex);
		a->SetupdateFunc([&](float dt, GameObject* a) {
			//std::cout << a->GetParent()->GetTransform().GetScale() << std::endl;
			a->GetLocalTransform().SetOrientation(a->GetLocalTransform().GetOrientation() * Quaternion(Matrix4::Rotation(20 * dt,Vector3(0,1,0))));
			});
		return a;
	}
	else if (num == 1) {
		GameObject* a = AddFloorToWorld(t.GetPosition(), t.GetScale(), 1.0f, 1.0f, basicTex);
		a->SetupdateFunc([&](float dt, GameObject* a) {
			//std::cout << a->GetParent()->GetTransform().GetScale() << std::endl;
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::R)) {
				a->GetLocalTransform().SetPosition(a->GetLocalTransform().GetPosition() + Vector3(0, 10, 0) * dt);
			}
			else if (Window::GetKeyboard()->KeyDown(KeyboardKeys::T)) {
				a->GetLocalTransform().SetPosition(a->GetLocalTransform().GetPosition() + Vector3(0, -10, 0) * dt);
			}
			});
		return a;
		return nullptr;
	}
}

GameObject* MainGame::CreateLavaFloor(const Vector3& position, const Vector3& floorSize) {
	GameObject* floor = new GameObject("lavaFloor");
	floor->SetLayer(ObjectMask::LAVA);
	floor->SetWorldID(rand());
	OBBVolume* volume = new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetLocalTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, lavaTex, basicShader));

	return floor;
}

GameObject* MainGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();
	character->SetLayer(ObjectMask::PLAYER);

	CapsuleVolume* volume = new CapsuleVolume(meshSize, meshSize/2.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetLocalTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	//world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* MainGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	StateGameObject* character = new StateGameObject();
	character->SetLayer(ObjectMask::ENEMY);

	CapsuleVolume* volume = new CapsuleVolume(meshSize, meshSize / 2.0f);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetLocalTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	//world->AddGameObject(character);

	return character;
}

GameObject* MainGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();
	apple->SetLayer(ObjectMask::BONUS);

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetLocalTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	//world->AddGameObject(apple);

	return apple;
}

StateGameObject* MainGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* obj = new StateGameObject();
	obj->SetLayer(ObjectMask::STATEOBJ);

	SphereVolume* volume = new SphereVolume(0.25f);
	obj->SetBoundingVolume((CollisionVolume*)volume);
	obj->GetLocalTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	obj->SetRenderObject(new RenderObject(&obj->GetTransform(), bonusMesh, nullptr, basicShader));
	obj->SetPhysicsObject(new PhysicsObject(&obj->GetTransform(), obj->GetBoundingVolume()));

	obj->GetPhysicsObject()->SetInverseMass(1.0f);
	obj->GetPhysicsObject()->InitSphereInertia();

	//world->AddGameObject(obj);

	return obj;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool MainGame::SelectObject() {
	//Debug::DrawLine(Vector3(100, 100, 100), Vector3(0, 0, 0));
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			Vector4 colour(1, 0.3, 1, 1);
			//Debug::DrawLine(Vector3(100, 100, 100), Vector3(0, 0, 0), colour);
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				//lockedObject	= nullptr;
			}
			//GG
			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			Vector3 pos = ray.GetPosition() - Vector3(0,1,0);

			RayCollision closestCollision;

			int mask = 0xFFFFFFFF; //1 | 2 | 4 | 16;
			if (world->Raycast(ray, closestCollision, true, nullptr, mask)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				//selectionObject->GetPhysicsObject()->SetLinearVelocity(Vector3(Window::GetMouse()->GetRelativePosition().x * 10, 0, Window::GetMouse()->GetRelativePosition().y * 10));
				Debug::DrawLine(pos , closestCollision.collidedAt, colour);
				return true;
			}
			else {
				Debug::DrawLine(pos, ray.GetDirection() * 10000, colour);
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

void MainGame::UpdateErasables() {
	selectionObject = world->NullptrIfErase(selectionObject);
	lockedObject = world->NullptrIfErase(lockedObject);
}

void MainGame::UpdateTransForms() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		(*i)->UpdateGlobalTransform();
	}
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void MainGame::MoveSelectedObject() {
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement();
	if (!selectionObject){
		return;
	}
	/*if (inSelectionMode) {
		selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(Window::GetMouse()->GetAbsolutePosition().x/100,Vector3(1,0,0))* Matrix4::Rotation(Window::GetMouse()->GetAbsolutePosition().y/100, Vector3(0, 0, 1))));
	}*/
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if ((GameObject*)closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}

}

void MainGame::CheckIfObjectSee() {
	GameObjectIterator start;
	GameObjectIterator end;
	world->GetObjectIterators(start, end);

	for (auto& it = start; it != end; ++it) {
		Ray ray = Ray((*it)->GetTransform().GetPosition(), Vector3(0, 0, -1));
		RayCollision closestCollision;
		world->Raycast(ray, closestCollision, true,(*it));
		if (closestCollision.node && ((GameObject*)closestCollision.node)->GetRenderObject()) {
			(*it)->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
			((GameObject*)closestCollision.node)->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
			Debug::DrawLine((*it)->GetTransform().GetPosition(), closestCollision.collidedAt, Vector4(1, 0, 1, 1),1.0f);
		}
		else {
			(*it)->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		}
	}
}