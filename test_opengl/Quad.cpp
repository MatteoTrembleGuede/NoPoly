#include "Quad.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

Quad::Quad()
{
	vertices = new float[20]{
		// positions			// uvs
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,		0.0f, 1.0f,
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,		1.0f, 1.0f
	};

	indices = new int[6]{
		0, 1, 2,
		1, 2, 3
	};
	InitGlSelf();
}

Quad::~Quad()
{
	delete vertices;
	delete indices;
	delete uvs;
}

void Quad::Draw()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

Vertex* Quad::GetVertex(QuadVertId ID)
{
	if (ID >= 0 && ID < 4)
	{
		return (Vertex*)&vertices[(int)(ID) * 5];
	}
	return nullptr;
}

void Quad::UpdateGlSelf()
{
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, vertices, GL_STATIC_DRAW);

	// define positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// define uvs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, indices, GL_STATIC_DRAW);
}

void Quad::InitGlSelf()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, vertices, GL_STATIC_DRAW);
	
	// define positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	// define uvs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, indices, GL_STATIC_DRAW);
}
