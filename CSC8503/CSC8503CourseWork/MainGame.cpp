#include "MainGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "../../Common/Maths.h"

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

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
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
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

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
	UpdateKeys();
	//CheckIfObjectSee();
	SelectObject();
	MoveSelectedObject();
	if (dt < 0.01f) {
		Sleep(1);
	}
	if (!pause) {
		physics->Update(dt);

		if (testStateObject) {
			testStateObject->Update(dt);
		}

		LockCameraMovment(dt);

		world->UpdateWorld(dt);
	}
	
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

bool MainGame::IsEnd() {
	return isEnd;
}

void MainGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
		//isEnd = true;
		pause = !pause;
		physics->Pause(pause);
	}
	PauseAction();

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Y)) {
		world->AddGameObject(AddSphereToWorld(Vector3(0, 20, 0), 1, 10.0f, true, 0.9))
			->AddChild(AddSphereToWorld(Vector3(4, 20, 0), 1, 10.0f))
			->AddChild(AddCapsuleToWorld(Vector3(8, 20, 0), 1, 0.5f, 10.0f));
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

void MainGame::PauseAction() {
	if (pause) {
		renderer->DrawString("Paused.", Vector2(1, 5));
		//ShowMenu();
	}
	else {
		renderer->DrawString("Press Esc to Pause.", Vector2(1, 5));
		//HideMenu();
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
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
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
		Vector3 camTargetPos = objPos - Vector3(v.x, 0, v.z) + Vector3(0, 20, 0);
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

void MainGame::WorldFloorMovement() {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0));

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector2 movement = Window::GetMouse()->GetRelativePosition();

	worldFloor->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(movement.x, fwdAxis)) * Quaternion(Matrix4::Rotation(movement.y, rightAxis)));
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
	world->GetMainCamera()->SetFarPlane(500.0f);
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
	InitDefaultFloor();
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
GameObject* MainGame::AddFloorToWorld(const Vector3& position, const Vector3& floorSize) {
	GameObject* floor = new GameObject("floor");
	floor->SetLayer(ObjectMask::FLOOR);
	floor->SetWorldID(rand());
	OBBVolume* volume	= new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position)
		.SetOrientation(Quaternion(Matrix4::Rotation(5, Vector3(0,0,1))));

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetMask(0xFFFFFFFF ^ ObjectMask::FLOOR);
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

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
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

	capsule->GetTransform()
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

	cube->GetTransform()
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
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100));
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

void MainGame::InitDefaultFloor() {
	float floorSize = 20;
	worldFloor = new GameObject("WorldFloor");
	world->AddGameObject(worldFloor)->GetTransform().SetOrientation(Quaternion());

	worldFloor->AddChild(AddFloorToWorld(Vector3(0, -2, 0), Vector3(floorSize, 2, floorSize)))->GetTransform()
		.SetOrientation(Matrix4::Rotation(0, Vector3(1, 0, 0)));

	float degree = 30;
	float angle = degree * 3.1415f / 180;
	float elavation = floorSize * sin(angle);
	float eleError = floorSize - sqrt(floorSize * floorSize - elavation * elavation);
	float CurrX = 2 * floorSize - eleError;
	float CurrZ = 0;

	worldFloor->AddChild(AddFloorToWorld(Vector3(CurrX, elavation - 2, CurrZ), Vector3(floorSize, 2, floorSize/2)))->GetTransform()
		.SetOrientation(Matrix4::Rotation(degree, Vector3(0, 0, 1)));

	CurrX = CurrX + 2 * floorSize - eleError;
	elavation = elavation * 2 ;
	worldFloor->AddChild(AddFloorToWorld(Vector3(CurrX , elavation -2, CurrZ), Vector3(floorSize, 2, floorSize)))->GetTransform()
		.SetOrientation(Matrix4::Rotation(0, Vector3(1, 0, 0)));

	CurrZ = CurrZ + 2 * floorSize;
	worldFloor->AddChild(AddFloorToWorld(Vector3(CurrX , elavation -2, CurrZ), Vector3(floorSize, 2, floorSize)))->GetTransform()
		.SetOrientation(Matrix4::Rotation(0, Vector3(1, 0, 0)));

	CurrZ = CurrZ + 2 * floorSize;
	worldFloor->AddChild(AddFloorToWorld(Vector3(CurrX , elavation - 2, CurrZ), Vector3(floorSize, 2, floorSize)))->GetTransform()
		.SetOrientation(Matrix4::Rotation(0, Vector3(1, 0, 0)));


	
}

void MainGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

GameObject* MainGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();
	character->SetLayer(ObjectMask::PLAYER);

	CapsuleVolume* volume = new CapsuleVolume(meshSize, meshSize/2.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
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

	GameObject* character = new GameObject();
	character->SetLayer(ObjectMask::ENEMY);

	CapsuleVolume* volume = new CapsuleVolume(meshSize, meshSize / 2.0f);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
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
	apple->GetTransform()
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
	obj->GetTransform()
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
				lockedObject	= nullptr;
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