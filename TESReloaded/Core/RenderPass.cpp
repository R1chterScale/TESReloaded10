#include "RenderPass.h"


void RenderPass::RenderAccum() {
	if (GeometryList.empty()) return;

	// Could add setup of device/renderstate/current shaders here
	NiDX9RenderState* RenderState = TheRenderManager->renderState;
	RenderState->SetVertexShader(VertexShader->ShaderHandle, false);
	RenderState->SetPixelShader(PixelShader->ShaderHandle, false);

	// Render normal geometry
	while (!GeometryList.empty()) {
		NiGeometry* Geo = GeometryList.top();
		if (!Geo) continue;

		TheRenderManager->CreateD3DMatrix(&TheShaderManager->ShaderConst.ShadowWorld, &Geo->m_worldTransform);

		UpdateConstants(Geo);
		PixelShader->SetCT();
		VertexShader->SetCT();

		RenderGeometry(Geo);
		GeometryList.pop();
	}
}


void RenderPass::RenderGeometry(NiGeometry* Geo) {
	NiGeometryData* ModelData = Geo->geomData;
	NiGeometryBufferData* GeoData = ModelData->BuffData;
	NiD3DShaderDeclaration* ShaderDeclaration = Geo->shader->ShaderDeclaration;

	TheRenderManager->PackGeometryBuffer(GeoData, ModelData, NULL, ShaderDeclaration);
	if (GeoData && GeoData->VertCount) DrawGeometryBuffer(GeoData, GeoData->VertCount);
}


// draws the geo data from the Geometry buffer
void RenderPass::DrawGeometryBuffer(NiGeometryBufferData* GeoData, UINT verticesCount) {
	int StartIndex = 0;
	int PrimitiveCount = 0;

	for (UInt32 i = 0; i < GeoData->StreamCount; i++) {
		TheRenderManager->device->SetStreamSource(i, GeoData->VBChip[i]->VB, 0, GeoData->VertexStride[i]);
	}
	TheRenderManager->device->SetIndices(GeoData->IB);
	if (GeoData->FVF)
		TheRenderManager->renderState->SetFVF(GeoData->FVF, false);
	else
		TheRenderManager->renderState->SetVertexDeclaration(GeoData->VertexDeclaration, false);

	for (UInt32 i = 0; i < GeoData->NumArrays; i++) {
		PrimitiveCount = GeoData->ArrayLengths?GeoData->ArrayLengths[i] - 2 : GeoData->TriCount;

		TheRenderManager->device->DrawIndexedPrimitive(GeoData->PrimitiveType, GeoData->BaseVertexIndex, 0, verticesCount, StartIndex, PrimitiveCount);
		StartIndex += PrimitiveCount + 2;
	}
}


ShadowRenderPass::ShadowRenderPass() {
	PixelShader = TheShadowManager->ShadowMapPixel;
	VertexShader = TheShadowManager->ShadowMapVertex;
	RegisterConstants();
}


bool ShadowRenderPass::AccumObject(NiGeometry* Geo) {
	if (!Geo->geomData->BuffData) return false; // discard objects without buffer data

	BSShaderProperty* ShaderProperty = (BSShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
	if (!ShaderProperty || !ShaderProperty->IsLightingProperty()) return false;

	GeometryList.push(Geo);
	return true;
}


void ShadowRenderPass::RegisterConstants() {
	TheShaderManager->RegisterConstant("TESR_ShadowWorldTransform", (D3DXVECTOR4*)&TheShaderManager->ShaderConst.ShadowWorld);
}


void ShadowRenderPass::UpdateConstants(NiGeometry* Geo) {
	ShadowsExteriorEffect::ShadowStruct* Constants = &TheShaderManager->Effects.ShadowsExteriors->Constants;
	Constants->Data.x = 0.0f; // Type of geo (0 normal, 1 actors (skinned), 2 speedtree leaves)
	Constants->Data.y = 0.0f; // Alpha Control

	BSShaderProperty* ShaderProperty = (BSShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
	NiAlphaProperty* AProp = (NiAlphaProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Alpha);
	if (AProp->flags & NiAlphaProperty::AlphaFlags::ALPHA_BLEND_MASK || AProp->flags & NiAlphaProperty::AlphaFlags::TEST_ENABLE_MASK) {
		if (NiTexture* Texture = *((BSShaderPPLightingProperty*)ShaderProperty)->ppTextures[0]) {

			Constants->Data.y = 1.0f; // Alpha Control

			// Set diffuse texture at register 0
			NiDX9RenderState* RenderState = TheRenderManager->renderState;
			RenderState->SetTexture(0, Texture->rendererData->dTexture);
			RenderState->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP, false);
			RenderState->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP, false);
			RenderState->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, false);
			RenderState->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, false);
			RenderState->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT, false);
		}
	}
}


SkinnedGeoShadowRenderPass::SkinnedGeoShadowRenderPass() {
	PixelShader = TheShadowManager->ShadowMapPixel;
	VertexShader = TheShadowManager->ShadowMapVertex;
	RegisterConstants();
}


bool SkinnedGeoShadowRenderPass::AccumObject(NiGeometry* Geo) {
	NiShadeProperty* shaderProp = static_cast<NiShadeProperty*>(Geo->GetProperty(NiProperty::kType_Shade));

	// check data for rigged geometry
	if (Geo->skinInstance &&
		Geo->skinInstance->SkinPartition &&
		Geo->skinInstance->SkinPartition->Partitions) {

		// only accum if valid data preset
		if (Geo->skinInstance->SkinPartition->Partitions[0].BuffData) GeometryList.push(Geo);
	
		// we return true in any case because we still found skinned geo either way
		return true;
	}

	return false;
}


