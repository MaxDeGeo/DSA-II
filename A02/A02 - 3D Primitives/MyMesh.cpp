#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	
	float theta = glm::radians(360.0f / a_nSubdivisions); //Gets the angle of the base triangles in radians.
	
	vector3 baseCenter = vector3(0.0f, 0.0f - a_fHeight / 2, 0.0f); //Sets the base to be half the height below the origin.
	vector3 heightPoint = baseCenter; //Sets the position of the tip of the cone.
	heightPoint.y = baseCenter.y + a_fHeight; // Makes the center point the height above the bottom center, setting ths center of the object to the origin.
	vector3 initialPoint = baseCenter; //Initial starting point.
	initialPoint.x = a_fRadius;
	vector3 nextPoint = baseCenter;

	for (uint x = 0; x < a_nSubdivisions; x++)
	{
		//Sets the points of the nextPoint to go to.
		nextPoint.x = glm::cos(theta + (theta * x)) * a_fRadius;
		nextPoint.z = glm::sin(theta + (theta * x)) * a_fRadius;

		AddTri(baseCenter, initialPoint, nextPoint); //Creates the base.

		AddTri(initialPoint, heightPoint, nextPoint); //Creates the height triangle.

		initialPoint = nextPoint; //Moves the initial point to the last made point.
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float theta = glm::radians(360.0f / a_nSubdivisions); //Gets the angle of the base triangles in radians.

	//Sets the bottom base values of the cylinder.
	vector3 bottomCenter = vector3(0.0f, 0.0f - a_fHeight / 2, 0.0f);
	vector3 bottomInitial = bottomCenter;
	bottomInitial.x = a_fRadius;
	vector3 bottomNext = bottomCenter;

	//Sets the top base values of the cylinder. 
	vector3 topCenter = bottomCenter;
	topCenter.y += a_fHeight;
	vector3 topInitial = bottomInitial;
	topInitial.y += a_fHeight;
	vector3 topNext;


	//Loops through the number of subdivisions to create the primitive object
	for (uint x = 0; x < a_nSubdivisions; x++)
	{
		//Sets the third point for the bottom base.
		bottomNext.x = glm::cos(theta + (theta * x)) * a_fRadius;
		bottomNext.z = glm::sin(theta + (theta * x)) * a_fRadius;

		//Sets the top third point equal to the bottom third point but the height above it.
		topNext = bottomNext;
		topNext.y += a_fHeight;

		//Adds the top and bottom bases
		AddTri(bottomInitial, bottomNext, bottomCenter); //Creates the bottom base.
		AddTri(topInitial, topCenter, topNext); //Creates the top base.

		//Adds the quad connecting the top and bottom edges.
		AddQuad(bottomNext, bottomInitial, topNext, topInitial);

		//Sets the initial points of the top and bottom to the recently created next point.
		bottomInitial = bottomNext;
		topInitial = topNext;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float theta = glm::radians(360.0f / a_nSubdivisions);

	//Bottom Base
	vector3 bottomCenter = vector3(0.0f, 0.0f - a_fHeight / 2, 0.0f);
	vector3 bottomInitial = vector3(a_fInnerRadius, 0.0f - a_fHeight / 2, 0.0f);
	vector3 bottomInitial2 = vector3(a_fOuterRadius, 0.0f - a_fHeight / 2, 0.0f);
	vector3 bottomNext = vector3(0.0f, 0.0f - a_fHeight / 2, 0.0f);
	vector3 bottomNext2 = vector3(0.0f, 0.0f - a_fHeight / 2, 0.0f);

	//Top Base
	vector3 topCenter = vector3(0.0f, 0.0f + a_fHeight / 2, 0.0f);
	vector3 topInitial = vector3(a_fInnerRadius, 0.0f + a_fHeight / 2, 0.0f);
	vector3 topInitial2 = vector3(a_fOuterRadius, 0.0f + a_fHeight / 2, 0.0f);
	vector3 topNext = vector3(0.0f, 0.0f + a_fHeight / 2, 0.0f);
	vector3 topNext2 = vector3(0.0f, 0.0f + a_fHeight / 2, 0.0f);

	//Creating the faces
	for (uint x = 0; x < a_nSubdivisions; x++)
	{
		//Sets bottom values
		bottomNext.x = glm::cos(theta + (theta) * x) * a_fInnerRadius;
		bottomNext.z = glm::sin(theta + (theta) * x) * a_fInnerRadius;

		bottomNext2.x = glm::cos(theta + (theta)* x) * a_fOuterRadius;
		bottomNext2.z = glm::sin(theta + (theta)* x) * a_fOuterRadius;

		//Sets top values
		topNext.x = glm::cos(theta + (theta)* x) * a_fInnerRadius;
		topNext.z = glm::sin(theta + (theta)* x) * a_fInnerRadius;

		topNext2.x = glm::cos(theta + (theta)* x) * a_fOuterRadius;
		topNext2.z = glm::sin(theta + (theta)* x) * a_fOuterRadius;

		//Adding Points
		AddQuad(bottomInitial, bottomInitial2, bottomNext, bottomNext2);
		AddQuad(topNext, topNext2, topInitial, topInitial2);

		AddQuad(bottomInitial, bottomNext, topInitial, topNext);
		AddQuad(topInitial2, topNext2, bottomInitial2, bottomNext2);
		
		//Set new values
		bottomInitial = bottomNext;
		bottomInitial2 = bottomNext2;
		
		topInitial = topNext;
		topInitial2 = topNext2;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);

	float theta = glm::radians(360.0f / a_nSubdivisionsA);
	float phi = glm::radians(360.0f / a_nSubdivisionsB);
	
	vector3 tubeCenter;// = vector3(a_fInnerRadius + (a_fOuterRadius), 0.0f, 0.0f);

	std::vector<vector3> initialPoints;

	std::vector<vector3> nextPoints;

	theta = glm::radians(0.0f);
	phi = glm::radians(0.0f);

	tubeCenter = vector3((a_fInnerRadius + a_fOuterRadius) * glm::cos(theta), 0.0f, (a_fInnerRadius + a_fOuterRadius) * glm::sin(theta));

	//std::cout << tubeCenter.x << tubeCenter.y << tubeCenter.z << std::endl;

	for (uint x = 0; x < a_nSubdivisionsB; x++)
	{
		float xCoor = tubeCenter.x + (glm::cos(phi) * a_fOuterRadius);
		float yCoor = tubeCenter.y + (glm::sin(phi) * a_fOuterRadius);
		float zCoor = 0.0f;// tubeCenter.z + (glm::cos(phi) * glm::sin(phi) * a_fOuterRadius);

		initialPoints.push_back(vector3(xCoor, yCoor, zCoor));
	}

	for (uint z = 0; z < initialPoints.size(); z++)
	{
		if (z != initialPoints.size() - 1)
		{
			std::cout << tubeCenter.x << " " << tubeCenter.y << " " << tubeCenter.z << std::endl;
			std::cout << initialPoints[z].x << " " << initialPoints[z].y << " " << initialPoints[z].z << std::endl;
			std::cout << initialPoints[z+1].x << " " << initialPoints[z+1].y << " " << initialPoints[z+1].z << std::endl;
			AddTri(tubeCenter, initialPoints[z], initialPoints[z + 1]);
		}
		else
		{
			AddTri(tubeCenter, initialPoints[z], initialPoints[0]);
		}
	}

	//initialPoints.clear();

	/*
	for (uint x = 0; x < a_nSubdivisionsA; x++)
	{
		theta += (theta * x);

		tubeCenter = vector3((a_fInnerRadius + a_fOuterRadius) * glm::cos(theta), 0.0f, (a_fInnerRadius + a_fOuterRadius) * glm::sin(theta));

		for (uint y = 0; y < a_nSubdivisionsB; y++)
		{
			phi += (phi * y);

			float xCoor = tubeCenter.x + (glm::cos(phi) * glm::cos(theta) * a_fOuterRadius);
			float yCoor = tubeCenter.y + (glm::sin(phi) * glm::sin(theta) * a_fOuterRadius);
			float zCoor = tubeCenter.z + (glm::cos(phi) * glm::cos(theta) * glm::sin(phi) * glm::sin(theta) * a_fOuterRadius);

			initialPoints.push_back(vector3(xCoor, yCoor, zCoor));
		}

		for (uint z = 0; z < initialPoints.size(); z++)
		{
			if (z != initialPoints.size() - 1)
			{
				AddTri(tubeCenter, initialPoints[z], initialPoints[z + 1]);
			}
			else
			{
				AddTri(tubeCenter, initialPoints[z], initialPoints[0]);
			}
		}

		initialPoints.clear();
	}
	*/
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	float theta = 0; // glm::radians(360.0f / a_nSubdivisions);
	float phi = 0;


	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}