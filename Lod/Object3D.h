#pragma once
#include "Mesh.h"
#include "GPUBuffer.h"
#include "ShaderMaterial.h"
#include "framework.h"
#include "ThreadPool/Task.h"
#include "Uniform/Object3DUniformBlock.h"
#include "Texture/ColorMap.h"
#include "Texture/ObjectSpaceNormalMap.h"
//#include "Octree.h"

class Object3D {
private:
	Object3DUniformBlock* uniform_block;

	// Uniforms
	mat4 modelMatrix = IdentityMatrix();
	vec3 drawColor = vec3(1.0, 1.0, 1.0);
	bool useTrueColor = false;

	bool      useColorTexture = false;
	ColorMap* colorTexture;

	bool                  useObjectSpaceNormalTexture = false;
	ObjectSpaceNormalMap* objectSpaceNormalTexture;

	// Mesh
	Mesh* original_mesh = nullptr;
	Mesh* current_mesh = nullptr;

	// Lod stuff
	float last_lod_distance = 0.0f;
	float min_lod_distance = 10.0f;
	float lod_distance_step = 1.0f;
	Task<Mesh>* lod_task = nullptr;

	float distance_from_camera = 0.0f;

	GPUBuffer buffer;
	ShaderMaterial shader_material;

	bool wireFrameEnabled = false;

public:
	Object3D() {
		uniform_block = new Object3DUniformBlock();
		uniform_block->SetModelMatrix(modelMatrix);
		uniform_block->SetModelMatrixInverse(Invert(modelMatrix));
		uniform_block->SetDrawColor(drawColor);
		uniform_block->SetUseTrueColor(useTrueColor);
	}

	~Object3D() {
		delete uniform_block;
	}

	void SetMesh(Mesh* mesh) {
		this->current_mesh = mesh;
		if (mesh) {
			this->buffer.Fill(mesh);
		}
	}

	void SetOriginalMesh(Mesh* mesh) {
		original_mesh = mesh;
		if (!current_mesh)
			SetMesh(mesh);
	}

	void UpdateLod() {
		if (distance_from_camera < min_lod_distance) {
			if (current_mesh != original_mesh) {
				delete current_mesh;
				this->SetMesh(original_mesh);
			}

			if (lod_task && lod_task->Finished()) {
				delete lod_task->Result();
				delete lod_task;
				lod_task = nullptr;
			}
			return;
		}

		if (lod_task) {
			if (!lod_task->Finished()) return;

			Mesh* old_mesh = current_mesh;
			this->SetMesh(lod_task->Result());
			if (old_mesh != original_mesh)
				delete old_mesh;
			delete lod_task;
			lod_task = nullptr;
		}

		if (last_lod_distance + lod_distance_step < distance_from_camera || last_lod_distance - lod_distance_step > distance_from_camera) {
			last_lod_distance = distance_from_camera;
			//lod_task = Octree::SimplifyMeshAsync(original_mesh, ALG_QEF, (last_lod_distance) * 0.003f);
		}
	}

	Mesh* GetMesh() {
		return current_mesh;
	}

	void EnableWireframe() {
		wireFrameEnabled = true;
	}

	void DisableWireframe() {
		wireFrameEnabled = false;
	}

	void SwitchWireframe() {
		wireFrameEnabled = !wireFrameEnabled;
	}

	void SetDrawColor(const vec3& color) {
		drawColor = color;
		uniform_block->SetDrawColor(color);
	}

	void ToggleTrueColor() {
		useTrueColor = !useTrueColor;
		uniform_block->SetUseTrueColor(useTrueColor);
	}

	void AttachColorMap(ColorMap* colorMap) {
		if (!colorMap) {
			return;
		}

		this->colorTexture = colorMap;
		useColorTexture = true;
		uniform_block->SetUseColorTexture(true);
	}

	void DetachColorMap() {
		this->colorTexture = nullptr;
		useColorTexture = false;
		uniform_block->SetUseColorTexture(false);
	}

	void AttachObjectSpaceNormalMap(ObjectSpaceNormalMap* objectSpaceNormalMap) {
		if (!objectSpaceNormalMap) {
			return;
		}

		this->objectSpaceNormalTexture = objectSpaceNormalMap;
		useObjectSpaceNormalTexture = true;
		uniform_block->SetUseObjectSpaceNormalTexture(true);
	}

	void DetachObjectSpaceNormalMap() {
		this->objectSpaceNormalTexture = nullptr;
		useObjectSpaceNormalTexture = false;
		uniform_block->SetUseObjectSpaceNormalTexture(false);
	}

	void SetShader(Shader* shader) {
		this->shader_material.SetShader(shader);
	}

	void UpdateMesh(const vec3& camera_pos) {
		if (current_mesh) {
			current_mesh->Update(distance_from_camera);
		}
	}

	void Draw() {

		if (current_mesh->GetUpdated()) {
			buffer.Fill(current_mesh);
			current_mesh->SetUpdated(false);
		}

		uniform_block->Bind();
		shader_material.Activate();

		if (useColorTexture) {
			colorTexture->Bind();
		}

		if (useObjectSpaceNormalTexture) {
			objectSpaceNormalTexture->Bind();
		}

		if (current_mesh) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_POLYGON_OFFSET_FILL);
			this->buffer.Draw();

			if (wireFrameEnabled) {
				uniform_block->SetUseTrueColor(false);
				uniform_block->SetDrawColor(vec3(0.0, 0.0, 0.0));
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDisable(GL_POLYGON_OFFSET_FILL);
					this->buffer.Draw();
				}
				uniform_block->SetUseTrueColor(useTrueColor);
				uniform_block->SetDrawColor(drawColor);
			}

		}
	}

	ShaderMaterial& Material() {
		return this->shader_material;
	}
};