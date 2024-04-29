#pragma once

// ---------- STD ----------
#include <numbers>
#include <bit>

#include <fstream>
#include <sstream>
#include <iostream>

#include <chrono>
#include <cassert>
#include <functional>

#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cstring>
#include <memory>
#include <span>

// ---------- GLM ----------
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>

// ---------- STB ----------
#include <stb_image.h>
#include <stb_perlin.h>

// ---------- ASSIMP ----------
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// ---------- IMGUI ----------
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// ---------- GLFW3 ----------
#include <glad/glad.h>
#include <GLFW/glfw3.h>