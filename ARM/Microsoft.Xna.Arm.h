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
			enum AssetType {
				AssetType_Physical,
			};

			enum ProviderType {
				ProviderType_Primary,
			};


			struct IProviderRelationship {
				virtual void AddProperty(const char* key, const char* value) = 0;
				virtual ~IProviderRelationship() {}
			};
			typedef IProviderRelationship *IProviderRelationshipPtr;

            struct IProviderAsset
            {
				virtual IProviderRelationshipPtr AddRelationship(const char* texture, const char* filename) = 0;
				virtual void AddProperty(const char* key, const char* value) = 0;
                virtual void Close(void) = 0;
				virtual ~IProviderAsset() {};
            };
            typedef IProviderAsset *IProviderAssetPtr;

            struct IProviderEngine {
				virtual IProviderAsset* OpenAsset(const char* iniName, AssetType, ProviderType) = 0;
				virtual ~IProviderEngine() {};
            };
            typedef IProviderEngine *IProviderEnginePtr;


			class MyProviderRelationship : public IProviderRelationship {
				virtual void AddProperty(const char* , const char* ) override {}
			};

			class MyProviderAsset : public IProviderAsset {
			public:
				virtual IProviderRelationshipPtr AddRelationship(const char*, const char*) override { return new MyProviderRelationship(); }
				virtual void AddProperty(const char* , const char* ) override {}
				virtual void Close(void) override {}
			};

			class MyProviderEngine : public IProviderEngine {
			public:
				IProviderAsset* OpenAsset(const char* iniName, AssetType, ProviderType) override {
					return new MyProviderAsset();
				}
			};

			inline IProviderEnginePtr CreateProviderEngine(const char *toolName, const char *toolVersion) {
				(void)toolName;
				(void)toolVersion;
				return new MyProviderEngine();
			}
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
