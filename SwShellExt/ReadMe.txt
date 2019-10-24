========================================================================
    活动模板库：SwShellExt 项目概述
========================================================================

应用程序向导已为您创建了此 SwShellExt 项目，作为编写动态链接库 (DLL) 的起点。

本文件概要介绍组成项目的每个文件的内容。

SwShellExt.vcxproj
    这是使用应用程序向导生成的 VC++ 项目的主项目文件，其中包含生成该文件的 Visual C++ 的版本信息，以及有关使用应用程序向导选择的平台、配置和项目功能的信息。

SwShellExt.vcxproj.filters
    这是使用“应用程序向导”生成的 VC++ 项目筛选器文件。它包含有关项目文件与筛选器之间的关联信息。在 IDE 中，通过这种关联，在特定节点下以分组形式显示具有相似扩展名的文件。例如，“.cpp”文件与“源文件”筛选器关联。

SwShellExt.idl
    此文件包含项目中定义的类型库、接口和组件类的 IDL 定义。
    此文件将由 MIDL 编译器处理，用于生成：
        C++ 接口定义和 GUID 声明 (SwShellExt.h)
        GUID 定义                                (SwShellExt_i.c)
        类型库                                  (SwShellExt.tlb)
        封送处理代码                                 （SwShellExt_p.c 和 dlldata.c）

SwShellExt.h
    此文件包含 SwShellExt.idl 中定义的项目的 C++ 接口定义和 GUID 声明。它将在编译过程中由 MIDL 重新生成。

SwShellExt.cpp
    此文件包含对象映射和 DLL 导出的实现。

SwShellExt.rc
    这是程序使用的所有 Microsoft Windows 资源的列表。

SwShellExt.def
    此模块定义文件为链接器提供有关 DLL 所要求的导出的信息，它包含用于以下内容的导出：
        DllGetClassObject
        DllCanUnloadNow
        DllRegisterServer
        DllUnregisterServer
        DllInstall

/////////////////////////////////////////////////////////////////////////////
其他标准文件:

StdAfx.h, StdAfx.cpp
    这些文件用于生成名为 SwShellExt.pch 的预编译头 (PCH) 文件和名为 StdAfx.obj 的预编译类型文件。

Resource.h
    这是用于定义资源 ID 的标准头文件。

/////////////////////////////////////////////////////////////////////////////
代理/存根 (stub) DLL 项目和模块定义文件：

SwShellExtps.vcxproj
    此文件是用于生成代理/存根 (stub) DLL 的项目文件（若有必要）。
	主项目中的 IDL 文件必须至少包含一个接口，并且在生成代理/存根 (stub) DLL 之前必须先编译 IDL 文件。
	此过程生成 dlldata.c、SwShellExt_i.c 和 SwShellExt_p.c，这些是生成代理/存根 (stub) DLL 所必需的。

SwShellExtps.vcxproj.filters
    此文件是代理/存根项目的筛选器文件。它包含有关项目文件与筛选器之间的关联信息。在 IDE 中，通过这种关联，在特定节点下以分组形式显示具有相似扩展名的文件。例如，“.cpp”文件与“源文件”筛选器关联。

SwShellExtps.def
    此模块定义文件为链接器提供有关代理/存根 (stub) 所要求的导出的信息。

/////////////////////////////////////////////////////////////////////////////
