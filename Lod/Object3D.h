#pragma once
#include "Mesh.h"
#include "GPUBuffer.h"
#include "ShaderMaterial.h"
#include "framework.h"
#include "ThreadPool/Task.h"
#include "Octree.h"

class Object3D {
private:
	Mesh* original_mesh = nullptr;

	float last_lod_distance = 0.0f;
	float min_lod_distance = 10.0f;
	float lod_distance_step = 1.0f;
	Task<Mesh>* lod_task = nullptr;
	Mesh* current_mesh = nullptr;

	float distance_from_camera = 0.0f;

	GPUBuffer buffer;
	ShaderMaterial shader_material;

	bool wireFrameEnabled = false;
	bool trueColorEnabled = false;

	mat4 modelMatrix = IdentityMatrix();
	std::string modelMatrixUniformName;

public:
	Object3D() {}

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
			lod_task = Octree::SimplifyMeshAsync(original_mesh, ALG_QEF, (last_lod_distance) * 0.003f);
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

	void ToggleTrueColor() {
		trueColorEnabled = !trueColorEnabled;
		shader_material.SetUniform("useTrueColor", trueColorEnabled);
		shader_material.ApplyUniforms();
	}

	void SetShader(Shader* shader) {
		this->shader_material.SetShader(shader);
	}

	void ModelMatrixEnableAutoUpdate(const char* uniform_name) {
		modelMatrixUniformName = std::string(uniform_name);
		UpdateModelMatrixUniform();
	}

	void UpdateModelMatrixUniform() {
		if (!modelMatrixUniformName.empty()) {
			shader_material.SetUniform(modelMatrixUniformName.c_str(), modelMatrix);
		}
	}

	void UpdateMesh(const vec3& camera_pos) {
		if (current_mesh) {
			current_mesh->Update(distance_from_camera);
		}
	}

	void UpdateCameraPosition(vec3& camera_pos) {
		shader_material.SetUniform("cameraPosition", camera_pos);
	}

	void UpdateModelMVP(mat4& view_mat, mat4& project_mat) {
		mat4 MVP = project_mat * view_mat * modelMatrix;
		vec4 pos = (vec4(0, 0, 0, 1) * MVP);
		distance_from_camera = pos.w;
		printf("Camera distance: %f\n", distance_from_camera);
		shader_material.SetUniform("MVP", (project_mat * view_mat * modelMatrix));
		shader_material.SetUniform("modelMatrix", modelMatrix);
		shader_material.SetUniform("modelMatrixInverse", Invert(modelMatrix));
		shader_material.SetUniform("viewMatrix", view_mat);
		shader_material.SetUniform("projMatrix", project_mat);
		//printf("%f\n", distance_from_camera);
		////shader_material.SetUniform("MVP", (modelMatrix * view_mat * project_mat));
	}

	void Draw() {
		if (current_mesh->GetUpdated()) {
			buffer.Fill(current_mesh);
			current_mesh->SetUpdated(false);
		}

		shader_material.Activate();

		if (current_mesh) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_POLYGON_OFFSET_FILL);
			this->buffer.Draw();

			if (wireFrameEnabled) {
				shader_material.SetUniform("useTrueColor", false);
				vec3 old_color = shader_material.GetUniformField()->GetUniformVec3("drawColor");
				shader_material.SetUniform("drawColor", vec3(0.0, 0.0, 0.0));
				shader_material.ApplyUniforms();

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDisable(GL_POLYGON_OFFSET_FILL);
				this->buffer.Draw();

				shader_material.SetUniform("useTrueColor", trueColorEnabled);
				shader_material.SetUniform("drawColor", old_color);
				shader_material.ApplyUniforms();
			}

		}
	}

	ShaderMaterial& Material() {
		return this->shader_material;
	}
};