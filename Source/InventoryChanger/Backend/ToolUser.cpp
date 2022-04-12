#include "BackendSimulator.h"
#include "ToolUser.h"

#include <InventoryChanger/ItemGenerator.h>
#include <InventoryChanger/GameItems/Lookup.h>

namespace inventory_changer::backend
{

std::optional<Response> ToolUser::applySticker(std::list<inventory::Item>::iterator item, std::list<inventory::Item>::const_iterator sticker, std::uint8_t slot)
{
    const auto skin = item->get<inventory::Skin>();
    if (!skin)
        return {};

    skin->stickers[slot].stickerID = gameItemLookup.getStorage().getStickerKit(sticker->gameItem()).id;
    skin->stickers[slot].wear = 0.0f;

    backend.moveToFront(item);
    backend.removeItem(sticker);
    return Response{ Response::StickerApplied{ item, slot } };
}

std::optional<Response> ToolUser::applyPatch(std::list<inventory::Item>::iterator item, std::list<inventory::Item>::const_iterator patch, std::uint8_t slot)
{
    const auto agent = item->getOrCreate<inventory::Agent>();
    if (!agent)
        return {};

    agent->patches[slot].patchID = gameItemLookup.getStorage().getPatch(patch->gameItem()).id;
    backend.moveToFront(item);
    backend.removeItem(patch);
    return Response{ Response::PatchApplied{ item, slot } };
}

std::optional<Response> ToolUser::removePatch(std::list<inventory::Item>::iterator item, std::uint8_t slot)
{
    const auto agent = item->get<inventory::Agent>();
    if (!agent)
        return {};

    agent->patches[slot].patchID = 0;
    backend.moveToFront(item);
    return Response{ Response::PatchRemoved{ item, slot } };
}

void ToolUser::activateOperationPass(std::list<inventory::Item>::const_iterator item)
{
    const auto& gameItem = item->gameItem();
    if (!gameItem.isOperationPass())
        return;

    const auto coinID = gameItem.getWeaponID() != WeaponId::OperationHydraPass ? static_cast<WeaponId>(static_cast<int>(gameItem.getWeaponID()) + 1) : WeaponId::BronzeOperationHydraCoin;
    if (const auto operationCoin = gameItemLookup.findItem(coinID); operationCoin.has_value()) {
        backend.addItemUnacknowledged(inventory::Item{ *operationCoin });
        backend.removeItem(item);
    }
}

std::optional<Response> ToolUser::activateViewerPass(std::list<inventory::Item>::const_iterator item)
{
    const auto& gameItem = item->gameItem();
    if (!gameItem.isViewerPass())
        return {};

    const auto coinID = static_cast<WeaponId>(static_cast<int>(gameItem.getWeaponID()) + 1);
    if (const auto eventCoin = gameItemLookup.findItem(coinID); eventCoin.has_value()) {
        const auto addedEventCoin = backend.addItemUnacknowledged(inventory::Item{ *eventCoin });
        backend.removeItem(item);
        return Response{ Response::ViewerPassActivated{ addedEventCoin } };
    }
    return {};
}

std::optional<Response> ToolUser::wearSticker(std::list<inventory::Item>::iterator item, std::uint8_t slot)
{
    const auto skin = item->get<inventory::Skin>();
    if (!skin)
        return {};

    constexpr auto wearStep = 0.12f;
    const auto newWear = (skin->stickers[slot].wear += wearStep);

    if (const auto shouldRemove = (newWear >= 1.0f + wearStep)) {
        skin->stickers[slot] = {};
        return Response{ Response::StickerRemoved{ item, slot } };
    }

    return Response{ Response::StickerScraped{ item, slot } };
}

std::optional<Response> ToolUser::addNameTag(std::list<inventory::Item>::iterator item, std::list<inventory::Item>::const_iterator nameTagItem, std::string_view nameTag)
{
    const auto skin = item->get<inventory::Skin>();
    if (!skin)
        return {};

    skin->nameTag = nameTag;
    backend.removeItem(nameTagItem);
    backend.moveToFront(item);
    return Response{ Response::NameTagAdded{ item } };
}

std::optional<Response> ToolUser::removeNameTag(std::list<inventory::Item>::iterator item)
{
    if (const auto skin = item->get<inventory::Skin>()) {
        skin->nameTag.clear();
        backend.moveToFront(item);
        return Response{ Response::NameTagRemoved{ item } };
    }
    return {};
}

std::optional<Response> ToolUser::openContainer(std::list<inventory::Item>::const_iterator container, std::optional<std::list<inventory::Item>::const_iterator> key)
{
    if (!container->gameItem().isCase())
        return {};

    if (key.has_value()) {
        if (const auto& keyItem = *key; keyItem->gameItem().isCaseKey())
            backend.removeItem(keyItem);
    }

    auto generatedItem = ItemGenerator::generateItemFromContainer(*container);
    backend.removeItem(container);
    const auto receivedItem = backend.addItemUnacknowledged(std::move(generatedItem));
    return Response{ Response::ContainerOpened{ receivedItem } };
}

std::optional<Response> ToolUser::activateSouvenirToken(std::list<inventory::Item>::const_iterator item, std::list<inventory::Item>::iterator tournamentCoin)
{
    if (!item->gameItem().isSouvenirToken())
        return {};

    const auto tournamentCoinData = tournamentCoin->get<inventory::TournamentCoin>();
    if (!tournamentCoinData)
        return {};

    ++tournamentCoinData->dropsAwarded;
    backend.removeItem(item);
    return Response{ Response::SouvenirTokenActivated{ tournamentCoin } };
}

std::optional<Response> ToolUser::unsealGraffiti(std::list<inventory::Item>::iterator item)
{
    if (!item->gameItem().isGraffiti())
        return {};

    const auto graffiti = item->getOrCreate<inventory::Graffiti>();
    if (!graffiti)
        return {};

    graffiti->usesLeft = 50;

    backend.moveToFront(item);
    return Response{ Response::GraffitiUnsealed{ item } };
}

std::optional<Response> ToolUser::swapStatTrak(std::list<inventory::Item>::iterator itemFrom, std::list<inventory::Item>::iterator itemTo, std::list<inventory::Item>::const_iterator statTrakSwapTool)
{
    if (!(itemFrom->gameItem().isSkin() && itemTo->gameItem().isSkin() && statTrakSwapTool->gameItem().isStatTrakSwapTool()))
        return {};

    const auto skinFrom = itemFrom->get<inventory::Skin>();
    if (!skinFrom)
        return {};

    const auto skinTo = itemFrom->get<inventory::Skin>();
    if (!skinTo)
        return {};

    std::swap(skinFrom->statTrak, skinTo->statTrak);
    backend.removeItem(statTrakSwapTool);
    backend.moveToFront(itemFrom);
    backend.moveToFront(itemTo);
    return Response{ Response::StatTrakSwapped{ itemFrom, itemTo } };
}

}