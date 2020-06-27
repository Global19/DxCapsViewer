//-----------------------------------------------------------------------------
// Name: DXGI.cpp
//
// Desc: DX Graphics stuff for Direct3D 10.x / 11.x
//
// Copyright Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#include "dxview.h"
#include <D3Dcommon.h>
#include <dxgi1_4.h>
#include <d3d10_1.h>
#include <d3d11_3.h>

// Define for some debug output
//#define EXTRA_DEBUG

//-----------------------------------------------------------------------------

enum FLMASK
{
    FLMASK_9_1  = 0x1,
    FLMASK_9_2  = 0x2,
    FLMASK_9_3  = 0x4,
    FLMASK_10_0 = 0x8,
    FLMASK_10_1 = 0x10,
    FLMASK_11_0 = 0x20,
    FLMASK_11_1 = 0x40,
};

//-----------------------------------------------------------------------------

const char* D3D10_NOTE = "Most Direct3D 10 features are required. Tool only shows optional features.";
const char* D3D10_NOTE1 = "Most Direct3D 10.1 features are required. Tool only shows optional features.";
const char* D3D11_NOTE = "Most Direct3D 11 features are required. Tool only shows optional features.";
const char* D3D11_NOTE1 = "Most Direct3D 11.1 features are required. Tool only shows optional features.";
const char* D3D11_NOTE2 = "Most Direct3D 11.2 features are required. Tool only shows optional features.";
const char* D3D11_NOTE3 = "Most Direct3D 11.3 features are required. Tool only shows optional features.";
const char* _10L9_NOTE = "Most 10level9 features are required. Tool only shows optional features.";
const char* SEE_D3D10 = "See Direct3D 10 node for device details.";
const char* SEE_D3D10_1 = "See Direct3D 10.1 node for device details.";
const char* SEE_D3D11 = "See Direct3D 11 node for device details.";
const char* SEE_D3D11_1 = "See Direct3D 11.1 node for device details.";

const char* FL_NOTE = "This feature summary is derived from hardware feature level";

//-----------------------------------------------------------------------------

typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY )( REFIID, void** );
typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY2 )( UINT, REFIID, void** );
typedef HRESULT ( WINAPI* LPD3D10CREATEDEVICE )( IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT32,
                                                 ID3D10Device** );
typedef HRESULT ( WINAPI* LPD3D10CREATEDEVICE1 )( IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT,
                                                  D3D10_FEATURE_LEVEL1, UINT, ID3D10Device1** );
typedef HRESULT ( WINAPI* LPD3D11CREATEDEVICE)( IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT32,
                  D3D_FEATURE_LEVEL*, UINT, UINT32, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext** );

IDXGIFactory* g_DXGIFactory = NULL;
IDXGIFactory1* g_DXGIFactory1 = NULL;
IDXGIFactory2* g_DXGIFactory2 = NULL;
HMODULE g_dxgi = NULL;

LPD3D10CREATEDEVICE g_D3D10CreateDevice = NULL;
HMODULE g_d3d10 = NULL;

LPD3D10CREATEDEVICE1 g_D3D10CreateDevice1 = NULL;
HMODULE g_d3d10_1 = NULL;

LPD3D11CREATEDEVICE g_D3D11CreateDevice = NULL;
HMODULE g_d3d11 = NULL;

extern HRESULT PrintValueLine( const CHAR* szText, DWORD dwValue, PRINTCBINFO *lpInfo );
extern HRESULT PrintHexValueLine( const CHAR* szText, DWORD dwValue, PRINTCBINFO *lpInfo );
extern HRESULT PrintStringValueLine( const CHAR* szText, const CHAR* szText2, PRINTCBINFO *lpInfo );
extern HRESULT PrintStringLine( const CHAR * szText, PRINTCBINFO *lpInfo );

const char* szRotation[] = 
{
    "DXGI_MODE_ROTATION_UNSPECIFIED",
    "DXGI_MODE_ROTATION_IDENTITY",
    "DXGI_MODE_ROTATION_ROTATE90",
    "DXGI_MODE_ROTATION_ROTATE180",
    "DXGI_MODE_ROTATION_ROTATE270"
};

// The DXGI formats that can be used as display devices
DXGI_FORMAT AdapterFormatArray[] = 
{
DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
DXGI_FORMAT_R8G8B8A8_UNORM,
DXGI_FORMAT_R16G16B16A16_FLOAT,
DXGI_FORMAT_R10G10B10A2_UNORM,
DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, // DXGI 1.1
DXGI_FORMAT_B8G8R8A8_UNORM,             // DXGI 1.1
DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,        // DXGI 1.1
};
const UINT NumAdapterFormats = sizeof(AdapterFormatArray) / sizeof(AdapterFormatArray[0]);

static const DXGI_FORMAT g_cfsMSAA_10level9[] = 
{
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM
};

static const DXGI_FORMAT g_cfsMSAA_10[] =
{
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_D24_UNORM_S8_UINT, 
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM
};

static const DXGI_FORMAT g_cfsMSAA_11[] =
{
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8X8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_B5G5R5A1_UNORM,
    DXGI_FORMAT_B4G4R4A4_UNORM,
};


extern DWORD g_dwViewState;
extern char c_szYes[];
extern char c_szNo[];

char c_szNA[] = "n/a";

static BOOL g_sampCount10[D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT];
static BOOL g_sampCount10_1[D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT];
static BOOL g_sampCount11[D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT];
static BOOL g_sampCount11_1[D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT];

//-----------------------------------------------------------------------------
// Name: DXGI_Init()
//-----------------------------------------------------------------------------
VOID DXGI_Init()
{
    // DXGI
    g_dxgi = LoadLibrary( "dxgi.dll" );
    if ( g_dxgi != NULL )
    {
        auto fpCreateDXGIFactory2 = reinterpret_cast<LPCREATEDXGIFACTORY2>( GetProcAddress( g_dxgi, "CreateDXGIFactory2" ) );
        if ( fpCreateDXGIFactory2 )
        {
            // DXGI 1.3
            HRESULT hr = fpCreateDXGIFactory2( 0,_uuidof( IDXGIFactory2 ), ( LPVOID *)&g_DXGIFactory2 );
            if ( SUCCEEDED(hr) )
            {
                g_DXGIFactory1 = g_DXGIFactory2;
                g_DXGIFactory = g_DXGIFactory2;
            }
            else
            {
                g_DXGIFactory2 = nullptr;
                // This should have succeeded or already fallen through to the DXGI 1.2 case
            }
        }
        else
        {
            // DXGI 1.1, 1.2
            LPCREATEDXGIFACTORY fpCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress( g_dxgi, "CreateDXGIFactory1" );

            if ( fpCreateDXGIFactory != NULL )
            {
                HRESULT hr = fpCreateDXGIFactory( __uuidof( IDXGIFactory2 ), ( LPVOID *)&g_DXGIFactory2 );
                if ( SUCCEEDED(hr) )
                {
                    g_DXGIFactory1 = g_DXGIFactory2;
                    g_DXGIFactory = g_DXGIFactory2;
                }
                else
                {
                    g_DXGIFactory2 = NULL;

                    hr = fpCreateDXGIFactory( __uuidof( IDXGIFactory1 ), ( LPVOID *)&g_DXGIFactory1 );
                    if ( SUCCEEDED(hr) )
                        g_DXGIFactory = g_DXGIFactory1;
                    else
                        g_DXGIFactory1 = NULL;
                }
            }
            else
            {
                // DXGI 1.0
                fpCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress( g_dxgi, "CreateDXGIFactory" );

                if ( fpCreateDXGIFactory != NULL )
                {
                    HRESULT hr = fpCreateDXGIFactory( __uuidof( IDXGIFactory ), ( LPVOID *)&g_DXGIFactory );
                    if ( FAILED(hr) )
                        g_DXGIFactory = NULL;
                }
            }
        }
    }

    // Direct3D 10.x
    g_d3d10_1 = LoadLibrary( "d3d10_1.dll" );
    if ( g_d3d10_1 != NULL )
    {
        g_D3D10CreateDevice1 = (LPD3D10CREATEDEVICE1)GetProcAddress( g_d3d10_1, "D3D10CreateDevice1" );
    }
    else
    {
        g_d3d10 = LoadLibrary( "d3d10.dll" );
        if ( g_d3d10 != NULL )
        {
            g_D3D10CreateDevice = (LPD3D10CREATEDEVICE)GetProcAddress( g_d3d10, "D3D10CreateDevice" );
        }
    }

    // Direct3D 11
    g_d3d11 = LoadLibrary( "d3d11.dll" );
    if ( g_d3d11 != NULL )
    {
        g_D3D11CreateDevice = (LPD3D11CREATEDEVICE)GetProcAddress( g_d3d11, "D3D11CreateDevice" );
    }
}

//-----------------------------------------------------------------------------
#define ENUMNAME(a) case a: return TEXT(#a);

