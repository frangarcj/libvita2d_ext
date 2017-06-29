#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/message_dialog.h>
#include <psp2/sysmodule.h>
#include <string.h>
#include <stdlib.h>
#include <vita2d.h>
#include <vita2d_ext.h>
#include "utils.h"

#ifdef DEBUG_BUILD
#  include <stdio.h>
#  define DEBUG(...) printf(__VA_ARGS__)
#else
#  define DEBUG(...)
#endif

#define MSAA_MODE			SCE_GXM_MULTISAMPLE_NONE

SceGxmShaderPatcher *_vita2d_ext_shader_patcher = NULL;
SceGxmContext *_vita2d_ext_context = NULL;

int vita2d_ext_init(SceGxmContext *context, SceGxmShaderPatcher *shaderPatcher)
{
	if(!(context && shaderPatcher))
		return -1;
	_vita2d_ext_context = context;
	_vita2d_ext_shader_patcher = shaderPatcher;
	return 1;
}

int vita2d_ext_fini(SceGxmShaderPatcher *shaderPatcher)
{
	_vita2d_ext_context = NULL;
	_vita2d_ext_shader_patcher = NULL;
	return 1;
}

vita2d_shader *vita2d_create_shader(const SceGxmProgram* vertexProgramGxp, const SceGxmProgram* fragmentProgramGxp)
{
	int err;
	UNUSED(err);

	vita2d_shader *shader = malloc(sizeof(*shader));

	err = sceGxmProgramCheck(vertexProgramGxp);
	DEBUG("texture_v sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(fragmentProgramGxp);
	DEBUG("texture_f sceGxmProgramCheck(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(_vita2d_ext_shader_patcher, vertexProgramGxp, &shader->vertexProgramId);
	DEBUG("texture_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);
	err = sceGxmShaderPatcherRegisterProgram(_vita2d_ext_shader_patcher, fragmentProgramGxp, &shader->fragmentProgramId);
	DEBUG("texture_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	shader->paramPositionAttribute = sceGxmProgramFindParameterByName(vertexProgramGxp, "aPosition");
	DEBUG("aPosition sceGxmProgramFindParameterByName(): %p\n", shader->paramPositionAttribute);

	shader->paramTexcoordAttribute = sceGxmProgramFindParameterByName(vertexProgramGxp, "aTexcoord");
	DEBUG("aTexcoord sceGxmProgramFindParameterByName(): %p\n", shader->paramTexcoordAttribute);

	// create texture vertex format
	SceGxmVertexAttribute textureVertexAttributes[2];
	SceGxmVertexStream textureVertexStreams[1];
	/* x,y,z: 3 float 32 bits */
	textureVertexAttributes[0].streamIndex = 0;
	textureVertexAttributes[0].offset = 0;
	textureVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	textureVertexAttributes[0].componentCount = 3; // (x, y, z)
	textureVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(shader->paramPositionAttribute);
	/* u,v: 2 floats 32 bits */
	textureVertexAttributes[1].streamIndex = 0;
	textureVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 11 bytes
	textureVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	textureVertexAttributes[1].componentCount = 2; // (u, v)
	textureVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(shader->paramTexcoordAttribute);
	// 16 bit (short) indices
	textureVertexStreams[0].stride = sizeof(vita2d_texture_vertex);
	textureVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// create texture shaders
	err = sceGxmShaderPatcherCreateVertexProgram(
	  _vita2d_ext_shader_patcher,
	  shader->vertexProgramId,
	  textureVertexAttributes,
	  2,
	  textureVertexStreams,
	  1,
	  &shader->vertexProgram);

	DEBUG("texture sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);

	// Fill SceGxmBlendInfo
	static const SceGxmBlendInfo blend_info = {
		.colorFunc = SCE_GXM_BLEND_FUNC_ADD,
		.alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
		.colorSrc  = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
		.colorDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.alphaSrc  = SCE_GXM_BLEND_FACTOR_ONE,
		.alphaDst  = SCE_GXM_BLEND_FACTOR_ZERO,
		.colorMask = SCE_GXM_COLOR_MASK_ALL
	};

	err = sceGxmShaderPatcherCreateFragmentProgram(
	  _vita2d_ext_shader_patcher,
	  shader->fragmentProgramId,
	  SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
	  MSAA_MODE,
	  &blend_info,
	  vertexProgramGxp,
	  &shader->fragmentProgram);

	DEBUG("texture sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

	shader->wvpParam = sceGxmProgramFindParameterByName(vertexProgramGxp, "wvp");
	DEBUG("texture wvp sceGxmProgramFindParameterByName(): %p\n", shader->wvpParam);

	return shader;
}

void vita2d_free_shader(vita2d_shader* shader)
{

	sceGxmShaderPatcherReleaseFragmentProgram(_vita2d_ext_shader_patcher, shader->fragmentProgram);
	sceGxmShaderPatcherReleaseVertexProgram(_vita2d_ext_shader_patcher, shader->vertexProgram);

	sceGxmShaderPatcherUnregisterProgram(_vita2d_ext_shader_patcher, shader->fragmentProgramId);
	sceGxmShaderPatcherUnregisterProgram(_vita2d_ext_shader_patcher, shader->vertexProgramId);

	free(shader);
}
