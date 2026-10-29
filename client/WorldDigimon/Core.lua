-- SPDX-License-Identifier: GPL-2.0-or-later
-- WoW 3.3.5a / Lua 5.1. The server owns progression and combat.
WorldDigimonUI={state={active=0,level=0,xp=0,xpRequired=0,bond=0,happiness=50,damageBuffPct=0,damageBuffExpires=0,protectionBuffPct=0,protectionBuffExpires=0,hp=0,maxhp=1,focus=0,maxfocus=100,mode="defensive",slots={}},
 companions={},techniques={},routes={},names={},frames={},talents=nil,received=0,page=1}
local W=WorldDigimonUI
W.asset="Interface\\AddOns\\WorldDigimon\\Textures\\"
W.defaults={scale=1,combatScale=1,opacity=.96,glow=.65,locked=true,portraits=true,
 secondary=true,careUI=true,minimap=true,quests=true,chat=true,classic=true,fight=false,positions={}}
-- Shared visibility gate for empathy, happiness and companion food/buff widgets.
function W.CareVisible() return not W.cfg or W.cfg.careUI~=false end
function W.Notice(text) UIErrorsFrame:AddMessage(text,.4,.92,1) end
function W.Request(command)
 if type(command)~="string" or #command>80 or command:find("[\r\n|]") then return end
 SendChatMessage(".digimon "..command,"WHISPER",nil,UnitName("player"))
end
local last={}
function W.Sync(kind)
 kind=kind or "sync"; if GetTime()-(last[kind] or -100)<1 then return end
 last[kind]=GetTime(); W.Request(kind)
end
function W.Number(v,max)
 local n=tonumber(v)
 if not n or n~=n or n<0 or n>(max or 1000000000) or n%1~=0 then return nil end
 return n
end
function W.Text(s,limit) return type(s)=="string" and #s>0 and #s<=(limit or 160) and not s:find("[|\r\n]") end
function W.InitConfig()
 if type(WorldDigimonDB)~="table" then WorldDigimonDB={} end; W.cfg=WorldDigimonDB
 for k,v in pairs(W.defaults) do if type(W.cfg[k])~=type(v) then W.cfg[k]=type(v)=="table" and {} or v end end
 for _,k in ipairs({"scale","combatScale"}) do W.cfg[k]=math.max(.65,math.min(1.4,W.cfg[k]==W.cfg[k] and W.cfg[k] or 1)) end
 W.cfg.opacity=math.max(.5,math.min(1,W.cfg.opacity)); W.cfg.glow=math.max(0,math.min(1,W.cfg.glow))
end
function W.Place(frame,key,point,x,y)
 frame.defaultPosition={point,x,y}; frame.positionKey=key
 frame:SetMovable(true); frame:SetClampedToScreen(true); frame:EnableMouse(true); frame:RegisterForDrag("LeftButton")
 frame:SetScript("OnDragStart",function(self) if W.cfg and not W.cfg.locked and not InCombatLockdown() then self:StartMoving() end end)
 frame:SetScript("OnDragStop",function(self)
  self:StopMovingOrSizing(); local p,_,_,dx,dy=self:GetPoint(); W.cfg.positions[key]={p,dx,dy}
 end)
 frame:SetPoint(point,UIParent,point,x,y); W.frames[#W.frames+1]=frame
end
function W.ApplyLayout()
 if InCombatLockdown() then W.layoutPending=true; return end; W.layoutPending=false
 local auto=math.min(1,UIParent:GetWidth()/1250,UIParent:GetHeight()/740)
 for _,f in ipairs(W.frames) do
  local p=W.cfg.positions[f.positionKey] or f.defaultPosition
  local allowed={TOP=true,TOPLEFT=true,TOPRIGHT=true,BOTTOM=true,BOTTOMLEFT=true,BOTTOMRIGHT=true,CENTER=true,LEFT=true,RIGHT=true}
  if type(p)~="table" or not allowed[p[1]] or type(p[2])~="number" or type(p[3])~="number" or p[2]~=p[2] or p[3]~=p[3] or math.abs(p[2])>4000 or math.abs(p[3])>3000 then p=f.defaultPosition end
  f:ClearAllPoints(); f:SetPoint(p[1],UIParent,p[1],p[2],p[3]); f:SetScale(auto*W.cfg.scale*(f==W.bar and W.cfg.combatScale or 1))
  if f.skin then f.skin:SetAlpha(W.cfg.opacity) end; if f.circuit then f.circuit:SetAlpha(W.cfg.glow) end
 end
 if W.ApplySkins then W.ApplySkins() end
end
local event=CreateFrame("Frame","WorldDigimonEvents"); W.event=event
for _,name in ipairs({"PLAYER_ENTERING_WORLD","PLAYER_REGEN_ENABLED","UPDATE_BINDINGS","CHAT_MSG_ADDON",
 "UNIT_HEALTH","UNIT_MAXHEALTH","UNIT_MANA","UNIT_RAGE","UNIT_ENERGY","UNIT_RUNIC_POWER",
 "UNIT_DISPLAYPOWER","PLAYER_LEVEL_UP","DISPLAY_SIZE_CHANGED","UI_SCALE_CHANGED","PLAYER_TARGET_CHANGED"}) do event:RegisterEvent(name) end
event:SetScript("OnEvent",function(_,ev,a,b,c,d)
 if ev=="PLAYER_ENTERING_WORLD" then W.InitConfig(); W.ApplyLayout(); W.SetFight(W.cfg.fight); W.Sync(); W.Sync("details"); W.Sync("routes"); W.RefreshHUD()
 elseif ev=="PLAYER_REGEN_ENABLED" then W.SetFight(W.cfg and W.cfg.fight); W.ApplyLayout()
 elseif ev=="UPDATE_BINDINGS" then W.BindBar()
 elseif ev=="CHAT_MSG_ADDON" then
  if a=="WDMMO" and c=="WHISPER" and d==UnitName("player") and type(b)=="string" and #b<=250 then W.Receive(b) end
 elseif ev=="DISPLAY_SIZE_CHANGED" or ev=="UI_SCALE_CHANGED" then if W.cfg then W.ApplyLayout() end
 elseif W.cfg then W.RefreshHUD(); W.RefreshTarget() end
end)
local elapsed=0
event:SetScript("OnUpdate",function(_,delta)
 elapsed=elapsed+delta; if elapsed<.25 then return end; elapsed=0
 if not W.cfg then return end; W.RefreshCombat(); if GetTime()-W.received>4 then W.Sync() end
end)
SLASH_WORLDDIGIMON1="/digimon"
SlashCmdList.WORLDDIGIMON=function(text)
 if not W.cfg then W.InitConfig() end
 if text=="luchar" then W.SetFight(true) elseif text=="salir" then W.SetFight(false)
 elseif text=="config" then W.Open("config") elseif text=="reset" then W.cfg.positions={}; W.ApplyLayout(); W.Notice("Posiciones restauradas.")
 else W.Toggle() end
end
