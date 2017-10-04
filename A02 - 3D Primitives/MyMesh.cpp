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

	vector3 point0(-fValue, -fValue, fValue); //0
	vector3 point1(fValue, -fValue, fValue); //1
	vector3 point2(fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue, -fValue, -fValue); //4
	vector3 point5(fValue, -fValue, -fValue); //5
	vector3 point6(fValue, fValue, -fValue); //6
	vector3 point7(-fValue, fValue, -fValue); //7

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
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, float a_nSubdivisions, vector3 a_v3Color)
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

	// set radius and height to temp variables
	float fValue = a_fRadius;
	float scaleH = a_fHeight *.5f;

	// create the angle to be set
	float topAng = 360 / a_nSubdivisions;
	float baseAngRad = topAng * PI / 180;

	// create vertex to hold all vertices
	std::vector<vector3> verts;

	// set top and bottom center points
	vector3 topPoint(0, scaleH, 0);
	vector3 bottomPoint(0, -scaleH, 0);

	for (float i = 0; i < a_nSubdivisions; i++) {
		// add vertex3 to vertices for each value
		verts.push_back(vector3(cos(baseAngRad*i)*fValue, -scaleH, sin(baseAngRad*i)*fValue));

		// add tris with two generated points
		if (i >= 1) {
			AddTri(verts[i - 1], topPoint, verts[i]);		// creates faces on the sides
			AddTri(verts[i], bottomPoint, verts[i - 1]);	// creates faces on the bottom
		}

		// connect the frist and last value
		if (i == a_nSubdivisions - 1) {
			AddTri(verts[i], topPoint, verts[0]);
			AddTri(verts[0], bottomPoint, verts[i]);
		}
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

	// set radius and height to temp variables
	float fValue = a_fRadius;
	float scaleH = a_fHeight *.5f;

	// create the angle to be set
	float topAng = 360 / a_nSubdivisions;
	float baseAngRad = topAng * PI / 180;

	// create lists for top and bottom vertices
	std::vector<vector3> bottomVerts;
	std::vector<vector3> topVerts;

	// sets the top and bottom heights
	vector3 topPoint(0, scaleH, 0);
	vector3 bottomPoint(0, -scaleH, 0);

	for (float i = 0; i < a_nSubdivisions; i++) {

		// add vertex3 to vertices for each value on the bottom
		bottomVerts.push_back(vector3(cos(baseAngRad*i)*fValue, -scaleH, sin(baseAngRad*i)*fValue));

		// add vertex3 to vertices for each value on the top
		topVerts.push_back(vector3(cos(baseAngRad*i)*fValue, scaleH, sin(baseAngRad*i)*fValue));

		// add tris
		if (i >= 1) {
			AddTri(bottomVerts[i], bottomPoint, bottomVerts[i - 1]);		// create tris on the bottom
			AddTri(topVerts[i - 1], topPoint, topVerts[i]);					// create tris on the top
			AddTri(bottomVerts[i - 1], topVerts[i], bottomVerts[i]);		// create 1/2 of side face 
			AddTri(topVerts[i], bottomVerts[i - 1], topVerts[i - 1]);		// create other 1/2 of side face
		}

		// connect the frist and last value
		if (i == a_nSubdivisions - 1) {
			AddTri(bottomVerts[0], bottomPoint, bottomVerts[i]);
			AddTri(topVerts[i], topPoint, topVerts[0]);

			AddTri(bottomVerts[i], topVerts[0], bottomVerts[0]);
			AddTri(topVerts[0], bottomVerts[i], topVerts[i]);
		}
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

	// sets the height
	float scaleH = a_fHeight *.5f;

	// find the angle for tris
	float topAng = 360 / a_nSubdivisions;
	float baseAngRad = topAng * PI / 180;

	// create vectors for the outer bottom and top verts
	std::vector<vector3> outerBottomVerts;
	std::vector<vector3> outerTopVerts;

	// create vectors for inner bottom and top vers
	std::vector<vector3> innerVertsBottom;
	std::vector<vector3> innerVertsTop;

	for (float i = 0; i < a_nSubdivisions; i++) {

		// add outer vertices
		outerBottomVerts.push_back(vector3(cos(baseAngRad*i)*a_fOuterRadius, -scaleH, sin(baseAngRad*i)*a_fOuterRadius));
		outerTopVerts.push_back(vector3(cos(baseAngRad*i)*a_fOuterRadius, scaleH, sin(baseAngRad*i)*a_fOuterRadius));

		// add inner vertices
		innerVertsBottom.push_back(vector3(cos(baseAngRad*i)*a_fInnerRadius, -scaleH, sin(baseAngRad*i)*a_fInnerRadius));
		innerVertsTop.push_back(vector3(cos(baseAngRad*i)*a_fInnerRadius, scaleH, sin(baseAngRad*i)*a_fInnerRadius));

		if (i >= 1) {
			// add bottom of tube
			AddQuad(outerBottomVerts[i - 1], outerBottomVerts[i], innerVertsBottom[i - 1], innerVertsBottom[i]);

			// add top of tube
			AddQuad(outerTopVerts[i], outerTopVerts[i - 1], innerVertsTop[i], innerVertsTop[i - 1]);

			// add outer sides
			AddQuad(outerBottomVerts[i], outerBottomVerts[i - 1], outerTopVerts[i], outerTopVerts[i - 1]);

			// add inner sides
			AddQuad(innerVertsBottom[i - 1], innerVertsBottom[i], innerVertsTop[i - 1], innerVertsTop[i]);
		}
		// add shapes that connect the first and last shape
		if (i == a_nSubdivisions - 1) {
			// connect bottom shapes
			AddQuad(outerBottomVerts[i], outerBottomVerts[0], innerVertsBottom[i], innerVertsBottom[0]);

			// connect top shapes
			AddQuad(outerTopVerts[0], outerTopVerts[i], innerVertsTop[0], innerVertsTop[i]);

			// connect outer sides
			AddQuad(outerBottomVerts[0], outerBottomVerts[i], outerTopVerts[0], outerTopVerts[i]);

			// connect inner sides
			AddQuad(innerVertsBottom[i], innerVertsBottom[0], innerVertsTop[i], innerVertsTop[0]);
		}
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

	float tubeRadius = (a_fOuterRadius - a_fInnerRadius) / 2;
	float centerRadius = tubeRadius + a_fInnerRadius;

	// Replace this with your code

	// using theta and phi to figure out the circle
	for (int i = 1; i <= a_nSubdivisionsA; i++) {
		float theta0 = 2 * PI / a_nSubdivisionsA* (i - 1);
		float theta1 = 2 * PI / a_nSubdivisionsA* i;
		for (int j = 1; j <= 2 * a_nSubdivisionsA; j++) {
			float phi0 = 2 * PI / a_nSubdivisionsA* (j - 1);
			float phi1 = 2 * PI / a_nSubdivisionsA* j;

			vector3 point0 = vector3((centerRadius + tubeRadius*cos(theta0))*cos(phi0), (centerRadius + tubeRadius*cos(theta0))*sin(phi0), tubeRadius*sin(theta0));
			vector3 point1 = vector3((centerRadius + tubeRadius*cos(theta0))*cos(phi1), (centerRadius + tubeRadius*cos(theta0))*sin(phi1), tubeRadius*sin(theta0));
			vector3 point2 = vector3((centerRadius + tubeRadius*cos(theta1))*cos(phi0), (centerRadius + tubeRadius*cos(theta1))*sin(phi0), tubeRadius*sin(theta1));
			vector3 point3 = vector3((centerRadius + tubeRadius*cos(theta1))*cos(phi1), (centerRadius + tubeRadius*cos(theta1))*sin(phi1), tubeRadius*sin(theta1));

			AddQuad(point0, point1, point2, point3);
		}
	}
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
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code

	// using theta and phi to figure out the circle
	for (int i = 1; i <= a_nSubdivisions; i++) {
		float theta0 = 2 * PI / a_nSubdivisions* (i - 1);
		float theta1 = 2 * PI / a_nSubdivisions* i;
		for (int j = 1; j <= a_nSubdivisions; j++) {
			float phi0 = PI / a_nSubdivisions* (j - 1);
			float phi1 = PI / a_nSubdivisions* j;

			vector3 point0 = vector3(a_fRadius*cos(theta0)*sin(phi0), a_fRadius*sin(theta0)*sin(phi0), a_fRadius*cos(phi0));
			vector3 point1 = vector3(a_fRadius*cos(theta0)*sin(phi1), a_fRadius*sin(theta0)*sin(phi1), a_fRadius*cos(phi1));
			vector3 point2 = vector3(a_fRadius*cos(theta1)*sin(phi0), a_fRadius*sin(theta1)*sin(phi0), a_fRadius*cos(phi0));
			vector3 point3 = vector3(a_fRadius*cos(theta1)*sin(phi1), a_fRadius*sin(theta1)*sin(phi1), a_fRadius*cos(phi1));

			AddQuad(point0, point1, point2, point3);
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}