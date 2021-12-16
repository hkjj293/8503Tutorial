#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include <set>
#include "../../Common/Camera.h"
#include <algorithm>

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld(){
	mainCamera = new Camera();

	shuffleConstraints	= false;
	shuffleObjects		= false;
	worldIDCounter		= 0;

	octTree = new OctTree<GameObject*>(Vector3(50, 50, 50), Vector3(100, 100, 100), 7, 6);
}

GameWorld::~GameWorld()	{
}

void GameWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : constraints) {
		delete i;
	}
	Clear();
}

GameObject* GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
	return o;
}

void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete) {
		delete o;
	}
}

void GameWorld::GetObjectIterators(
	GameObjectIterator& first,
	GameObjectIterator& last) const {

	first	= gameObjects.begin();
	last	= gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) {
		f(g);
	}
}

void GameWorld::UpdateWorld(float dt) {

	ClearEraseList();
	if (shuffleObjects) {
		std::random_shuffle(gameObjects.begin(), gameObjects.end());
	}

	if (shuffleConstraints) {
		std::random_shuffle(constraints.begin(), constraints.end());
	}
	
	CheckObjectsHeight();
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject, GameObject* ignore,  unsigned int mask) {
	//The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;
	
	std::vector<GameObject*> list;

	std::set<GameObject*> broadphaseCollisions;
	//octTree->DebugDraw();
	octTree->OperateOnRayCastContents(
		r,
		[&](std::list <OctTreeEntry <GameObject*>>& data) {
			for (auto it = data.begin(); it != data.end(); ++it) {
				broadphaseCollisions.insert(it->object);
			}
		});

	for (auto& i : broadphaseCollisions) {
		list.push_back(i);
	}


	for (auto& i : list) {
		if (ignore && i == ignore) {
			continue;
		}
		if (!(i->GetLayer() & mask)) {
			continue;
		}
		if (!i->GetBoundingVolume()) { //objects might not be collideable etc...
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
				
			if (!closestObject) {	
				closestCollision		= thisCollision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}
	if (collision.node) {
		closestCollision		= collision;
		closestCollision.node	= collision.node;
		return true;
	}
	return false;
}

/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c, bool andDelete) {
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
	if (andDelete) {
		delete c;
	}
}

void GameWorld::GetConstraintIterators(
	std::vector<Constraint*>::const_iterator& first,
	std::vector<Constraint*>::const_iterator& last) const {
	first	= constraints.begin();
	last	= constraints.end();
}

void GameWorld::UpdateOctTree() {
	//delete octTree;
	octTree->Clear();
	//std::cout << "update" << std::endl;
	GameObjectFunc f = [&](GameObject* o) {
		Vector3 halfSizes;
		if (!o->GetBroadphaseAABB(halfSizes)) {
			return;
		}
		Vector3 pos = o->GetTransform().GetPosition();
		//std::cout << o->GetName() << o->GetWorldID() << std::endl;
		octTree->Insert(o, pos, halfSizes);
	};

	for (auto& i : gameObjects) {
		if(NullptrIfErase(i)){
			f(i);
			i->OnSpreadChild(f);
		}
	}
}

void GameWorld::CheckObjectsHeight() {
	GameObjectFunc f = [&](GameObject* o) {
		Vector3 pos = o->GetTransform().GetPosition();
		if (pos.y < -10) {
			GameObject* temp = o->Pop();
			if (temp) {
				eraseList.push_back(temp);
			}
			else {
				GameObject* popout = EraseObject(o);
				if (popout) {
					eraseList.push_back(popout);
				}
			}
		}
	};
	for (auto& i : gameObjects) {
		i->OnSpreadChild(f);
		f(i);
	}
}

GameObject* GameWorld::EraseObject(GameObject* obj) {
	for (auto it = gameObjects.begin(); it != gameObjects.end(); ++it) {
		if ((*it) == obj) {
			gameObjects.erase(it);
			for (int i = 0; i < obj->GetChildrenSize(); ++i) {
				AddGameObject(obj->GetChildren()[i])->SetParent(nullptr);
			}
			return obj;
		}
	}
	return nullptr;
}

GameObject* GameWorld::NullptrIfErase(GameObject* obj) {
	for (auto it = eraseList.begin(); it != eraseList.end(); ++it) {
		if (*it == obj) {
			return nullptr;
		}
	}
	return obj;
}

void GameWorld::ClearEraseList() {
	for (auto it = eraseList.begin(); it != eraseList.end(); ++it) {
		delete (*it);
	}
	eraseList.clear();
}