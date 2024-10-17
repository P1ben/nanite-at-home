#pragma once
#include <cstdint>
#include <glew.h>
#include <iostream>

#include "../Texture/Texture.h"
#include "../Shader.h"
#include "../Uniform/ToolsUniformBlock.h"

class Framebuffer {
private:
	uint32_t fbo_id;

	uint32_t rbo_id;
	Texture* color_tex = nullptr;

	uint32_t size_x;
	uint32_t size_y;

public:
	Framebuffer(uint32_t size_x, uint32_t size_y) {
		this->size_x = size_x;
		this->size_y = size_y;

		// Create framebuffer object
		glGenFramebuffers(1, &fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

		// Create color texture
		color_tex = new Texture(size_x, size_y);

		// Attach color texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex->GetID(), 0);

		// Create renderbuffer object for depth and stencil attachment
		glGenRenderbuffers(1, &rbo_id);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size_x, size_y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Attach renderbuffer to framebuffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_id);

		// Check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "SUCCESS::FRAMEBUFFER:: Framebuffer is complete!" << std::endl;
		}
		else {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}

		// Unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	~Framebuffer() {
		glDeleteFramebuffers(1, &fbo_id);
		glDeleteRenderbuffers(1, &rbo_id);
		delete color_tex;
	}

	void Use() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
		glViewport(0, 0, size_x, size_y);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	static void RenderOntoImage(Scene* scene, const char* file_path, uint32_t img_w, uint32_t img_h) {
		Framebuffer fb = Framebuffer(img_w, img_h);
		fb.Use();

		scene->Draw();

		fb.Save(file_path, true);
		Framebuffer::UseDefault();
	}

	static void CreateObjNormalMap(Object3D* object, const char* file_path) {
		Shader shader = Shader("shaders/obj-space-normal-vx.glsl", "shaders/obj-space-normal-fg.glsl");
		//Shader shader = Shader("shaders/maxblinn-vx.glsl", "shaders/maxblinn-fg.glsl");
		Shader* remember = object->Material().GetShader();
		object->SetShader(&shader);

		Framebuffer fb = Framebuffer(1024, 1024);
		fb.Use();

		object->Draw();

		object->SetShader(remember);
		fb.Save(file_path, true);
		Framebuffer::UseDefault();
	}

	static void CreateObjNormalMap(Mesh* mesh, const char* file_path) {
		Shader shader = Shader("shaders/obj-space-normal-vx.glsl", "shaders/obj-space-normal-fg.glsl");
		//Shader shader = Shader("shaders/maxblinn-vx.glsl", "shaders/maxblinn-fg.glsl");
		Object3D temp_obj = Object3D();
		temp_obj.SetOriginalMesh(mesh);
		mesh->Update(.0f, nullptr, nullptr);
		temp_obj.SetShader(&shader);

		Framebuffer fb = Framebuffer(2048, 2048);
		fb.Use();

		const float offset_amount = 0.002f;
		const int   no_passes = 8;

		ToolsUniformBlock tools_ubo = ToolsUniformBlock();
		tools_ubo.Bind();

		for (int i = 0; i < no_passes; i++) {
			float angle = 2.0f * M_PI * i / no_passes;
			tools_ubo.SetUVXOffset(offset_amount * cos(angle));
			tools_ubo.SetUVYOffset(offset_amount * sin(angle));
			temp_obj.Draw();
		}

		tools_ubo.SetUVXOffset(.0f);
		tools_ubo.SetUVYOffset(.0f);
		temp_obj.Draw();

		fb.Save(file_path, true);
		Framebuffer::UseDefault();
	}

	static void RenderUVMap(Object3D* object, const char* file_path) {
		Shader shader = Shader("shaders/uv-map-render-vx.glsl", "shaders/uv-map-render-fg.glsl");
		//Shader shader = Shader("shaders/maxblinn-vx.glsl", "shaders/maxblinn-fg.glsl");
		Shader* remember = object->Material().GetShader();
		vec3 orig_draw_color = object->GetDrawColor();

		object->SetShader(&shader);
		object->SetDrawColor(vec3(0.3f, 0.3f, 0.3f));
		object->SetWireframe(true);

		Framebuffer fb = Framebuffer(2048, 2048);
		fb.Use();

		object->Draw();

		object->SetShader(remember);
		object->SetDrawColor(orig_draw_color);
		object->SetWireframe(false);

		fb.Save(file_path, true);
		Framebuffer::UseDefault();
	}

	static void RenderUVMap(Mesh* mesh, const char* file_path) {
		Shader shader = Shader("shaders/uv-map-render-vx.glsl", "shaders/uv-map-render-fg.glsl");
		//Shader shader = Shader("shaders/maxblinn-vx.glsl", "shaders/maxblinn-fg.glsl");
		Object3D temp_obj = Object3D();
		temp_obj.SetOriginalMesh(mesh);
		temp_obj.SetShader(&shader);
		temp_obj.SetDrawColor(vec3(0.3f, 0.3f, 0.3f));
		temp_obj.SetWireframe(true);

		Framebuffer fb = Framebuffer(1024, 1024);
		fb.Use();

		temp_obj.Draw();

		fb.Save(file_path, true);
		Framebuffer::UseDefault();
	}

	void Save(const char* file_name, bool flipped_vertically = false) {
		color_tex->SaveAsJPG(file_name, flipped_vertically);
	}

	static void UseDefault() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// TODO: Change to window size
		glViewport(0, 0, 1280, 720);
	}
};