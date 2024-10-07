#pragma once
#include "Mesh.h"
#include "GPUBuffer.h"
#include "ShaderMaterial.h"
#include "framework.h"
#include "ThreadPool/Task.h"
#include "Uniform/Object3DUniformBlock.h"
#include "Texture/ColorMap.h"
#include "Texture/ObjectSpaceNormalMap.h"
#include "Nanite/NaniteMesh.h"
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
//#include "Octree.h"

class Object3D {
private:
	Object3DUniformBlock* uniform_block;

	// Uniforms
	vec3  worldPosition = vec3(0.0, 0.0, 0.0);
	vec3  rotation      = vec3(0.0, 0.0, 0.0);
	float rotationAngle = 0.0f;
	vec3  scale         = vec3(1.0, 1.0, 1.0);

	mat4 modelMatrix = IdentityMatrix();
	vec3 drawColor = vec3(1.0, 1.0, 1.0);
	bool useTrueColor = false;

	bool      useColorTexture = false;
	ColorMap* colorTexture = nullptr;

	bool                  useObjectSpaceNormalTexture = false;
	ObjectSpaceNormalMap* objectSpaceNormalTexture = nullptr;

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

	Object3D(const Object3D& other) {
		this->worldPosition = other.worldPosition;
		this->rotation = other.rotation;
		this->rotationAngle = other.rotationAngle;
		this->scale = other.scale;
		this->modelMatrix = other.modelMatrix;
		this->drawColor = other.drawColor;
		this->useTrueColor = other.useTrueColor;
		this->useColorTexture = other.useColorTexture;
		this->colorTexture = other.colorTexture;
		this->useObjectSpaceNormalTexture = other.useObjectSpaceNormalTexture;
		this->objectSpaceNormalTexture = other.objectSpaceNormalTexture;
		this->last_lod_distance = other.last_lod_distance;
		this->min_lod_distance = other.min_lod_distance;
		this->lod_distance_step = other.lod_distance_step;
		this->distance_from_camera = other.distance_from_camera;
		this->shader_material = other.shader_material;
		this->wireFrameEnabled = other.wireFrameEnabled;
		uniform_block = new Object3DUniformBlock();
		RecalculateModelMatrix();
		uniform_block->SetDrawColor(drawColor);
		uniform_block->SetUseTrueColor(useTrueColor);
		uniform_block->SetUseObjectSpaceNormalTexture(useObjectSpaceNormalTexture);
		uniform_block->SetUseColorTexture(useColorTexture);
		SetOriginalMesh(other.current_mesh);
	}

	~Object3D() {
		delete uniform_block;
	}

	static Object3D* LoadNaniteObject(const std::string& folder_path, bool texture_flipped = false) {
		Object3D* object = new Object3D();
		bool load_texture = false;
		std::string texture_path;

		bool load_normals = false;
		std::string normals_path;

		for (const auto& entry : std::experimental::filesystem::directory_iterator(folder_path)) {
			std::string file = entry.path().filename().string();
			if (file.find("texture.") != std::string::npos) {
				load_texture = true;
				texture_path = entry.path().string();
			}
			else if (file.find("normals.") != std::string::npos) {
				load_normals = true;
				normals_path = entry.path().string();
			}
		}

		if (load_texture) {
			object->AttachColorMap(new ColorMap(texture_path.c_str(), texture_flipped));
		}

		if (load_normals) {
			object->AttachObjectSpaceNormalMap(new ObjectSpaceNormalMap(normals_path.c_str(), texture_flipped));
		}

		NaniteMesh* nanite_mesh = nullptr;

		nanite_mesh = new NaniteMesh(folder_path);

		nanite_mesh->SetChangeStepForClusters(10.2312423f);

		object->SetOriginalMesh(nanite_mesh);
		return object;
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

	uint32_t GetVertexCount() {
		return buffer.GetVertexCount();
	}

	uint32_t GetFaceCount() {
		return buffer.GetFaceCount();
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

	void SetModelMatrix(const mat4& modelMatrix) {
		this->modelMatrix = modelMatrix;
		uniform_block->SetModelMatrix(modelMatrix);
		vec4 new_model_position = vec4(worldPosition, 1.0f) * modelMatrix;
		vec3 new_world_position = vec3(new_model_position.x, new_model_position.y, new_model_position.z);

		worldPosition = new_world_position;
	}

	void RecalculateModelMatrix() {
		mat4 translation     = TranslateMatrix(worldPosition);
		mat4 rotation_matrix = RotationMatrix(rotationAngle, rotation);
		mat4 scaling         = ScaleMatrix(scale);

		modelMatrix = translation * rotation_matrix * scaling;
		uniform_block->SetModelMatrix(modelMatrix);
		uniform_block->SetModelMatrixInverse(Invert(modelMatrix));
	}

	void SetWorldPosition(const vec3& position) {
		worldPosition = position;
		RecalculateModelMatrix();
	}

	void SetRotation(const vec3& rotation_vector, float rotation) {
		this->rotation = rotation_vector;
		this->rotationAngle = rotation;
		RecalculateModelMatrix();
	}

	void SetScale(const vec3& scale) {
		this->scale = scale;
		RecalculateModelMatrix();
	}

	vec3& GetDrawColor() {
		return drawColor;
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
		float new_distance = length(camera_pos - worldPosition);
		if (current_mesh && new_distance != distance_from_camera) {
			current_mesh->Update(length(camera_pos - worldPosition), buffer.GetFaceBuffer());
			distance_from_camera = new_distance;
		}

		auto upd = current_mesh->GetUpdated();

		if (upd == VERTEX_FACE_UPDATE) {
			buffer.Fill(current_mesh);
			current_mesh->SetUpdated(NO_UPDATE);
		}
		else if (upd == FACE_UPDATE) {
			buffer.FillFacesOnly(current_mesh);
			current_mesh->SetUpdated(NO_UPDATE);
		}
	}

	// Only used when performing async loading
	void UpdateMeshNoRefill(const vec3& camera_pos) {
		float new_distance = length(camera_pos - worldPosition);
		if (current_mesh && new_distance != distance_from_camera) {
			current_mesh->Update(length(camera_pos - worldPosition), buffer.GetFaceBuffer());
			distance_from_camera = new_distance;
		}
	}

	// Only used when performing async loading
	void RefillBufferIfNeeded() {
		auto upd = current_mesh->GetUpdated();

		if (upd == VERTEX_FACE_UPDATE) {
			buffer.Fill(current_mesh);
			current_mesh->SetUpdated(NO_UPDATE);
		}
		else if (upd == FACE_UPDATE) {
			buffer.FillFacesOnly(current_mesh);
			current_mesh->SetUpdated(NO_UPDATE);
		}
	}

	void Draw() {
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
				uniform_block->SetUseColorTexture(false);
				uniform_block->SetDrawColor(vec3(0.0, 0.0, 0.0));
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDisable(GL_POLYGON_OFFSET_FILL);
					this->buffer.Draw();
				}
				uniform_block->SetUseColorTexture(useColorTexture);
				uniform_block->SetUseTrueColor(useTrueColor);
				uniform_block->SetDrawColor(drawColor);
			}

		}
	}

	ShaderMaterial& Material() {
		return this->shader_material;
	}
};