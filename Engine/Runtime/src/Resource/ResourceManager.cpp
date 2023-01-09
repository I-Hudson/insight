#include "Resource/ResourceManager.h"
#include "Resource/ResourceDatabase.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Threading/TaskSystem.h"

namespace Insight
{
    namespace Runtime
    {
#define RESOURCE_LOAD_THREAD

        ResourceDatabase* ResourceManager::m_database;

        TObjectPtr<IResource> ResourceManager::Load(ResourceId const& resourceId)
        {
            ASSERT(m_database);

            TObjectPtr<IResource> resource;
            if (m_database->HasResource(resourceId))
            {
                resource = m_database->GetResource(resourceId);
            }
            else
            {
                resource = m_database->AddResource(resourceId);
            }

            if (resource)
            {
                if (resource->IsLoaded())
                {
                    // Item is already loaded, just return it.
                    return resource;
                }

                if (resource->IsNotLoaded() || resource->IsUnloaded())
                {
                    if (resource->GetResourceStorageType() == ResourceStorageTypes::Disk)
                    {
                        if (!FileSystem::FileSystem::Exists(resource->GetFilePath()))
                        {
                            // File does not exists. Set the resource state and return nullptr.
                            resource->m_resource_state = EResoruceStates::Not_Found;
                            return resource;
                        }
                        else
                        {
                            resource->m_resource_state = EResoruceStates::Loading;
                            // Try and load the resource as it exists.
#ifdef RESOURCE_LOAD_THREAD
                            Threading::TaskSystem::CreateTask([resource]()
                                {
#endif
                                    resource->StartLoadTimer();
                            {
                                std::lock_guard resourceLock(resource->m_mutex); // FIXME Maybe don't do this?
                                resource->Load();
                            }
                            resource->StopLoadTimer();
                            if (resource->IsLoaded())
                            {
                                resource->OnLoaded(resource);
                            }
#ifdef RESOURCE_LOAD_THREAD
                                });
#endif
                        }
                    }
                    else
                    {
                        FAIL_ASSERT_MSG("[ResourceManager::Load] Maybe this should be done. Maybe when an resource is being loaded from disk it should handle loading memory resources.");
                    }
                }
            }
            return resource;
        }

        void ResourceManager::Unload(ResourceId const& resourceId)
        {
            ASSERT(m_database);

            TObjectPtr<IResource> resource = nullptr;
            if (m_database->HasResource(resourceId))
            {
                resource = m_database->GetResource(resourceId);
            }

            if (!resource)
            {
                IS_CORE_WARN("[ResourceManager::UnloadResource] The resource '{0}' is not valid (null). The Resource isn't tracked by the ResourceDatabase.", resourceId.GetPath());
                return;
            }

            if (resource->GetResourceState() != EResoruceStates::Loaded)
            {
                IS_CORE_WARN("[ResourceManager::Unload] 'resource' current state is '{0}'. Resource must be loaded to be unloaded."
                    , ERsourceStatesToString(resource->GetResourceState()));
                return;
            }

            // Unload the resource,
            resource->m_resource_state = EResoruceStates::Unloading;
            resource->StartUnloadTimer();
            {
                std::lock_guard resourceLock(resource->m_mutex);
                resource->UnLoad();
            }
            resource->StopUnloadTimer();
            resource->m_resource_state = EResoruceStates::Unloaded;
            resource->OnUnloaded(resource);
        }

        void ResourceManager::Unload(TObjectPtr<IResource> Resource)
        {
            if (Resource)
            {
                Unload(Resource->m_resourceId);
            }
        }

        void ResourceManager::UnloadAll()
        {
            ASSERT(m_database);
            std::vector<ResourceId> ResourceIds = m_database->GetAllResourceIds();
            for (const ResourceId& id : ResourceIds)
            {
                Unload(id);
            }
        }

        bool ResourceManager::HasResource(ResourceId const& resourceId)
        {
            ASSERT(m_database);
            return m_database->HasResource(resourceId);
        }

        bool ResourceManager::HasResource(TObjectPtr<IResource> Resource)
        {
            if (Resource)
            {
                return HasResource(Resource->GetResourceId());
            }
            return false;
        }

        ResourceDatabase::ResourceMap ResourceManager::GetResourceMap()
        {
            ASSERT(m_database);
            return m_database->GetResourceMap();
        }

        u32 ResourceManager::GetLoadedResourcesCount()
        {
            ASSERT(m_database);
            return m_database->GetLoadedResourceCount();
        }

        u32 ResourceManager::GetLoadingCount()
        {
            ASSERT(m_database);
            return m_database->GetLoadingResourceCount();
        }
    }
}