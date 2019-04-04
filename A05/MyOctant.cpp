#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;


//Returns the the octant count
uint MyOctant::GetOctantCount(void) 
{ 
	return m_uOctantCount; 
}

//Initializes the Octants default variables
void MyOctant::Init(void)
{
	m_pRoot = nullptr;
	m_pParent = nullptr;

	m_uChildren = 0;
	m_uLevel = 0;

	m_fSize = 0.0f;
	m_uID = m_uOctantCount;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	for (int x = 0; x < 8; x++)
		m_pChild[x] = nullptr;
}

//Swaps two Octant objects
void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uChildren, other.m_uChildren);
	
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (int x = 0; x < 8; x++)
		std::swap(m_pChild[x], other.m_pChild[x]);
}

//Gets the parent of the current Octant
MyOctant* MyOctant::GetParent(void)
{
	return m_pParent;
}

//Releases the data for the Octant and prevent memory leaks
void MyOctant::Release(void)
{
	if (m_uLevel == 0)
	{
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

//Constructor
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init(); //Sets default values

	//Sets the values of the octant to the arguments.
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;
	
	//Sets the root of the octant to the new Octant
	m_pRoot = this;
	m_lChild.clear(); //Clears the list of nodes that contains elements

	std::vector<vector3> bounds; //The bounds of the object

	int objectCount = m_pEntityMngr->GetEntityCount();

	//Pushes the bounds to the vector
	for (int x = 0; x < objectCount; x++)
	{
		MyEntity* entity = m_pEntityMngr->GetEntity(x);
		MyRigidBody* rigidBody = entity->GetRigidBody();
		bounds.push_back(rigidBody->GetMinGlobal()); 
		bounds.push_back(rigidBody->GetMaxGlobal());
	}

	MyRigidBody* rigidBody = new MyRigidBody(bounds);

	//Sets the halfwidth to the largest halfwidth
	vector3 halfWidth = rigidBody->GetHalfWidth();
	float max = halfWidth.x;
	
	for (int x = 1; x < 3; x++)
	{
		if (max < halfWidth[x])
			max = halfWidth[x];
	}
	vector3 center = rigidBody->GetCenterLocal();
	bounds.clear();
	SafeDelete(rigidBody);

	
	m_fSize = max * 2.0f;
	m_v3Center = center;
	m_v3Min = center - (vector3(max));
	m_v3Max = center + (vector3(max));

	m_uOctantCount++;

	//Constructs the tree
	ConstructTree(m_uMaxLevel);
}

//Sets the center of an Octant and the width of that Octant
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init(); //Creates Octant default values
	m_v3Center = a_v3Center; //sets center
	m_fSize = a_fSize; //sets size

	//The bounds of the Cctant
	m_v3Min = m_v3Center - (vector3(m_fSize / 2.0f));
	m_v3Max = m_v3Center + (vector3(m_fSize / 2.0f));

	m_uOctantCount++; //Increase octant count
}

//Copy Constructor
MyOctant::MyOctant(MyOctant const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot, other.m_pRoot;
	m_lChild, other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (int x = 0; x < 8; x++)
		m_pChild[x] = other.m_pChild[x];
}

//Copy Assignment Operator
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}


//Destructor
MyOctant::~MyOctant()
{
	Release();
};

//Returns the size of the octant
float MyOctant::GetSize(void)
{
	return m_fSize;
}

//Returns the center of the octant
vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

//Returns the min of the octant
vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

//Returns the max of the octant
vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

//Displays the wireframe of the octant starting at the index passed in.
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE); //Adds wireframe to the MeshManager

		return;
	}

	//recursively goes through the children to display all the wireframes.
	for (int x = 0; x < m_uChildren; x++)
		m_pChild[x]->Display(a_nIndex);

}

//Displays the octant wireframe if only a color is given.
void MyOctant::Display(vector3 a_v3Color)
{

	//recursively goes through the children to display all the wireframes.
	for (int x = 0; x < m_uChildren; x++)
		m_pChild[x]->Display(a_v3Color);

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE); //Adds wireframe to the MeshManager
}

//Divides the previous octant into segmants.
void MyOctant::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel) //Can't subdivide if the level is greater than maxLevel
		return;

	if (m_uChildren != 0) //If children exist, do not subdivide this octant. It already has been
		return;

	m_uChildren = 8; //Octant, therefore 8 parts.

	float sizeRadius = m_fSize / 4.0f; //the radius of the octant is 1/4 of the size of the previous level.
	float sizeDiameter = sizeRadius * 2.0f; //The diameter is twice that size.
	vector3 center;


	//Sets the center locations for the new octants in the current octant.
	center = m_v3Center;
	center.x -= sizeRadius;
	center.y -= sizeRadius;
	center.z -= sizeRadius;
	m_pChild[0] = new MyOctant(center, sizeDiameter);

	center.x += sizeDiameter;
	m_pChild[1] = new MyOctant(center, sizeDiameter);

	center.z += sizeDiameter;
	m_pChild[2] = new MyOctant(center, sizeDiameter);

	center.x -= sizeDiameter;
	m_pChild[3] = new MyOctant(center, sizeDiameter);

	center.y += sizeDiameter;
	m_pChild[4] = new MyOctant(center, sizeDiameter);

	center.z -= sizeDiameter;
	m_pChild[5] = new MyOctant(center, sizeDiameter);

	center.x += sizeDiameter;
	m_pChild[6] = new MyOctant(center, sizeDiameter);

	center.z += sizeDiameter;
	m_pChild[7] = new MyOctant(center, sizeDiameter);

	//Sets the values for those new octants.
	for (int x = 0; x < 8; x++)
	{
		m_pChild[x]->m_pRoot = m_pRoot;
		m_pChild[x]->m_pParent = this;
		m_pChild[x]->m_uLevel = m_uLevel + 1;
		if (m_pChild[x]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[x]->Subdivide();
		}
	}
}

