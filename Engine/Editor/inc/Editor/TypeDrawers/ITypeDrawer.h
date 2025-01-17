#pragma once

#include "Editor/Defines.h"
#include "Editor/TypeDrawers/TypeDrawerRegister.h"

#include <Reflect/Reflect.h>

namespace Insight
{
    namespace Editor
    {
        class IS_EDITOR ITypeDrawer
        {
        public:
            virtual void Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label = "") const = 0;
            virtual void DrawDebug(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label = "") const { Draw(data, memberInfo, label); }
        };
    }
}