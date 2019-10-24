

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Sun Sep 03 09:47:19 2017
 */
/* Compiler settings for SwShellExt.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0595 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __SwShellExt_i_h__
#define __SwShellExt_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISafeWallExt_FWD_DEFINED__
#define __ISafeWallExt_FWD_DEFINED__
typedef interface ISafeWallExt ISafeWallExt;

#endif 	/* __ISafeWallExt_FWD_DEFINED__ */


#ifndef __SafeWallExt_FWD_DEFINED__
#define __SafeWallExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class SafeWallExt SafeWallExt;
#else
typedef struct SafeWallExt SafeWallExt;
#endif /* __cplusplus */

#endif 	/* __SafeWallExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISafeWallExt_INTERFACE_DEFINED__
#define __ISafeWallExt_INTERFACE_DEFINED__

/* interface ISafeWallExt */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ISafeWallExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F9F3DEED-359B-4299-921F-CA6B08B187D2")
    ISafeWallExt : public IDispatch
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct ISafeWallExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISafeWallExt * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISafeWallExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISafeWallExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISafeWallExt * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISafeWallExt * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISafeWallExt * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISafeWallExt * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ISafeWallExtVtbl;

    interface ISafeWallExt
    {
        CONST_VTBL struct ISafeWallExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISafeWallExt_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISafeWallExt_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISafeWallExt_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISafeWallExt_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISafeWallExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISafeWallExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISafeWallExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISafeWallExt_INTERFACE_DEFINED__ */



#ifndef __SwShellExtLib_LIBRARY_DEFINED__
#define __SwShellExtLib_LIBRARY_DEFINED__

/* library SwShellExtLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_SwShellExtLib;

EXTERN_C const CLSID CLSID_SafeWallExt;

#ifdef __cplusplus

class DECLSPEC_UUID("0631C202-F576-4046-88F2-EA1B4DC3C71A")
SafeWallExt;
#endif
#endif /* __SwShellExtLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