void SkinnedGeoShadowRenderPass::RegisterConstants() {
	TheShaderManager->RegisterConstant("TESR_BoneMatrices", NULL);
}


void SkinnedGeoShadowRenderPass::UpdateConstants(NiGeometry* Geo) {
	// Render skinned geometry
	ShadowsExteriorEffect::ShadowStruct* Constants = &TheShaderManager->Effects.ShadowsExteriors->Constants;
	Constants->Data.x = 1.0f; // Type of geo (0 normal, 1 actors (skinned), 2 speedtree leaves)
	Constants->Data.y = 0.0f; // Alpha control
}


void SkinnedGeoShadowRenderPass::RenderGeometry(NiGeometry* Geo) {
	NiGeometryData* ModelData = Geo->geomData;
	NiGeometryBufferData* GeoData = ModelData->BuffData;
	NiSkinInstance* SkinInstance = Geo->skinInstance;
	NiD3DShaderDeclaration* ShaderDeclaration = Geo->shader->ShaderDeclaration;

	NiSkinPartition* SkinPartition = SkinInstance->SkinPartition;
	D3DPRIMITIVETYPE PrimitiveType = (SkinPartition->Partitions[0].Strips == 0) ? D3DPT_TRIANGLELIST : D3DPT_TRIANGLESTRIP;
	TheRenderManager->CalculateBoneMatrixes(SkinInstance, &Geo->m_worldTransform);
	
	if (SkinInstance->SkinToWorldWorldToSkin) 
		memcpy(&TheShaderManager->Effects.ShadowsExteriors->Constants.ShadowWorld, SkinInstance->SkinToWorldWorldToSkin, 0x40);

	for (UInt32 p = 0; p < SkinPartition->PartitionsCount; p++) {
		if (!SkinInstance->IsPartitionEnabled(p)) continue;

		NiSkinPartition::Partition* Partition = &SkinPartition->Partitions[p];
		GeoData = Partition->BuffData;

		int StartRegister = 9;
		for (int i = 0; i < Partition->Bones; i++) {
			UInt16 NewIndex = (Partition->pBones == NULL) ? i : Partition->pBones[i];
			TheRenderManager->device->SetVertexShaderConstantF(StartRegister, ((float*)SkinInstance->BoneMatrixes) + (NewIndex * 3 * 4), 3);
			StartRegister += 3;
		}

		TheRenderManager->PackSkinnedGeometryBuffer(GeoData, ModelData, SkinInstance, Partition, ShaderDeclaration);
		DrawGeometryBuffer(GeoData, Partition->Vertices);
	}
}


SpeedTreeShadowRenderPass::SpeedTreeShadowRenderPass() {
	PixelShader = TheShadowManager->ShadowMapPixel;
	VertexShader = TheShadowManager->ShadowMapVertex;
	RegisterConstants();
}


void SpeedTreeShadowRenderPass::RegisterConstants() {
	TheShaderManager->RegisterConstant("TESR_BillboardRight", NULL);
	TheShaderManager->RegisterConstant("TESR_BillboardLeft", NULL);
	TheShaderManager->RegisterConstant("TESR_RockParams", NULL);
	TheShaderManager->RegisterConstant("TESR_RustleParams", NULL);
	TheShaderManager->RegisterConstant("TESR_WindMatrixes", NULL);
	TheShaderManager->RegisterConstant("TESR_LeafBase", NULL);
}


bool SpeedTreeShadowRenderPass::AccumObject(NiGeometry* Geo) {

	NiShadeProperty* shaderProp = static_cast<NiShadeProperty*>(Geo->GetProperty(NiProperty::kType_Shade));
	if (shaderProp->type != NiShadeProperty::kProp_SpeedTreeLeaf) return false;

	GeometryList.push(Geo);
	return true;
}


void SpeedTreeShadowRenderPass::UpdateConstants(NiGeometry* Geo) {

	ShadowsExteriorEffect::ShadowStruct* Constants = &TheShaderManager->Effects.ShadowsExteriors->Constants;
	IDirect3DDevice9* Device = TheRenderManager->device;
	NiDX9RenderState* RenderState = TheRenderManager->renderState;

	Constants->Data.x = 2.0f; // Type of geo (0 normal, 1 actors (skinned), 2 speedtree leaves)
	Constants->Data.y = 0.0f; // Alpha control

	// Bind constant values for leaf transformation
	Device->SetVertexShaderConstantF(63, (float*)&TheShadowManager->BillboardRight, 1);
	Device->SetVertexShaderConstantF(64, (float*)&TheShadowManager->BillboardUp, 1);
	Device->SetVertexShaderConstantF(65, Pointers::ShaderParams::RockParams, 1);
	Device->SetVertexShaderConstantF(66, Pointers::ShaderParams::RustleParams, 1);
	Device->SetVertexShaderConstantF(67, Pointers::ShaderParams::WindMatrixes, 16);

	SpeedTreeLeafShaderProperty* STProp = (SpeedTreeLeafShaderProperty*)Geo->GetProperty(NiProperty::PropertyType::kType_Shade);
	BSTreeNode* Node = (BSTreeNode*)Geo->m_parent->m_parent;
	NiDX9SourceTextureData* Texture = (NiDX9SourceTextureData*)Node->TreeModel->LeavesTexture->rendererData;

	// Bind constant values for leaf transformation
	Device->SetVertexShaderConstantF(83, STProp->leafData->leafBase, 48);

	// Set diffuse texture at register 0
	RenderState->SetTexture(0, Texture->dTexture);
	RenderState->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP, false);
	RenderState->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP, false);
	RenderState->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, false);
	RenderState->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, false);
	RenderState->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT, false);
}