const TCHAR* FormatName( DXGI_FORMAT format )
{
    switch( format )
    {
ENUMNAME( DXGI_FORMAT_R32G32B32A32_TYPELESS)
ENUMNAME( DXGI_FORMAT_R32G32B32A32_FLOAT)
ENUMNAME( DXGI_FORMAT_R32G32B32A32_UINT)
ENUMNAME( DXGI_FORMAT_R32G32B32A32_SINT)
ENUMNAME( DXGI_FORMAT_R32G32B32_TYPELESS)
ENUMNAME( DXGI_FORMAT_R32G32B32_FLOAT)
ENUMNAME( DXGI_FORMAT_R32G32B32_UINT)
ENUMNAME( DXGI_FORMAT_R32G32B32_SINT)
ENUMNAME( DXGI_FORMAT_R16G16B16A16_TYPELESS)
ENUMNAME( DXGI_FORMAT_R16G16B16A16_FLOAT)
ENUMNAME( DXGI_FORMAT_R16G16B16A16_UNORM)
ENUMNAME( DXGI_FORMAT_R16G16B16A16_UINT)
ENUMNAME( DXGI_FORMAT_R16G16B16A16_SNORM)
ENUMNAME( DXGI_FORMAT_R16G16B16A16_SINT)
ENUMNAME( DXGI_FORMAT_R32G32_TYPELESS)
ENUMNAME( DXGI_FORMAT_R32G32_FLOAT)
ENUMNAME( DXGI_FORMAT_R32G32_UINT)
ENUMNAME( DXGI_FORMAT_R32G32_SINT)
ENUMNAME( DXGI_FORMAT_R32G8X24_TYPELESS)
ENUMNAME( DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
ENUMNAME( DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS)
ENUMNAME( DXGI_FORMAT_X32_TYPELESS_G8X24_UINT)
ENUMNAME( DXGI_FORMAT_R10G10B10A2_TYPELESS)
ENUMNAME( DXGI_FORMAT_R10G10B10A2_UNORM)
ENUMNAME( DXGI_FORMAT_R10G10B10A2_UINT)
ENUMNAME( DXGI_FORMAT_R11G11B10_FLOAT)
ENUMNAME( DXGI_FORMAT_R8G8B8A8_TYPELESS)
ENUMNAME( DXGI_FORMAT_R8G8B8A8_UNORM)
ENUMNAME( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_R8G8B8A8_UINT)
ENUMNAME( DXGI_FORMAT_R8G8B8A8_SNORM)
ENUMNAME( DXGI_FORMAT_R8G8B8A8_SINT)
ENUMNAME( DXGI_FORMAT_R16G16_TYPELESS)
ENUMNAME( DXGI_FORMAT_R16G16_FLOAT)
ENUMNAME( DXGI_FORMAT_R16G16_UNORM)
ENUMNAME( DXGI_FORMAT_R16G16_UINT)
ENUMNAME( DXGI_FORMAT_R16G16_SNORM)
ENUMNAME( DXGI_FORMAT_R16G16_SINT)
ENUMNAME( DXGI_FORMAT_R32_TYPELESS)
ENUMNAME( DXGI_FORMAT_D32_FLOAT)
ENUMNAME( DXGI_FORMAT_R32_FLOAT)
ENUMNAME( DXGI_FORMAT_R32_UINT)
ENUMNAME( DXGI_FORMAT_R32_SINT)
ENUMNAME( DXGI_FORMAT_R24G8_TYPELESS)
ENUMNAME( DXGI_FORMAT_D24_UNORM_S8_UINT)
ENUMNAME( DXGI_FORMAT_R24_UNORM_X8_TYPELESS)
ENUMNAME( DXGI_FORMAT_X24_TYPELESS_G8_UINT)
ENUMNAME( DXGI_FORMAT_R8G8_TYPELESS)
ENUMNAME( DXGI_FORMAT_R8G8_UNORM)
ENUMNAME( DXGI_FORMAT_R8G8_UINT)
ENUMNAME( DXGI_FORMAT_R8G8_SNORM)
ENUMNAME( DXGI_FORMAT_R8G8_SINT)
ENUMNAME( DXGI_FORMAT_R16_TYPELESS)
ENUMNAME( DXGI_FORMAT_R16_FLOAT)
ENUMNAME( DXGI_FORMAT_D16_UNORM)
ENUMNAME( DXGI_FORMAT_R16_UNORM)
ENUMNAME( DXGI_FORMAT_R16_UINT)
ENUMNAME( DXGI_FORMAT_R16_SNORM)
ENUMNAME( DXGI_FORMAT_R16_SINT)
ENUMNAME( DXGI_FORMAT_R8_TYPELESS)
ENUMNAME( DXGI_FORMAT_R8_UNORM)
ENUMNAME( DXGI_FORMAT_R8_UINT)
ENUMNAME( DXGI_FORMAT_R8_SNORM)
ENUMNAME( DXGI_FORMAT_R8_SINT)
ENUMNAME( DXGI_FORMAT_A8_UNORM)
ENUMNAME( DXGI_FORMAT_R1_UNORM)
ENUMNAME( DXGI_FORMAT_R9G9B9E5_SHAREDEXP)
ENUMNAME( DXGI_FORMAT_R8G8_B8G8_UNORM)
ENUMNAME( DXGI_FORMAT_G8R8_G8B8_UNORM)
ENUMNAME( DXGI_FORMAT_BC1_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC1_UNORM)
ENUMNAME( DXGI_FORMAT_BC1_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_BC2_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC2_UNORM)
ENUMNAME( DXGI_FORMAT_BC2_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_BC3_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC3_UNORM)
ENUMNAME( DXGI_FORMAT_BC3_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_BC4_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC4_UNORM)
ENUMNAME( DXGI_FORMAT_BC4_SNORM)
ENUMNAME( DXGI_FORMAT_BC5_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC5_UNORM)
ENUMNAME( DXGI_FORMAT_BC5_SNORM)
ENUMNAME( DXGI_FORMAT_B5G6R5_UNORM)
ENUMNAME( DXGI_FORMAT_B5G5R5A1_UNORM)
ENUMNAME( DXGI_FORMAT_B8G8R8A8_UNORM)
ENUMNAME( DXGI_FORMAT_B8G8R8X8_UNORM)
ENUMNAME( DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM)
ENUMNAME( DXGI_FORMAT_B8G8R8A8_TYPELESS)
ENUMNAME( DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_B8G8R8X8_TYPELESS)
ENUMNAME( DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_BC6H_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC6H_UF16)
ENUMNAME( DXGI_FORMAT_BC6H_SF16)
ENUMNAME( DXGI_FORMAT_BC7_TYPELESS)
ENUMNAME( DXGI_FORMAT_BC7_UNORM)
ENUMNAME( DXGI_FORMAT_BC7_UNORM_SRGB)
ENUMNAME( DXGI_FORMAT_AYUV )
ENUMNAME( DXGI_FORMAT_Y410 )
ENUMNAME( DXGI_FORMAT_Y416 )
ENUMNAME( DXGI_FORMAT_NV12 )
ENUMNAME( DXGI_FORMAT_P010 )
ENUMNAME( DXGI_FORMAT_P016 )
ENUMNAME( DXGI_FORMAT_420_OPAQUE )
ENUMNAME( DXGI_FORMAT_YUY2 )
ENUMNAME( DXGI_FORMAT_Y210 )
ENUMNAME( DXGI_FORMAT_Y216 )
ENUMNAME( DXGI_FORMAT_NV11 )
ENUMNAME( DXGI_FORMAT_AI44 )
ENUMNAME( DXGI_FORMAT_IA44 )
ENUMNAME( DXGI_FORMAT_P8 )
ENUMNAME( DXGI_FORMAT_A8P8 )
ENUMNAME( DXGI_FORMAT_B4G4R4A4_UNORM )
ENUMNAME( DXGI_FORMAT_P208 )
ENUMNAME( DXGI_FORMAT_V208 )
ENUMNAME( DXGI_FORMAT_V408 )
    default:
        return TEXT("DXGI_FORMAT_UNKNOWN");
    }
}

const TCHAR* FLName( D3D10_FEATURE_LEVEL1 lvl )
{
    switch( lvl )
    {
ENUMNAME( D3D10_FEATURE_LEVEL_10_0 )
ENUMNAME( D3D10_FEATURE_LEVEL_10_1 )
ENUMNAME( D3D10_FEATURE_LEVEL_9_1 )
ENUMNAME( D3D10_FEATURE_LEVEL_9_2 ) 
ENUMNAME( D3D10_FEATURE_LEVEL_9_3 )
    default:
        return TEXT("D3D10_FEATURE_LEVEL_UNKNOWN");
    }
}

const TCHAR* FLName( D3D_FEATURE_LEVEL lvl )
{
    switch( lvl )
    {
ENUMNAME( D3D_FEATURE_LEVEL_9_1 )
ENUMNAME( D3D_FEATURE_LEVEL_9_2 )
ENUMNAME( D3D_FEATURE_LEVEL_9_3 )
ENUMNAME( D3D_FEATURE_LEVEL_10_0 )
ENUMNAME( D3D_FEATURE_LEVEL_10_1 )
ENUMNAME( D3D_FEATURE_LEVEL_11_0 )
ENUMNAME( D3D_FEATURE_LEVEL_11_1 )
    default:
        return TEXT("D3D_FEATURE_LEVEL_UNKNOWN");
    }
}

//-----------------------------------------------------------------------------
UINT RefreshRate( const DXGI_RATIONAL& Rate )
{
    if ( Rate.Numerator == 0 )
        return 0;
    else if ( Rate.Denominator == 0 )
        return 0;
    else if ( Rate.Denominator == 1 )
        return Rate.Numerator;
    else
        return (UINT)( float(Rate.Numerator) / float(Rate.Denominator) );
}


//-----------------------------------------------------------------------------
HRESULT DXGIAdapterInfo( LPARAM lParam1, LPARAM lParam2, PRINTCBINFO* pPrintInfo )
{
    HRESULT hr;
    IDXGIAdapter* pAdapter = (IDXGIAdapter*)lParam2;

    if ( pAdapter == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 50);
    }

    DXGI_ADAPTER_DESC desc;
    hr = pAdapter->GetDesc( &desc );

    if ( FAILED(hr) )
        return hr;

    DWORD dvm = (DWORD)(desc.DedicatedVideoMemory / (1024*1024));
    DWORD dsm = (DWORD)(desc.DedicatedSystemMemory / (1024*1024));
    DWORD ssm = (DWORD)(desc.SharedSystemMemory / (1024*1024));

    char szDesc[128];

    wcstombs( szDesc, desc.Description, 128 );

    if ( pPrintInfo == NULL)
    {
        LVAddText( g_hwndLV, 0, "Description");
        LVAddText( g_hwndLV, 1, "%s", szDesc);

        LVAddText( g_hwndLV, 0, "VendorId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.VendorId);

        LVAddText( g_hwndLV, 0, "DeviceId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.DeviceId);

        LVAddText( g_hwndLV, 0, "SubSysId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.SubSysId);

        LVAddText( g_hwndLV, 0, "Revision");
        LVAddText( g_hwndLV, 1, "%d", desc.Revision);

        LVAddText( g_hwndLV, 0, "DedicatedVideoMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", dvm);

        LVAddText( g_hwndLV, 0, "DedicatedSystemMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", dsm);

        LVAddText( g_hwndLV, 0, "SharedSystemMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", ssm);
    }
    else
    {
        PrintStringValueLine( "Description", szDesc, pPrintInfo );
        PrintHexValueLine( "VendorId", desc.VendorId, pPrintInfo );
        PrintHexValueLine( "DeviceId", desc.DeviceId, pPrintInfo );
        PrintHexValueLine( "SubSysId", desc.SubSysId, pPrintInfo );
        PrintValueLine( "Revision", desc.Revision, pPrintInfo );
        PrintValueLine( "DedicatedVideoMemory (MB)", dvm, pPrintInfo );
        PrintValueLine( "DedicatedSystemMemory (MB)", dsm, pPrintInfo );
        PrintValueLine( "SharedSystemMemory (MB)", ssm, pPrintInfo );
    }

    return S_OK;
}

HRESULT DXGIAdapterInfo1( LPARAM lParam1, LPARAM lParam2, PRINTCBINFO* pPrintInfo )
{
    HRESULT hr;
    IDXGIAdapter1* pAdapter = (IDXGIAdapter1*)lParam2;

    if ( pAdapter == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 50);
    }

    DXGI_ADAPTER_DESC1 desc;
    hr = pAdapter->GetDesc1( &desc );

    if ( FAILED(hr) )
        return hr;

    DWORD dvm = (DWORD)(desc.DedicatedVideoMemory / (1024*1024));
    DWORD dsm = (DWORD)(desc.DedicatedSystemMemory / (1024*1024));
    DWORD ssm = (DWORD)(desc.SharedSystemMemory / (1024*1024));

    char szDesc[128];

    wcstombs( szDesc, desc.Description, 128 );

    if ( pPrintInfo == NULL)
    {
        LVAddText( g_hwndLV, 0, "Description");
        LVAddText( g_hwndLV, 1, "%s", szDesc);

        LVAddText( g_hwndLV, 0, "VendorId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.VendorId);

        LVAddText( g_hwndLV, 0, "DeviceId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.DeviceId);

        LVAddText( g_hwndLV, 0, "SubSysId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.SubSysId);

        LVAddText( g_hwndLV, 0, "Revision");
        LVAddText( g_hwndLV, 1, "%d", desc.Revision);

        LVAddText( g_hwndLV, 0, "DedicatedVideoMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", dvm);

        LVAddText( g_hwndLV, 0, "DedicatedSystemMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", dsm);

        LVAddText( g_hwndLV, 0, "SharedSystemMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", ssm);

        LVAddText( g_hwndLV, 0, "Remote" );
        LVAddText( g_hwndLV, 1, (desc.Flags & DXGI_ADAPTER_FLAG_REMOTE) ? c_szYes : c_szNo );
    }
    else
    {
        PrintStringValueLine( "Description", szDesc, pPrintInfo );
        PrintHexValueLine( "VendorId", desc.VendorId, pPrintInfo );
        PrintHexValueLine( "DeviceId", desc.DeviceId, pPrintInfo );
        PrintHexValueLine( "SubSysId", desc.SubSysId, pPrintInfo );
        PrintValueLine( "Revision", desc.Revision, pPrintInfo );
        PrintValueLine( "DedicatedVideoMemory (MB)", dvm, pPrintInfo );
        PrintValueLine( "DedicatedSystemMemory (MB)", dsm, pPrintInfo );
        PrintValueLine( "SharedSystemMemory (MB)", ssm, pPrintInfo );
        PrintStringValueLine( "Remote", (desc.Flags & DXGI_ADAPTER_FLAG_REMOTE) ? c_szYes : c_szNo, pPrintInfo );
    }

    return S_OK;
}

HRESULT DXGIAdapterInfo2( LPARAM lParam1, LPARAM lParam2, PRINTCBINFO* pPrintInfo )
{
    HRESULT hr;
    auto pAdapter = reinterpret_cast<IDXGIAdapter2*>(lParam2);

    if ( !pAdapter )
        return S_OK;

    if( !pPrintInfo )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 50);
    }

    DXGI_ADAPTER_DESC2 desc;
    hr = pAdapter->GetDesc2( &desc );

    if ( FAILED(hr) )
        return hr;

    DWORD dvm = (DWORD)(desc.DedicatedVideoMemory / (1024*1024));
    DWORD dsm = (DWORD)(desc.DedicatedSystemMemory / (1024*1024));
    DWORD ssm = (DWORD)(desc.SharedSystemMemory / (1024*1024));

    char szDesc[128];

    wcstombs( szDesc, desc.Description, 128 );

    const char *gpg = nullptr;
    const char *cpg = nullptr;

    switch ( desc.GraphicsPreemptionGranularity )
    {
   case DXGI_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY:    gpg = "DMA Buffer"; break;
   case DXGI_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY:     gpg = "Primitive"; break;
   case DXGI_GRAPHICS_PREEMPTION_TRIANGLE_BOUNDARY:      gpg = "Triangle"; break;
   case DXGI_GRAPHICS_PREEMPTION_PIXEL_BOUNDARY:         gpg = "Pixel"; break;
   case DXGI_GRAPHICS_PREEMPTION_INSTRUCTION_BOUNDARY:   gpg = "Instruction"; break;
   default:                                              gpg = "Unknown"; break;
    }

    switch ( desc.ComputePreemptionGranularity )
    {
    case DXGI_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY:    cpg = "DMA Buffer"; break;
    case DXGI_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY:      cpg = "Primitive"; break;
    case DXGI_COMPUTE_PREEMPTION_THREAD_GROUP_BOUNDARY:  cpg = "Triangle"; break;
    case DXGI_COMPUTE_PREEMPTION_THREAD_BOUNDARY:        cpg = "Pixel"; break;
    case DXGI_COMPUTE_PREEMPTION_INSTRUCTION_BOUNDARY:   cpg = "Instruction"; break;
    default:                                             cpg = "Unknown"; break;
    }

    if ( !pPrintInfo )
    {
        LVAddText( g_hwndLV, 0, "Description");
        LVAddText( g_hwndLV, 1, "%s", szDesc);

        LVAddText( g_hwndLV, 0, "VendorId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.VendorId);

        LVAddText( g_hwndLV, 0, "DeviceId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.DeviceId);

        LVAddText( g_hwndLV, 0, "SubSysId");
        LVAddText( g_hwndLV, 1, "0x%08x", desc.SubSysId);

        LVAddText( g_hwndLV, 0, "Revision");
        LVAddText( g_hwndLV, 1, "%d", desc.Revision);

        LVAddText( g_hwndLV, 0, "DedicatedVideoMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", dvm);

        LVAddText( g_hwndLV, 0, "DedicatedSystemMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", dsm);

        LVAddText( g_hwndLV, 0, "SharedSystemMemory (MB)");
        LVAddText( g_hwndLV, 1, "%d", ssm);

        LVAddText( g_hwndLV, 0, "Remote" );
        LVAddText( g_hwndLV, 1, (desc.Flags & DXGI_ADAPTER_FLAG_REMOTE) ? c_szYes : c_szNo );

        LVAddText( g_hwndLV, 0, "Graphics Preemption Granularity" );
        LVAddText( g_hwndLV, 1, gpg );

        LVAddText( g_hwndLV, 0, "Compute Preemption Granularity" );
        LVAddText( g_hwndLV, 1, cpg );
    }
    else
    {
        PrintStringValueLine( "Description", szDesc, pPrintInfo );
        PrintHexValueLine( "VendorId", desc.VendorId, pPrintInfo );
        PrintHexValueLine( "DeviceId", desc.DeviceId, pPrintInfo );
        PrintHexValueLine( "SubSysId", desc.SubSysId, pPrintInfo );
        PrintValueLine( "Revision", desc.Revision, pPrintInfo );
        PrintValueLine( "DedicatedVideoMemory (MB)", dvm, pPrintInfo );
        PrintValueLine( "DedicatedSystemMemory (MB)", dsm, pPrintInfo );
        PrintValueLine( "SharedSystemMemory (MB)", ssm, pPrintInfo );
        PrintStringValueLine( "Remote", (desc.Flags & DXGI_ADAPTER_FLAG_REMOTE) ? c_szYes : c_szNo, pPrintInfo );
        PrintStringValueLine( "Graphics Preemption Granularity", gpg, pPrintInfo );
        PrintStringValueLine( "Compute Preemption Granularity", cpg, pPrintInfo );
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
HRESULT DXGIOutputInfo( LPARAM lParam1, LPARAM lParam2, PRINTCBINFO* pPrintInfo )
{
    HRESULT hr;
    IDXGIOutput* pOutput = (IDXGIOutput*)lParam2;

    if ( pOutput == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 30);
    }

    DXGI_OUTPUT_DESC desc;
    hr = pOutput->GetDesc( &desc );

    if ( FAILED(hr) )
        return hr;

    char szDevName[32];

    wcstombs( szDevName, desc.DeviceName, 32 );

    if ( pPrintInfo == NULL)
    {
        LVAddText( g_hwndLV, 0, "DeviceName");
        LVAddText( g_hwndLV, 1, "%s", szDevName);

        LVAddText( g_hwndLV, 0, "AttachedToDesktop");
        LVAddText( g_hwndLV, 1, desc.AttachedToDesktop ? c_szYes : c_szNo );

        LVAddText( g_hwndLV, 0, "Rotation");
        LVAddText( g_hwndLV, 1, szRotation[ desc.Rotation ]);
    }
    else
    {
        PrintStringValueLine( "DeviceName", szDevName, pPrintInfo );
        PrintStringValueLine( "AttachedToDesktop", desc.AttachedToDesktop ? c_szYes : c_szNo, pPrintInfo );
        PrintStringValueLine( "Rotation", szRotation[ desc.Rotation ], pPrintInfo );
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
HRESULT DXGIOutputModes( LPARAM lParam1, LPARAM lParam2, PRINTCBINFO* pPrintInfo )
{
    HRESULT hr;
    IDXGIOutput* pOutput = (IDXGIOutput*)lParam2;

    if ( pOutput == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Resolution", 10);
        LVAddColumn(g_hwndLV, 1, "Pixel Format", 30);
        LVAddColumn(g_hwndLV, 2, "Refresh Rate", 10);
    }

    for (UINT iFormat = 0; iFormat < NumAdapterFormats; ++iFormat)
    {
        DXGI_FORMAT fmt = AdapterFormatArray[iFormat];

        if ( !g_DXGIFactory1 )
        {
            switch (fmt)
            {
            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                continue;
            }
        }

        UINT num = 0;
        const DWORD flags = 0;
        hr = pOutput->GetDisplayModeList( fmt, flags, &num, 0 );

        if ( SUCCEEDED(hr) && num > 0 )
        {
            DXGI_MODE_DESC* pDescs = new DXGI_MODE_DESC[num];

            hr = pOutput->GetDisplayModeList( fmt, flags, &num, pDescs );

            if ( SUCCEEDED(hr) )
            {
                for( UINT iMode = 0; iMode < num; ++iMode )
                {
                    const DXGI_MODE_DESC* pDesc = &pDescs[ iMode ];
                    if (pPrintInfo == NULL)
                    {
                        LVAddText( g_hwndLV, 0, "%d x %d", pDesc->Width, pDesc->Height);
                        LVAddText( g_hwndLV, 1, FormatName(pDesc->Format));
                        LVAddText( g_hwndLV, 2, "%d", RefreshRate( pDesc->RefreshRate ) );
                    }
                    else
                    {
                       char  szBuff[80];
                       DWORD cchLen;

                       // Calculate Name and Value column x offsets
                       int x1    = (pPrintInfo->dwCurrIndent * DEF_TAB_SIZE * pPrintInfo->dwCharWidth);
                       int x2    = x1 + (30 * pPrintInfo->dwCharWidth);
                       int x3    = x2 + (20 * pPrintInfo->dwCharWidth);
                       int yLine = (pPrintInfo->dwCurrLine * pPrintInfo->dwLineHeight);

                       sprintf_s (szBuff, sizeof(szBuff), "%d x %d", pDesc->Width, pDesc->Height);
                       cchLen = _tcslen (szBuff);
                       if( FAILED( PrintLine (x1, yLine, szBuff, cchLen, pPrintInfo) ) )
                           return E_FAIL;

                       strcpy_s (szBuff, sizeof(szBuff), FormatName(pDesc->Format));
                       cchLen = _tcslen (szBuff);
                       if( FAILED( PrintLine (x2, yLine, szBuff, cchLen, pPrintInfo) ) )
                           return E_FAIL;

                       sprintf_s (szBuff, sizeof(szBuff), "%d", RefreshRate( pDesc->RefreshRate ) );
                       cchLen = _tcslen (szBuff);
                       if( FAILED( PrintLine (x3, yLine, szBuff, cchLen, pPrintInfo) ) )
                           return E_FAIL;

                       // Advance to next line on page
                       if( FAILED( PrintNextLine(pPrintInfo) ) )
                           return E_FAIL;
                    }
                }
            }

            delete [] pDescs;
        }
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
#define LVYESNO(a,b) \
        if ( g_dwViewState == IDM_VIEWALL || (b) ) \
        { \
            LVAddText( g_hwndLV, 0, a ); \
            LVAddText( g_hwndLV, 1, (b) ? c_szYes : c_szNo ); \
        }

#define PRINTYESNO(a,b) \
        PrintStringValueLine( a, (b) ? c_szYes : c_szNo, pPrintInfo );

#define LVLINE(a,b) \
        if ( g_dwViewState == IDM_VIEWALL || (b != c_szNA && b != c_szNo) ) \
        { \
            LVAddText( g_hwndLV, 0, a ); \
            LVAddText( g_hwndLV, 1, b ); \
        }

#define PRINTLINE(a,b) \
        if ( g_dwViewState == IDM_VIEWALL || (b != c_szNA && b != c_szNo) ) \
        { \
            PrintStringValueLine( a, b, pPrintInfo ); \
        }

#define XTOSTRING(a) #a TOSTRING(a)
#define TOSTRING(a) #a

#define XTOSTRING2(a) #a TOSTRING2(a)
#define TOSTRING2(a) "( " #a " )"


//-----------------------------------------------------------------------------
BOOL IsMSAAPowerOf2( UINT value )
{
    // only supports values 0..32
    if ( value <= 1 )
        return TRUE;

    if ( value & 0x1 )
        return FALSE;

    for(UINT mask = 0x2; mask < 0x100; mask <<= 1 )
    {
        if (( value & ~mask ) == 0)
            return TRUE;
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
void CheckExtendedFormats( ID3D10Device* pDevice, BOOL& ext, BOOL& x2 )
{
    ext = x2 = FALSE;

    // DXGI 1.1 required for extended formats
    if ( !g_DXGIFactory1 )
        return;

    UINT fmtSupport = 0;
    HRESULT hr = pDevice->CheckFormatSupport( DXGI_FORMAT_B8G8R8A8_UNORM, &fmtSupport );
    if ( FAILED(hr) )
        fmtSupport = 0;
    
    if ( fmtSupport & D3D10_FORMAT_SUPPORT_RENDER_TARGET )
        ext = TRUE;
    
    hr = pDevice->CheckFormatSupport( DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, &fmtSupport );
    if ( FAILED(hr) )
        fmtSupport = 0;
    
    if ( fmtSupport & D3D10_FORMAT_SUPPORT_DISPLAY )
        x2 = TRUE;
}

void CheckExtendedFormats( ID3D11Device* pDevice, BOOL& ext, BOOL& x2, BOOL& bpp565 )
{
    ext = x2 = bpp565 = FALSE;

    UINT fmtSupport = 0;
    HRESULT hr = pDevice->CheckFormatSupport( DXGI_FORMAT_B8G8R8A8_UNORM, &fmtSupport );
    if ( FAILED(hr) )
        fmtSupport = 0;
    
    if ( fmtSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET )
        ext = TRUE;
    
    fmtSupport = 0;
    hr = pDevice->CheckFormatSupport( DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, &fmtSupport );
    if ( FAILED(hr) )
        fmtSupport = 0;

    if ( fmtSupport & D3D11_FORMAT_SUPPORT_DISPLAY )
        x2 = TRUE;

    // DXGI 1.2 is required for 16bpp support
    if ( g_DXGIFactory2 )
    {
        fmtSupport = 0;
        hr = pDevice->CheckFormatSupport( DXGI_FORMAT_B5G6R5_UNORM, &fmtSupport );
        if ( FAILED(hr) )
            fmtSupport = 0;

        if ( fmtSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D )
            bpp565 = TRUE;
    }
}


//-----------------------------------------------------------------------------
void CheckD3D11Ops( ID3D11Device* pDevice, bool &logicOps, bool &cbPartial, bool& cbOffsetting )
{
    D3D11_FEATURE_DATA_D3D11_OPTIONS d3d11opts;
    HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS, &d3d11opts, sizeof(d3d11opts) );
    if ( FAILED(hr) )
        memset( &d3d11opts, 0, sizeof(d3d11opts) );

    logicOps = ( d3d11opts.OutputMergerLogicOp ) ? true : false;
    cbPartial = ( d3d11opts.ConstantBufferPartialUpdate ) ? true : false;
    cbOffsetting = ( d3d11opts.ConstantBufferOffsetting ) ? true : false;
}


//-----------------------------------------------------------------------------
void CheckD3D11Ops1( ID3D11Device* pDevice, D3D11_TILED_RESOURCES_TIER &tiled, bool &minmaxfilter, bool& mapdefaultbuff )
{
    D3D11_FEATURE_DATA_D3D11_OPTIONS1 d3d11opts1;
    HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS1, &d3d11opts1, sizeof(d3d11opts1) );
    if ( FAILED(hr) )
        memset( &d3d11opts1, 0, sizeof(d3d11opts1) );

    tiled = d3d11opts1.TiledResourcesTier;
    minmaxfilter = ( d3d11opts1.MinMaxFiltering ) ? true : false;
    mapdefaultbuff = ( d3d11opts1.MapOnDefaultBuffers ) ? true : false;
}


//-----------------------------------------------------------------------------
void CheckD3D11Ops2( ID3D11Device* pDevice, D3D11_TILED_RESOURCES_TIER &tiled, D3D11_CONSERVATIVE_RASTERIZATION_TIER& crast, 
                     bool& rovs, bool& pssref )
{
    D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11opts2;
    HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS2, &d3d11opts2, sizeof(d3d11opts2) );
    if ( FAILED(hr) )
        memset( &d3d11opts2, 0, sizeof(d3d11opts2) );

    crast = d3d11opts2.ConservativeRasterizationTier;
    tiled = d3d11opts2.TiledResourcesTier;
    rovs = ( d3d11opts2.ROVsSupported ) ? true : false;
    pssref = (d3d11opts2.PSSpecifiedStencilRefSupported ) ? true : false;
}


//-----------------------------------------------------------------------------
void CheckD3D9Ops( ID3D11Device* pDevice, bool& nonpow2, bool &shadows )
{
    D3D11_FEATURE_DATA_D3D9_OPTIONS d3d9opts;
    HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D9_OPTIONS, &d3d9opts, sizeof(d3d9opts) );
    if ( FAILED(hr) )
        memset( &d3d9opts, 0, sizeof(d3d9opts) );

    nonpow2 = ( d3d9opts.FullNonPow2TextureSupport ) ? true : false;

    D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT d3d9shadows;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D9_SHADOW_SUPPORT, &d3d9shadows, sizeof(d3d9shadows) );
    if ( FAILED(hr) )
        memset( &d3d9shadows, 0, sizeof(d3d9shadows) );

    shadows = ( d3d9shadows.SupportsDepthAsTextureWithLessEqualComparisonFilter ) ? true : false;
}


//-----------------------------------------------------------------------------
void CheckD3D9Ops1( ID3D11Device* pDevice, bool& nonpow2, bool &shadows, bool& instancing, bool& cubemapRT )
{
    D3D11_FEATURE_DATA_D3D9_OPTIONS1 d3d9opts;
    HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D9_OPTIONS1, &d3d9opts, sizeof(d3d9opts) );
    if ( FAILED(hr) )
    {
        memset( &d3d9opts, 0, sizeof(d3d9opts) );

        // Handle Windows 8.1 Preview by falling back to D3D9_OPTIONS, D3D9_SHADOW_SUPPORT, and D3D9_SIMPLE_INSTANCING_SUPPORT
        CheckD3D9Ops( pDevice, nonpow2, shadows );

        D3D11_FEATURE_DATA_D3D9_SIMPLE_INSTANCING_SUPPORT d3d9si;
        hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D9_SIMPLE_INSTANCING_SUPPORT, &d3d9si, sizeof(d3d9si) );
        if ( FAILED(hr) )
            memset( &d3d9si, 0, sizeof(d3d9si) );

        instancing = ( d3d9si.SimpleInstancingSupported ) ? true : false;

        cubemapRT = false;

        return;
    }

    nonpow2 = ( d3d9opts.FullNonPow2TextureSupported ) ? true : false;

    shadows = ( d3d9opts.DepthAsTextureWithLessEqualComparisonFilterSupported ) ? true : false;

    instancing = ( d3d9opts.SimpleInstancingSupported ) ? true : false;

    cubemapRT = ( d3d9opts.TextureCubeFaceRenderTargetWithNonCubeDepthStencilSupported ) ? true : false;
}


//-----------------------------------------------------------------------------
#define D3D_FL_LPARAM3_D3D10( d3dType ) ( ( (d3dType & 0xff) << 8 ) | 0 )
#define D3D_FL_LPARAM3_D3D10_1( d3dType ) ( ( (d3dType & 0xff) << 8 ) | 1 )
#define D3D_FL_LPARAM3_D3D11( d3dType ) ( ( (d3dType & 0xff) << 8 ) | 2 )
#define D3D_FL_LPARAM3_D3D11_1( d3dType ) ( ( (d3dType & 0xff) << 8 ) | 3 )
#define D3D_FL_LPARAM3_D3D11_2( d3dType ) ( ( (d3dType & 0xff) << 8 ) | 4 )
#define D3D_FL_LPARAM3_D3D11_3( d3dType ) ( ( (d3dType & 0xff) << 8 ) | 5 )
HRESULT D3D_FeatureLevel( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    D3D_FEATURE_LEVEL fl = (D3D_FEATURE_LEVEL)lParam1;

    if ( lParam2 == NULL )
        return S_OK;

    ID3D10Device* pD3D10 = NULL;
    ID3D10Device1* pD3D10_1 = NULL;
    ID3D11Device* pD3D11 = NULL;
    ID3D11Device1* pD3D11_1 = NULL;
    ID3D11Device2* pD3D11_2 = nullptr;
    ID3D11Device3* pD3D11_3 = nullptr;

    auto d3dVer = static_cast<unsigned int>( lParam3 & 0xff );
    auto d3dType = static_cast<D3D_DRIVER_TYPE>( ( lParam3 & 0xff00 ) >> 8 );
    
    if ( d3dVer == 5 )
    {
        pD3D11_3 = reinterpret_cast<ID3D11Device3*>(lParam2);
    }
    if ( d3dVer == 4 )
    {
        pD3D11_2 = reinterpret_cast<ID3D11Device2*>(lParam2);
    }
    if ( d3dVer == 3 )
        pD3D11_1 = (ID3D11Device1*)lParam2;
    if ( d3dVer == 2 )
    {
        if ( fl > D3D_FEATURE_LEVEL_11_0 )
            fl = D3D_FEATURE_LEVEL_11_0;
        pD3D11 = (ID3D11Device*)lParam2;
    }
    else if ( d3dVer == 1 )
        pD3D10_1 = (ID3D10Device1*)lParam2;
    else
        pD3D10 = (ID3D10Device*)lParam2;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 60);
    }

    const char *shaderModel = NULL;
    const char *computeShader = c_szNo;
    const char *maxTexDim = NULL;
    const char *maxCubeDim = NULL;
    const char *maxVolDim = NULL;
    const char *maxTexRepeat = NULL;
    const char *maxAnisotropy = NULL;
    const char *maxPrimCount = "4294967296";
    const char *maxInputSlots = NULL;
    const char *mrt = NULL;
    const char *extFormats = NULL;
    const char *x2_10BitFormat = NULL;
    const char *logic_ops = c_szNo;
    const char *cb_partial = c_szNA;
    const char *cb_offsetting = c_szNA;
    const char *uavSlots = nullptr;
    const char *uavEveryStage = nullptr;
    const char *uavOnlyRender = nullptr;
    const char *nonpow2 = NULL;
    const char *bpp16 = c_szNo;
    const char *shadows = nullptr;
    const char *cubeRT = nullptr;
    const char *tiled_rsc = nullptr;
    const char *minmaxfilter = nullptr;
    const char *mapdefaultbuff = nullptr;
    const char *consrv_rast = nullptr;
    const char *rast_ordered_views = nullptr;
    const char *ps_stencil_ref = nullptr;
    const char *instancing = nullptr;

    BOOL _10level9 = FALSE;

    switch( fl )
    {
    case D3D_FEATURE_LEVEL_11_1:
        shaderModel = "5.0";
        computeShader = "Yes (CS 5.0)";
        maxTexDim = XTOSTRING( D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING( D3D11_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING( D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING( D3D11_REQ_FILTERING_HW_ADDRESSABLE_RESOURCE_DIMENSION );
        maxAnisotropy = XTOSTRING( D3D11_REQ_MAXANISOTROPY );
        maxInputSlots = XTOSTRING( D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING( D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT );
        extFormats = c_szYes;
        x2_10BitFormat = c_szYes;
        logic_ops = c_szYes;
        cb_partial = c_szYes;
        cb_offsetting = c_szYes;
        uavSlots = XTOSTRING( D3D11_1_UAV_SLOT_COUNT );
        uavEveryStage = c_szYes;
        uavOnlyRender = "16";
        nonpow2 = "Full";
        bpp16 = c_szYes;
        instancing = c_szYes;

        if (pD3D11_3)
        {
            D3D11_TILED_RESOURCES_TIER tiled = D3D11_TILED_RESOURCES_NOT_SUPPORTED;
            bool bMinMaxFilter, bMapDefaultBuff; 
            CheckD3D11Ops1( pD3D11_3, tiled, bMinMaxFilter, bMapDefaultBuff );

            D3D11_CONSERVATIVE_RASTERIZATION_TIER crast = D3D11_CONSERVATIVE_RASTERIZATION_NOT_SUPPORTED;
            bool rovs, pssref;
            CheckD3D11Ops2( pD3D11_3, tiled, crast, rovs, pssref );

            switch (tiled)
            {
            case D3D11_TILED_RESOURCES_NOT_SUPPORTED:   tiled_rsc = "Optional (No)";            break;
            case D3D11_TILED_RESOURCES_TIER_1:          tiled_rsc = "Optional (Yes - Tier 1)";  break;
            case D3D11_TILED_RESOURCES_TIER_2:          tiled_rsc = "Optional (Yes - Tier 2)";  break;
            case D3D11_TILED_RESOURCES_TIER_3:          tiled_rsc = "Optional (Yes - Tier 3)";  break;
            default:                                    tiled_rsc = "Optional (Yes)";           break;
            }

            switch(crast)
            {
            case D3D11_CONSERVATIVE_RASTERIZATION_NOT_SUPPORTED:    consrv_rast = "Optional (No)";            break;
            case D3D11_CONSERVATIVE_RASTERIZATION_TIER_1:           consrv_rast = "Optional (Yes - Tier 1)";  break;
            case D3D11_CONSERVATIVE_RASTERIZATION_TIER_2:           consrv_rast = "Optional (Yes - Tier 2)";  break;
            case D3D11_CONSERVATIVE_RASTERIZATION_TIER_3:           consrv_rast = "Optional (Yes - Tier 3)";  break;
            default:                                                consrv_rast = "Optional (Yes)";           break;
            }

            rast_ordered_views = (rovs) ? "Optional (Yes)" : "Optional (No)";
            ps_stencil_ref = (pssref) ? "Optional (Yes)" : "Optional (No)";
            minmaxfilter = bMinMaxFilter ? "Optional (Yes)" : "Optional (No)";
            mapdefaultbuff = bMapDefaultBuff ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11_2)
        {
            D3D11_TILED_RESOURCES_TIER tiled = D3D11_TILED_RESOURCES_NOT_SUPPORTED;
            bool bMinMaxFilter, bMapDefaultBuff; 
            CheckD3D11Ops1( pD3D11_2, tiled, bMinMaxFilter, bMapDefaultBuff );

            switch (tiled)
            {
            case D3D11_TILED_RESOURCES_NOT_SUPPORTED:   tiled_rsc = "Optional (No)";            break;
            case D3D11_TILED_RESOURCES_TIER_1:          tiled_rsc = "Optional (Yes - Tier 1)";  break;
            case D3D11_TILED_RESOURCES_TIER_2:          tiled_rsc = "Optional (Yes - Tier 2)";  break;
            default:                                    tiled_rsc = "Optional (Yes)";           break;
            }

            minmaxfilter = bMinMaxFilter ? "Optional (Yes)" : "Optional (No)";
            mapdefaultbuff = bMapDefaultBuff ? "Optional (Yes)" : "Optional (No)";
        }
        break;

    case D3D_FEATURE_LEVEL_11_0:
        shaderModel = "5.0";
        computeShader = "Yes (CS 5.0)";
        maxTexDim = XTOSTRING( D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING( D3D11_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING( D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING( D3D11_REQ_FILTERING_HW_ADDRESSABLE_RESOURCE_DIMENSION );
        maxAnisotropy = XTOSTRING( D3D11_REQ_MAXANISOTROPY );
        maxInputSlots = XTOSTRING( D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING( D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT );
        extFormats = c_szYes;
        x2_10BitFormat = c_szYes;
        instancing = c_szYes;

        if (pD3D11_3)
        {
            D3D11_TILED_RESOURCES_TIER tiled = D3D11_TILED_RESOURCES_NOT_SUPPORTED;
            bool bMinMaxFilter, bMapDefaultBuff; 
            CheckD3D11Ops1( pD3D11_3, tiled, bMinMaxFilter, bMapDefaultBuff );

            D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11opts2;
            HRESULT hr = pD3D11_3->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS2, &d3d11opts2, sizeof(d3d11opts2) );
            if ( SUCCEEDED(hr) )
            {
                // D3D11_FEATURE_DATA_D3D11_OPTIONS1 caps this at Tier 2
                tiled = d3d11opts2.TiledResourcesTier;
            }

            switch (tiled)
            {
            case D3D11_TILED_RESOURCES_NOT_SUPPORTED:   tiled_rsc = "Optional (No)";            break;
            case D3D11_TILED_RESOURCES_TIER_1:          tiled_rsc = "Optional (Yes - Tier 1)";  break;
            case D3D11_TILED_RESOURCES_TIER_2:          tiled_rsc = "Optional (Yes - Tier 2)";  break;
            case D3D11_TILED_RESOURCES_TIER_3:          tiled_rsc = "Optional (Yes - Tier 3)";  break;
            default:                                    tiled_rsc = "Optional (Yes)";           break;
            }

            consrv_rast = rast_ordered_views = ps_stencil_ref = minmaxfilter = c_szNo;
            mapdefaultbuff = bMapDefaultBuff ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11_2)
        {
            D3D11_TILED_RESOURCES_TIER tiled = D3D11_TILED_RESOURCES_NOT_SUPPORTED;
            bool bMinMaxFilter, bMapDefaultBuff; 
            CheckD3D11Ops1( pD3D11_2, tiled, bMinMaxFilter, bMapDefaultBuff );

            switch (tiled)
            {
            case D3D11_TILED_RESOURCES_NOT_SUPPORTED:   tiled_rsc = "Optional (No)";            break;
            case D3D11_TILED_RESOURCES_TIER_1:          tiled_rsc = "Optional (Yes - Tier 1)";  break;
            case D3D11_TILED_RESOURCES_TIER_2:          tiled_rsc = "Optional (Yes - Tier 2)";  break;
            default:                                    tiled_rsc = "Optional (Yes)";           break;
            }

            minmaxfilter = c_szNo;
            mapdefaultbuff = bMapDefaultBuff ? "Optional (Yes)" : "Optional (No)";
        }
        
        if (pD3D11_1 || pD3D11_2 || pD3D11_3)
        {
            ID3D11Device* pD3D = (pD3D11_3) ? pD3D11_3 : ( (pD3D11_2) ? pD3D11_2 : pD3D11_1 );

            bool bLogicOps, bCBpartial, bCBoffsetting;
            CheckD3D11Ops( pD3D, bLogicOps, bCBpartial, bCBoffsetting );
            logic_ops = bLogicOps ? "Optional (Yes)" : "Optional (No)";
            cb_partial = bCBpartial ? "Optional (Yes)" : "Optional (No)";
            cb_offsetting = bCBoffsetting  ? "Optional (Yes)" : "Optional (No)";

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D, ext, x2, bpp565 );
            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            uavSlots = "8";
            uavEveryStage = c_szNo;
            uavOnlyRender = "8";
            nonpow2 = "Full";
        }
        break;

    case D3D_FEATURE_LEVEL_10_1:
        shaderModel = "4.x";
        instancing = c_szYes;

        if (pD3D11_3)
        {
            consrv_rast = rast_ordered_views = ps_stencil_ref = c_szNo;
        }

        if (pD3D11_2 || pD3D11_3)
        {
            tiled_rsc = minmaxfilter = mapdefaultbuff = c_szNo;
        }

        if (pD3D11_1 || pD3D11_2 || pD3D11_3)
        {
            ID3D11Device* pD3D = (pD3D11_3) ? pD3D11_3 : ( (pD3D11_2) ? pD3D11_2 : pD3D11_1 );

            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10xhw;
            HRESULT hr = pD3D->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10xhw, sizeof(d3d10xhw) );
            if ( FAILED(hr) )
                memset( &d3d10xhw, 0, sizeof(d3d10xhw) );

            if (d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
            {
                computeShader = "Optional (Yes - CS 4.x)";
                uavSlots = "1";
                uavEveryStage = c_szNo;
                uavOnlyRender = c_szNo;
            }
            else
            {
                computeShader = "Optional (No)";
            }

            bool bLogicOps, bCBpartial, bCBoffsetting;
            CheckD3D11Ops( pD3D, bLogicOps, bCBpartial, bCBoffsetting );
            logic_ops = bLogicOps ? "Optional (Yes)" : "Optional (No)";
            cb_partial = bCBpartial ? "Optional (Yes)" : "Optional (No)";
            cb_offsetting = bCBoffsetting  ? "Optional (Yes)" : "Optional (No)";

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D, ext, x2, bpp565 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
            nonpow2 = "Full";
            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11 != NULL)
        {
            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10xhw;
            HRESULT hr = pD3D11->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10xhw, sizeof(d3d10xhw) );
            if ( FAILED(hr) )
                memset( &d3d10xhw, 0, sizeof(d3d10xhw) );

            computeShader = (d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x) ? "Optional (Yes - CS 4.x)": "Optional (No)";

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D11, ext, x2, bpp565 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
        }
        else if ( pD3D10_1 != NULL )
        {
            BOOL ext, x2;
            CheckExtendedFormats( pD3D10_1, ext, x2 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
        }
        else if ( pD3D10 != NULL )
        {
            BOOL ext, x2;
            CheckExtendedFormats( pD3D10, ext, x2 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
        }

        maxTexDim = XTOSTRING( D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING( D3D10_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING( D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING( D3D10_REQ_FILTERING_HW_ADDRESSABLE_RESOURCE_DIMENSION );
        maxAnisotropy = XTOSTRING( D3D10_REQ_MAXANISOTROPY );
        maxInputSlots = XTOSTRING( D3D10_1_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING( D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT );
        break;
        
    case D3D_FEATURE_LEVEL_10_0:
        shaderModel = "4.0";
        instancing = c_szYes;

        if (pD3D11_3)
        {
            consrv_rast = rast_ordered_views = ps_stencil_ref = c_szNo;
        }

        if (pD3D11_2 || pD3D11_3)
        {
            tiled_rsc = minmaxfilter = mapdefaultbuff = c_szNo;
        }

        if (pD3D11_1 || pD3D11_2 || pD3D11_3)
        {
            ID3D11Device* pD3D = (pD3D11_3) ? pD3D11_3 : ( (pD3D11_2) ? pD3D11_2 : pD3D11_1 );

            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10xhw;
            HRESULT hr = pD3D->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10xhw, sizeof(d3d10xhw) );
            if ( FAILED(hr) )
                memset( &d3d10xhw, 0, sizeof(d3d10xhw) );

            if (d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
            {
                computeShader = "Optional (Yes - CS 4.x)";
                uavSlots = "1";
                uavEveryStage = c_szNo;
                uavOnlyRender = c_szNo;
            }
            else
            {
                computeShader = "Optional (No)";
            }

            bool bLogicOps, bCBpartial, bCBoffsetting;
            CheckD3D11Ops( pD3D, bLogicOps, bCBpartial, bCBoffsetting );
            logic_ops = bLogicOps ? "Optional (Yes)" : "Optional (No)";
            cb_partial = bCBpartial ? "Optional (Yes)" : "Optional (No)";
            cb_offsetting = bCBoffsetting  ? "Optional (Yes)" : "Optional (No)";

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D, ext, x2, bpp565 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
            nonpow2 = "Full";
            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11 != NULL)
        {
            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10xhw;
            HRESULT hr = pD3D11->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10xhw, sizeof(d3d10xhw) );
            if ( FAILED(hr) )
                memset( &d3d10xhw, 0, sizeof(d3d10xhw) );

            computeShader = (d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x) ? "Optional (Yes - CS 4.0)": "Optional (No)";

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D11, ext, x2, bpp565 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
        }
        else if ( pD3D10_1 != NULL )
        {
            BOOL ext, x2;
            CheckExtendedFormats( pD3D10_1, ext, x2 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
        }
        else if ( pD3D10 != NULL )
        {
            BOOL ext, x2;
            CheckExtendedFormats( pD3D10, ext, x2 );

            extFormats = (ext) ? "Optional (Yes)" : "Optional (No)";
            x2_10BitFormat = (x2) ? "Optional (Yes)" : "Optional (No)";
        }

        maxTexDim = XTOSTRING( D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING( D3D10_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING( D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING( D3D10_REQ_FILTERING_HW_ADDRESSABLE_RESOURCE_DIMENSION );
        maxAnisotropy = XTOSTRING( D3D10_REQ_MAXANISOTROPY );
        maxInputSlots = XTOSTRING( D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING( D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT );
        break;

    case D3D_FEATURE_LEVEL_9_3:
        _10level9 = TRUE;
        shaderModel = "2.0 (4_0_level_9_3) [vs_2_a/ps_2_b]";
        computeShader = c_szNA;
        maxTexDim = XTOSTRING2( D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING2( D3D_FL9_3_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING2( D3D_FL9_3_MAX_TEXTURE_REPEAT );
        maxAnisotropy = XTOSTRING( D3D11_REQ_MAXANISOTROPY );
        maxPrimCount = XTOSTRING2( D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT );
        maxInputSlots = XTOSTRING( D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING2( D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT );
        extFormats = c_szYes;
        instancing = c_szYes;

        if (pD3D11_2 || pD3D11_3)
        {
            ID3D11Device* pD3D = (pD3D11_3) ? pD3D11_3 : pD3D11_2;

            cb_partial = c_szYes;
            cb_offsetting = c_szYes;

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D, ext, x2, bpp565 );

            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            bool bNonPow2, bShadows, bInstancing, bCubeRT;
            CheckD3D9Ops1( pD3D, bNonPow2, bShadows, bInstancing, bCubeRT );
            nonpow2 = bNonPow2 ? "Optional (Full)" : "Conditional";
            shadows = bShadows ? "Optional (Yes)" : "Optional (No)";
            cubeRT = bCubeRT ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11_1)
        {
            cb_partial = c_szYes;
            cb_offsetting = c_szYes;

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D11_1, ext, x2, bpp565 );

            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            bool bNonPow2, bShadows;
            CheckD3D9Ops( pD3D11_1, bNonPow2, bShadows );
            nonpow2 = bNonPow2 ? "Optional (Full)" : "Conditional";
            shadows = bShadows ? "Optional (Yes)" : "Optional (No)";
        }
        break;

    case D3D_FEATURE_LEVEL_9_2:
        _10level9 = TRUE;
        shaderModel = "2.0 (4_0_level_9_1)";
        computeShader = c_szNA;
        maxTexDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING2( D3D_FL9_2_MAX_TEXTURE_REPEAT );
        maxAnisotropy = XTOSTRING( D3D11_REQ_MAXANISOTROPY );
        maxPrimCount = XTOSTRING2( D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT );
        maxInputSlots = XTOSTRING( D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING2( D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT );
        extFormats = c_szYes;
        instancing = c_szNo;

        if (pD3D11_2 || pD3D11_3)
        {
            ID3D11Device* pD3D = (pD3D11_3) ? pD3D11_3 : pD3D11_2;

            cb_partial = c_szYes;
            cb_offsetting = c_szYes;

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D, ext, x2, bpp565 );

            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            bool bNonPow2, bShadows, bInstancing, bCubeRT;
            CheckD3D9Ops1( pD3D, bNonPow2, bShadows, bInstancing, bCubeRT );
            nonpow2 = bNonPow2 ? "Optional (Full)" : "Conditional";
            shadows = bShadows ? "Optional (Yes)" : "Optional (No)";
            instancing = bInstancing ? "Optional (Simple)" : "Optional (No)";
            cubeRT = bCubeRT ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11_1)
        {
            cb_partial = c_szYes;
            cb_offsetting = c_szYes;

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D11_1, ext, x2, bpp565 );

            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            bool bNonPow2, bShadows;
            CheckD3D9Ops( pD3D11_1, bNonPow2, bShadows );
            nonpow2 = bNonPow2 ? "Optional (Full)" : "Conditional";
            shadows = bShadows ? "Optional (Yes)" : "Optional (No)";
        }
        break;

    case D3D_FEATURE_LEVEL_9_1:
        _10level9 = TRUE;
        shaderModel = "2.0 (4_0_level_9_1)";
        computeShader = c_szNA;
        maxTexDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION );
        maxCubeDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION );
        maxVolDim = XTOSTRING2( D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION );
        maxTexRepeat = XTOSTRING2( D3D_FL9_1_MAX_TEXTURE_REPEAT );
        maxAnisotropy = XTOSTRING2( D3D_FL9_1_DEFAULT_MAX_ANISOTROPY );
        maxPrimCount = XTOSTRING2( D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT );
        maxInputSlots = XTOSTRING( D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        mrt = XTOSTRING2( D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT );
        extFormats = c_szYes;
        instancing = c_szNo;

        if (pD3D11_2 || pD3D11_3)
        {
            ID3D11Device* pD3D = (pD3D11_3) ? pD3D11_3 : pD3D11_2;

            cb_partial = c_szYes;
            cb_offsetting = c_szYes;

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D, ext, x2, bpp565 );

            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            bool bNonPow2, bShadows, bInstancing, bCubeRT;
            CheckD3D9Ops1( pD3D, bNonPow2, bShadows, bInstancing, bCubeRT );
            nonpow2 = bNonPow2 ? "Optional (Full)" : "Conditional";
            shadows = bShadows ? "Optional (Yes)" : "Optional (No)";
            instancing = bInstancing ? "Optional (Simple)" : "Optional (No)";
            cubeRT = bCubeRT ? "Optional (Yes)" : "Optional (No)";
        }
        else if (pD3D11_1)
        {
            cb_partial = c_szYes;
            cb_offsetting = c_szYes;

            BOOL ext, x2, bpp565;
            CheckExtendedFormats( pD3D11_1, ext, x2, bpp565 );

            bpp16 = (bpp565) ? "Optional (Yes)" : "Optional (No)";

            bool bNonPow2, bShadows;
            CheckD3D9Ops( pD3D11_1, bNonPow2, bShadows );
            nonpow2 = bNonPow2 ? "Optional (Full)" : "Conditional";
            shadows = bShadows ? "Optional (Yes)" : "Optional (No)";
        }
        break;

    default:
        return E_FAIL;
    }

    if ( d3dType == D3D_DRIVER_TYPE_WARP )
    {
        maxTexDim = ( pD3D11_3 || pD3D11_2 || pD3D11_1 ) ? "16777216" : "65536";
    }

    if ( pPrintInfo == NULL )
    {
        LVLINE( "Shader Model", shaderModel )
        LVYESNO( "Geometry Shader", (fl >= D3D_FEATURE_LEVEL_10_0) )
        LVYESNO( "Stream Out", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( pD3D11_3 || pD3D11_2 || pD3D11_1 || pD3D11 )
        {
            if ( g_dwViewState == IDM_VIEWALL || computeShader != c_szNo )
            {
                LVLINE( "DirectCompute", computeShader )
            }

            LVYESNO( "Hull & Domain Shaders", (fl >= D3D_FEATURE_LEVEL_11_0) )
        }

        LVYESNO( "Texture Resource Arrays", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( d3dVer != 0 )
        {
            LVYESNO( "Cubemap Resource Arrays", (fl >= D3D_FEATURE_LEVEL_10_1) )
        }

        LVYESNO( "BC4/BC5 Compression", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( pD3D11_3 || pD3D11_2 || pD3D11_1 || pD3D11 )
        {
            LVYESNO( "BC6H/BC7 Compression", (fl >= D3D_FEATURE_LEVEL_11_0) )
        }

        LVYESNO( "Alpha-to-coverage", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( pD3D11_1 || pD3D11_2 || pD3D11_3 )
        {
            LVLINE( "Logic Ops (Output Merger)", logic_ops );
            LVLINE( "Constant Buffer Partial Updates", cb_partial );
            LVLINE( "Constant Buffer Offsetting", cb_offsetting );

            if (uavEveryStage)
            {
                LVLINE( "UAVs at Every Stage", uavEveryStage )
            }

            if (uavOnlyRender)
            {
                LVLINE( "UAV-only rendering", uavOnlyRender );
            }
        }

        if ( pD3D11_2 || pD3D11_3 )
        {
            if (tiled_rsc)
            {
                LVLINE( "Tiled Resources", tiled_rsc );
            }

            if (minmaxfilter)
            {
                LVLINE( "Min/Max Filtering", minmaxfilter );
            }

            if (mapdefaultbuff)
            {
                LVLINE( "Map DEFAULT Buffers", mapdefaultbuff );
            }
        }

        if ( pD3D11_3 )
        {
            if (consrv_rast)
            {
                LVLINE( "Conservative Rasterization", consrv_rast );
            }

            if (ps_stencil_ref)
            {
                LVLINE( "PS-Specified Stencil Ref ", ps_stencil_ref );
            }

            if (rast_ordered_views)
            {
                LVLINE( "Rasterizer Ordered Views", rast_ordered_views );
            }
        }

        if ( g_DXGIFactory1 )
        {
            LVLINE( "Extended Formats (BGRA, etc.)", extFormats )

            if ( x2_10BitFormat && (g_dwViewState == IDM_VIEWALL || x2_10BitFormat != c_szNo))
            {
                LVLINE( "10-bit XR High Color Format", x2_10BitFormat )
            }
        }

        if ( pD3D11_1 || pD3D11_2 || pD3D11_3 )
        {
            LVLINE( "16-bit Formats (565/5551/4444)", bpp16 );
        }

        if ( nonpow2 )
        {
            LVLINE( "Non-Power-of-2 Textures", nonpow2 );
        }

        LVLINE( "Max Texture Dimension", maxTexDim )
        LVLINE( "Max Cubemap Dimension", maxCubeDim )

        if ( maxVolDim )
        {
            LVLINE( "Max Volume Extent", maxVolDim )
        }

        if ( maxTexRepeat )
        {
            LVLINE( "Max Texture Repeat", maxTexRepeat )
        }

        LVLINE( "Max Input Slots", maxInputSlots )

        if ( uavSlots )
        {
            LVLINE( "UAV Slots", uavSlots )
        }

        if (maxAnisotropy)
        {
            LVLINE( "Max Anisotropy", maxAnisotropy )
        }

        LVLINE( "Max Primitive Count", maxPrimCount )

        if (mrt)
        {
            LVLINE( "Simultaneous Render Targets", mrt )
        }

        if ( _10level9 )
        {
            LVYESNO( "Occlusion Queries", (fl >= D3D_FEATURE_LEVEL_9_2) )
            LVYESNO( "Separate Alpha Blend", (fl >= D3D_FEATURE_LEVEL_9_2) )
            LVYESNO( "Mirror Once", (fl >= D3D_FEATURE_LEVEL_9_2) )
            LVYESNO( "Overlapping Vertex Elements", (fl >= D3D_FEATURE_LEVEL_9_2) )
            LVYESNO( "Independant Write Masks", (fl >= D3D_FEATURE_LEVEL_9_3) )

            LVLINE( "Instancing", instancing )

            if ( pD3D11_1 || pD3D11_2 || pD3D11_3 )
            {
                LVLINE( "Shadow Support", shadows )
            }

            if ( pD3D11_2 || pD3D11_3 )
            {
                LVLINE( "Cubemap Render w/ non-Cube Depth", cubeRT );
            }
        }

        LVLINE( "Note", FL_NOTE )
    }
    else
    {
        PRINTLINE( "Shader Model", shaderModel )
        PRINTYESNO( "Geometry Shader", (fl >= D3D_FEATURE_LEVEL_10_0) )
        PRINTYESNO( "Stream Out", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( pD3D11 || pD3D11_1 || pD3D11_2 || pD3D11_3 )
        {
            PRINTLINE( "DirectCompute", computeShader )
            PRINTYESNO( "Hull & Domain Shaders", (fl >= D3D_FEATURE_LEVEL_11_0) )
        }

        PRINTYESNO( "Texture Resource Arrays", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( d3dVer != 0 )
        {
            PRINTYESNO( "Cubemap Resource Arrays", (fl >= D3D_FEATURE_LEVEL_10_1) )
        }

        PRINTYESNO( "BC4/BC5 Compression", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( pD3D11_3 || pD3D11_2 || pD3D11_1 || pD3D11 )
        {
            PRINTYESNO( "BC6H/BC7 Compression", (fl >= D3D_FEATURE_LEVEL_11_0) )
        }

        PRINTYESNO( "Alpha-to-coverage", (fl >= D3D_FEATURE_LEVEL_10_0) )

        if ( pD3D11_1 || pD3D11_2 || pD3D11_3 )
        {
            PRINTLINE( "Logic Ops (Output Merger)", logic_ops );
            PRINTLINE( "Constant Buffer Partial Updates", cb_partial );
            PRINTLINE( "Constant Buffer Offsetting", cb_offsetting );

            if (uavEveryStage)
            {
                PRINTLINE( "UAVs at Every Stage", uavEveryStage )
            }

            if (uavOnlyRender)
            {
                PRINTLINE( "UAV-only rendering", uavOnlyRender );
            }
        }

        if ( pD3D11_2 || pD3D11_3 )
        {
            if (tiled_rsc)
            {
                PRINTLINE( "Tiled Resources", tiled_rsc );
            }

            if (minmaxfilter)
            {
                PRINTLINE( "Min/Max Filtering", minmaxfilter );
            }

            if (mapdefaultbuff)
            {
                PRINTLINE( "Map DEFAULT Buffers", mapdefaultbuff );
            }
        }

        if ( pD3D11_3 )
        {
            if (consrv_rast)
            {
                PRINTLINE( "Conservative Rasterization", consrv_rast );
            }

            if (ps_stencil_ref)
            {
                PRINTLINE( "PS-Specified Stencil Ref ", ps_stencil_ref );
            }

            if (rast_ordered_views)
            {
                PRINTLINE( "Rasterizer Ordered Views", rast_ordered_views );
            }
        }

        if ( g_DXGIFactory1 )
        {
            PRINTLINE( "Extended Formats (BGRA, etc.)", extFormats )

            if (x2_10BitFormat)
            {
                PRINTLINE( "10-bit XR High Color Format", x2_10BitFormat )
            }
        }

        if ( pD3D11_1 || pD3D11_2 || pD3D11_3 )
        {
            PRINTLINE( "16-bit Formats (565/5551/4444)", bpp16 );
        }

        if ( nonpow2 )
        {
            PRINTLINE( "Non-Power-of-2 Textures", nonpow2 );
        }

        PRINTLINE( "Max Texture Dimension", maxTexDim )
        PRINTLINE( "Max Cubemap Dimension", maxCubeDim )

        if ( maxVolDim )
        {
            PRINTLINE( "Max Volume Extent", maxVolDim )
        }

        if ( maxTexRepeat )
        {
            PRINTLINE( "Max Texture Repeat", maxTexRepeat )
        }

        PRINTLINE( "Max Input Slots", maxInputSlots )

        if ( uavSlots )
        {
            PRINTLINE( "UAV Slots", uavSlots )
        }

        if (maxAnisotropy)
        {
            PRINTLINE( "Max Anisotropy", maxAnisotropy )
        }

        PRINTLINE( "Max Primitive Count", maxPrimCount )

        if (mrt)
        {
            PRINTLINE( "Simultaneous Render Targets", mrt )
        }

        if ( _10level9 )
        {
            PRINTYESNO( "Occlusion Queries", (fl >= D3D_FEATURE_LEVEL_9_2) )
            PRINTYESNO( "Separate Alpha Blend", (fl >= D3D_FEATURE_LEVEL_9_2) )
            PRINTYESNO( "Mirror Once", (fl >= D3D_FEATURE_LEVEL_9_2) )
            PRINTYESNO( "Overlapping Vertex Elements", (fl >= D3D_FEATURE_LEVEL_9_2) )
            PRINTYESNO( "Independant Write Masks", (fl >= D3D_FEATURE_LEVEL_9_3) )

            PRINTLINE( "Instancing", instancing )

            if ( pD3D11_1 || pD3D11_2 || pD3D11_3 )
            {
                PRINTLINE( "Shadow Support", shadows )
            }

            if ( pD3D11_2 || pD3D11_3 )
            {
                PRINTLINE( "Cubemap Render w/ non-Cube Depth", cubeRT );
            }
        }

        PRINTLINE( "Note", FL_NOTE )
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT D3D10Info( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D10Device* pDevice = (ID3D10Device*)lParam1;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);

        if ( lParam2 == D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            LVAddColumn(g_hwndLV, 1, "Value", 25);
            LVAddColumn(g_hwndLV, 2, "Quality Level", 25);
        }
        else
        {
            LVAddColumn(g_hwndLV, 1, "Value", 60);
        }
    }

    if ( !lParam2 )
    {
        // General Direct3D 10.0 device information

        BOOL ext, x2;
        CheckExtendedFormats( pDevice, ext, x2 );

        if ( pPrintInfo == NULL)
        {
            if ( g_DXGIFactory1 != NULL )
            {
                LVYESNO( "Extended Formats (BGRA, etc.)", ext )
                LVYESNO( "10-bit XR High Color Format", x2 )
            }

            LVLINE( "Note", D3D10_NOTE )
        }
        else
        {
            if ( g_DXGIFactory1 != NULL )
            {
                PRINTYESNO( "Extended Formats (BGRA, etc.)", ext )
                PRINTYESNO( "10-bit XR High Color Format", x2 )
            }

            PRINTLINE( "Note", D3D10_NOTE )
        }

        return S_OK;
    }

    // Use CheckFormatSupport for format usage defined as optional for Direct3D 10 devices
    static const DXGI_FORMAT cfsShaderSample[] =
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    };

    static const DXGI_FORMAT cfsMipAutoGen[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsRenderTarget[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT
    };

    static const DXGI_FORMAT cfsBlendableRT[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_R16_UNORM
    };

    UINT count = 0;
    const DXGI_FORMAT *array = NULL;
    BOOL skips = FALSE;

    switch( lParam2 )
    {
    case D3D10_FORMAT_SUPPORT_SHADER_SAMPLE:
        count = sizeof(cfsShaderSample) / sizeof(DXGI_FORMAT);
        array = cfsShaderSample;
        break;

    case D3D10_FORMAT_SUPPORT_MIP_AUTOGEN:
        count = sizeof(cfsMipAutoGen) / sizeof(DXGI_FORMAT);
        array = cfsMipAutoGen;
        break;

    case D3D10_FORMAT_SUPPORT_RENDER_TARGET:
        count = sizeof(cfsRenderTarget) / sizeof(DXGI_FORMAT);
        array = cfsRenderTarget;
        break;

    case D3D10_FORMAT_SUPPORT_BLENDABLE:
        count = sizeof(cfsBlendableRT) / sizeof(DXGI_FORMAT);
        array = cfsBlendableRT;
        break;

    case D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET:
        count = sizeof(g_cfsMSAA_10) / sizeof(DXGI_FORMAT);
        array = g_cfsMSAA_10;
        break;

    case D3D10_FORMAT_SUPPORT_MULTISAMPLE_LOAD:
        count = sizeof(g_cfsMSAA_10) / sizeof(DXGI_FORMAT);
        array = g_cfsMSAA_10;
        skips = TRUE;
        break;

    default:
        return E_FAIL;
    }

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = array[i];

        if (skips)
        {
            // Special logic to let us reuse the MSAA array twice with a few special cases that are skipped
            switch( fmt )
            {
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_D16_UNORM:
                continue;
            }
        }

        if ( lParam2 == D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( fmt, (UINT)lParam3, &quality );

            BOOL msaa = ( SUCCEEDED(hr) && quality > 0 ) != 0;

            if ( pPrintInfo == NULL )
            {
                if ( g_dwViewState == IDM_VIEWALL || msaa )
                {
                    LVAddText( g_hwndLV, 0, FormatName(fmt) );
                    LVAddText( g_hwndLV, 1, msaa ? c_szYes : c_szNo ); \

                    TCHAR strBuffer[ 16 ];
                    sprintf_s(strBuffer, 16, "%d", msaa ? quality : 0 );
                    LVAddText( g_hwndLV, 2, strBuffer );
                }
            }
            else
            {
                TCHAR strBuffer[ 32 ];
                if ( msaa )
                    sprintf_s(strBuffer, 32, "Yes (%d)", quality );
                else
                    strcpy_s( strBuffer, 32, c_szNo );

                PrintStringValueLine( FormatName(fmt), strBuffer, pPrintInfo );
            }
        }
        else
        {
            UINT fmtSupport = 0;
            HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
            if ( FAILED(hr) )
                fmtSupport = 0;
    
            if ( pPrintInfo == NULL)
            {
                LVYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
        }
    }

    return S_OK;
}

HRESULT D3D10Info1( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D10Device1* pDevice = (ID3D10Device1*)lParam1;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);

        if ( lParam2 == D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            LVAddColumn(g_hwndLV, 1, "Value", 25);
            LVAddColumn(g_hwndLV, 2, "Quality Level", 25);
        }
        else
        {
            LVAddColumn(g_hwndLV, 1, "Value", 60);
        }
    }

    if ( !lParam2 )
    {
        // General Direct3D 10.1 device information
        D3D10_FEATURE_LEVEL1 fl = pDevice->GetFeatureLevel();

        const char *szNote = NULL;

        switch ( fl )
        {
        case D3D10_FEATURE_LEVEL_10_0:
            szNote = SEE_D3D10;
            break;

        case D3D10_FEATURE_LEVEL_9_3:
        case D3D10_FEATURE_LEVEL_9_2:
        case D3D10_FEATURE_LEVEL_9_1:
            szNote = _10L9_NOTE;
            break;

        default:
            szNote = D3D10_NOTE1;
            break;
        }

        BOOL ext, x2;
        CheckExtendedFormats( pDevice, ext, x2 );

        if ( pPrintInfo == NULL)
        {
            LVLINE( "Feature Level", FLName(fl) )

            if ( g_DXGIFactory1 != NULL && (fl >= D3D10_FEATURE_LEVEL_10_0) )
            {
                LVYESNO( "Extended Formats (BGRA, etc.)", ext )
                LVYESNO( "10-bit XR High Color Format", x2 )
            }

            LVLINE( "Note", szNote )
        }
        else
        {
            PRINTLINE( "Feature Level", FLName(fl) )

            if ( g_DXGIFactory1 != NULL && (fl >= D3D10_FEATURE_LEVEL_10_0) )
            {
                PRINTYESNO( "Extended Formats (BGRA, etc.)", ext )
                PRINTYESNO( "10-bit XR High Color Format", x2 )
            }

            PRINTLINE( "Note", szNote )
        }

        return S_OK;
    }

    // Use CheckFormatSupport for format usage defined as optional for Direct3D 10.1 devices
    static const DXGI_FORMAT cfsShaderSample[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsMipAutoGen[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsRenderTarget[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT
    };

    // Most RT formats are required to support 4x MSAA for 10.1 devices
    static const DXGI_FORMAT cfsMSAA4x[] = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT,
    };

    UINT count = 0;
    const DXGI_FORMAT *array = NULL;

    switch( lParam2 )
    {
    case D3D10_FORMAT_SUPPORT_SHADER_SAMPLE:
        count = sizeof(cfsShaderSample) / sizeof(DXGI_FORMAT);
        array = cfsShaderSample;
        break;

    case D3D10_FORMAT_SUPPORT_MIP_AUTOGEN:
        count = sizeof(cfsMipAutoGen) / sizeof(DXGI_FORMAT);
        array = cfsMipAutoGen;
        break;

    case D3D10_FORMAT_SUPPORT_RENDER_TARGET:
        count = sizeof(cfsRenderTarget) / sizeof(DXGI_FORMAT);
        array = cfsRenderTarget;
        break;

    case D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET:
        if ( pDevice->GetFeatureLevel() <= D3D10_FEATURE_LEVEL_9_3 )
        {
            count = sizeof(g_cfsMSAA_10level9) / sizeof(DXGI_FORMAT);
            array = g_cfsMSAA_10level9;
        }
        else if ( g_dwViewState != IDM_VIEWALL && lParam3 == 4 )
        {
            count = sizeof(cfsMSAA4x) / sizeof(DXGI_FORMAT);
            array = cfsMSAA4x;
        }
        else
        {
            count = sizeof(g_cfsMSAA_10) / sizeof(DXGI_FORMAT);
            array = g_cfsMSAA_10;
        }
        break;

    default:
        return E_FAIL;
    }

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = array[i];

        if ( lParam2 == D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( fmt, (UINT)lParam3, &quality );

            BOOL msaa = ( SUCCEEDED(hr) && quality > 0 ) != 0;

            if ( pPrintInfo == NULL )
            {
                if ( g_dwViewState == IDM_VIEWALL || msaa )
                {
                    LVAddText( g_hwndLV, 0, FormatName(fmt) );
                    LVAddText( g_hwndLV, 1, msaa ? c_szYes : c_szNo ); \

                    TCHAR strBuffer[ 16 ];
                    sprintf_s(strBuffer, 16, "%d", msaa ? quality : 0  );
                    LVAddText( g_hwndLV, 2, strBuffer );
                }
            }
            else
            {
                TCHAR strBuffer[ 32 ];
                if ( msaa )
                    sprintf_s(strBuffer, 32, "Yes (%d)", quality );
                else
                    strcpy_s( strBuffer, 32, c_szNo );

                PrintStringValueLine( FormatName(fmt), strBuffer, pPrintInfo );
            }
        }
        else
        {
            UINT fmtSupport = 0;
            HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
            if ( FAILED(hr) )
                fmtSupport = 0;
    
            if ( pPrintInfo == NULL)
            {
                LVYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
        }
    }

    return S_OK;
}

HRESULT D3D10InfoMSAA( LPARAM lParam1, LPARAM lParam2, PRINTCBINFO* pPrintInfo )
{
    ID3D10Device* pDevice = (ID3D10Device*)lParam1;
    BOOL* sampCount = (BOOL*)lParam2;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);

        UINT column = 1;
        for( UINT samples = 2; samples <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
        {
            if (!sampCount[ samples - 1 ] && !IsMSAAPowerOf2(samples))
                continue;

            TCHAR strBuffer[ 8 ];
            sprintf_s( strBuffer, 8, "%dx", samples );
            LVAddColumn( g_hwndLV, column, strBuffer, 8 );
            ++column;
        }
    }

    const UINT count = sizeof(g_cfsMSAA_10) / sizeof(DXGI_FORMAT);

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = g_cfsMSAA_10[i];

        UINT sampQ[ D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT ];
        memset( sampQ, 0, sizeof(sampQ) );

        BOOL any = FALSE;
        for( UINT samples = 2; samples <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
        {
            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( fmt, samples, &quality );

            if ( SUCCEEDED(hr) && quality > 0 )
            {
                sampQ[ samples-1 ] = quality;
                any = TRUE;
            }
        }

        if ( pPrintInfo == NULL)
        {
            if ( g_dwViewState != IDM_VIEWALL && !any )
                continue;

            LVAddText( g_hwndLV, 0, FormatName(fmt) );

            UINT column = 1;
            for( UINT samples = 2; samples <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
            {
                if (!sampCount[ samples - 1 ] && !IsMSAAPowerOf2(samples))
                    continue;

                if ( sampQ[ samples-1 ] > 0 )
                {
                    TCHAR strBuffer[ 32 ];
                    sprintf_s(strBuffer, 32, "Yes (%d)", sampQ[ samples-1 ] );
                    LVAddText( g_hwndLV, column, strBuffer );
                }
                else
                    LVAddText( g_hwndLV, column, c_szNo );
 
                ++column;
            }
        }
        else
        {
            TCHAR buff[ 1024 ];
            *buff = 0;

            for( UINT samples = 2; samples <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
            {
                if (!sampCount[ samples - 1 ] && !IsMSAAPowerOf2(samples))
                    continue;
    
                TCHAR strBuffer[ 32 ];
                if ( sampQ[ samples-1 ] > 0 )
                    sprintf_s(strBuffer, 32, "%dx: Yes (%d)   ", samples, sampQ[ samples-1 ] );
                else
                    sprintf_s(strBuffer, 32, "%dx: No   ", samples );

                strcat_s( buff, 1024, strBuffer );
            }

            PrintStringValueLine( FormatName(fmt), buff, pPrintInfo );
        }
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
HRESULT D3D11Info( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D11Device* pDevice = (ID3D11Device*)lParam1;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);

        if ( lParam2 == D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            LVAddColumn(g_hwndLV, 1, "Value", 25);
            LVAddColumn(g_hwndLV, 2, "Quality Level", 25);
        }
        else
        {
            LVAddColumn(g_hwndLV, 1, "Value", 60);
        }
    }

    if ( !lParam2 )
    {
        // General Direct3D 11 device information
        D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();
        if ( fl > D3D_FEATURE_LEVEL_11_0 )
            fl = D3D_FEATURE_LEVEL_11_0;
    
        // CheckFeatureSupport
        D3D11_FEATURE_DATA_THREADING threading;
        HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_THREADING, &threading, sizeof(threading) );
        if ( FAILED(hr) )
            memset( &threading, 0, sizeof(threading) );
    
        D3D11_FEATURE_DATA_DOUBLES doubles;
        hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_DOUBLES, &doubles, sizeof(doubles) );
        if ( FAILED(hr) )
            memset( &doubles, 0, sizeof(doubles) );
    
        D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10xhw;
        hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10xhw, sizeof(d3d10xhw) );
        if ( FAILED(hr) )
            memset( &d3d10xhw, 0, sizeof(d3d10xhw) );

        // Setup note
        const char *szNote = NULL;

        switch ( fl )
        {
        case D3D_FEATURE_LEVEL_10_0:
            szNote = SEE_D3D10;
            break;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            szNote = SEE_D3D10_1;
            break;

        default:
            szNote = D3D11_NOTE;
            break;
        }

        if ( pPrintInfo == NULL)
        {
            LVLINE( "Feature Level", FLName(fl) )

            LVYESNO( "Driver Concurrent Creates", threading.DriverConcurrentCreates )
            LVYESNO( "Driver Command Lists", threading.DriverCommandLists )
            LVYESNO( "Double-precision Shaders", doubles.DoublePrecisionFloatShaderOps )
            LVYESNO( "DirectCompute CS 4.x", d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x )

            LVLINE( "Note", szNote )
        }
        else
        {
            PRINTLINE( "Feature Level", FLName(fl) )

            PRINTYESNO( "Driver Concurrent Creates", threading.DriverConcurrentCreates )
            PRINTYESNO( "Driver Command Lists", threading.DriverCommandLists )
            PRINTYESNO( "Double-precision Shaders", doubles.DoublePrecisionFloatShaderOps )
            PRINTYESNO( "DirectCompute CS 4.x", d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x )

            PRINTLINE( "Note", szNote )
        }

        return S_OK;
    }

    // Use CheckFormatSupport for format usage defined as optional for Direct3D 11 devices
    static const DXGI_FORMAT cfsShaderSample[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsShaderGather[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsMipAutoGen[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsRenderTarget[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT
    };

    // Most RT formats are required to support 8x MSAA for 11 devices
    static const DXGI_FORMAT cfsMSAA8x[] = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT
    };

    UINT count = 0;
    const DXGI_FORMAT *array = NULL;

    switch( lParam2 )
    {
    case D3D11_FORMAT_SUPPORT_SHADER_SAMPLE:
        count = sizeof(cfsShaderSample) / sizeof(DXGI_FORMAT);
        array = cfsShaderSample;
        break;

    case D3D11_FORMAT_SUPPORT_SHADER_GATHER:
        count = sizeof(cfsShaderGather) / sizeof(DXGI_FORMAT);
        array = cfsShaderGather;
        break;

    case D3D11_FORMAT_SUPPORT_MIP_AUTOGEN:
        count = sizeof(cfsMipAutoGen) / sizeof(DXGI_FORMAT);
        array = cfsMipAutoGen;
        break;

    case D3D11_FORMAT_SUPPORT_RENDER_TARGET:
        count = sizeof(cfsRenderTarget) / sizeof(DXGI_FORMAT);
        array = cfsRenderTarget;
        break;

    case D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET:
        if ( g_dwViewState != IDM_VIEWALL && lParam3 == 8 )
        {
            count = sizeof(cfsMSAA8x) / sizeof(DXGI_FORMAT);
            array = cfsMSAA8x;
        }
        else if ( g_dwViewState != IDM_VIEWALL && lParam3 == 4 )
        {
            count = 0;
        }
        else
        {
            count = _countof(g_cfsMSAA_11);
            array = g_cfsMSAA_11;
        }
        break;

    default:
        return E_FAIL;
    }

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = array[i];

        if ( lParam2 == D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            // Skip 16 bits-per-pixel formats for DX 11.0
            if ( fmt == DXGI_FORMAT_B5G6R5_UNORM || fmt == DXGI_FORMAT_B5G5R5A1_UNORM || fmt == DXGI_FORMAT_B4G4R4A4_UNORM )
                continue;

            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( fmt, (UINT)lParam3, &quality );

            BOOL msaa = ( SUCCEEDED(hr) && quality > 0 ) != 0;

            if ( pPrintInfo == NULL )
            {
                if ( g_dwViewState == IDM_VIEWALL || msaa )
                {
                    LVAddText( g_hwndLV, 0, FormatName(fmt) );
                    LVAddText( g_hwndLV, 1, msaa ? c_szYes : c_szNo ); \

                    TCHAR strBuffer[ 16 ];
                    sprintf_s(strBuffer, 16, "%d", msaa ? quality : 0 );
                    LVAddText( g_hwndLV, 2, strBuffer );
                }
            }
            else
            {
                TCHAR strBuffer[ 32 ];
                if ( msaa )
                    sprintf_s(strBuffer, 32, "Yes (%d)", quality );
                else
                    strcpy_s( strBuffer, 32, c_szNo );

                PrintStringValueLine( FormatName(fmt), strBuffer, pPrintInfo );
            }
        }
        else
        {
            UINT fmtSupport = 0;
            HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
            if ( FAILED(hr) )
                fmtSupport = 0;
            
            if ( pPrintInfo == NULL)
            {
               LVYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
        }
    }

    return S_OK;
}

static void D3D11FeatureSupportInfo1( ID3D11Device1* pDevice, bool bDev2, PRINTCBINFO* pPrintInfo )
{
    if ( !pDevice )
        return;

    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    // CheckFeatureSupport
    D3D11_FEATURE_DATA_THREADING threading;
    HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_THREADING, &threading, sizeof(threading) );
    if ( FAILED(hr) )
        memset( &threading, 0, sizeof(threading) );
    
    D3D11_FEATURE_DATA_DOUBLES doubles;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_DOUBLES, &doubles, sizeof(doubles) );
    if ( FAILED(hr) )
        memset( &doubles, 0, sizeof(doubles) );
    
    D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10xhw;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10xhw, sizeof(d3d10xhw) );
    if ( FAILED(hr) )
        memset( &d3d10xhw, 0, sizeof(d3d10xhw) );

    D3D11_FEATURE_DATA_D3D11_OPTIONS d3d11opts;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS, &d3d11opts, sizeof(d3d11opts) );
    if ( FAILED(hr) )
        memset( &d3d11opts, 0, sizeof(d3d11opts) );

    D3D11_FEATURE_DATA_D3D9_OPTIONS d3d9opts;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D9_OPTIONS, &d3d9opts, sizeof(d3d9opts) );
    if ( FAILED(hr) )
        memset( &d3d9opts, 0, sizeof(d3d9opts) );

    D3D11_FEATURE_DATA_ARCHITECTURE_INFO d3d11arch;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_ARCHITECTURE_INFO, &d3d11arch, sizeof(d3d11arch) );
    if ( FAILED(hr) )
        memset( &d3d11arch, 0, sizeof(d3d11arch) );

    D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT minprecis;
    hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT, &minprecis, sizeof(minprecis) );
    if ( FAILED(hr) )
        memset( &minprecis, 0, sizeof(minprecis) );

    D3D11_FEATURE_DATA_D3D11_OPTIONS1 d3d11opts1;
    memset( &d3d11opts1, 0, sizeof(d3d11opts1) );
    if ( bDev2 )
    {
        hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS1, &d3d11opts1, sizeof(d3d11opts1) );
        if ( FAILED(hr) )
            memset( &d3d11opts1, 0, sizeof(d3d11opts1) );
    }

    const char* clearview = nullptr;
    if ( d3d11opts.ClearView )
    {
        clearview = d3d11opts1.ClearViewAlsoSupportsDepthOnlyFormats ? "RTV, UAV, and Depth (Driver sees)" : "RTV and UAV (Driver sees)";
    }
    else
    {
        clearview = d3d11opts1.ClearViewAlsoSupportsDepthOnlyFormats ? "RTV, UAV, and Depth (Driver doesn't see)" : "RTV and UAV (Driver doesn't see)";
    }

    const char* double_shaders = NULL;
    if ( d3d11opts.ExtendedDoublesShaderInstructions )
    {
        double_shaders = "Extended";
    }
    else if ( doubles.DoublePrecisionFloatShaderOps )
    {
        double_shaders = c_szYes;
    }
    else
    {
        double_shaders = c_szNo;
    }

    const char* ps_precis = NULL;
    switch( minprecis.PixelShaderMinPrecision & (D3D11_SHADER_MIN_PRECISION_16_BIT|D3D11_SHADER_MIN_PRECISION_10_BIT) )
    {
    case 0:                                 ps_precis = "Full";         break;
    case D3D11_SHADER_MIN_PRECISION_16_BIT: ps_precis = "16/32-bit";    break;
    case D3D11_SHADER_MIN_PRECISION_10_BIT: ps_precis = "10/32-bit";    break;
    default:                                ps_precis = "10/16/32-bit"; break;
    }

    const char* other_precis = NULL;
    switch( minprecis.AllOtherShaderStagesMinPrecision & (D3D11_SHADER_MIN_PRECISION_16_BIT|D3D11_SHADER_MIN_PRECISION_10_BIT) )
    {
    case 0:                                 other_precis = "Full";      break;
    case D3D11_SHADER_MIN_PRECISION_16_BIT: other_precis = "16/32-bit"; break;
    case D3D11_SHADER_MIN_PRECISION_10_BIT: other_precis = "10/32-bit"; break;
    default:                                other_precis = "10/16/32-bit"; break;
    }

    const char* nonpow2 = (d3d9opts.FullNonPow2TextureSupport) ? "Full" : "Conditional";

    if ( !pPrintInfo )
    {
        LVLINE( "Feature Level", FLName(fl) )

        LVYESNO( "Driver Concurrent Creates", threading.DriverConcurrentCreates )
        LVYESNO( "Driver Command Lists", threading.DriverCommandLists )
        LVLINE( "Double-precision Shaders", double_shaders )
        LVYESNO( "DirectCompute CS 4.x", d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x )
            
        LVYESNO( "Driver sees DiscardResource/View", d3d11opts.DiscardAPIsSeenByDriver )
        LVYESNO( "Driver sees COPY_FLAGS", d3d11opts.FlagsForUpdateAndCopySeenByDriver )
        LVLINE( "ClearView", clearview )
        LVYESNO( "Copy w/ Overlapping Rect", d3d11opts.CopyWithOverlap )
        LVYESNO( "CB Partial Update", d3d11opts.ConstantBufferPartialUpdate )
        LVYESNO( "CB Offsetting", d3d11opts.ConstantBufferOffsetting )
        LVYESNO( "Map NO_OVERWRITE on Dynamic CB", d3d11opts.MapNoOverwriteOnDynamicConstantBuffer )

        if ( fl >= D3D_FEATURE_LEVEL_10_0 )
        {
            LVYESNO( "Map NO_OVERWRITE on Dynamic SRV", d3d11opts.MapNoOverwriteOnDynamicBufferSRV )
            LVYESNO( "MSAA with ForcedSampleCount=1", d3d11opts.MultisampleRTVWithForcedSampleCountOne )
            LVYESNO( "Extended resource sharing", d3d11opts.ExtendedResourceSharing );
        }

        if ( fl >= D3D_FEATURE_LEVEL_11_0 )
        {
            LVYESNO( "Saturating Add Instruction", d3d11opts.SAD4ShaderInstructions )
        }

        LVYESNO( "Tile-based Deferred Renderer", d3d11arch.TileBasedDeferredRenderer )

        LVLINE( "Non-Power-of-2 Textures", nonpow2 );

        LVLINE( "Pixel Shader Precision", ps_precis );
        LVLINE( "Other Stage Precision", other_precis );
    }
    else
    {
        PRINTLINE( "Feature Level", FLName(fl) )

        PRINTYESNO( "Driver Concurrent Creates", threading.DriverConcurrentCreates )
        PRINTYESNO( "Driver Command Lists", threading.DriverCommandLists )
        PRINTLINE( "Double-precision Shaders", double_shaders )
        PRINTYESNO( "DirectCompute CS 4.x", d3d10xhw.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x )

        PRINTYESNO( "Driver sees DiscardResource/View", d3d11opts.DiscardAPIsSeenByDriver )
        PRINTYESNO( "Driver sees COPY_FLAGS", d3d11opts.FlagsForUpdateAndCopySeenByDriver )
        PRINTLINE( "ClearView", clearview )
        PRINTYESNO( "Copy w/ Overlapping Rect", d3d11opts.CopyWithOverlap )
        PRINTYESNO( "CB Partial Update", d3d11opts.ConstantBufferPartialUpdate )
        PRINTYESNO( "CB Offsetting", d3d11opts.ConstantBufferOffsetting )
        PRINTYESNO( "Map NO_OVERWRITE on Dynamic CB", d3d11opts.MapNoOverwriteOnDynamicConstantBuffer )

        if ( fl >= D3D_FEATURE_LEVEL_10_0 )
        {
            PRINTYESNO( "Map NO_OVERWRITE on Dynamic SRV", d3d11opts.MapNoOverwriteOnDynamicBufferSRV )
            PRINTYESNO( "MSAA with ForcedSampleCount=1", d3d11opts.MultisampleRTVWithForcedSampleCountOne )
            PRINTYESNO( "Extended resource sharing", d3d11opts.ExtendedResourceSharing );
        }

        if ( fl >= D3D_FEATURE_LEVEL_11_0 )
        {
            PRINTYESNO( "Saturating Add Instruction", d3d11opts.SAD4ShaderInstructions )
        }

        PRINTYESNO( "Tile-based Deferred Renderer", d3d11arch.TileBasedDeferredRenderer )

        PRINTLINE( "Non-Power-of-2 Textures", nonpow2 );

        PRINTLINE( "Pixel Shader Precision", ps_precis );
        PRINTLINE( "Other Stage Precision", other_precis );
    }
}

HRESULT D3D11Info1( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D11Device1* pDevice = (ID3D11Device1*)lParam1;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);

        if ( lParam2 == D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            LVAddColumn(g_hwndLV, 1, "Value", 25);
            LVAddColumn(g_hwndLV, 2, "Quality Level", 25);
        }
        else
        {
            LVAddColumn(g_hwndLV, 1, "Value", 60);
        }
    }

    // General Direct3D 11.1 device information
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    if ( !lParam2 )
    {
        D3D11FeatureSupportInfo1( pDevice, false, pPrintInfo );

        // Setup note
        const char *szNote = NULL;

        switch ( fl )
        {
        case D3D_FEATURE_LEVEL_10_0:
            szNote = SEE_D3D10;
            break;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            szNote = SEE_D3D10_1;
            break;

        case D3D_FEATURE_LEVEL_11_0:
            szNote = SEE_D3D11;
            break;

        default:
            szNote = D3D11_NOTE1;
            break;
        }

        if ( pPrintInfo == NULL)
        {
            LVLINE( "Note", szNote )
        }
        else
        {
            PRINTLINE( "Note", szNote )
        }
        
        return S_OK;
    }

    // Use CheckFormatSupport for format usage defined as optional for Direct3D 11.1 devices

    static const DXGI_FORMAT cfs16bpp[] =
    {
        DXGI_FORMAT_B5G6R5_UNORM,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfs16bpp2[] =
    {
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfsShaderSample[] =
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    };

    static const DXGI_FORMAT cfsShaderSample10_1[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsShaderGather[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT
    };

    static const DXGI_FORMAT cfsMipAutoGen[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_B5G6R5_UNORM,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfsMipAutoGen11_1[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfsRenderTarget[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT,
        DXGI_FORMAT_B5G6R5_UNORM,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfsRenderTarget11_1[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfsBlendableRT[] =
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_B5G6R5_UNORM,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    static const DXGI_FORMAT cfsLogicOps[] = 
    {
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R16G16B16A16_UINT,
        DXGI_FORMAT_R32G32_UINT,
        DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R16G16_UINT,
        DXGI_FORMAT_R32_UINT,
        DXGI_FORMAT_R8G8_UINT,
        DXGI_FORMAT_R16_UINT,
        DXGI_FORMAT_R8_UINT,
    };

    // Most RT formats are required to support 8x MSAA for 11.x devices
    static const DXGI_FORMAT cfsMSAA8x[] = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_B5G6R5_UNORM,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B4G4R4A4_UNORM,
    };

    UINT count = 0;
    const DXGI_FORMAT *array = NULL;
    BOOL skips = FALSE;

    switch( lParam2 )
    {
    case D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER:
        count = sizeof(cfs16bpp) / sizeof(DXGI_FORMAT);
        array = cfs16bpp;
        break;

    case D3D11_FORMAT_SUPPORT_SHADER_SAMPLE:
        if ( fl >= D3D_FEATURE_LEVEL_10_1 )
        {
            count = sizeof(cfsShaderSample10_1) / sizeof(DXGI_FORMAT);
            array = cfsShaderSample10_1;
        }
        else
        {
            count = sizeof(cfsShaderSample) / sizeof(DXGI_FORMAT);
            array = cfsShaderSample;
        }
        break;

    case D3D11_FORMAT_SUPPORT_SHADER_GATHER:
        count = sizeof(cfsShaderGather) / sizeof(DXGI_FORMAT);
        array = cfsShaderGather;
        break;

    case D3D11_FORMAT_SUPPORT_MIP_AUTOGEN:
        if ( fl >= D3D_FEATURE_LEVEL_11_1 )
        {
            count = sizeof(cfsMipAutoGen11_1) / sizeof(DXGI_FORMAT);
            array = cfsMipAutoGen11_1;
        }
        else
        {
            count = sizeof(cfsMipAutoGen) / sizeof(DXGI_FORMAT);
            array = cfsMipAutoGen;
        }
        break;

    case D3D11_FORMAT_SUPPORT_RENDER_TARGET:
        if ( fl >= D3D_FEATURE_LEVEL_11_1 )
        {
            count = sizeof(cfsRenderTarget11_1) / sizeof(DXGI_FORMAT);
            array = cfsRenderTarget11_1;
        }
        else
        {
            count = sizeof(cfsRenderTarget) / sizeof(DXGI_FORMAT);
            array = cfsRenderTarget;
        }
        break;

    case D3D11_FORMAT_SUPPORT_BLENDABLE:
        if ( fl >= D3D_FEATURE_LEVEL_10_0 )
        {
            count = sizeof(cfs16bpp2) / sizeof(DXGI_FORMAT);
            array = cfs16bpp2;
        }
        else
        {
            count = sizeof(cfsBlendableRT) / sizeof(DXGI_FORMAT);
            array = cfsBlendableRT;
        }
        break;

    case D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET:
        if ( g_dwViewState != IDM_VIEWALL && lParam3 == 8 )
        {
            count = sizeof(cfsMSAA8x) / sizeof(DXGI_FORMAT);
            array = cfsMSAA8x;
        }
        else if ( g_dwViewState != IDM_VIEWALL && (lParam3 == 2 || lParam3 == 4))
        {
            count = sizeof(cfs16bpp) / sizeof(DXGI_FORMAT);
            array = cfs16bpp;
        }
        else
        {
            count = sizeof(g_cfsMSAA_11) / sizeof(DXGI_FORMAT);
            array = g_cfsMSAA_11;
        }
        break;

    case D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD:
        if ( fl >= D3D_FEATURE_LEVEL_10_1 )
        {
            count = sizeof(cfs16bpp2) / sizeof(DXGI_FORMAT);
            array = cfs16bpp2;
        }
        else if ( fl >= D3D_FEATURE_LEVEL_10_0 )
        {
            count = sizeof(cfs16bpp) / sizeof(DXGI_FORMAT);
            array = cfs16bpp;
        }
        else
        {
            count = sizeof(g_cfsMSAA_11) / sizeof(DXGI_FORMAT);
            array = g_cfsMSAA_11;
        }
        skips = TRUE;
        break;

    case D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW:
        count = sizeof(cfs16bpp) / sizeof(DXGI_FORMAT);
        array = cfs16bpp;
        break;

    case 0xffffffff: // D3D11_FORMAT_SUPPORT2
        switch( lParam3 )
        {
        case D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP:
            count = sizeof(cfsLogicOps) / sizeof(DXGI_FORMAT);
            array = cfsLogicOps;
            break;

        case D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE:
            count = sizeof(cfs16bpp) / sizeof(DXGI_FORMAT);
            array = cfs16bpp;
            break;

        default:
            return E_FAIL;
        }
        break;

    default:
        return E_FAIL;
    }

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = array[i];

        if (skips)
        {
            // Special logic to let us reuse the MSAA array twice with a few special cases that are skipped
            switch( fmt )
            {
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                continue;
            }
        }

        if ( lParam2 == D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET )
        {
            if ( g_dwViewState != IDM_VIEWALL && fmt == DXGI_FORMAT_B5G6R5_UNORM )
                continue;

            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( fmt, (UINT)lParam3, &quality );

            BOOL msaa = ( SUCCEEDED(hr) && quality > 0 ) != 0;

            if ( pPrintInfo == NULL )
            {
                if ( g_dwViewState == IDM_VIEWALL || msaa )
                {
                    LVAddText( g_hwndLV, 0, FormatName(fmt) );
                    LVAddText( g_hwndLV, 1, msaa ? c_szYes : c_szNo ); \

                    TCHAR strBuffer[ 16 ];
                    sprintf_s(strBuffer, 16, "%d", msaa ? quality : 0 );
                    LVAddText( g_hwndLV, 2, strBuffer );
                }
            }
            else
            {
                TCHAR strBuffer[ 32 ];
                if ( msaa )
                    sprintf_s(strBuffer, 32, "Yes (%d)", quality );
                else
                    strcpy_s( strBuffer, 32, c_szNo );

                PrintStringValueLine( FormatName(fmt), strBuffer, pPrintInfo );
            }
        }
        else if ( lParam2 != 0xffffffff )
        {
            UINT fmtSupport = 0;
            HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
            if ( FAILED(hr) )
                fmtSupport = 0;
            
            if ( pPrintInfo == NULL )
            {
               LVYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
        }
        else
        {
            D3D11_FEATURE_DATA_FORMAT_SUPPORT2 cfs2;
            cfs2.InFormat = fmt;
            cfs2.OutFormatSupport2 = 0;
            HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_FORMAT_SUPPORT2, &cfs2, sizeof(cfs2) );
            if ( FAILED(hr) )
                cfs2.OutFormatSupport2 = 0;

            if ( pPrintInfo == NULL )
            {
               LVYESNO( FormatName(fmt), cfs2.OutFormatSupport2 & (UINT)lParam3 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), cfs2.OutFormatSupport2 & (UINT)lParam3 );
            }
        }
    }

    return S_OK;
}

HRESULT D3D11Info2( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D11Device2* pDevice = reinterpret_cast<ID3D11Device2*>(lParam1);

    if ( !pDevice )
        return S_OK;

    if( !pPrintInfo )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 60);
    }

    // General Direct3D 11.2 device information
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    if ( !lParam2 )
    {
        D3D11FeatureSupportInfo1( pDevice, true, pPrintInfo );

        D3D11_FEATURE_DATA_MARKER_SUPPORT marker;
        HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_MARKER_SUPPORT, &marker, sizeof(marker) );
        if ( FAILED(hr) )
            memset( &marker, 0, sizeof(marker) );

        // Setup note
        const char *szNote = nullptr;

        switch ( fl )
        {
        case D3D_FEATURE_LEVEL_10_0:
            szNote = SEE_D3D10;
            break;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            szNote = SEE_D3D10_1;
            break;

        case D3D_FEATURE_LEVEL_11_0:
            szNote = SEE_D3D11;
            break;

        case D3D_FEATURE_LEVEL_11_1:
            szNote = SEE_D3D11_1;
            break;

        default:
            szNote = D3D11_NOTE2;
            break;
        }

        if ( !pPrintInfo )
        {
            LVYESNO( "Profile Marker Support", marker.Profile )

            LVLINE( "Note", szNote )
        }
        else
        {
            PRINTYESNO( "Profile Marker Support", marker.Profile )

            PRINTLINE( "Note", szNote )
        }
        
        return S_OK;
    }

    // Use CheckFormatSupport for format usage defined as optional for Direct3D 11.2 devices

    static const DXGI_FORMAT cfsShareable[] = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R16G16B16A16_UINT,
        DXGI_FORMAT_R16G16B16A16_SNORM,
        DXGI_FORMAT_R16G16B16A16_SINT,
        DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
        DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R8G8B8A8_SNORM,
        DXGI_FORMAT_R8G8B8A8_SINT,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R32_UINT,
        DXGI_FORMAT_R32_SINT,
        DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_R16_FLOAT,
        DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_R16_UINT,
        DXGI_FORMAT_R16_SNORM,
        DXGI_FORMAT_R16_SINT,
        DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_R8_UINT,
        DXGI_FORMAT_R8_SNORM, 
        DXGI_FORMAT_R8_SINT, 
        DXGI_FORMAT_A8_UNORM,
        DXGI_FORMAT_BC1_UNORM,
        DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT_BC2_UNORM,
        DXGI_FORMAT_BC2_UNORM_SRGB,
        DXGI_FORMAT_BC3_UNORM,
        DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8X8_UNORM,
        DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    };

    UINT count = 0;
    const DXGI_FORMAT *array = nullptr;

    switch( lParam2 )
    {
    case 0xffffffff: // D3D11_FORMAT_SUPPORT2
        switch( lParam3 )
        {
        case D3D11_FORMAT_SUPPORT2_SHAREABLE:
            count = _countof(cfsShareable);
            array = cfsShareable;
            break;

        default:
            return E_FAIL;
        }
        break;

    default:
        return E_FAIL;
    }

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = array[i];

        if ( lParam2 != 0xffffffff )
        {
            UINT fmtSupport = 0;
            HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
            if ( FAILED(hr) )
                fmtSupport = 0;
            
            if ( !pPrintInfo )
            {
               LVYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
        }
        else
        {
            D3D11_FEATURE_DATA_FORMAT_SUPPORT2 cfs2;
            cfs2.InFormat = fmt;
            cfs2.OutFormatSupport2 = 0;
            HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_FORMAT_SUPPORT2, &cfs2, sizeof(cfs2) );
            if ( FAILED(hr) )
                cfs2.OutFormatSupport2 = 0;

            if ( !pPrintInfo )
            {
               LVYESNO( FormatName(fmt), cfs2.OutFormatSupport2 & (UINT)lParam3 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), cfs2.OutFormatSupport2 & (UINT)lParam3 );
            }
        }
    }

    return S_OK;
}

HRESULT D3D11Info3( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D11Device3* pDevice = reinterpret_cast<ID3D11Device3*>(lParam1);

    if ( !pDevice )
        return S_OK;

    if( !pPrintInfo )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn(g_hwndLV, 1, "Value", 60);
    }

    // General Direct3D 11.3 device information
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    if ( !lParam2 )
    {
        D3D11FeatureSupportInfo1( pDevice, true, pPrintInfo );

        D3D11_FEATURE_DATA_MARKER_SUPPORT marker;
        HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_MARKER_SUPPORT, &marker, sizeof(marker) );
        if ( FAILED(hr) )
            memset( &marker, 0, sizeof(marker) );

        D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11opts2;
        hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS2, &d3d11opts2, sizeof(d3d11opts2) );
        if ( FAILED(hr) )
            memset( &d3d11opts2, 0, sizeof(d3d11opts2) );

        // Setup note
        const char *szNote = nullptr;

        switch ( fl )
        {
        case D3D_FEATURE_LEVEL_10_0:
            szNote = SEE_D3D10;
            break;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            szNote = SEE_D3D10_1;
            break;

        case D3D_FEATURE_LEVEL_11_0:
            szNote = SEE_D3D11;
            break;

        case D3D_FEATURE_LEVEL_11_1:
            szNote = SEE_D3D11_1;
            break;

        default:
            szNote = D3D11_NOTE3;
            break;
        }

        if ( !pPrintInfo )
        {
            LVYESNO( "Profile Marker Support", marker.Profile )

            LVYESNO( "Map DEFAULT Textures", d3d11opts2.MapOnDefaultTextures )
            LVYESNO( "Standard Swizzle", d3d11opts2.StandardSwizzle )
            LVYESNO( "UMA", d3d11opts2.UnifiedMemoryArchitecture )

            LVLINE( "Note", szNote )
        }
        else
        {
            PRINTYESNO( "Profile Marker Support", marker.Profile )

            PRINTYESNO( "Map DEFAULT Textures", d3d11opts2.MapOnDefaultTextures )
            PRINTYESNO( "Standard Swizzle", d3d11opts2.StandardSwizzle )
            PRINTYESNO( "UMA", d3d11opts2.UnifiedMemoryArchitecture )

            PRINTLINE( "Note", szNote )
        }
        
        return S_OK;
    }

    // Use CheckFormatSupport for format usage defined as optional for Direct3D 11.3 devices

    static const DXGI_FORMAT cfsUAVTypedLoad[] = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R16G16B16A16_UINT,
        DXGI_FORMAT_R16G16B16A16_SNORM,
        DXGI_FORMAT_R16G16B16A16_SINT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32_UINT,
        DXGI_FORMAT_R32G32_SINT,
        DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R11G11B10_FLOAT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R8G8B8A8_SNORM,
        DXGI_FORMAT_R8G8B8A8_SINT,
        DXGI_FORMAT_R16G16_FLOAT,
        DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_R16G16_UINT,
        DXGI_FORMAT_R16G16_SNORM,
        DXGI_FORMAT_R16G16_SINT,
        DXGI_FORMAT_R32_FLOAT, // req
        DXGI_FORMAT_R32_UINT, // req
        DXGI_FORMAT_R32_SINT, // req
        DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_R8G8_UINT,
        DXGI_FORMAT_R8G8_SNORM,
        DXGI_FORMAT_R8G8_SINT,
        DXGI_FORMAT_R16_FLOAT,
        DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_R16_UINT,
        DXGI_FORMAT_R16_SNORM,
        DXGI_FORMAT_R16_SINT,
        DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_R8_UINT,
        DXGI_FORMAT_R8_SNORM,
        DXGI_FORMAT_R8_SINT,
        DXGI_FORMAT_A8_UNORM,
    };

    UINT count = 0;
    const DXGI_FORMAT *array = nullptr;

    switch( lParam2 )
    {
    case 0xffffffff: // D3D11_FORMAT_SUPPORT2
        switch( lParam3 )
        {
        case D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD:
            count = _countof(cfsUAVTypedLoad);
            array = cfsUAVTypedLoad;
            break;

        default:
            return E_FAIL;
        }
        break;

    default:
        return E_FAIL;
    }

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = array[i];

        if ( lParam2 != 0xffffffff )
        {
            UINT fmtSupport = 0;
            HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
            if ( FAILED(hr) )
                fmtSupport = 0;
            
            if ( !pPrintInfo )
            {
               LVYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), fmtSupport & (UINT)lParam2 );
            }
        }
        else
        {
            D3D11_FEATURE_DATA_FORMAT_SUPPORT2 cfs2;
            cfs2.InFormat = fmt;
            cfs2.OutFormatSupport2 = 0;
            HRESULT hr = pDevice->CheckFeatureSupport( D3D11_FEATURE_FORMAT_SUPPORT2, &cfs2, sizeof(cfs2) );
            if ( FAILED(hr) )
                cfs2.OutFormatSupport2 = 0;

            if ( !pPrintInfo )
            {
               LVYESNO( FormatName(fmt), cfs2.OutFormatSupport2 & (UINT)lParam3 );
            }
            else
            {
                PRINTYESNO( FormatName(fmt), cfs2.OutFormatSupport2 & (UINT)lParam3 );
            }
        }
    }

    return S_OK;
}

HRESULT D3D11InfoMSAA( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D11Device* pDevice = (ID3D11Device*)lParam1;
    BOOL* sampCount = (BOOL*)lParam2;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);

        UINT column = 1;
        for( UINT samples = 2; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
        {
            if (!sampCount[ samples - 1 ] && !IsMSAAPowerOf2(samples))
                continue;

            TCHAR strBuffer[ 8 ];
            sprintf_s( strBuffer, 8, "%dx", samples );
            LVAddColumn( g_hwndLV, column, strBuffer, 8 );
            ++column;
        }
    }

    const UINT count = sizeof(g_cfsMSAA_11) / sizeof(DXGI_FORMAT);

    for(UINT i = 0; i < count; ++i)
    {
        DXGI_FORMAT fmt = g_cfsMSAA_11[i];

        // Skip 16 bits-per-pixel formats for DX 11.0
        if ( lParam3 == 0
             && ( fmt == DXGI_FORMAT_B5G6R5_UNORM || fmt == DXGI_FORMAT_B5G5R5A1_UNORM || fmt == DXGI_FORMAT_B4G4R4A4_UNORM ) )
            continue;

        UINT sampQ[ D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT ];
        memset( sampQ, 0, sizeof(sampQ) );

        BOOL any = FALSE;
        for( UINT samples = 2; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
        {
            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( fmt, samples, &quality );

            if ( SUCCEEDED(hr) && quality > 0 )
            {
                sampQ[ samples-1 ] = quality;
                any = TRUE;
            }
        }

        if ( pPrintInfo == NULL)
        {
            if ( g_dwViewState != IDM_VIEWALL && !any )
                continue;

            LVAddText( g_hwndLV, 0, FormatName(fmt) );

            UINT column = 1;
            for( UINT samples = 2; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
            {
                if (!sampCount[ samples - 1 ] && !IsMSAAPowerOf2(samples))
                    continue;
    
                if ( sampQ[ samples-1 ] > 0 )
                {
                    TCHAR strBuffer[ 32 ];
                    sprintf_s(strBuffer, 32, "Yes (%d)", sampQ[ samples-1 ] );
                    LVAddText( g_hwndLV, column, strBuffer );
                }
                else
                    LVAddText( g_hwndLV, column, c_szNo );

                ++column;
            }
        }
        else
        {
            TCHAR buff[ 1024 ];
            *buff = 0;

            for( UINT samples = 2; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
            {
                if (!sampCount[ samples - 1 ] && !IsMSAAPowerOf2(samples))
                    continue;
    
                TCHAR strBuffer[ 32 ];
                if ( sampQ[ samples-1 ] > 0 )
                    sprintf_s(strBuffer, 32, "%dx: Yes (%d)   ", samples, sampQ[ samples-1 ] );
                else
                    sprintf_s(strBuffer, 32, "%dx: No   ", samples );

                strcat_s( buff, 1024, strBuffer );
            }

            PrintStringValueLine( FormatName(fmt), buff, pPrintInfo );
        }
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
static void FillMSAASampleTable( ID3D10Device* pDevice, BOOL sampCount[], BOOL _10level9 )
{
    memset( sampCount, 0, sizeof(BOOL) * D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT );
    sampCount[0] = TRUE;    // sample count of 1 is always required

    UINT count = 0;
    const DXGI_FORMAT *array = NULL;

    if ( _10level9 )
    {
        count = sizeof(g_cfsMSAA_10level9) / sizeof(DXGI_FORMAT);
        array = g_cfsMSAA_10level9;
    }
    else
    {
        count = sizeof(g_cfsMSAA_10) / sizeof(DXGI_FORMAT);
        array = g_cfsMSAA_10;
    }

    for( UINT samples = 2; samples <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples )
    {
        for(UINT i = 0; i < count; ++i)
        {
            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( array[i], samples, &quality );

            if ( SUCCEEDED(hr) && quality > 0 )
            {
                sampCount[ samples - 1 ] = TRUE;
                break;
            }
        }
    }
}

static void FillMSAASampleTable( ID3D11Device* pDevice, BOOL sampCount[] )
{
    memset( sampCount, 0, sizeof(BOOL) * D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT );
    sampCount[0] = TRUE;    // sample count of 1 is always required

    for( UINT samples = 2; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
        const UINT count = sizeof(g_cfsMSAA_11) / sizeof(DXGI_FORMAT);

        for(UINT i = 0; i < count; ++i)
        {
            UINT quality;
            HRESULT hr = pDevice->CheckMultisampleQualityLevels( g_cfsMSAA_11[i], samples, &quality );

            if ( SUCCEEDED(hr) && quality > 0 )
            {
                sampCount[ samples - 1 ] = TRUE;
                break;
            }
        }
    }
}


//-----------------------------------------------------------------------------
HRESULT D3D11InfoVideo( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3, PRINTCBINFO* pPrintInfo )
{
    ID3D11Device* pDevice = (ID3D11Device*)lParam1;
    BOOL* sampCount = (BOOL*)lParam2;

    if ( pDevice == NULL )
        return S_OK;

    if( pPrintInfo == NULL )
    {
        LVAddColumn(g_hwndLV, 0, "Name", 30);
        LVAddColumn( g_hwndLV, 1, "Texture2D", 15 );
        LVAddColumn( g_hwndLV, 2, "Input", 15 );
        LVAddColumn( g_hwndLV, 3, "Output", 15 );
        LVAddColumn( g_hwndLV, 4, "Encoder", 15 );
    }

    static const DXGI_FORMAT cfsVideo[] = 
    {
        DXGI_FORMAT_NV12,
        DXGI_FORMAT_420_OPAQUE,
        DXGI_FORMAT_YUY2,
        DXGI_FORMAT_AYUV,
        DXGI_FORMAT_Y410,
        DXGI_FORMAT_Y416,
        DXGI_FORMAT_P010,
        DXGI_FORMAT_P016,
        DXGI_FORMAT_Y210,
        DXGI_FORMAT_Y216,
        DXGI_FORMAT_NV11,
        DXGI_FORMAT_AI44,
        DXGI_FORMAT_IA44,
        DXGI_FORMAT_P8,
        DXGI_FORMAT_A8P8,
    };

    for(UINT i = 0; i < _countof(cfsVideo); ++i)
    {
        DXGI_FORMAT fmt = cfsVideo[i];

        UINT fmtSupport = 0;
        HRESULT hr = pDevice->CheckFormatSupport( fmt, &fmtSupport );
        if ( FAILED(hr) )
            fmtSupport = 0;

        bool any = ( fmtSupport & ( D3D11_FORMAT_SUPPORT_TEXTURE2D
                                    | D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT 
                                    | D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT
                                    | D3D11_FORMAT_SUPPORT_VIDEO_ENCODER ) ) ? true : false;

        if ( pPrintInfo == NULL)
        {
            if ( g_dwViewState != IDM_VIEWALL && !any )
                continue;

            LVAddText( g_hwndLV, 0, FormatName(fmt) );

            LVAddText( g_hwndLV, 1, (fmtSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? c_szYes : c_szNo );
            LVAddText( g_hwndLV, 2, (fmtSupport & D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT) ? c_szYes : c_szNo );
            LVAddText( g_hwndLV, 3, (fmtSupport & D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT) ? c_szYes : c_szNo );
            LVAddText( g_hwndLV, 4, (fmtSupport & D3D11_FORMAT_SUPPORT_VIDEO_ENCODER) ? c_szYes : c_szNo );
        }
        else
        {
            TCHAR buff[ 1024 ];

            sprintf_s( buff, "Texture2D: %-3s   Input: %-3s   Output: %-3s   Encoder: %-3s",
                             (fmtSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? c_szYes : c_szNo,
                             (fmtSupport & D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT) ? c_szYes : c_szNo,
                             (fmtSupport & D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT) ? c_szYes : c_szNo,
                             (fmtSupport & D3D11_FORMAT_SUPPORT_VIDEO_ENCODER) ? c_szYes : c_szNo );

            PrintStringValueLine( FormatName(fmt), buff, pPrintInfo );
        }
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
static void D3D10_FillTree( HTREEITEM hTree, ID3D10Device* pDevice, D3D_DRIVER_TYPE devType )
{
    HTREEITEM hTreeD3D = TVAddNodeEx( hTree, "Direct3D 10", TRUE, IDI_CAPS, D3D10Info,
                                    (LPARAM)pDevice, 0, 0 );

    TVAddNodeEx( hTreeD3D, "Features", FALSE, IDI_CAPS, D3D_FeatureLevel,
                 (LPARAM)D3D_FEATURE_LEVEL_10_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D10( devType ) );

    TVAddNodeEx( hTreeD3D, "Shader sample (any filter)", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_SHADER_SAMPLE, 0 );

    TVAddNodeEx( hTreeD3D, "Mipmap Auto-Generation", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MIP_AUTOGEN, 0 );

    TVAddNodeEx( hTreeD3D, "Render Target", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_RENDER_TARGET, 0 );

    TVAddNodeEx( hTreeD3D, "Blendable Render Target", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_BLENDABLE, 0 );

    // MSAA
    FillMSAASampleTable( pDevice, g_sampCount10, FALSE );

    TVAddNodeEx( hTreeD3D, "2x MSAA", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 2 );

    TVAddNodeEx( hTreeD3D, "4x MSAA", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 4 );

    TVAddNodeEx( hTreeD3D, "8x MSAA", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 8 );

    TVAddNode( hTreeD3D, "Other MSAA", FALSE, IDI_CAPS, D3D10InfoMSAA, (LPARAM)pDevice, (LPARAM)g_sampCount10 );

    TVAddNodeEx( hTreeD3D, "MSAA Load", FALSE, IDI_CAPS, D3D10Info,
                 (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_LOAD, 0 );
}

static void D3D10_FillTree1( HTREEITEM hTree, ID3D10Device1* pDevice, DWORD flMask, D3D_DRIVER_TYPE devType )
{
    D3D10_FEATURE_LEVEL1 fl = pDevice->GetFeatureLevel();

    HTREEITEM hTreeD3D = TVAddNodeEx( hTree, "Direct3D 10.1", TRUE,
                                      IDI_CAPS, D3D10Info1, (LPARAM)pDevice, 0, 0 );

    TVAddNodeEx( hTreeD3D, FLName( fl ), FALSE, IDI_CAPS, D3D_FeatureLevel, (LPARAM)fl, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D10_1( devType ) );

    if ( (g_DXGIFactory1 != NULL && fl != D3D10_FEATURE_LEVEL_9_1)
         || (g_DXGIFactory1 == NULL && fl != D3D10_FEATURE_LEVEL_10_0) )
    {
        HTREEITEM hTreeF = TVAddNode( hTreeD3D, "Additional Feature Levels", TRUE, IDI_CAPS, NULL, 0, 0 );
    
        switch( fl )
        {
        case D3D10_FEATURE_LEVEL_10_1:
            if ( flMask & FLMASK_10_0 )
            {
                TVAddNodeEx( hTreeF, "D3D10_FEATURE_LEVEL_10_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D10_1( devType ) );
            }
            // Fall thru
    
        case D3D10_FEATURE_LEVEL_10_0:
            if ( flMask & FLMASK_9_3 )
            {
                TVAddNodeEx( hTreeF, "D3D10_FEATURE_LEVEL_9_3", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_3, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D10_1( devType ) );
            }
            // Fall thru
    
        case D3D10_FEATURE_LEVEL_9_3:
            if ( flMask & FLMASK_9_2 )
            {
                TVAddNodeEx( hTreeF, "D3D10_FEATURE_LEVEL_9_2", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_2, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D10_1( devType ) );
            }
            // Fall thru
    
        case D3D10_FEATURE_LEVEL_9_2:
            if ( flMask & FLMASK_9_1 )
            {
                TVAddNodeEx( hTreeF, "D3D10_FEATURE_LEVEL_9_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D10_1( devType ) );
            }
            break;
        }
    }

    // Only display for 10.1 devices. 10 and 11 devices are handled in their "native" node
    // Nothing optional about 10level9 feature levels
    if ( fl == D3D10_FEATURE_LEVEL_10_1 )
    {
        TVAddNodeEx( hTreeD3D, "Shader sample (any filter)", FALSE, IDI_CAPS, D3D10Info1,
                     (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_SHADER_SAMPLE, 0 );

        TVAddNodeEx( hTreeD3D, "Mipmap Auto-Generation", FALSE, IDI_CAPS, D3D10Info1,
                     (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MIP_AUTOGEN, 0 );

        TVAddNodeEx( hTreeD3D, "Render Target", FALSE, IDI_CAPS, D3D10Info1,
                     (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_RENDER_TARGET, 0 );
    }

    // MSAA (for all but 10 devices, which are handled in their "native" node)
    if ( fl != D3D10_FEATURE_LEVEL_10_0 )
    {
        FillMSAASampleTable( pDevice, g_sampCount10_1, (fl != D3D10_FEATURE_LEVEL_10_1) );

        if ( fl == D3D10_FEATURE_LEVEL_10_1 )
        {
            TVAddNodeEx( hTreeD3D, "2x MSAA", FALSE, IDI_CAPS, D3D10Info1,
                         (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 2 );

            TVAddNodeEx( hTreeD3D, "4x MSAA (most required)", FALSE, IDI_CAPS, D3D10Info1,
                         (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 4 );

            TVAddNodeEx( hTreeD3D, "8x MSAA", FALSE, IDI_CAPS, D3D10Info1,
                         (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 8 );

            TVAddNode( hTreeD3D, "Other MSAA", FALSE, IDI_CAPS, D3D10InfoMSAA, (LPARAM)pDevice, (LPARAM)g_sampCount10_1 );
        }
        else // 10level9
        {
            TVAddNodeEx( hTreeD3D, "2x MSAA", FALSE, IDI_CAPS, D3D10Info1,
                         (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 2 );

            if ( fl >= D3D10_FEATURE_LEVEL_9_3 )
            {
                TVAddNodeEx( hTreeD3D, "4x MSAA", FALSE, IDI_CAPS, D3D10Info1,
                             (LPARAM)pDevice, (LPARAM)D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 4 );
            }
        }
    }
}


//-----------------------------------------------------------------------------
static void D3D11_FillTree( HTREEITEM hTree, ID3D11Device* pDevice, DWORD flMask, D3D_DRIVER_TYPE devType )
{
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();
    if ( fl > D3D_FEATURE_LEVEL_11_0 )
        fl = D3D_FEATURE_LEVEL_11_0;

    HTREEITEM hTreeD3D = TVAddNodeEx( hTree, "Direct3D 11", fl != D3D_FEATURE_LEVEL_9_1,
                                      IDI_CAPS, D3D11Info, (LPARAM)pDevice, 0, 0 );

    TVAddNodeEx( hTreeD3D, FLName( fl ), FALSE, IDI_CAPS, D3D_FeatureLevel,
                 (LPARAM)fl, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11( devType ) );

    if ( fl != D3D_FEATURE_LEVEL_9_1 )
    {
        HTREEITEM hTreeF = TVAddNode( hTreeD3D, "Additional Feature Levels", TRUE, IDI_CAPS, NULL, 0, 0 );
    
        switch( fl )
        {
        case D3D_FEATURE_LEVEL_11_0:
            if ( flMask & FLMASK_10_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_10_1:
            if ( flMask & FLMASK_10_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_10_0:
            if ( flMask & FLMASK_9_3 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_3", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_3, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_3:
            if ( flMask & FLMASK_9_2 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_2", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_2, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_2:
            if ( flMask & FLMASK_9_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11( devType ) );
            }
            break;
        }
    }

    if ( fl >= D3D_FEATURE_LEVEL_11_0 )
    {
        TVAddNodeEx( hTreeD3D, "Shader sample (any filter)", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_SHADER_SAMPLE, 0 );

        TVAddNodeEx( hTreeD3D, "Shader gather4", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_SHADER_GATHER, 0 );

        TVAddNodeEx( hTreeD3D, "Mipmap Auto-Generation", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MIP_AUTOGEN, 0 );

        TVAddNodeEx( hTreeD3D, "Render Target", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_RENDER_TARGET, 0 );

        // MSAA (MSAA data for 10level9 is shown under the 10.1 node)
        FillMSAASampleTable( pDevice, g_sampCount11 );

        TVAddNodeEx( hTreeD3D, "2x MSAA", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 2 );

        TVAddNodeEx( hTreeD3D, "4x MSAA (all required)", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 4 );

        TVAddNodeEx( hTreeD3D, "8x MSAA (most required)", FALSE, IDI_CAPS, D3D11Info,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 8 );

        TVAddNodeEx( hTreeD3D, "Other MSAA", FALSE, IDI_CAPS, D3D11InfoMSAA,
                     (LPARAM)pDevice, (LPARAM)g_sampCount11, 0 );
    }    
}

static void D3D11_FillTree1( HTREEITEM hTree, ID3D11Device1* pDevice, DWORD flMask, D3D_DRIVER_TYPE devType )
{
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    HTREEITEM hTreeD3D = TVAddNodeEx( hTree, "Direct3D 11.1", fl != D3D_FEATURE_LEVEL_9_1,
                                      IDI_CAPS, D3D11Info1, (LPARAM)pDevice, 0, 0 );

    TVAddNodeEx( hTreeD3D, FLName( fl ), FALSE, IDI_CAPS, D3D_FeatureLevel,
                 (LPARAM)fl, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );

    if ( fl != D3D_FEATURE_LEVEL_9_1 )
    {
        HTREEITEM hTreeF = TVAddNode( hTreeD3D, "Additional Feature Levels", TRUE, IDI_CAPS, NULL, 0, 0 );
    
        switch( fl )
        {
        case D3D_FEATURE_LEVEL_11_1:
            if( flMask & FLMASK_11_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_11_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_11_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_11_0:
            if ( flMask & FLMASK_10_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_10_1:
            if ( flMask & FLMASK_10_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_10_0:
            if ( flMask & FLMASK_9_3 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_3", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_3, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_3:
            if ( flMask & FLMASK_9_2 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_2", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_2, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_2:
            if ( flMask & FLMASK_9_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_1( devType ) );
            }
            break;
        }
    }

    if ( fl >= D3D_FEATURE_LEVEL_10_0 )
    {
        TVAddNodeEx( hTreeD3D, "IA Vertex Buffer", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER, 0 );

        TVAddNodeEx( hTreeD3D, "Shader sample (any filter)", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_SHADER_SAMPLE, 0 );

        if ( fl >= D3D_FEATURE_LEVEL_11_0 )
        {
            TVAddNodeEx( hTreeD3D, "Shader gather4", FALSE, IDI_CAPS, D3D11Info1,
                         (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_SHADER_GATHER, 0 );
        }

        TVAddNodeEx( hTreeD3D, "Mipmap Auto-Generation", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MIP_AUTOGEN, 0 );

        TVAddNodeEx( hTreeD3D, "Render Target", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_RENDER_TARGET, 0 );

        TVAddNodeEx( hTreeD3D, "Blendable Render Target", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_BLENDABLE, 0 );

        if ( fl < D3D_FEATURE_LEVEL_11_1 )
        {
            // This is required for 11.1, but is optional for 10.x and 11.0
            TVAddNodeEx( hTreeD3D, "OM Logic Ops", FALSE, IDI_CAPS, D3D11Info1,
                         (LPARAM)pDevice, (LPARAM)0xffffffff, (LPARAM)D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP );
        }

        if ( fl >= D3D_FEATURE_LEVEL_11_0 )
        {
            TVAddNodeEx( hTreeD3D, "Typed UAV (most required)", FALSE, IDI_CAPS, D3D11Info1,
                         (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW, 0 );

            TVAddNodeEx( hTreeD3D, "UAV Typed Store (most required)", FALSE, IDI_CAPS, D3D11Info1,
                         (LPARAM)pDevice, (LPARAM)0xffffffff, (LPARAM)D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE );
        }

        // MSAA (MSAA data for 10level9 is shown under the 10.1 node)
        FillMSAASampleTable( pDevice, g_sampCount11_1 );

        TVAddNodeEx( hTreeD3D, "2x MSAA", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 2 );

        TVAddNodeEx( hTreeD3D, "4x MSAA (all required)", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 4 );

        TVAddNodeEx( hTreeD3D, "8x MSAA (most required)", FALSE, IDI_CAPS, D3D11Info1,
                    (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET, 8 );

        TVAddNodeEx( hTreeD3D, "Other MSAA", FALSE, IDI_CAPS, D3D11InfoMSAA,
                     (LPARAM)pDevice, (LPARAM)g_sampCount11_1, 1 );

        TVAddNodeEx( hTreeD3D, "MSAA Load", FALSE, IDI_CAPS, D3D11Info1,
                     (LPARAM)pDevice, (LPARAM)D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD, 0 );
    }

    if ( devType != D3D_DRIVER_TYPE_REFERENCE )
    {
        TVAddNodeEx( hTreeD3D, "Video", FALSE, IDI_CAPS, D3D11InfoVideo, (LPARAM)pDevice, 0, 1 );
    }
}

static void D3D11_FillTree2( HTREEITEM hTree, ID3D11Device2* pDevice, DWORD flMask, D3D_DRIVER_TYPE devType )
{
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    HTREEITEM hTreeD3D = TVAddNodeEx( hTree, "Direct3D 11.2", fl != D3D_FEATURE_LEVEL_9_1,
                                      IDI_CAPS, D3D11Info2, (LPARAM)pDevice, 0, 0 );

    TVAddNodeEx( hTreeD3D, FLName( fl ), FALSE, IDI_CAPS, D3D_FeatureLevel,
                 (LPARAM)fl, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );

    if ( fl != D3D_FEATURE_LEVEL_9_1 )
    {
        HTREEITEM hTreeF = TVAddNode( hTreeD3D, "Additional Feature Levels", TRUE, IDI_CAPS, NULL, 0, 0 );
    
        switch( fl )
        {
        case D3D_FEATURE_LEVEL_11_1:
            if( flMask & FLMASK_11_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_11_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_11_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_11_0:
            if ( flMask & FLMASK_10_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_10_1:
            if ( flMask & FLMASK_10_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_10_0:
            if ( flMask & FLMASK_9_3 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_3", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_3, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_3:
            if ( flMask & FLMASK_9_2 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_2", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_2, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_2:
            if ( flMask & FLMASK_9_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_2( devType ) );
            }
            break;
        }
    }

    // The majority of this data is already shown under the DirectX 11.1 node, so we only show the 'new' info

    TVAddNodeEx( hTreeD3D, "Shareable", FALSE, IDI_CAPS, D3D11Info2,
                    (LPARAM)pDevice, (LPARAM)0xffffffff, (LPARAM)D3D11_FORMAT_SUPPORT2_SHAREABLE );
}

static void D3D11_FillTree3( HTREEITEM hTree, ID3D11Device3* pDevice, DWORD flMask, D3D_DRIVER_TYPE devType )
{
    D3D_FEATURE_LEVEL fl = pDevice->GetFeatureLevel();

    HTREEITEM hTreeD3D = TVAddNodeEx( hTree, "Direct3D 11.3", fl != D3D_FEATURE_LEVEL_9_1,
                                      IDI_CAPS, D3D11Info3, (LPARAM)pDevice, 0, 0 );

    TVAddNodeEx( hTreeD3D, FLName( fl ), FALSE, IDI_CAPS, D3D_FeatureLevel,
                 (LPARAM)fl, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );

    if ( fl != D3D_FEATURE_LEVEL_9_1 )
    {
        HTREEITEM hTreeF = TVAddNode( hTreeD3D, "Additional Feature Levels", TRUE, IDI_CAPS, NULL, 0, 0 );
    
        switch( fl )
        {
        case D3D_FEATURE_LEVEL_11_1:
            if( flMask & FLMASK_11_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_11_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_11_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_11_0:
            if ( flMask & FLMASK_10_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );
            }
            // Fall thru

        case D3D_FEATURE_LEVEL_10_1:
            if ( flMask & FLMASK_10_0 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_10_0", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_10_0, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_10_0:
            if ( flMask & FLMASK_9_3 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_3", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_3, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_3:
            if ( flMask & FLMASK_9_2 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_2", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_2, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );
            }
            // Fall thru
    
        case D3D_FEATURE_LEVEL_9_2:
            if ( flMask & FLMASK_9_1 )
            {
                TVAddNodeEx( hTreeF, "D3D_FEATURE_LEVEL_9_1", FALSE, IDI_CAPS, D3D_FeatureLevel,
                             (LPARAM)D3D_FEATURE_LEVEL_9_1, (LPARAM)pDevice, D3D_FL_LPARAM3_D3D11_3( devType ) );
            }
            break;
        }
    }

    // The majority of this data is already shown under the DirectX 11.1 or 11.2 node, so we only show the 'new' info

    TVAddNodeEx( hTreeD3D, "UAV Typed Load", FALSE, IDI_CAPS, D3D11Info3,
                    (LPARAM)pDevice, (LPARAM)0xffffffff, (LPARAM)D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD );

    // TODO - Hardware JPEG information
}


//-----------------------------------------------------------------------------
// Name: DXGI_FillTree()
//-----------------------------------------------------------------------------
VOID DXGI_FillTree( HWND hwndTV )
{
    HRESULT hr;

    if ( g_DXGIFactory == NULL )
        return;

    HTREEITEM hTree = TVAddNode( TVI_ROOT, "DXGI Devices", TRUE, IDI_DIRECTX, NULL, 0, 0 );

    // Hardware driver types
    IDXGIAdapter *pAdapter = NULL;
    IDXGIAdapter1 *pAdapter1 = NULL;
    IDXGIAdapter2 *pAdapter2 = nullptr;
    for(UINT iAdapter = 0; ; ++iAdapter)
    {
        if ( g_DXGIFactory1 != NULL )
        {
            hr = g_DXGIFactory1->EnumAdapters1( iAdapter, &pAdapter1 );
            pAdapter = pAdapter1;

            if ( SUCCEEDED(hr) )
            {
                HRESULT hr2 = pAdapter1->QueryInterface( __uuidof(IDXGIAdapter2), ( LPVOID* )&pAdapter2 );
                if ( FAILED(hr2) )
                    pAdapter2 = nullptr;
            }
        }
        else
        {
            hr = g_DXGIFactory->EnumAdapters( iAdapter, &pAdapter );
        }

        if ( FAILED(hr) )
            break;

        if ( pAdapter2 )
        {
            DXGI_ADAPTER_DESC2 aDesc2;
            pAdapter2->GetDesc2( &aDesc2 );

            if ( aDesc2.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
            {
                // Skip "always there" Microsoft Basics Display Driver
                pAdapter2->Release();
                pAdapter1->Release();
                continue;
            }
        }

        DXGI_ADAPTER_DESC aDesc;
        pAdapter->GetDesc( &aDesc );

        char szDesc[128];
        wcstombs( szDesc, aDesc.Description, 128 );

        HTREEITEM hTreeA;
        if ( pAdapter2 )
        {
            hTreeA = TVAddNode(hTree, szDesc, TRUE, IDI_CAPS, DXGIAdapterInfo2, iAdapter, (LPARAM)( pAdapter2 ));
        }
        else if ( pAdapter1 )
        {
            hTreeA = TVAddNode(hTree, szDesc, TRUE, IDI_CAPS, DXGIAdapterInfo1, iAdapter, (LPARAM)( pAdapter1 ));
        }
        else
        {
            hTreeA = TVAddNode(hTree, szDesc, TRUE, IDI_CAPS, DXGIAdapterInfo, iAdapter, (LPARAM)( pAdapter ));
        }

        // Outputs
        HTREEITEM hTreeO = NULL;
        
        IDXGIOutput* pOutput = NULL;
        for(UINT iOutput = 0; ; ++iOutput)
        {
            hr = pAdapter->EnumOutputs( iOutput, &pOutput );

            if ( FAILED(hr) )
                break;

            if ( iOutput == 0 )
            {
                hTreeO = TVAddNode(hTreeA, "Outputs", TRUE, IDI_CAPS, NULL, 0, 0 );
            }

            DXGI_OUTPUT_DESC oDesc;
            pOutput->GetDesc( &oDesc );

            char szDeviceName[32];
            wcstombs( szDeviceName, oDesc.DeviceName, 32 );

            HTREEITEM hTreeD = TVAddNode( hTreeO, szDeviceName, TRUE, IDI_CAPS, DXGIOutputInfo, iOutput, (LPARAM)pOutput );

            TVAddNode( hTreeD, "Display Modes", FALSE, IDI_CAPS, DXGIOutputModes, iOutput, (LPARAM)pOutput);
        }

        // Direct3D 10.x
#ifdef EXTRA_DEBUG
        OutputDebugStringA( "Direct3D 10.x\n" );
#endif
        ID3D10Device* pDevice10 = NULL;
        ID3D10Device1* pDevice10_1 = NULL;
        DWORD flMaskDX10 = 0;
        if ( g_D3D10CreateDevice1 != NULL )
        {
            // Since 10 & 10.1 are so close, try to create just one device object for both...
            D3D10_FEATURE_LEVEL1 lvl[] = { D3D10_FEATURE_LEVEL_10_1, D3D10_FEATURE_LEVEL_10_0,
                                           D3D10_FEATURE_LEVEL_9_3, D3D10_FEATURE_LEVEL_9_2, D3D10_FEATURE_LEVEL_9_1 };

            // Test every feature-level since some devices might be missing some
            D3D10_FEATURE_LEVEL1 flHigh = (D3D10_FEATURE_LEVEL1)0;
            for(UINT i=0; i < (sizeof(lvl) / sizeof(D3D10_FEATURE_LEVEL1)); ++i)
            {
                if ( g_DXGIFactory1 == 0 )
                {
                    // Skip 10level9 if using DXGI 1.0
                    if ( lvl[i] == D3D10_FEATURE_LEVEL_9_1
                         || lvl[i] == D3D10_FEATURE_LEVEL_9_2
                         || lvl[i] == D3D10_FEATURE_LEVEL_9_3 )
                        continue;
                }

#ifdef EXTRA_DEBUG
                OutputDebugString( FLName( lvl[i] ) );
#endif

                hr = g_D3D10CreateDevice1( pAdapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, lvl[i], D3D10_1_SDK_VERSION, &pDevice10_1 );
                if ( SUCCEEDED(hr) )
                {
#ifdef EXTRA_DEBUG
                    OutputDebugString( ": Success\n");
#endif

                    switch( lvl[i] )
                    {
                    case D3D10_FEATURE_LEVEL_9_1:  flMaskDX10 |= FLMASK_9_1; break;
                    case D3D10_FEATURE_LEVEL_9_2:  flMaskDX10 |= FLMASK_9_2; break;
                    case D3D10_FEATURE_LEVEL_9_3:  flMaskDX10 |= FLMASK_9_3; break;
                    case D3D10_FEATURE_LEVEL_10_0: flMaskDX10 |= FLMASK_10_0; break;
                    case D3D10_FEATURE_LEVEL_10_1: flMaskDX10 |= FLMASK_10_1; break;
                    }

                    if ( lvl[i] > flHigh )
                        flHigh = lvl[i];
                }
                else
                {
#ifdef EXTRA_DEBUG
                    OutputDebugString( ": Failed\n");
#endif

                    pDevice10_1 = NULL;
                }

                if ( pDevice10_1 )
                {
                    pDevice10_1->Release();
                    pDevice10_1 = NULL;
                }
            }

            if (flHigh > 0)
            {
                hr = g_D3D10CreateDevice1( pAdapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, flHigh, D3D10_1_SDK_VERSION, &pDevice10_1 );
                if ( SUCCEEDED(hr) )
                {
                    if ( flHigh >= D3D10_FEATURE_LEVEL_10_0 )
                    {
                        hr = pDevice10_1->QueryInterface( __uuidof( ID3D10Device ), ( LPVOID* )&pDevice10 );
                        if ( FAILED(hr) )
                            pDevice10 = NULL;
                    }
                }
                else
                {
                    pDevice10_1 = NULL;
                }
            }
        }
        else if ( g_D3D10CreateDevice != NULL )
        {
            hr = g_D3D10CreateDevice( pAdapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &pDevice10 );
            if ( FAILED(hr) )
                pDevice10 = NULL;
        }

        // Direct3D 10
        if ( pDevice10 != NULL )
            D3D10_FillTree( hTreeA, pDevice10, D3D_DRIVER_TYPE_HARDWARE );

        // Direct3D 10.1 (includes 10level9 feature levels)
        if ( pDevice10_1 != NULL )
            D3D10_FillTree1( hTreeA, pDevice10_1, flMaskDX10, D3D_DRIVER_TYPE_HARDWARE );

        // Direct3D 11.x
#ifdef EXTRA_DEBUG
        OutputDebugStringA( "Direct3D 11.x\n" );
#endif

        ID3D11Device* pDevice11 = NULL;
        ID3D11Device1* pDevice11_1 = NULL;
        ID3D11Device2* pDevice11_2 = nullptr;
        ID3D11Device3* pDevice11_3 = nullptr;
        DWORD flMaskDX11 = 0;
        if (pAdapter1 != NULL && g_D3D11CreateDevice != NULL)
        {
            D3D_FEATURE_LEVEL lvl[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                        D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                                        D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };

            D3D_FEATURE_LEVEL flHigh = (D3D_FEATURE_LEVEL)0;

            for(UINT i=0; i < (sizeof(lvl) / sizeof(D3D_FEATURE_LEVEL)); ++i)
            {
#ifdef EXTRA_DEBUG
                OutputDebugString( FLName( lvl[i] ) );
#endif
                hr = g_D3D11CreateDevice( pAdapter1, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &lvl[ i ], 1,
                                          D3D11_SDK_VERSION, &pDevice11, NULL, NULL );
                if ( SUCCEEDED(hr) )
                {
#ifdef EXTRA_DEBUG
                    OutputDebugString( ": Success\n");
#endif
                    switch( lvl[i] )
                    {
                    case D3D_FEATURE_LEVEL_9_1:  flMaskDX11 |= FLMASK_9_1; break;
                    case D3D_FEATURE_LEVEL_9_2:  flMaskDX11 |= FLMASK_9_2; break;
                    case D3D_FEATURE_LEVEL_9_3:  flMaskDX11 |= FLMASK_9_3; break;
                    case D3D_FEATURE_LEVEL_10_0: flMaskDX11 |= FLMASK_10_0; break;
                    case D3D_FEATURE_LEVEL_10_1: flMaskDX11 |= FLMASK_10_1; break;
                    case D3D_FEATURE_LEVEL_11_0: flMaskDX11 |= FLMASK_11_0; break;
                    case D3D_FEATURE_LEVEL_11_1: flMaskDX11 |= FLMASK_11_1; break;
                    }

                    if ( lvl[i] > flHigh )
                        flHigh = lvl[i];
                }
                else
                {
#ifdef EXTRA_DEBUG
                    OutputDebugString( ": Failed\n");
#endif
                    pDevice11 = NULL;
                }

                if ( pDevice11 )
                {
                    // Some Intel Integrated Graphics WDDM 1.0 drivers will crash if you try to release here
                    // For this application, leaking a few device instances is not a big deal
                    if ( aDesc.VendorId != 0x8086 )
                        pDevice11->Release();

                    pDevice11 = NULL;
                }
            }

            if (flHigh > 0)
            {
                hr = g_D3D11CreateDevice( pAdapter1, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &flHigh, 1,
                                          D3D11_SDK_VERSION, &pDevice11, NULL, NULL );
              
                if ( SUCCEEDED(hr) )
                {
                    hr = pDevice11->QueryInterface( __uuidof(ID3D11Device1), ( LPVOID* )&pDevice11_1 );
                    if ( FAILED(hr) )
                        pDevice11_1 = NULL;

                    hr = pDevice11->QueryInterface( __uuidof(ID3D11Device2), ( LPVOID* )&pDevice11_2 );
                    if ( FAILED(hr) )
                        pDevice11_2 = nullptr;

                    hr = pDevice11->QueryInterface( __uuidof(ID3D11Device3), ( LPVOID* )&pDevice11_3 );
                    if ( FAILED(hr) )
                        pDevice11_3 = nullptr;
                }
                else if ( FAILED(hr) )
                    pDevice11 = NULL;
            }
        }
           
        if ( pDevice11 != NULL )
            D3D11_FillTree( hTreeA, pDevice11, flMaskDX11, D3D_DRIVER_TYPE_HARDWARE );

        if ( pDevice11_1 != NULL )
            D3D11_FillTree1( hTreeA, pDevice11_1, flMaskDX11, D3D_DRIVER_TYPE_HARDWARE );

        if ( pDevice11_2 )
            D3D11_FillTree2( hTreeA, pDevice11_2, flMaskDX11, D3D_DRIVER_TYPE_HARDWARE );

        if ( pDevice11_3 )
            D3D11_FillTree3( hTreeA, pDevice11_3, flMaskDX11, D3D_DRIVER_TYPE_HARDWARE );
    }

    // WARP
    DWORD flMaskWARP = FLMASK_9_1 | FLMASK_9_2 | FLMASK_9_3 | FLMASK_10_0 | FLMASK_10_1;
    ID3D10Device1* pDeviceWARP10 = NULL;
    if ( g_D3D10CreateDevice1 != NULL )
    {
        hr = g_D3D10CreateDevice1( NULL, D3D10_DRIVER_TYPE_WARP, NULL, 0, D3D10_FEATURE_LEVEL_10_1,
                                   D3D10_1_SDK_VERSION, &pDeviceWARP10 );
        if ( FAILED(hr) )
            pDeviceWARP10 = NULL;
    }
    
    ID3D11Device* pDeviceWARP11 = NULL;
    ID3D11Device1* pDeviceWARP11_1 = NULL;
    ID3D11Device2* pDeviceWARP11_2 = nullptr;
    ID3D11Device3* pDeviceWARP11_3 = nullptr;
    if ( g_D3D11CreateDevice != NULL )
    {
        D3D_FEATURE_LEVEL fl;
        D3D_FEATURE_LEVEL lvl[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                    D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                                    D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
        hr = g_D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, lvl, _countof(lvl),
                                  D3D11_SDK_VERSION, &pDeviceWARP11, &fl, NULL );
        if ( FAILED(hr) )
        {
            hr = g_D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, NULL, 0,
                                      D3D11_SDK_VERSION, &pDeviceWARP11, &fl, NULL );
        }
        if ( FAILED(hr) )
            pDeviceWARP11 = NULL;
        else
        {
            if ( fl >= D3D_FEATURE_LEVEL_11_1 )
                flMaskWARP |= FLMASK_11_1;

            if ( fl >= D3D_FEATURE_LEVEL_11_0 )
                flMaskWARP |= FLMASK_11_0;

            hr = pDeviceWARP11->QueryInterface( __uuidof(ID3D11Device1), ( LPVOID* )&pDeviceWARP11_1 );
            if ( FAILED(hr) )
                pDeviceWARP11_1 = NULL;

            hr = pDeviceWARP11->QueryInterface( __uuidof(ID3D11Device2), ( LPVOID* )&pDeviceWARP11_2 );
            if ( FAILED(hr) )
                pDeviceWARP11_2 = nullptr;

            hr = pDeviceWARP11->QueryInterface( __uuidof(ID3D11Device3), ( LPVOID* )&pDeviceWARP11_3 );
            if ( FAILED(hr) )
                pDeviceWARP11_3 = nullptr;
        }
    }

    if ( pDeviceWARP10 || pDeviceWARP11 || pDeviceWARP11_1 || pDeviceWARP11_2 || pDeviceWARP11_3 )
    {
        HTREEITEM hTreeW = TVAddNode(hTree, "Windows Advanced Rasterization Platform (WARP)", TRUE, IDI_CAPS, NULL, 0, 0 );

        if ( pDeviceWARP10 != NULL )
            D3D10_FillTree( hTreeW,  pDeviceWARP10, D3D_DRIVER_TYPE_WARP );

        if ( pDeviceWARP10 != NULL )
            D3D10_FillTree1( hTreeW, pDeviceWARP10, flMaskWARP, D3D_DRIVER_TYPE_WARP );

        if ( pDeviceWARP11 != NULL )
            D3D11_FillTree( hTreeW, pDeviceWARP11, flMaskWARP, D3D_DRIVER_TYPE_WARP );

        if ( pDeviceWARP11_1 != NULL )
            D3D11_FillTree1( hTreeW, pDeviceWARP11_1, flMaskWARP, D3D_DRIVER_TYPE_WARP );

        if ( pDeviceWARP11_2 )
            D3D11_FillTree2( hTreeW, pDeviceWARP11_2, flMaskWARP, D3D_DRIVER_TYPE_WARP );

        if ( pDeviceWARP11_3 )
            D3D11_FillTree3( hTreeW, pDeviceWARP11_3, flMaskWARP, D3D_DRIVER_TYPE_WARP );
    }

    // REFERENCE
    ID3D10Device1* pDeviceREF10_1 = NULL;
    ID3D10Device* pDeviceREF10 = NULL;
    if ( g_D3D10CreateDevice1 != NULL )
    {
        hr = g_D3D10CreateDevice1( NULL, D3D10_DRIVER_TYPE_REFERENCE, NULL, 0, D3D10_FEATURE_LEVEL_10_1,
                                   D3D10_1_SDK_VERSION, &pDeviceREF10_1 );
        if ( SUCCEEDED(hr) )
        {
            hr = pDeviceREF10_1->QueryInterface( __uuidof( ID3D10Device ), ( LPVOID* )&pDeviceREF10 );
            if ( FAILED(hr) )
                pDeviceREF10 = NULL;
        }
        else
            pDeviceREF10_1 = NULL;
    }
    else if ( g_D3D10CreateDevice != NULL )
    {
        hr = g_D3D10CreateDevice( NULL, D3D10_DRIVER_TYPE_REFERENCE, NULL, 0, D3D10_SDK_VERSION, &pDeviceREF10 );
        if ( FAILED(hr) )
            pDeviceREF10 = NULL;
    }

    ID3D11Device* pDeviceREF11 = NULL;
    ID3D11Device1* pDeviceREF11_1 = NULL;
    ID3D11Device2* pDeviceREF11_2 = nullptr;
    ID3D11Device3* pDeviceREF11_3 = nullptr;
    DWORD flMaskREF = FLMASK_9_1 | FLMASK_9_2 | FLMASK_9_3 | FLMASK_10_0 | FLMASK_10_1 | FLMASK_11_0;
    if ( g_D3D11CreateDevice != NULL )
    {
        D3D_FEATURE_LEVEL lvl = D3D_FEATURE_LEVEL_11_1;
        hr = g_D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, 0, &lvl, 1,
                                  D3D11_SDK_VERSION, &pDeviceREF11, NULL, NULL );

        if ( SUCCEEDED(hr) )
        {
            flMaskREF |= FLMASK_11_1;
            hr = pDeviceREF11->QueryInterface( __uuidof( ID3D11Device1), ( LPVOID* )&pDeviceREF11_1 );
            if ( FAILED(hr) )
                pDeviceREF11_1 = NULL;

            hr = pDeviceREF11->QueryInterface( __uuidof( ID3D11Device2), ( LPVOID* )&pDeviceREF11_2 );
            if ( FAILED(hr) )
                pDeviceREF11_2 = nullptr;

            hr = pDeviceREF11->QueryInterface( __uuidof( ID3D11Device3), ( LPVOID* )&pDeviceREF11_3 );
            if ( FAILED(hr) )
                pDeviceREF11_3 = nullptr;
        }
        else
        {
            hr = g_D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, 0, NULL, 0,
                                      D3D11_SDK_VERSION, &pDeviceREF11, NULL, NULL );
            if ( SUCCEEDED(hr) )
            {
                hr = pDeviceREF11->QueryInterface( __uuidof( ID3D11Device1), ( LPVOID* )&pDeviceREF11_1 );
                if ( FAILED(hr) )
                    pDeviceREF11_1 = NULL;
            }
        }
    }

    if ( pDeviceREF10 || pDeviceREF10_1 || pDeviceREF11 || pDeviceREF11_1 || pDeviceREF11_2 || pDeviceREF11_3 )
    {
        HTREEITEM hTreeR = TVAddNode(hTree, "Reference", TRUE, IDI_CAPS, NULL, 0, 0 );

        if ( pDeviceREF10 != NULL )
            D3D10_FillTree( hTreeR, pDeviceREF10, D3D_DRIVER_TYPE_REFERENCE );

        if ( pDeviceREF10_1 != NULL )
            D3D10_FillTree1( hTreeR, pDeviceREF10_1, FLMASK_10_0 | FLMASK_10_1, D3D_DRIVER_TYPE_REFERENCE );

        if ( pDeviceREF11 != NULL )
            D3D11_FillTree( hTreeR, pDeviceREF11, flMaskREF, D3D_DRIVER_TYPE_REFERENCE );

        if ( pDeviceREF11_1 != NULL )
            D3D11_FillTree1( hTreeR, pDeviceREF11_1, flMaskREF, D3D_DRIVER_TYPE_REFERENCE );

        if ( pDeviceREF11_2 )
            D3D11_FillTree2( hTreeR, pDeviceREF11_2, flMaskREF, D3D_DRIVER_TYPE_REFERENCE );

        if ( pDeviceREF11_3 )
            D3D11_FillTree3( hTreeR, pDeviceREF11_3, flMaskREF, D3D_DRIVER_TYPE_REFERENCE );
    }

    TreeView_Expand( hwndTV, hTree, TVE_EXPAND );
}


//-----------------------------------------------------------------------------
// Name: DXGI_CleanUp()
//-----------------------------------------------------------------------------
VOID DXGI_CleanUp()
{
    if (g_DXGIFactory)
    {
        g_DXGIFactory->Release();
        g_DXGIFactory = NULL;
        g_DXGIFactory1 = NULL;
        g_DXGIFactory2 = NULL;
    }

    if (g_dxgi)
    {
        FreeLibrary( g_dxgi );
        g_dxgi = NULL;
    }

    if ( g_d3d10 )
    {
        FreeLibrary( g_d3d10 );
        g_d3d10 = NULL;
    }

    if ( g_d3d10_1 )
    {
        FreeLibrary( g_d3d10_1 );
        g_d3d10_1 = NULL;
    }

    if ( g_d3d11 )
    {
        FreeLibrary( g_d3d11 );
        g_d3d11 = NULL;
    }
}
