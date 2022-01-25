#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName) {
	name = objectName;
	this->updateFunc = [&](float dt, GameObject* a)-> void {};
	layer = 0;
	worldID = -1;
	isActive = true;
	parent = nullptr;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	parent = nullptr;
	offset = Vector3();
}

GameObject::GameObject(string objectName, std::function<void(float dt, GameObject* a)> updatefunc)	{
	this->updateFunc = updateFunc;
	name			= objectName;
	layer			= 0;
	worldID			= -1;
	isActive		= true;
	parent = nullptr;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	parent          = nullptr;
	offset = Vector3();

}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
}

void GameObject::UpdateGlobalTransform() {
	if (parent) {
		Matrix4 newGlobal = parent->GetTransform().GetMatrix() * localTransform.GetMatrix();
		//Matrix4 localrotate = 
		transform.SetOrientation(parent->GetTransform().GetOrientation() * localTransform.GetOrientation());
		transform.SetPosition(newGlobal.GetPositionVector());
		transform.SetScale(localTransform.GetScale());
		/*if (parent->GetName() == "floor") {
			std::cout << transform.GetScale() << transform.GetPosition() << transform.GetOrientation() << parent->GetTransform().GetScale() << parent->GetTransform().GetPosition() << parent->GetTransform().GetOrientation() << std::endl << std::endl;
		}*/
				//std::cout << "=========================" << std::endl;
	}
	
	for (auto it = children.begin(); it != children.end(); ++it) {
		(*it)->UpdateGlobalTransform();
	}

}

void GameObject::changeOrigin(Vector3 offset) {
	this->offset = offset;
	for (auto it = children.begin(); it != children.end(); ++it) {
		(*it)->changeOrigin((*it)->GetLocalOffset() + offset);
	}
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
	else if (boundingVolume->type == VolumeType::Capsule) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		CapsuleVolume volume = (CapsuleVolume&)*boundingVolume;
		Vector3 halfSizes = Vector3(volume.GetRadius(),volume.GetHalfHeight(), volume.GetRadius());
		broadphaseAABB = mat * halfSizes;
	}
}

GameObject* GameObject::AddChild(GameObject* newChild) {
	if (newChild) {
		children.push_back(newChild);
		newChild->SetParent(this);
		return newChild;
	}
	return nullptr;
}

GameObject* GameObject::Pop() {
	if (parent) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			(*it)->SetParent(parent);
			parent->AddChild(*it);
		}
		return parent->PopChild(this);
	}
	return nullptr;
}

GameObject* GameObject::PopChild(string name) {
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->GetName() == name) {
			GameObject* temp = (*it);
			children.erase(it);
			return temp;
		}
	}
	return nullptr;
}

GameObject* GameObject::PopChild(int id) {
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->GetWorldID() == id) {
			GameObject* temp = (*it);
			children.erase(it);
			return temp;
		}
	}
	return nullptr;
}

GameObject* GameObject::PopChild(GameObject* ptr) {
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it) == ptr) {
			GameObject* temp = (*it);
			children.erase(it);
			return temp;
		}
	}
	return nullptr;
}

GameObject* GameObject::FindChild(string name) {
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->GetName() == name) {
			return (*it);
		}
	}
	return nullptr;
}

GameObject* GameObject::FindChild(int id) {
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->GetWorldID() == id) {
			return (*it);
		}
	}
	return nullptr;
}

GameObject* GameObject::FindChild(GameObject* ptr) {
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it) == ptr) {
			return (*it);
		}
	}
	return nullptr;
}