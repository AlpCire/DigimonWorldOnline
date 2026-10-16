-- SPDX-License-Identifier: GPL-2.0-or-later
-- Lua 5.1 client contract checks. Does not emulate the protected WoW runtime.
local frames, sent, bindings = {}, {}, {}
local combat, now, notices, changed = false, 10, {}, 0
local frameMethods = {}
local hooks = {}
local function frame(name)
    local result = setmetatable({name = name, shown = true, scripts = {}, attributes = {}}, {__index = frameMethods})
    if name then frames[name] = result; _G[name] = result end
    return result
end
function frameMethods:SetScript(event, fn) self.scripts[event] = fn end
function frameMethods:HookScript(event, fn) self.scripts[event] = fn end
function frameMethods:GetID() return self.id or 0 end
function frameMethods:GetAlpha() return self.alpha or 1 end
function frameMethods:SetAlpha(value) assert(not combat); self.alpha = value end
function frameMethods:SetFrameStrata(value) self.strata = value end
function frameMethods:SetFrameLevel(value) self.level = value end
function frameMethods:GetItem() return "Dispositivo", "item:900110:0:0:0" end
function frameMethods:ClearLines() end
function frameMethods:SetAttribute(key, value)
    assert(not combat, "protected attribute changed in combat")
    self.attributes[key] = value
end
function frameMethods:GetName() return self.name end
function frameMethods:Show()
    if self.name == "WorldDigimonCombatBar" then assert(not combat, "bar shown in combat") end
    self.shown = true
end
function frameMethods:Hide()
    if self.name == "WorldDigimonCombatBar" then assert(not combat, "bar hidden in combat") end
    self.shown = false
end
function frameMethods:IsShown() return self.shown end
function frameMethods:SetText(value) self.text = value end
function frameMethods:SetTexture(value) self.texture = value end
function frameMethods:SetCooldown(start, duration) self.duration = duration end
function frameMethods:CreateTexture() return frame() end
function frameMethods:CreateFontString() return frame() end
for _, name in ipairs({"SetSize", "SetPoint", "SetBackdrop", "SetBackdropColor",
    "SetAllPoints", "SetNormalTexture", "SetPushedTexture", "SetHighlightTexture", "RegisterForClicks",
    "RegisterEvent", "SetVertexColor", "SetOwner", "AddLine"}) do
    frameMethods[name] = function() end
end
function CreateFrame(_, name) return frame(name) end
UIParent = frame("UIParent")
for i = 1, 12 do frame("ActionButton" .. i) end
ActionButton2.alpha = 0.6
UISpecialFrames, SlashCmdList = {}, {}
tinsert = table.insert
function GetTime() return now end
function InCombatLockdown() return combat end
function UnitName() return "Prueba" end
function CloseGossip() end
function GetBindingKey(action) return action:gsub("ACTIONBUTTON", ""), "ALT-" .. action end
function SetOverrideBindingClick(_, _, key, name)
    assert(not combat)
    bindings[key] = name
end
function ClearOverrideBindings() assert(not combat); bindings = {} end
function SendChatMessage(text, channel, _, target)
    assert(channel == "WHISPER" and target == "Prueba")
    table.insert(sent, text)
end
function GetSpellInfo(id) return "Spell " .. id, nil, "Icon " .. id end
function hooksecurefunc(name, fn) hooks[name] = fn end
function GetContainerItemLink() return "item:900110:0:0:0" end
function SetItemButtonTexture(button, icon) button.texture = icon end
function strsplit(separator, input)
    local result, start = {}, 1
    while true do
        local pos = string.find(input, separator, start, true)
        if not pos then table.insert(result, string.sub(input, start)); break end
        table.insert(result, string.sub(input, start, pos - 1)); start = pos + 1
    end
    return unpack(result)
end
UIErrorsFrame = {AddMessage = function(_, text) table.insert(notices, text) end}
GameTooltip = frame()
-- Bar setup must not rewrite the player's action slots or saved key bindings.
function PlaceAction() error("player action slot modified") end
function SetBinding() error("persistent binding modified") end
function CreateMacro() error("player macro modified") end

dofile("client/WorldDigimon/WorldDigimon.lua")
local events = frames.WorldDigimonEvents.scripts.OnEvent
local bar = frames.WorldDigimonCombatBar
events(nil, "PLAYER_ENTERING_WORLD")
assert(sent[#sent] == ".digimon sync")
assert(not bar:IsShown())

local snapshot = "V1|S|123|3|110|135|75|100|17253,25,0|24604,20,39000|0,0,0|0,0,0|0,0,0"
events(nil, "CHAT_MSG_ADDON", "WDMMO", snapshot, "WHISPER", "Prueba")
assert(bar.label.text:find("Nivel 3"))
assert(frames.WorldDigimonAction3.label == "Mordisco Digital")
assert(frames.WorldDigimonAction4.cooldown.duration == 39)
local goodText = bar.label.text
events(nil, "CHAT_MSG_ADDON", "WDMMO", snapshot:gsub("|3|", "|80|"), "WHISPER", "Otro")
events(nil, "CHAT_MSG_ADDON", "WDMMO", "V1|S|malformed", "WHISPER", "Prueba")
assert(bar.label.text == goodText, "invalid or foreign snapshot accepted")

SlashCmdList.WORLDDIGIMON("luchar")
assert(bar:IsShown() and bindings["3"] == "WorldDigimonAction3")
assert(ActionButton1:GetAlpha() == 0 and ActionButton2:GetAlpha() == 0)
assert(frames.WorldDigimonAction3.strata == "HIGH" and frames.WorldDigimonAction3.level > bar.level)
assert(frames.WorldDigimonAction3.cooldown.level > frames.WorldDigimonAction3.level)
assert(frames.WorldDigimonAction3.attributes.macrotext == "/s .digimon cast 1")
assert(frames.WorldDigimonAction4.attributes.macrotext == "/s .digimon cast 2")
assert(frames.WorldDigimonAction1.attributes.macrotext == "/s .digimon attack")
assert(frames.WorldDigimonAction2.attributes.macrotext == "/s .digimon follow")
assert(frames.WorldDigimonAction12.attributes.macrotext == "/digimon")

combat = true
SlashCmdList.WORLDDIGIMON("salir")
assert(bar:IsShown(), "protected combat transition was not deferred")
assert(bindings["3"] == "WorldDigimonAction3")
combat = false
events(nil, "PLAYER_REGEN_ENABLED")
assert(not bar:IsShown() and next(bindings) == nil, "original bindings were not restored")
assert(ActionButton1:GetAlpha() == 1 and ActionButton2:GetAlpha() == 0.6, "original visuals were not restored")
combat = true
events(nil, "CHAT_MSG_ADDON", "WDMMO", "V1|FIGHT", "WHISPER", "Prueba")
assert(not bar:IsShown())
combat = false
events(nil, "PLAYER_REGEN_ENABLED")
assert(bar:IsShown())
events(nil, "CHAT_MSG_ADDON", "WDMMO", "V1|CAST|17253", "WHISPER", "Prueba")
assert(notices[#notices] == "Orden enviada: Mordisco Digital")
local bag = frame("ContainerFrameTest")
bag.size = 1
local device = frame("ContainerFrameTestItem1")
hooks.ContainerFrame_Update(bag)
assert(device.texture == "Interface\\Icons\\INV_Misc_Book_07")
GameTooltip.scripts.OnTooltipSetItem(GameTooltip)
assert(GameTooltip.text == "Dispositivo Sagrado")
print("PASS: equipped slots, cooldown, own-server messages, malformed input, secure macros, combat deferral, binding restoration")
