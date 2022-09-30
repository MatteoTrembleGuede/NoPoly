#pragma once
#include "Texture.h"
class RenderTexture :
    public Texture
{

public:

    RenderTexture(float texWidth, float texHeight);
    ~RenderTexture();

    void SetSize(float _width, float _height);
    void BindAsTarget();
    void Unbind();
    void Clear();
    void UpdateBuffer() override;

private:

    unsigned int glFBO, glRBO;

};

