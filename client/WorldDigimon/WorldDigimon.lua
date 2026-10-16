-- SPDX-License-Identifier: GPL-2.0-or-later
-- WoW 3.3.5a. Secure buttons send orders; only the server executes techniques.
local addon = CreateFrame("Frame", "WorldDigimonEvents")
local bar = CreateFrame("Frame", "WorldDigimonCombatBar", UIParent)
bar:SetFrameStrata("HIGH")
bar:SetFrameLevel(100)
bar:SetSize(510, 66)
-- A separate companion row above the stock bottom bars. Children must anchor
-- to this frame too; moving only its backdrop leaves the buttons underneath.
bar:SetPoint("BOTTOM", UIParent, "BOTTOM", 0, 148)
bar:SetBackdrop({bgFile = "Interface\\Tooltips\\UI-Tooltip-Background",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border", tile = true, tileSize = 16,
    edgeSize = 12, insets = {left = 3, right = 3, top = 3, bottom = 3}})
bar:SetBackdropColor(0.02, 0.06, 0.10, 1)
bar:Hide()
bar.label = bar:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall")
bar.label:SetPoint("TOPLEFT", 10, -6)

local menu = CreateFrame("Frame", "WorldDigimonMenu", UIParent)
menu:SetSize(310, 350)
menu:SetPoint("CENTER")
menu:SetFrameStrata("DIALOG")
menu:SetBackdrop({bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border", tile = true, tileSize = 32,
    edgeSize = 32, insets = {left = 10, right = 10, top = 10, bottom = 10}})
menu:Hide()
local title = menu:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
title:SetPoint("TOP", 0, -25)
title:SetText("Dispositivo Sagrado")
tinsert(UISpecialFrames, "WorldDigimonMenu")

local buttons = {}
local state = {active = 0, hp = 0, focus = 0, slots = {}}
local wantedFight = false
local receivedAt = 0
local lastRequest = -100
local updateElapsed = 0
local names = {[17253] = "Mordisco Digital", [24604] = "Aullido de Guerra"}
local actions = {
    {"Atacar", "attack", "Ability_Hunter_Pet_Attack"},
    {"Seguir", "follow", "Ability_Hunter_Pet_Follow"},
    {"Técnica 1", "cast 1"}, {"Técnica 2", "cast 2"},
    {"Técnica 3", "cast 3"}, {"Técnica 4", "cast 4"}, {"Especial", "cast 5"},
    {"Capturar", "capture", "Spell_Arcane_PortalDalaran"},
    {"Invocar", "summon", "Ability_Hunter_BeastCall"},
    {"Retirar", "withdraw", "Ability_Hunter_BeastSoothe"},
    {"Descansar", "recover", "Spell_Holy_Heal"},
    {"Dispositivo", nil, "INV_Misc_Book_07"},
}

local function request(command)
    SendChatMessage(".digimon " .. command, "WHISPER", nil, UnitName("player"))
end

local function sync()
    if GetTime() - lastRequest >= 1 then
        lastRequest = GetTime()
        request("sync")
    end
end

local function notice(text)
    UIErrorsFrame:AddMessage(text, 1, 0.75, 0.15)
end

local function bindBar()
    if InCombatLockdown() then return end
    ClearOverrideBindings(bar)
    if not bar:IsShown() then return end
    for i, button in ipairs(buttons) do
        local keys = {GetBindingKey("ACTIONBUTTON" .. i)}
        for _, key in ipairs(keys) do
            SetOverrideBindingClick(bar, true, key, button:GetName(), "LeftButton")
        end
        button.key:SetText(keys[1] or "")
    end
end

local function setFight(enabled)
    wantedFight = enabled
    WorldDigimonDB = WorldDigimonDB or {}
    WorldDigimonDB.fight = enabled
    if InCombatLockdown() then
        notice("El cambio de barra se aplicará al salir de combate.")
        return
    end
    if enabled then
        CloseGossip()
        menu:Hide()
        bar:Show()
        sync()
    else
        bar:Hide()
    end
    bindBar()
end

