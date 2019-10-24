// dllmain.h : 模块类的声明。

class CSwShellExtModule : public ATL::CAtlDllModuleT< CSwShellExtModule >
{
public :
	DECLARE_LIBID(LIBID_SwShellExtLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SWSHELLEXT, "{30FAD006-708C-4975-8310-A169B532A0B0}")
};

extern class CSwShellExtModule _AtlModule;
