#include "MyOctant.h"

// declare static variables outside scope of class
Simplex::uint Simplex::MyOctant::m_uOctantCount;
Simplex::uint Simplex::MyOctant::m_uMaxLevel;
Simplex::uint Simplex::MyOctant::m_uIdealEntityCount;

// constructor
Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	// set up initial variables
	Init();
	m_pRoot = this;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;

	// get center values to compare for min/max
	m_v3Max = m_pEntityMngr->GetRigidBody()->GetCenterGlobal();
	m_v3Min = m_pEntityMngr->GetRigidBody()->GetCenterGlobal();

	// get number of entities in the manager to keep track
	m_uEntityCount = m_pEntityMngr->GetEntityCount();

	// cycle through to get min/max of entire rigidbody
	for (uint i = 0; i < m_uEntityCount; ++i) {

		// add entity to current list
		m_EntityList.push_back(i);

		// set temp values for min / max
		vector3 rigidbodyMin = m_pEntityMngr->GetRigidBody(i)->GetMinGlobal();
		vector3 rigidbodyMax = m_pEntityMngr->GetRigidBody(i)->GetMaxGlobal();

		// compare values to find true min / max
		if (rigidbodyMin.x < m_v3Min.x) 
			m_v3Min.x = rigidbodyMin.x;
		if (rigidbodyMax.x > m_v3Max.x)
			m_v3Max.x = rigidbodyMax.x;
		if (rigidbodyMin.y < m_v3Min.y)
			m_v3Min.y = rigidbodyMin.y;
		if (rigidbodyMax.y > m_v3Max.y)
			m_v3Max.y = rigidbodyMax.y;
		if (rigidbodyMin.z < m_v3Min.z) 
			m_v3Min.z = rigidbodyMin.z;
		if (rigidbodyMax.z > m_v3Max.z)
			m_v3Max.z = rigidbodyMax.z;
	}

	// calculate center value
	m_v3Center = (m_v3Min + m_v3Max) / 2.f;

	// calculate size of object, not calculating for different values
	m_fSize = m_v3Max.x - m_v3Min.x;

	// constructs tree with subdivisions
	ConstructTree(a_nMaxLevel);

	// constructs the list of objects in entity list
	ConstructList();
}

Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	// initializes values
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Max = a_v3Center + a_fSize / 2.f;
	m_v3Min = a_v3Center - a_fSize / 2.f;
}

Simplex::MyOctant::MyOctant(MyOctant const & other)
{
	// initializes values
	Init();
	m_fSize = other.m_fSize;
	m_pEntityMngr = other.m_pEntityMngr;
	m_pMeshMngr = other.m_pMeshMngr;
	m_pParent = other.m_pParent;
	m_pRoot = other.m_pRoot;
	m_uID = other.m_uID;
	m_uIdealEntityCount = other.m_uIdealEntityCount;
	m_uLevel = other.m_uLevel;
	m_uMaxLevel = other.m_uMaxLevel;
	m_uOctantCount = other.m_uOctantCount;
	m_v3Center = other.m_v3Center;
	m_v3Max = other.m_v3Max;
	m_v3Min = other.m_v3Min;

	// sets children to other children
	m_uChildren = other.m_uChildren;
	for (uint i = 0; i < m_uChildren; ++i) {
		m_pChild[i] = new MyOctant(*other.m_pChild[i]);
	}

	// sets entity list to other entity list
	m_uEntityCount = other.m_uEntityCount;
	for (uint i = 0; i < m_uEntityCount; ++i) {
		m_EntityList.push_back(other.m_EntityList[i]);
	}

	// sets root and child list
	if (this == m_pRoot) {
		float tempCount = other.m_lChild.size();
		for (uint i = 0; i < tempCount; ++i) {
			m_lChild.push_back(other.m_lChild[i]);
		}
	}
}

Simplex::MyOctant& Simplex::MyOctant::operator=(MyOctant const& other)
{
	// if other is the same value, return original value
	if (&other == this)
		return *this;

	// re-initialize values
	Release();
	Init();

	m_uLevel = other.m_uLevel;
	m_fSize = other.m_fSize;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;
	m_pParent = other.m_pParent;

	// cycles through children
	m_uChildren = other.m_uChildren;
	for (uint i = 0; i < m_uChildren; ++i) {
		m_pChild[i] = new MyOctant(*other.m_pChild[i]);
	}

	// re-initialize entity list
	m_uEntityCount = other.m_uEntityCount;
	for (uint i = 0; i < m_uEntityCount; ++i) {
		m_EntityList.push_back(other.m_EntityList[i]);
	}

	// sets root and children of root
	m_pRoot = other.m_pRoot;
	if (this == m_pRoot) {
		float tempCount = other.m_lChild.size();
		for (uint i = 0; i < tempCount; ++i) {
			m_lChild.push_back(other.m_lChild[i]);
		}
	}

	return *this;
}

Simplex::MyOctant::~MyOctant(void)
{
	Release();
}

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_EntityList, other.m_EntityList);
	std::swap(m_uEntityCount, other.m_uEntityCount);
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_pChild, other.m_pChild);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);
	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uMaxLevel, other.m_uMaxLevel);
	std::swap(m_uOctantCount, other.m_uOctantCount);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Max, other.m_v3Max);
	std::swap(m_v3Min, other.m_v3Min);
}

float Simplex::MyOctant::GetSize(void)
{
	return m_fSize;
}

Simplex::vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

Simplex::vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

