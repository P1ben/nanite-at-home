#pragma once
#include <cstdint>
#include <glew.h>
#include <iostream>

#include "Texture.h"
#include "../Shader.h"

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

	static void CreateObjNormalMap(Mesh* mesh, const char* file_path) {
		Shader shader = Shader("shaders/obj-space-normal-vx.glsl", "shaders/obj-space-normal-fg.glsl");
		Object3D temp_obj = Object3D();
		temp_obj.SetOriginalMesh(mesh);
		temp_obj.SetShader(&shader);

		Framebuffer fb = Framebuffer(1024, 1024);
		fb.Use();

		temp_obj.Draw();

		fb.Save(file_path);
		Framebuffer::UseDefault();
	}

	void Save(const char* file_name) {
		color_tex->SaveAsJPG(file_name);
	}

	static void UseDefault() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// TODO: Change to window size
		glViewport(0, 0, 1280, 720);
	}
};