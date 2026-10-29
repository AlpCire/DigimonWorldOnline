-- SPDX-License-Identifier: GPL-2.0-or-later
-- Per-character discovery for native Digital World maps (WoW 3.3.5a).
local W = WorldDigimonUI
local COLUMNS, ROWS = 12, 8
local stamps = {}
local fog, lastMap
local textureRoot = "Interface\\AddOns\\WorldDigimon\\Textures\\MapReveal\\"

local function IsDigitalMap(name)
 return name == "DWCContinent" or (type(name) == "string" and name:match("^DWCZone%d%d$"))
end

local function Seen()
 if type(WorldDigimonDB) ~= "table" then return nil end
 if type(WorldDigimonDB.mapDiscovery) ~= "table" then WorldDigimonDB.mapDiscovery = {} end
 return WorldDigimonDB.mapDiscovery
end

local function Mark(name, x, y)
 if not IsDigitalMap(name) or type(x) ~= "number" or type(y) ~= "number" or
    x <= 0 or y <= 0 or x >= 1 or y >= 1 then return end
 local all = Seen(); if not all then return end
 if type(all[name]) ~= "table" then all[name] = {} end
 local col = math.min(COLUMNS, math.floor(x * COLUMNS) + 1)
 local row = math.min(ROWS, math.floor(y * ROWS) + 1)
 local index = (row - 1) * COLUMNS + col
 if not all[name][index] then
  all[name][index] = true
  if WorldMapFrame and WorldMapFrame:IsShown() and GetMapInfo() == name then W.RefreshMapDiscovery() end
 end
end

local function BuildFog()
 if fog or not WorldMapDetailFrame then return end
 fog = CreateFrame("Frame", "WorldDigimonMapDiscovery", WorldMapDetailFrame)
 fog:SetWidth(1002); fog:SetHeight(668)
 fog:SetPoint("TOPLEFT", WorldMapDetailFrame, "TOPLEFT", 0, 0)
 fog:SetFrameLevel(WorldMapDetailFrame:GetFrameLevel() + 5)
 fog:EnableMouse(false)
 local shade = fog:CreateTexture(nil, "ARTWORK")
 shade:SetAllPoints(fog)
 shade:SetTexture(0.49, 0.36, 0.21, 1)
 shade:SetAlpha(0.82)
 for row = 1, ROWS do
  for col = 1, COLUMNS do
   local stamp = fog:CreateTexture(nil, "OVERLAY")
   stamp:SetWidth(256); stamp:SetHeight(256)
   stamp:SetPoint("TOPLEFT", fog, "TOPLEFT",
     math.floor((col - 0.5) * 1002 / COLUMNS - 128 + 0.5),
     -math.floor((row - 0.5) * 668 / ROWS - 128 + 0.5))
   stamps[(row - 1) * COLUMNS + col] = stamp
   stamp:Hide()
  end
 end
 fog:Hide()
end

function W.RefreshMapDiscovery()
 BuildFog()
 if not fog then return end
 local name = GetMapInfo()
 if not IsDigitalMap(name) then fog:Hide(); lastMap = name; return end
 local all = Seen()
 local seen = all and all[name]
 if type(seen) ~= "table" then seen = {} end
 for index = 1, COLUMNS * ROWS do
  local stamp = stamps[index]
  if seen[index] then
   stamp:SetTexture(textureRoot .. name .. "\\" .. string.format("%03d", index))
   stamp:Show()
  else
   stamp:Hide()
  end
 end
 lastMap = name
 fog:Show()
end

local function SamplePosition()
 if not SetMapToCurrentZone or not GetPlayerMapPosition or
    (WorldMapFrame and WorldMapFrame:IsShown()) then return end
 SetMapToCurrentZone()
 local name = GetMapInfo()
 if not IsDigitalMap(name) then return end
 local x, y = GetPlayerMapPosition("player")
 Mark(name, x, y)
 if name ~= "DWCContinent" and SetMapByID then
  SetMapByID(6200)
  if GetMapInfo() == "DWCContinent" then
   local worldX, worldY = GetPlayerMapPosition("player")
   Mark("DWCContinent", worldX, worldY)
  end
  SetMapToCurrentZone()
 end
end

local watcher = CreateFrame("Frame")
watcher:RegisterEvent("ADDON_LOADED")
watcher:RegisterEvent("PLAYER_ENTERING_WORLD")
watcher:SetScript("OnEvent", function(_, event, addon)
 if event == "ADDON_LOADED" and addon ~= "Blizzard_WorldMap" and addon ~= "WorldDigimon" then return end
 BuildFog()
 if not watcher.hooked and WorldMapFrame_UpdateMap and hooksecurefunc then
  hooksecurefunc("WorldMapFrame_UpdateMap", W.RefreshMapDiscovery)
  watcher.hooked = true
 end
end)
local elapsed = 0
watcher:SetScript("OnUpdate", function(_, delta)
 elapsed = elapsed + delta
 if elapsed < 3 then return end
 elapsed = 0
 BuildFog()
 if WorldMapFrame and WorldMapFrame:IsShown() then
  if GetMapInfo() ~= lastMap then W.RefreshMapDiscovery() end
 else
  SamplePosition()
 end
end)