Simplex::vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	// gets new rigidbody obj
	MyRigidBody* rigidbody = m_pEntityMngr->GetRigidBody(a_uRBIndex);

	// finds min and max values to compare
	vector3 rigidbodyMax = rigidbody->GetMaxGlobal();
	vector3 rigidbodyMin = rigidbody->GetMinGlobal();

	// check for collision
	if (rigidbodyMax.x > m_v3Min.x &&
		rigidbodyMin.x < m_v3Max.x &&
		rigidbodyMin.y < m_v3Max.y &&
		rigidbodyMax.y > m_v3Min.y &&
		rigidbodyMax.z > m_v3Min.z &&
		rigidbodyMin.z < m_v3Max.z) {
		return true;
	}
	else 
		return false;
}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	// if index is greater than number of octants, display everything
	if (a_nIndex >= m_uOctantCount) {
		DisplayLeafs();
		return;
	}

	// display each child node otherwise
	m_lChild[a_nIndex]->Display(a_v3Color);
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	// creates single cube to be rendered
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(IDENTITY_M4, m_v3Max-m_v3Min), a_v3Color);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	// checks if obj is a leaf obj
	// if yes, display single value
	if (IsLeaf()) {
		Display(a_v3Color);
	}
	// if no, recurse until it is a leaf value
	else {
		for (uint i = 0; i < m_uChildren; ++i) {
			m_pChild[i]->DisplayLeafs(a_v3Color);
		}
	}
}

void Simplex::MyOctant::ClearEntityList(void)
{
	// loops through children and recursively calls until no children left
	for (uint i = 0; i < m_uChildren; ++i) {
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}

void Simplex::MyOctant::Subdivide(void)
{
	// initialize new child objects at position / 4
	m_pChild[0] = new MyOctant(m_v3Center + vector3(-m_fSize / 4, m_fSize / 4, -m_fSize/ 4), m_fSize / 2.f);
	m_pChild[1] = new MyOctant(m_v3Center + vector3(-m_fSize / 4, m_fSize / 4, m_fSize / 4), m_fSize / 2.f);
	m_pChild[2] = new MyOctant(m_v3Center + vector3(-m_fSize / 4, -m_fSize / 4, -m_fSize / 4), m_fSize / 2.f);
	m_pChild[3] = new MyOctant(m_v3Center + vector3(-m_fSize / 4, -m_fSize / 4, m_fSize / 4), m_fSize / 2.f);
	m_pChild[4] = new MyOctant(m_v3Center + vector3(m_fSize / 4, -m_fSize / 4, -m_fSize/ 4), m_fSize / 2.f);
	m_pChild[5] = new MyOctant(m_v3Center + vector3(m_fSize / 4, -m_fSize / 4, m_fSize / 4), m_fSize / 2.f);
	m_pChild[6] = new MyOctant(m_v3Center + vector3(m_fSize / 4, m_fSize / 4, -m_fSize / 4), m_fSize / 2.f);
	m_pChild[7] = new MyOctant(m_v3Center + vector3(m_fSize / 4, m_fSize / 4, m_fSize / 4), m_fSize / 2.f);
	
	// sets child value
	m_uChildren = 8;
}

Simplex::MyOctant* Simplex::MyOctant::GetChild(uint a_nChild)
{
	// if there are no children, return nullptr
	if (m_uChildren == 0)
		return nullptr;
	// otherwise return index of child
	else 
		return m_pChild[a_nChild];
}

Simplex::MyOctant* Simplex::MyOctant::GetParent(void)
{
	return m_pParent;
}

bool Simplex::MyOctant::IsLeaf(void)
{
	if (m_uChildren == 0) 
		return true;
	return false;
}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	if (m_uEntityCount > a_nEntities)
		return true;
	return false;
}

void Simplex::MyOctant::KillBranches(void)
{
	// if is a leaf, then there are no branches to delete
	if (IsLeaf()) {
		return;
	}
	// otherwise check for branches and delete children
	else {
		for (uint i = 0; i < m_uChildren; ++i) {
			m_pChild[i]->KillBranches();
			SafeDelete(m_pChild[i]);
		}
	}
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	// checks if level is bigger than max level
	// checks if idealentity count is bigger than current entity cound
	if (m_uLevel >= a_nMaxLevel || !ContainsMoreThan(m_uIdealEntityCount)) {
		m_pRoot->m_lChild.push_back(this);
		return;
	}

	// if children exist, cannot subdivide
	if (m_uChildren == 8) {
		return;
	}

	// creates new child objects
	Subdivide();

	// goes through children and adds them to entity list
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		m_pChild[i]->m_pRoot = m_pRoot;

		for (uint j = 0; j < m_uEntityCount; j++) {
			if (m_pChild[i]->IsColliding(m_EntityList[j]))
				m_pChild[i]->m_EntityList.push_back(m_EntityList[j]);
		}

		// updates entity count
		m_pChild[i]->m_uEntityCount = m_pChild[i]->m_EntityList.size();

		// recursively call 
		m_pChild[i]->ConstructTree(a_nMaxLevel);
	}
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
	// cycles through entity and adds dimension
	for (uint i = 0; i < m_uEntityCount; ++i) {
		m_pEntityMngr->AddDimension(m_EntityList[i], m_uID);
	}
}

Simplex::uint Simplex::MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void Simplex::MyOctant::Release(void)
{
	// deletes all children first 
	if (this == m_pRoot) {
		KillBranches();
	}

	// resets octant count
	m_uOctantCount = 0;
}

void Simplex::MyOctant::Init(void)
{
	// initializes managers
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	// updates id and count
	m_uID = m_uOctantCount;
	m_uOctantCount += 1;
}

void Simplex::MyOctant::ConstructList(void)
{
	// assigns dimensions to leaves
	if (IsLeaf()) {
		AssignIDtoEntity();
	}
	// otherwise continues until it finds a leaf
	else {
		for (uint i = 0; i < m_uChildren; ++i) {
			m_pChild[i]->ConstructList();
		}
	}
}