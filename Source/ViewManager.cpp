///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Created for CS-330-Computational Graphics and Visualization
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// declaration of the global variables and defines
namespace
{
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;

	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object
	Camera* g_pCamera = nullptr;

	// mouse tracking
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// timing
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// projection mode
	bool bOrthographicProjection = false;
}

/***********************************************************
 * ViewManager()
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;

	g_pCamera = new Camera();

	// default camera position
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80.0f;
}

/***********************************************************
 * ~ViewManager()
 ***********************************************************/
ViewManager::~ViewManager()
{
	m_pShaderManager = NULL;
	m_pWindow = NULL;

	if (g_pCamera != NULL)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 * CreateDisplayWindow()
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(
	const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL,
		NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	// capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// mouse movement callback
	glfwSetCursorPosCallback(
		window,
		&ViewManager::Mouse_Position_Callback);

	// mouse wheel callback
	glfwSetScrollCallback(
		window,
		&ViewManager::Mouse_Scroll_Callback);

	// transparency support
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return window;
}

/***********************************************************
 * Mouse_Position_Callback()
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(
	GLFWwindow* window,
	double xMousePos,
	double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = (float)xMousePos;
		gLastY = (float)yMousePos;
		gFirstMouse = false;
	}

	float xOffset = (float)xMousePos - gLastX;
	float yOffset = gLastY - (float)yMousePos;

	gLastX = (float)xMousePos;
	gLastY = (float)yMousePos;

	if (g_pCamera != NULL)
	{
		g_pCamera->ProcessMouseMovement(
			xOffset,
			yOffset);
	}
}

/***********************************************************
 * Mouse_Scroll_Callback()
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(
	GLFWwindow* window,
	double xOffset,
	double yOffset)
{
	if (g_pCamera != NULL)
	{
		g_pCamera->ProcessMouseScroll(
			(float)yOffset);
	}
}

/***********************************************************
 * ProcessKeyboardEvents()
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// exit program
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(
			m_pWindow,
			true);
	}

	if (g_pCamera == NULL)
	{
		return;
	}

	// WASD movement
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(
			FORWARD,
			gDeltaTime);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(
			BACKWARD,
			gDeltaTime);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(
			LEFT,
			gDeltaTime);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(
			RIGHT,
			gDeltaTime);
	}

	// Q/E vertical movement
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(
			UP,
			gDeltaTime);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(
			DOWN,
			gDeltaTime);
	}

	// perspective mode
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		bOrthographicProjection = false;
	}

	// orthographic mode
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		bOrthographicProjection = true;
	}
}


/***********************************************************
 * PrepareSceneView()
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// keyboard controls
	ProcessKeyboardEvents();

	// camera view matrix
	view = g_pCamera->GetViewMatrix();

	// projection matrix
	if (bOrthographicProjection)
	{
		projection = glm::ortho(
			-10.0f,
			10.0f,
			-10.0f,
			10.0f,
			0.1f,
			100.0f);
	}
	else
	{
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
			0.1f,
			100.0f);
	}

	if (m_pShaderManager != NULL)
	{
		m_pShaderManager->setMat4Value(
			g_ViewName,
			view);

		m_pShaderManager->setMat4Value(
			g_ProjectionName,
			projection);

		// Camera position for lighting calculations
		m_pShaderManager->setVec3Value(
			"viewPosition",
			g_pCamera->Position);

		// Primary light source above the lamp
		m_pShaderManager->setVec3Value(
			"lightPosition",
			0.0f,
			3.0f,
			0.0f);

		// Warm white light color
		m_pShaderManager->setVec3Value(
			"lightColor",
			1.0f,
			1.0f,
			0.9f);

		// Ambient light to prevent complete shadows
		m_pShaderManager->setVec3Value(
			"ambientLight",
			0.3f,
			0.3f,
			0.3f);
	}
}