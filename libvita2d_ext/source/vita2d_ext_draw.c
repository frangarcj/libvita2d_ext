#include <psp2/kernel/sysmem.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vita2d.h>
#include "vita2d_ext.h"
#include "utils.h"
#include "shared.h"

void vita2d_set_shader(const vita2d_shader *shader)
{
	sceGxmSetVertexProgram(_vita2d_ext_context, shader->vertexProgram);
	sceGxmSetFragmentProgram(_vita2d_ext_context, shader->fragmentProgram);
}


void vita2d_set_wvp_uniform(const vita2d_shader *shader, float *matrix)
{
	void *vertex_wvp_buffer;
	sceGxmReserveVertexDefaultUniformBuffer(_vita2d_ext_context, &vertex_wvp_buffer);
	sceGxmSetUniformDataF(vertex_wvp_buffer, shader->wvpParam, 0, 16, matrix);
}

void vita2d_set_vertex_uniform(const vita2d_shader *shader, const char * param, const float *value, unsigned int length)
{
	void *vertex_wvp_buffer;
	const SceGxmProgram *program = sceGxmVertexProgramGetProgram(shader->vertexProgram);
	const SceGxmProgramParameter *program_parameter = sceGxmProgramFindParameterByName(program, param);
	sceGxmReserveVertexDefaultUniformBuffer(_vita2d_ext_context, &vertex_wvp_buffer);
	sceGxmSetUniformDataF(vertex_wvp_buffer, program_parameter, 0, length, value);
}

void vita2d_set_fragment_uniform(const vita2d_shader *shader, const char * param, const float *value, unsigned int length)
{
	void *fragment_wvp_buffer;
	const SceGxmProgram *program = sceGxmFragmentProgramGetProgram(shader->fragmentProgram);
	const SceGxmProgramParameter *program_parameter = sceGxmProgramFindParameterByName(program, param);
	sceGxmReserveFragmentDefaultUniformBuffer(_vita2d_ext_context, &fragment_wvp_buffer);
	sceGxmSetUniformDataF(fragment_wvp_buffer, program_parameter, 0, length, value);
}

void vita2d_draw_texture_part_scale_rotate_generic(const vita2d_texture *texture, float x, float y,
	float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad)
{
	vita2d_texture_vertex *vertices = (vita2d_texture_vertex *)vita2d_pool_memalign(
		4 * sizeof(vita2d_texture_vertex), // 4 vertices
		sizeof(vita2d_texture_vertex));

	uint16_t *indices = (uint16_t *)vita2d_pool_memalign(
		4 * sizeof(uint16_t), // 4 indices
		sizeof(uint16_t));

	const float w_full = vita2d_texture_get_width(texture);
	const float h_full = vita2d_texture_get_height(texture);

	const float w_half = (tex_w * x_scale) / 2.0f;
	const float h_half = (tex_h * y_scale) / 2.0f;

	const float u0 = tex_x / w_full;
	const float v0 = tex_y / h_full;
	const float u1 = (tex_x + tex_w) / w_full;
	const float v1 = (tex_y + tex_h) / h_full;

	vertices[0].x = -w_half;
	vertices[0].y = -h_half;
	vertices[0].z = +0.5f;
	vertices[0].u = u0;
	vertices[0].v = v0;

	vertices[1].x = w_half;
	vertices[1].y = -h_half;
	vertices[1].z = +0.5f;
	vertices[1].u = u1;
	vertices[1].v = v0;

	vertices[2].x = -w_half;
	vertices[2].y = h_half;
	vertices[2].z = +0.5f;
	vertices[2].u = u0;
	vertices[2].v = v1;

	vertices[3].x = w_half;
	vertices[3].y = h_half;
	vertices[3].z = +0.5f;
	vertices[3].u = u1;
	vertices[3].v = v1;

	const float c = cosf(rad);
	const float s = sinf(rad);
	int i;
	for (i = 0; i < 4; ++i) { // Rotate and translate
		float _x = vertices[i].x;
		float _y = vertices[i].y;
		vertices[i].x = _x*c - _y*s + x;
		vertices[i].y = _x*s + _y*c + y;
	}

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;

	// Set the texture to the TEXUNIT0
	sceGxmSetFragmentTexture(_vita2d_ext_context, 0, &texture->gxm_tex);

	sceGxmSetVertexStream(_vita2d_ext_context, 0, vertices);
	sceGxmDraw(_vita2d_ext_context, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, indices, 4);
}
