#ifndef SPHERE_H
#define SPHERE_H
#include <glad/glad.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

class Sphere
{
private:
	std::vector<float> sphereVerts;
	std::vector<float> sphereTexcoord;
	std::vector<int> sphereIndices;
	GLuint VBO, VAO, EBO;
	float radius = 1.0f;
	int sectorCount = 36;
	int stackCount = 18;

public:

	~Sphere()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
	Sphere(float r, int sectors, int stacks)
	{
		radius = r;
		sectorCount = sectors;
		stackCount = stacks;

		float x, y, z, xy;
		float lengthInv = 1.0f / radius;
		float s, t;

		float sectorStep = (float)(2 * M_PI / sectorCount);
		float stackStep = (float)(M_PI / stackCount);
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = (float)(M_PI / 2 - i * stackStep);
			xy = 1.02f * radius * cosf(stackAngle);
			z = radius * sinf(stackAngle);
			for (int j = 0; j <= sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;
				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle);
				y = xy * sinf(sectorAngle);
				sphereVerts.push_back(x);
				sphereVerts.push_back(y);
				sphereVerts.push_back(z);

				s = (float)j / sectorCount;
				t = (float)i / stackCount;
				sphereVerts.push_back(s);
				sphereVerts.push_back(t);

			}
		}

		int k1, k2;
		for (int i = 0; i < stackCount; ++i)
		{
			k1 = i * (sectorCount + 1);
			k2 = k1 + sectorCount + 1;

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				if (i != 0)
				{
					sphereIndices.push_back(k1);
					sphereIndices.push_back(k2);
					sphereIndices.push_back(k1 + 1);
				}
				if (i != (stackCount - 1))
				{
					sphereIndices.push_back(k1 + 1);
					sphereIndices.push_back(k2);
					sphereIndices.push_back(k2 + 1);
				}
			}
		}
		//Generate the VAO, VBO, & EBO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		//Bind buffers
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, (unsigned int)sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_DYNAMIC_DRAW);

		//Bind attribute pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	//Called to draw sphere
	void Draw()
	{
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (unsigned int)sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
};


#endif