local function tooltip(button)
    GameTooltip:SetOwner(button, "ANCHOR_TOP")
    GameTooltip:SetText(button.label)
    local slot = button.slot and state.slots[button.slot]
    if slot and slot.spell > 0 then
        GameTooltip:AddLine("Energía: " .. slot.cost .. " · El compañero ejecuta esta técnica.", 1, 1, 1, true)
        if slot.remaining > 0 then
            GameTooltip:AddLine("Recuperación: " .. math.ceil(slot.remaining / 1000) .. " s", 1, 0.7, 0.2)
        end
    elseif button.slot then
        GameTooltip:AddLine("Aprende y equipa una técnica desde el entrenador y el dispositivo.", 1, 1, 1, true)
    elseif button.command == "attack" then
        GameTooltip:AddLine("Selecciona una criatura enemiga. El lobo se acercará y atacará.", 1, 1, 1, true)
    elseif button.command == "follow" then
        GameTooltip:AddLine("Detiene sus ataques y vuelve a seguirte.", 1, 1, 1, true)
    end
    GameTooltip:Show()
end

for i, action in ipairs(actions) do
    local button = CreateFrame("CheckButton", "WorldDigimonAction" .. i, bar, "SecureActionButtonTemplate")
    -- Explicit layers keep the controls above their own backdrop.
    button:SetFrameStrata("HIGH")
    button:SetFrameLevel(120)
    button:SetSize(36, 36)
    button:SetPoint("BOTTOMLEFT", bar, "BOTTOMLEFT", 10 + (i - 1) * 41, 10)
    button:RegisterForClicks("AnyUp")
    button:SetAttribute("type", "macro")
    button:SetAttribute("macrotext", action[2] and ("/s .digimon " .. action[2]) or "/digimon")
    button.label, button.command = action[1], action[2]
    button.slot = i >= 3 and i <= 7 and i - 2 or nil
    button.icon = button:CreateTexture(nil, "BACKGROUND")
    button.icon:SetAllPoints()
    button.icon:SetTexture("Interface\\Icons\\" .. (action[3] or "INV_Misc_QuestionMark"))
    button:SetNormalTexture("Interface\\Buttons\\UI-Quickslot2")
    button:SetPushedTexture("Interface\\Buttons\\UI-Quickslot-Depress")
    button:SetHighlightTexture("Interface\\Buttons\\ButtonHilight-Square", "ADD")
    button.cooldown = CreateFrame("Cooldown", nil, button, "CooldownFrameTemplate")
    button.cooldown:SetFrameStrata("HIGH")
    button.cooldown:SetFrameLevel(121)
    button.cooldown:SetAllPoints()
    button.key = button:CreateFontString(nil, "OVERLAY", "NumberFontNormalSmall")
    button.key:SetPoint("TOPRIGHT", -1, -2)
    button:SetScript("OnEnter", tooltip)
    button:SetScript("OnLeave", function() GameTooltip:Hide() end)
    buttons[i] = button
end

local function refresh()
    local stale = GetTime() - receivedAt > 6
    if stale then
        bar.label:SetText("Luchar · esperando al compañero…")
    else
        bar.label:SetText(string.format("Luchar · Nivel %d · Salud %d/%d · Energía %d/%d",
            state.level, state.hp, state.maxhp, state.focus, state.maxfocus))
    end
    for i = 1, 5 do
        local button = buttons[i + 2]
        local slot = state.slots[i]
        if slot and slot.spell > 0 then
            local name, _, icon = GetSpellInfo(slot.spell)
            button.label = names[slot.spell] or name or ("Técnica " .. i)
            button.icon:SetTexture(icon or "Interface\\Icons\\INV_Misc_QuestionMark")
            local ready = not stale and state.hp > 0 and state.focus >= slot.cost
            local brightness = ready and 1 or 0.35
            button.icon:SetVertexColor(brightness, brightness, brightness)
            if slot.remaining > 0 then
                button.cooldown:SetCooldown(GetTime(), slot.remaining / 1000)
            else
                button.cooldown:SetCooldown(0, 0)
            end
        else
            button.label = "Espacio sin técnica equipada"
            button.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
            button.icon:SetVertexColor(0.25, 0.25, 0.25)
            button.cooldown:SetCooldown(0, 0)
        end
    end
end

local menuItems = {
    {"Luchar", function() setFight(true) end},
    {"Invocar compañero", function() request("summon"); menu:Hide() end},
    {"Retirar compañero", function() request("withdraw"); menu:Hide() end},
    {"Colección y espacios", function() request("collection"); menu:Hide() end},
    {"Gestionar técnicas", function() request("techniques"); menu:Hide() end},
    {"Salir de Luchar", function() setFight(false); menu:Hide() end},
    {"Cerrar", function() menu:Hide() end},
}
for i, entry in ipairs(menuItems) do
    local button = CreateFrame("Button", nil, menu, "UIPanelButtonTemplate")
    button:SetSize(245, 30)
    button:SetPoint("TOP", 0, -54 - (i - 1) * 39)
    button:SetText(entry[1])
    button:SetScript("OnClick", entry[2])
