#include "D3D10Renderer.h"

//Includes the addition D3D10 libraries
#include <D3D10.h>
#include <D3DX10.h>

struct Vertex{float x,y,z;
				floattu,tv;};

const char basicEffect[]=\
	"float4 VS( float4 Pos : POSITION ) : SV_POSITION"\
	"{"\
	"	return Pos;"\
	"}"\
	"float4 PS( float4 Pos : SV_POSITION ) : SV_Target"\
	"{"\
	"	return float4( 1.0f,1.0f,0.0f,1.0f);"\
	"}"\
	"technique10 Render"\
	"{"\
	"	pass P0"\
	"	{"\
	"		SetVertexShader(CompileShader( vs_4_0, VS() ) );"\
	"		SetGeometryShader( NULL );"\
	"		SetPixelShader( CompileShader( ps_4_0, PS() ) ) ;"\
	"	}"\
	"}";
const D3D10_INPUT_ELEMENT_DESC VerexLayout[] =
{
	{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D10_INPUT_PER_VERTEX_DATA,0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,12, D3D10_INPUT_PER_VERTEX_DATA,0 }, 


};

//Constructor
D3D10Renderer::D3D10Renderer()
{
	m_pD3D10Device=NULL;
	m_pRenderTargetView=NULL;
	m_pSwapChain=NULL;
	m_pDepthStencelView=NULL;
	m_pDepthStencilTexture=NULL;
	m_pTempEffect=NULL;
	m_pTempTechnique=NULL;
	m_pTempBuffer=NULL;
	m_pTempVertexLayout=NULL;
	m_View = XMMatrixIdentity();
	m_Projection=XMMatrixIdentity();
	m_World =XMMatrixIdentity();
}

//Deconstructor
D3D10Renderer::~D3D10Renderer()
{
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	//Decrements the reference count for each interface.
	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if (m_pDepthStencelView)
		m_pDepthStencelView->Release();
	if (m_pDepthStencilTexture)
		m_pDepthStencilTexture->Release();
	if (m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pD3D10Device)
		m_pD3D10Device->Release();
	if(m_pTempBuffer)
		m_pTempBuffer->Release();
	if(m_pTempEffect)
		m_pTempEffect->Release();
	if(m_pTempVertexLayout)
		m_pTempVertexLayout->Release();
}


bool D3D10Renderer::init(void *pWindowHandle,bool fullScreen)
{
	HWND window=(HWND)pWindowHandle;
	RECT windowRect;
	GetClientRect(window,&windowRect);

	UINT width=windowRect.right-windowRect.left;
	UINT height=windowRect.bottom-windowRect.top;

	if (!createDevice(window,width,height,fullScreen))
		return false;
	if (!createInitialRenderTarget(width,height))
		return false;
	if (!loadEffectFromFile("Effects/transform.fx"))
		return false;
	createBuffer();
	
	createVertexLayout();
	
	XMFLOAT3 cameraPos=XMFLOAT3(0.0f,0.0f,-10.0f);
	XMFLOAT3 focusPos=XMFLOAT3(0.0f,0.0f,0.0f);
	XMFLOAT3 up=XMFLOAT3(0.0f,1.0f,0.0f);

	createCamera(XMLoadFloat3(&cameraPos),XMLoadFloat3(&focusPos),XMLoadFloat3(&up),XM_PI/4,(float)width/(float)height,0.1f,100.0f);
	positionObject(1.0f,1.0f,1.0f);
	return true;
}


bool D3D10Renderer::createDevice(HWND window,int windowWidth, int windowHeight,bool fullScreen)
{
	UINT createDeviceFlags=0;
#ifdef _DEBUG
	createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif

  /* Creates a DXGI_SWAP_CHAIN_DESC which describes the swap chain, its contained variables are:
  DXGI_MODE_DESC	 BufferDesc; 
  DXGI_SAMPLE_DESC	 SampleDesc; 
  DXGI_USAGE		 BufferUsage;
  UINT				 BufferCount;
  HWND				 OutputWindow;
  BOOL				 Windowed;
  DXGI_SWAP_EFFECT	 SwapEffect;
  UINT				 Flags;
  */

	DXGI_SWAP_CHAIN_DESC sd;
       ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	if (fullScreen)
		sd.BufferCount = 2;
	else 
		sd.BufferCount=1;
	sd.OutputWindow = window;
	sd.Windowed = (BOOL)(!fullScreen);
       sd.SampleDesc.Count = 1;
       sd.SampleDesc.Quality = 0;
       sd.BufferDesc.Width = windowWidth;
       sd.BufferDesc.Height = windowHeight;
       sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
       sd.BufferDesc.RefreshRate.Numerator = 60;
       sd.BufferDesc.RefreshRate.Denominator = 1;

	// Creates a D3D10 device and swap chain using sd for the swap chain
	if (FAILED(D3D10CreateDeviceAndSwapChain(NULL, 
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL, 
		createDeviceFlags,
		D3D10_SDK_VERSION,		
              &sd,
		&m_pSwapChain, 
		&m_pD3D10Device)))                       
		return false;


	return true;
}

