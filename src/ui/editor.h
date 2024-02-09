#pragma once
#include "utils/camera.h"
#include "renderer/base_renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_stdlib.h"
#include "ImGuizmo/ImGuizmo.h"

class BaseRenderer;

class SceneEditor {
public:
	SceneEditor() = default;

	void render(Camera& camera);
	void renderAsList(Model& model);
	void renderDebug(Camera& camera);

	BaseRenderer* renderer = nullptr;
	std::vector<Model> *objs = nullptr;
	Mesh* chosenObj = nullptr;
	Material* chosenMaterial = nullptr;

	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

private:
};