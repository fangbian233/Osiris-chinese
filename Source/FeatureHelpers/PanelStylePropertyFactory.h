#pragma once

#include <CS2/Classes/Panorama.h>
#include <GameClasses/MemAlloc.h>
#include "ImageShadowParams.h"
#include "StylePropertiesSymbols.h"
#include "StylePropertiesVMTs.h"

struct PanelStylePropertyFactory {
    PanelStylePropertyFactory(const StylePropertiesVMTs& stylePropertyVMTs, const StylePropertiesSymbols& stylePropertySymbols) noexcept
        : VMTs{stylePropertyVMTs}
        , symbols{stylePropertySymbols}
    {
    }

    [[nodiscard]] cs2::CStylePropertyWidth* width(cs2::CUILength width) const noexcept
    {
        return create<cs2::CStylePropertyWidth>(VMTs.widthPropertyVmt, symbols.width, width);
    }

    [[nodiscard]] cs2::CStylePropertyOpacity* opacity(float opacity) const noexcept
    {
        return create<cs2::CStylePropertyOpacity>(VMTs.opacityPropertyVmt, symbols.opacity, opacity);
    }

    [[nodiscard]] cs2::CStylePropertyZIndex* zIndex(float zIndex) const noexcept
    {
        return create<cs2::CStylePropertyZIndex>(VMTs.zIndexPropertyVmt, symbols.zIndex, zIndex);
    }

    [[nodiscard]] cs2::CStylePropertyHeight* height(cs2::CUILength height) const noexcept
    {
        return create<cs2::CStylePropertyHeight>(VMTs.heightPropertyVmt, symbols.height, height);
    }

    [[nodiscard]] cs2::CStylePropertyImageShadow* imageShadow(const ImageShadowParams& params) const noexcept
    {
        return create<cs2::CStylePropertyImageShadow>(VMTs.imageShadowPropertyVmt, symbols.imageShadow, true, params.horizontalOffset, params.verticalOffset, params.blurRadius, params.strength, params.color);
    }

private:
    template <typename T, typename... Args>
    [[nodiscard]] T* create(const void* vmt, cs2::CStyleSymbol symbol, Args&&... args) const noexcept
    {
        if (vmt && symbol.isValid()) {
            if (const auto memory{MemAlloc::allocate(sizeof(T))})
                return new (memory) T{cs2::CStyleProperty{vmt, symbol, false}, std::forward<Args>(args)...};
        }
        return nullptr;
    }

    const StylePropertiesVMTs& VMTs;
    const StylePropertiesSymbols& symbols;
};
