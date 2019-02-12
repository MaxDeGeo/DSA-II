#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_BLACK);
	//m_pMesh->GenerateSphere(1.0f, 5, C_WHITE);

	//!!!!! Work for E05 !!!!!
#pragma region
	locations.push_back(vector3(-3.0f, 3.0f, 0.0f));
	locations.push_back(vector3(3.0f, 3.0f, 0.0f));
	locations.push_back(vector3(-2.0f, 2.0f, 0.0f));
	locations.push_back(vector3(2.0f, 2.0f, 0.0f));
	locations.push_back(vector3(-3.0f, 1.0f, 0.0f));
	locations.push_back(vector3(-2.0f, 1.0f, 0.0f));
	locations.push_back(vector3(-1.0f, 1.0f, 0.0f));
	locations.push_back(vector3(0.0f, 1.0f, 0.0f));
	locations.push_back(vector3(1.0f, 1.0f, 0.0f));
	locations.push_back(vector3(2.0f, 1.0f, 0.0f));
	locations.push_back(vector3(3.0f, 1.0f, 0.0f));
	locations.push_back(vector3(-4.0f, 0.0f, 0.0f));
	locations.push_back(vector3(-3.0f, 0.0f, 0.0f));
	locations.push_back(vector3(-1.0f, 0.0f, 0.0f));
	locations.push_back(vector3(0.0f, 0.0f, 0.0f));
	locations.push_back(vector3(1.0f, 0.0f, 0.0f));
	locations.push_back(vector3(3.0f, 0.0f, 0.0f));
	locations.push_back(vector3(4.0f, 0.0f, 0.0f));
	locations.push_back(vector3(-5.0f, -1.0f, 0.0f));
	locations.push_back(vector3(-4.0f, -1.0f, 0.0f));
	locations.push_back(vector3(-3.0f, -1.0f, 0.0f));
	locations.push_back(vector3(-2.0f, -1.0f, 0.0f));
	locations.push_back(vector3(-1.0f, -1.0f, 0.0f));
	locations.push_back(vector3(0.0f, -1.0f, 0.0f));
	locations.push_back(vector3(1.0f, -1.0f, 0.0f));
	locations.push_back(vector3(2.0f, -1.0f, 0.0f));
	locations.push_back(vector3(3.0f, -1.0f, 0.0f));
	locations.push_back(vector3(4.0f, -1.0f, 0.0f));
	locations.push_back(vector3(5.0f, -1.0f, 0.0f));
	locations.push_back(vector3(-5.0f, -2.0f, 0.0f));
	locations.push_back(vector3(-3.0f, -2.0f, 0.0f));
	locations.push_back(vector3(-2.0f, -2.0f, 0.0f));
	locations.push_back(vector3(-1.0f, -2.0f, 0.0f));
	locations.push_back(vector3(0.0f, -2.0f, 0.0f));
	locations.push_back(vector3(1.0f, -2.0f, 0.0f));
	locations.push_back(vector3(2.0f, -2.0f, 0.0f));
	locations.push_back(vector3(3.0f, -2.0f, 0.0f));
	locations.push_back(vector3(5.0f, -2.0f, 0.0f));
	locations.push_back(vector3(-5.0f, -3.0f, 0.0f));
	locations.push_back(vector3(-3.0f, -3.0f, 0.0f));
	locations.push_back(vector3(3.0f, -3.0f, 0.0f));
	locations.push_back(vector3(5.0f, -3.0f, 0.0f));
	locations.push_back(vector3(-2.0f, -4.0f, 0.0f));
	locations.push_back(vector3(-1.0f, -4.0f, 0.0f));
	locations.push_back(vector3(1.0f, -4.0f, 0.0f));
	locations.push_back(vector3(2.0f, -4.0f, 0.0f));
#pragma endregion
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	static float value = 0.0f;
	matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(value, 2.0f, 3.0f));
	value += 0.01f;

	//matrix4 m4Model = m4Translate * m4Scale;
	matrix4 m4Model = IDENTITY_M4; // m4Scale * m4Translate;

	matrix4 m4Position;// = glm::translate(IDENTITY_M4, vector3(0.0f, 0.0f, 0.0f));
	//m_pMesh->Render(m4Projection, m4View, m4Position);

	//!!!!! Work for E05 !!!!!
	for (uint x = 0; x < 46; x++)
	{
		m4Position = glm::translate(IDENTITY_M4, locations[x]);
		m4Model = m4Translate * m4Position;
		m_pMesh->Render(m4Projection, m4View, m4Model);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}