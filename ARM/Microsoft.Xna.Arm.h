//-----------------------------------------------------------------------------
// Wrapper to simplify using the ARM API from C++, via managed -> COM interop.
//
// Microsoft XNA
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __MICROSOFT_XNA_ARM_H__
#define __MICROSOFT_XNA_ARM_H__

#ifdef LINUX_BUILD

// sebi
namespace Microsoft
{
	namespace Xna
	{
		namespace Arm
		{
            struct IProviderEngine
            {
                // TODO: implement
            };
            struct IProviderAsset
            {
                // TODO: implement
                void AddRelationship(const char* texture, const char* filename) {};
                void Close(void) {};
            };

            typedef IProviderAsset *IProviderAssetPtr;
            typedef IProviderEngine *IProviderEnginePtr;
        }
    }
}


#else

#include <comdef.h>

namespace Microsoft
{
	namespace Xna
	{
		namespace Arm
		{
			#import "Microsoft.Xna.Arm.tlb" no_namespace

			inline IProviderEnginePtr CreateProviderEngine(const char *toolName, const char *toolVersion)
			{
				IComFactory *factory = NULL;

				HRESULT hr = CoCreateInstance(__uuidof(ComFactory),
										  NULL,
										  CLSCTX_INPROC_SERVER,
										  __uuidof(IComFactory),
										  reinterpret_cast<void **>(&factory));

				if (FAILED(hr))
					return NULL;

				IProviderEnginePtr provider = factory->Create(toolName, toolVersion);

				factory->Release();

				return provider;
			}
		}
	}
}


#endif // LINUX_BUILD

#endif // __MICROSOFT_XNA_ARM_H__
