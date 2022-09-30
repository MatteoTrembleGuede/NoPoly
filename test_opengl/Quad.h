#pragma once
#include "Vector3.h"

enum QuadVertId
{
    BotLeft,
    TopLeft,
    BotRight,
    TopRight
};

typedef struct Vertex
{
    Vector3 position;
    float u;
    float v;
} Vertex;

class Quad
{
public:

    Quad();
    ~Quad();
    void Draw();
    Vertex* GetVertex(QuadVertId ID);
    void UpdateGlSelf();

protected:

    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    float* vertices;
    float* uvs;
    int* indices;
    void InitGlSelf();
};