bool D3D10Renderer::createInitialRenderTarget(int windowWidth, int windowHeight)
{
	ID3D10Texture2D *pBackBuffer;

	//ID3D10Texture2D the 2d texture struct
	//GetBuffer Gets the surface from the swap chain
	if (FAILED(m_pSwapChain->GetBuffer(0, 
		__uuidof(ID3D10Texture2D),
		(void**)&pBackBuffer))) 
		return false;

	//A D3D10_TEXTURE2D_DESC Descirbes the 2d Texture
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width=windowWidth;
	descDepth.Height=windowHeight;
	descDepth.MipLevels=1;
	descDepth.ArraySize=1;
	descDepth.Format=DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count=1;
	descDepth.SampleDesc.Quality=0;
	descDepth.Usage=D3D10_USAGE_DEFAULT;
	descDepth.BindFlags=D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags=0;
	descDepth.MiscFlags=0;

	//CreateTexture2D is the method used to create an array of 2D Textures
	if (FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,
			&m_pDepthStencilTexture)))
		return false;
	//D3D10_DEPTH_STENCIL_VIEW_DESC specifies the subresource from a texture that are accessible from a depth-stencil view.
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format=descDepth.Format;
	descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice=0;

	//Creates a depth stencil view to access the resource data
	if (FAILED(m_pD3D10Device->CreateDepthStencilView(m_pDepthStencilTexture,
                   &descDSV,&m_pDepthStencelView)))
		return false;


	if (FAILED(m_pD3D10Device->CreateRenderTargetView( pBackBuffer, 
		NULL, 
		&m_pRenderTargetView ))){
             pBackBuffer->Release();
		return  false;
	}
       pBackBuffer->Release();

	//OMSetRenderTargets binds the render targets to the output-merger stage
	m_pD3D10Device->OMSetRenderTargets(1, 
		&m_pRenderTargetView,		
		m_pDepthStencelView);

	//D3D10_VIEWPORT descirbes the viewport
	D3D10_VIEWPORT vp;
   	vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    
	//RSSetViewports binds the viewports to the rasterizer stage
	m_pD3D10Device->RSSetViewports( 1 
		, &vp );
	return true;
}

void D3D10Renderer::clear(float r,float g,float b,float a)
{
    // Just clear the backbuffer, colours start at 0.0 to 1.0
	// Red, Green , Blue, Alpha - BMD
    const float ClearColor[4] = { r, g, b, a}; 
	//ClearRenderTargetView clears the Render Target
    m_pD3D10Device->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	//ClearDepthStencilView clears the depth-stencil resource
	m_pD3D10Device->ClearDepthStencilView(m_pDepthStencelView,D3D10_CLEAR_DEPTH,1.0f,0);
}


void D3D10Renderer::present()
{
	//Present swaps the buffers in the chain
    m_pSwapChain->Present( 0, 0 );
}

void D3D10Renderer::createCamera(XMVECTOR &position,XMVECTOR &focus,XMVECTOR &up,float fov, float aspectRatio,float nearClip,float farClip)
{

	m_View =XMMatrixLookAtLH(position,focus,up);
	m_Projection = XMMatrixPerspectiveFovLH(fov,aspectRatio,nearClip,farClip);
}


