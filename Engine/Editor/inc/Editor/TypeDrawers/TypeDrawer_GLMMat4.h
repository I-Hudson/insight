#pragma once

#include "Editor/TypeDrawers/ITypeDrawer.h"

namespace Insight
{
    namespace Editor
    {
        class IS_EDITOR TypeDrawer_GLMMat4 : public ITypeDrawer
        {
        public:
            virtual void Draw(void* data) const override;
        };
    }
}