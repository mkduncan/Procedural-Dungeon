#ifndef Header_H
#define Header_H

#ifdef NDEBUG
#define DEBUG_MODE false
#else
#define DEBUG_MODE true
#endif

#include "AlignedBox.hpp"
#include "AlignedBoxTree.hpp"
#include "Camera.hpp"
#include "Decoder.hpp"
#include "Dungeon.hpp"
#include "Frustum.hpp"
#include "Light.hpp"
#include "Logger.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "MeshObject.hpp"
#include "Model.hpp"
#include "Plane.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "StaticMap.hpp"
#include "StaticShader.hpp"
#include "Texture.hpp"
#include "Vertex.hpp"
#include "Window.hpp"

#endif