end

local launcher = CreateFrame("Button", "WorldDigimonLauncher", UIParent, "UIPanelButtonTemplate")
launcher:SetSize(110, 24)
launcher:SetPoint("TOP", 0, -30)
launcher:SetText("Dispositivo")
launcher:SetScript("OnClick", function()
    if menu:IsShown() then menu:Hide() else menu:Show(); sync() end
end)

-- Presentation only: leave the original secure bag buttons and their clicks intact.
hooksecurefunc("ContainerFrame_Update", function(container)
    for i = 1, (container.size or 0) do
        local button = _G[container:GetName() .. "Item" .. i]
        local link = button and GetContainerItemLink(container:GetID(), button:GetID())
        if link and link:find("item:900110:", 1, true) then
            SetItemButtonTexture(button, "Interface\\Icons\\INV_Misc_Book_07")
        end
    end
end)
GameTooltip:HookScript("OnTooltipSetItem", function(tooltipFrame)
    local _, link = tooltipFrame:GetItem()
    if link and link:find("item:900110:", 1, true) then
        tooltipFrame:ClearLines()
        tooltipFrame:SetText("Dispositivo Sagrado", 1, 0.82, 0)
        tooltipFrame:AddLine("Dirige a tu compañero y consulta tu colección.", 1, 1, 1, true)
        tooltipFrame:AddLine("Clic derecho: abrir el menú. Elige Luchar para usar la barra de combate.", 0.2, 1, 0.2, true)
        tooltipFrame:AddLine("También puedes usar el botón Dispositivo de la pantalla.", 0.7, 0.7, 0.7, true)
        tooltipFrame:Show()
    end
end)

SLASH_WORLDDIGIMON1 = "/digimon"
SlashCmdList.WORLDDIGIMON = function(text)
    if text == "luchar" then
        setFight(true)
    elseif text == "salir" then
        setFight(false)
    else
        if menu:IsShown() then menu:Hide() else menu:Show(); sync() end
    end
end

addon:RegisterEvent("PLAYER_ENTERING_WORLD")
addon:RegisterEvent("PLAYER_REGEN_ENABLED")
addon:RegisterEvent("UPDATE_BINDINGS")
addon:RegisterEvent("CHAT_MSG_ADDON")
addon:SetScript("OnEvent", function(_, event, prefix, message, channel, sender)
    if event == "PLAYER_ENTERING_WORLD" then
        WorldDigimonDB = WorldDigimonDB or {}
        wantedFight = WorldDigimonDB.fight == true
        setFight(wantedFight)
        sync()
    elseif event == "PLAYER_REGEN_ENABLED" then
        setFight(wantedFight)
    elseif event == "UPDATE_BINDINGS" then
        bindBar()
    elseif prefix == "WDMMO" and sender == UnitName("player") and channel == "WHISPER" then
        local fields = {strsplit("|", message)}
        if fields[1] ~= "V1" then return end
        if fields[2] == "S" and #fields == 13 then
            local numbers = {}
            for i = 3, 8 do
                numbers[i] = tonumber(fields[i])
                if not numbers[i] or numbers[i] < 0 then return end
            end
            local slots = {}
            for i = 1, 5 do
                local spell, cost, remaining = strsplit(",", fields[i + 8])
                spell, cost, remaining = tonumber(spell), tonumber(cost), tonumber(remaining)
                if not spell or not cost or not remaining or spell < 0 or cost < 0 or remaining < 0 then return end
                slots[i] = {spell = spell, cost = cost, remaining = remaining}
            end
            state = {active = numbers[3], level = numbers[4], hp = numbers[5], maxhp = numbers[6],
                focus = numbers[7], maxfocus = numbers[8], slots = slots}
            receivedAt = GetTime()
            refresh()
        elseif fields[2] == "FIGHT" then
            setFight(true)
        elseif fields[2] == "MENU" then
            menu:Show()
        elseif fields[2] == "NOTICE" then
            notice(fields[3] or "")
        elseif fields[2] == "CAST" then
            local spell = tonumber(fields[3])
            local name = spell and (names[spell] or GetSpellInfo(spell))
            if name then notice("Orden enviada: " .. name) end
        end
    end
end)
addon:SetScript("OnUpdate", function(_, elapsed)
    updateElapsed = updateElapsed + elapsed
    if updateElapsed < 3 then return end
    updateElapsed = 0
    if bar:IsShown() and GetTime() - receivedAt > 4 then sync(); refresh() end
end)
