#include "ECS/ECSWorld.h"

namespace Insight
{
	namespace ECS
	{
		ECSWorld::ECSWorld()
			: m_componentArrayManager(ComponentArrayManager(this))
		{ }

		ECSWorld::~ECSWorld()
		{
		}

		void ECSWorld::Update(float deltaTime)
		{
			m_componentArrayManager.Update(deltaTime);
		}

		Entity ECSWorld::AddEntity()
		{
			return m_entityManager.AddNewEntity();
		}

		void ECSWorld::RemoveEntity(Entity& entity)
		{
			m_entityManager.RemoveEntity(entity);
		}
		
		void ECSWorld::RemoveComponent(Entity entity, ComponentHandle& handle)
		{
			m_entityManager.RemoveComponentFromEntity(entity, handle);
			m_componentArrayManager.RemoveComponent(entity, handle);
		}
	}
}