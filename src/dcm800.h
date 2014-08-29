#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	// {7A3CFAFD-3168-4fa1-993A-11C024F2B00B}
	DEFINE_GUID(IID_IDcm800, 0x7a3cfafd, 0x3168, 0x4fa1, 0x99, 0x3a, 0x11, 0xc0, 0x24, 0xf2, 0xb0, 0xb);

	enum eDcm800Size{
		eDcm800Size_3264_2448,
		eDcm800Size_1600_1200,
		eDcm800Size_1280_960,
		eDcm800Size_1024_768,
		eDcm800Size_640_480
	};
	void GetWH(eDcm800Size& size, int& w, int &h);

	typedef void (__cdecl *PFUN_AutoWhiteBalanceCallback)(int RedGain, int GreenGain, int BlueGain, void* Context);

    DECLARE_INTERFACE_(IDcm800, IUnknown)
    {
		STDMETHOD(get_PreviewSize) (THIS_ enum eDcm800Size* Dcm800Size) PURE;				
		STDMETHOD(put_PreviewSize) (THIS_ enum eDcm800Size Dcm800Size) PURE;				
		STDMETHOD(get_StillSize) (THIS_ enum eDcm800Size* Dcm800Size) PURE;				
		STDMETHOD(put_StillSize) (THIS_ enum eDcm800Size Dcm800Size) PURE;				
		STDMETHOD(get_AutoExposure) (THIS_ BOOL* bAutoExposure) PURE;				
		STDMETHOD(put_AutoExposure) (THIS_ BOOL bAutoExposure) PURE;
		STDMETHOD(get_ExposureTarget) (THIS_ BYTE* Target) PURE;				
		STDMETHOD(put_ExposureTarget) (THIS_ BYTE Target) PURE;
		STDMETHOD(get_ExposureTime) (THIS_ USHORT* Time) PURE;				
		STDMETHOD(put_ExposureTime) (THIS_ USHORT Time) PURE;
		STDMETHOD(put_AutoWB) (THIS_ PFUN_AutoWhiteBalanceCallback, void* Context) PURE;
		STDMETHOD(get_Gain) (THIS_ int* RGain, int* GGain, int* BGain) PURE;				
		STDMETHOD(put_Gain) (THIS_ int RGain, int GGain, int BGain) PURE;
		STDMETHOD(get_Gamma) (THIS_ BYTE* Gamma) PURE;				
		STDMETHOD(put_Gamma) (THIS_ BYTE Gamma) PURE;				
		STDMETHOD(get_Contrast) (THIS_ BYTE* Contrast) PURE;				
		STDMETHOD(put_Contrast) (THIS_ BYTE Contrast) PURE;				
		STDMETHOD(get_Saturation) (THIS_ BYTE* Saturation) PURE;				
		STDMETHOD(put_Saturation) (THIS_ BYTE Saturation) PURE;
		STDMETHOD(get_VFlip) (THIS_ BOOL* bVFlip) PURE;				
		STDMETHOD(put_VFlip) (THIS_ BOOL bVFlip) PURE;
		STDMETHOD(get_HFlip) (THIS_ BOOL* bHFlip) PURE;
		STDMETHOD(put_HFlip) (THIS_ BOOL bHFlip) PURE;
		STDMETHOD(get_LightFrquency) (THIS_ BYTE* LightFrquency) PURE;
		STDMETHOD(put_LightFrquency) (THIS_ BYTE Saturation) PURE;
		STDMETHOD(get_Speed) (THIS_ BYTE* Speed) PURE;
		STDMETHOD(put_Speed) (THIS_ BYTE Speed) PURE;
		STDMETHOD(get_AnalogGain) (THIS_ USHORT* AnalogGain) PURE;
		STDMETHOD(put_AnalogGain) (THIS_ USHORT AnalogGain) PURE;
		STDMETHOD(get_WhiteBlack) (THIS_ BOOL* bWhiteBlack) PURE;
		STDMETHOD(put_WhiteBlack) (THIS_ BOOL bWhiteBlack) PURE;
    };

	BOOL WINAPI IsDevicePresent();
	
#ifdef __cplusplus
}
#endif