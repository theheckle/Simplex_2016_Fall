#include "AppClass.h"
void Application::InitVariables(void)
{

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));

	m_uMeshCount = 46;
	for (uint i = 0; i <= m_uMeshCount; i++) {
		MyMesh* pMesh = new MyMesh();
		m_pMeshList.push_back(pMesh);
		m_pMeshList[i]->GenerateCube(1.0f, C_BLACK);
	}
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

	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	matrix4 m4View = m_pCameraMngr->GetViewMatrix();

	std::vector<vector3> loc;
	static float value = 0;
	//first column
	loc.push_back(vector3(value+-5, 1, 0));
	loc.push_back(vector3(value+-5, 0, 0));
	loc.push_back(vector3(value+-5, -1, 0));

	// second column
	loc.push_back(vector3(value+-4, 2, 0));
	loc.push_back(vector3(value+-4, 1, 0));

	// third column
	loc.push_back(vector3(value+-3, 5, 0));
	loc.push_back(vector3(value+-3, 3, 0));
	loc.push_back(vector3(value+-3, 2, 0));
	loc.push_back(vector3(value+-3, 1, 0));
	loc.push_back(vector3(value+-3, 0, 0));
	loc.push_back(vector3(value+-3, -1, 0));

	// fourth column
	loc.push_back(vector3(value+-2, 4, 0));
	loc.push_back(vector3(value+-2, 3, 0));
	loc.push_back(vector3(value+-2, 1, 0));
	loc.push_back(vector3(value+-2, 0, 0));
	loc.push_back(vector3(value+-2, -2, 0));

	// fifth, sixth, seventh column
	for (int i = -1; i < 2; i++) {
		loc.push_back(vector3(value+i, 3, 0));
		loc.push_back(vector3(value+i, 2, 0));
		loc.push_back(vector3(value+i, 1, 0));
		loc.push_back(vector3(value+i, 0, 0));
		loc.push_back(vector3(value+i, -2, 0));
	}

	// eighth column
	loc.push_back(vector3(value+2, 4, 0));
	loc.push_back(vector3(value+2, 3, 0));
	loc.push_back(vector3(value+2, 1, 0));
	loc.push_back(vector3(value+2, 0, 0));
	loc.push_back(vector3(value+2, -2, 0));

	// ninth column
	loc.push_back(vector3(value+3, 5, 0));
	loc.push_back(vector3(value+3, 3, 0));
	loc.push_back(vector3(value+3, 2, 0));
	loc.push_back(vector3(value+3, 1, 0));
	loc.push_back(vector3(value+3, 0, 0));
	loc.push_back(vector3(value+3, -1, 0));

	// tenth column
	loc.push_back(vector3(value+4, 2, 0));
	loc.push_back(vector3(value+4, 1, 0));

	// eleventh column
	loc.push_back(vector3(value+5, 1, 0));
	loc.push_back(vector3(value+5, 0, 0));
	loc.push_back(vector3(value+5, -1, 0));

	value += 0.01f;

	for (uint k = 0; k <= m_uMeshCount; k++) {
		m_pMeshList[k]->Render(m4Projection, m4View, glm::translate(loc[k]));
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
	for (uint i = 0; i < m_uMeshCount; i++) {
		if (m_pMeshList[i] != nullptr)
		{
			delete m_pMeshList[i];
			m_pMeshList[i] = nullptr;
		}
		SafeDelete(m_pMeshList[i]);

	}
	//release GUI
	ShutdownGUI();
}