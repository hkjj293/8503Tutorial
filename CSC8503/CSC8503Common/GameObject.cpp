#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	layer			= 0;
	worldID			= -1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	parent          = nullptr;

}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
}

void GameObject::UpdateGlobalTransform(Transform& oldParent) {
	Transform oldTrans = transform;
	if (parent) {
		Matrix4 oldGlobal = transform.GetMatrix();
		Matrix4 local = oldParent.GetMatrix().Inverse() * transform.GetMatrix();
		Matrix4 newGlobal = local * parent->GetTransform().GetMatrix();
		transform.SetPosition(newGlobal.GetPositionVector());
		transform.SetOrientation(Quaternion(newGlobal));
		transform.SetScale(parent->GetTransform().GetScale() * oldTrans.GetScale());
		transform.UpdateMatrix();
	}
	for (auto it = children.begin(); it != children.end(); ++it) {
		(*it)->UpdateGlobalTransform(oldTrans);
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
	}
	return newChild;
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