void D3D10Renderer::render()
{
	m_pWorldEffectVarible->SetMatrix((float*)&m_World);
	m_pProjectionEffectVarible->SetMatrix((float*)&m_Projection);
	m_pViewEffectVarible->SetMatrix((float*)&m_View);
	//Tells the device what primitive we are going to use
	m_pD3D10Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	//Binds the created inputLayout to the input assembler stage
	m_pD3D10Device->IASetInputLayout(m_pTempVertexLayout);

	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	//Binds the vertex buffer to the input assembler stage
	m_pD3D10Device->IASetVertexBuffers(0,1,&m_pTempBuffer,&stride,&offset);

	D3D10_TECHNIQUE_DESC techniqueDesc;
	m_pTempTechnique->GetDesc(&techniqueDesc);

	for(unsigned int i=0; i<techniqueDesc.Passes;++i)
	{
		ID3D10EffectPass *pCurrentPass=m_pTempTechnique->GetPassByIndex(i);
		pCurrentPass->Apply(0);
		//Draws the primitives
		m_pD3D10Device->Draw(4,0);
	}
}
bool D3D10Renderer::createBuffer()
{
	//Square TRIANGLESTRIP, 4 vertex
	
	Vertex verts[]={
	{-0.5f,0.5f,0.0f,0.0f,0.0f},
	{0.5f,0.5f,0.0f,1.0f,0.0f},
	{-0.5f,-0.5f,0.0f,0.0f,1.0f},
	{0.5f,-0.5f,0.0f,1.0f,1.0f}
	};

	// Hexagon, TRIANGLELIST, 18 vertex
	/*Vertex verts[]={
	{0.0f,0.0f,0.0f},
	{0.25f,0.5f,0.0f},
	{0.5f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	{0.5f,0.0f,0.0f},
	{0.25f,-0.5f,0.0f},
	{0.0f,0.0f,0.0f},
	{0.25f,-0.5f,0.0f},
	{-0.25f,-0.5f,0.0f},
	{0.0f,0.0f,0.0f},
	{-0.25f,-0.5f,0.0f},
	{-0.5f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	{-0.5f,0.0f,0.0f},
	{-0.25f,0.5f,0.0f},
	{0.0f,0.0f,0.0f},
	{-0.25f,0.5f,0.0f},
	{0.25f,0.5f,0.0f},
	};*/
	
	
	// Hexagon TRIANGLESTRIP, 9 vertex
	/*
	Vertex verts[]={
	{0.25f,0.5f,0.0f},
	{0.5f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	{0.25f,-0.5f,0.0f},
	{-0.25f,-0.5f,0.0f},
	{-0.5f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	{-0.25f,0.5f,0.0f},
	{0.25f,0.5f,0.0f},
	};*/


	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( Vertex) * 4;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &verts;
	//Creates the vertex buffer from the vertex data
	if (FAILED(m_pD3D10Device->CreateBuffer(&bd,&InitData,&m_pTempBuffer)))
	{
		OutputDebugStringA("Can't create buffer");
		
	}
	return true;
}

bool D3D10Renderer::loadEffectFromMemory(const char* pMem)
{
		DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3D10_SHADER_DEBUG;
	#endif
	
	ID3D10Blob * pErrorBuffer=NULL;
	if(FAILED(D3DX10CreateEffectFromMemory(pMem,strlen(pMem),NULL,NULL,NULL,"fx_4_0",
		dwShaderFlags,0,m_pD3D10Device,NULL,NULL,&m_pTempEffect,&pErrorBuffer,NULL)))
	{
		OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
		return false;
	}

	m_pTempTechnique=m_pTempEffect->GetTechniqueByName("Render");
	return true;
}

bool D3D10Renderer::loadEffectFromFile(char* pFilename)
{
		DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3D10_SHADER_DEBUG;
	#endif
	
	ID3D10Blob * pErrorBuffer=NULL;
	if(FAILED(D3DX10CreateEffectFromFileA(pFilename,NULL,NULL,"fx_4_0",dwShaderFlags,0,m_pD3D10Device,NULL,NULL,&m_pTempEffect,&pErrorBuffer,NULL)))
	{
		OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
			return false;
	}
	m_pTempTechnique=m_pTempEffect->GetTechniqueByName("Render");
	m_pWorldEffectVarible=m_pTempEffect->GetVariableByName("matWorld")->AsMatrix();
	m_pViewEffectVarible=m_pTempEffect->GetVariableByName("matView")->AsMatrix();
	m_pProjectionEffectVarible=m_pTempEffect->GetVariableByName("matProjection")->AsMatrix();
	return true;
}


bool D3D10Renderer::createVertexLayout()
{
	UINT numElements = sizeof( VerexLayout ) / sizeof(D3D10_INPUT_ELEMENT_DESC);
	D3D10_PASS_DESC PassDesc;
	m_pTempTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	//Creates the inputlayout object with the verexlayout description
	if ( FAILED(m_pD3D10Device->CreateInputLayout( VerexLayout,numElements,PassDesc.pIAInputSignature,PassDesc.IAInputSignatureSize,&m_pTempVertexLayout)))
	{
		OutputDebugStringA("Can't create layout");
		return false;
	}
	return true;
}

void D3D10Renderer::positionObject(float x,float y, float z)
{
	m_World=XMMatrixTranslation(x,y,z);
}