//Returns the child at a specific position/number
MyOctant* MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7)
		return nullptr;

	return m_pChild[a_nChild];
}

//Checks if any of the objects in that octant are colliding.
bool MyOctant::IsColliding(uint a_uRBIndex)
{
	int objectCount = m_pEntityMngr->GetEntityCount(); //Object count for that octant

	if (a_uRBIndex >= objectCount)
		return false;

	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rigidBody = entity->GetRigidBody();
	vector3 min = rigidBody->GetMinGlobal();
	vector3 max = rigidBody->GetMaxGlobal();

	//Returns false if they are not
	if (m_v3Max.x < min.x)
		return false;
	if (m_v3Min.x > max.x)
		return false;

	if (m_v3Max.y < min.y)
		return false;
	if (m_v3Min.y > max.y)
		return false;

	if (m_v3Max.z < min.z)
		return false;
	if (m_v3Min.z > max.z)
		return false;

	return true;
}

//Determines if the octant is a leaf.
bool MyOctant::IsLeaf(void)
{
	return m_uChildren == 0; //If it has no children, it is a leaf.
}

//If the octant contains more than the argument of entities
bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	int count = 0;
	int objectCount = m_pEntityMngr->GetEntityCount(); //Gets entity count in that octant

	for (int x = 0; x < objectCount; x++) //Loops through them all.
	{
		if (IsColliding(x)) //If colliding, increase count
			count++;
		if (count > a_nEntities) //If more, return true, it does contain more.
			return true;
	}
	return false;
}

//Deallocates memory for the branches that are killed.
void MyOctant::KillBranches(void)
{
	//Recursive call
	for (int x = 0; x < m_uChildren; x++)
	{
		m_pChild[x]->KillBranches();
		delete m_pChild[x];
		m_pChild[x] = nullptr;
	}

	m_uChildren = 0;
}

//Displays the wireframes of the leafs
void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	int leafs = m_lChild.size();
	for (int x = 0; x < leafs; x++) //Recursively calls the method until no children.
	{
		m_lChild[x]->DisplayLeafs(a_v3Color);
	}
	
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE); //Adds the wireframe of the leaf to the mesh manager
}

//Clears the entity list for the octant
void MyOctant::ClearEntityList(void)
{
	//Recursive call to clear everything
	for (int x = 0; x < m_uChildren; x++)
	{
		m_pChild[x]->ClearEntityList();
	}
	m_EntityList.clear();
}

//Constructs the tree starting at the base level.
void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0) //If the level is not 0, return, else
		return;

	m_uMaxLevel = a_nMaxLevel; //set max level

	m_uOctantCount = 1; //Initial count of the octants present

	m_EntityList.clear(); //Clears the list because it will subdivide, so it doesn't need to track all of the obstacles again.

	KillBranches(); //Kill any branches left
	m_lChild.clear(); //Clear the children.

	if (ContainsMoreThan(m_uIdealEntityCount)) //If it cotains more than the ideal count, subdivide into a smaller collection
	{
		Subdivide(); //Subdivide the current octant into 8 parts.
	}

	AssignIDtoEntity(); //Assigns the id to the octant

	ConstructList(); //Constructs the list of children
}

//Assigns the id to entity
void MyOctant::AssignIDtoEntity(void)
{
	//Recursive call
	for (int x = 0; x < m_uChildren; x++)
		m_pChild[x]->AssignIDtoEntity();

	//If children don't exist, you've reached the lowest level.
	if (m_uChildren == 0)
	{
		int entities = m_pEntityMngr->GetEntityCount(); //Get the count.

		for (int x = 0; x < entities; x++)
		{
			if (IsColliding(x)) //If colliding, push the index of the entity back to the list and add dimensions
			{
				m_EntityList.push_back(x);
				m_pEntityMngr->AddDimension(x, m_uID);
			}
		}
	}
}

//Constructs the list of children to the current octant.
void MyOctant::ConstructList(void)
{
	//Recursive call
	for (int x = 0; x < m_uChildren; x++)
		m_pChild[x]->ConstructList();

	if (m_EntityList.size() != 0)
		m_pRoot->m_lChild.push_back(this);
}