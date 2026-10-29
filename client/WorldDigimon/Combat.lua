-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
local bar=CreateFrame("Frame","WorldDigimonCombatBar",UIParent); W.bar=bar
bar:SetSize(670,92); bar:SetFrameStrata("HIGH"); bar:SetFrameLevel(100); W.Skin(bar,"Combat"); W.Place(bar,"combat","BOTTOM",0,128); bar:Hide()
bar.label=W.Label(bar,12,"ENLACE DIGITAL",20,-12); W.buttons={}; W.modeButtons={}
local actions={{"Atacar","attack","Ability_Hunter_Pet_Attack"},{"Seguir","follow","Ability_Hunter_Pet_Follow"},
 {"Tecnica 1","cast 1"},{"Tecnica 2","cast 2"},{"Tecnica 3","cast 3"},{"Tecnica 4","cast 4"},{"Especial","cast 5"},
 {"Capturar","capture","Spell_Arcane_PortalDalaran"},{"Invocar","summon","Ability_Hunter_BeastCall"},
 {"Retirar","withdraw","Ability_Hunter_BeastSoothe"},{"Descansar","recover","Spell_Holy_Heal"},{"Dispositivo",nil,"INV_Misc_Book_07"}}
for i,a in ipairs(actions) do
 local b=CreateFrame("CheckButton","WorldDigimonAction"..i,bar,"SecureActionButtonTemplate"); b:SetSize(46,46); b:SetPoint("BOTTOMLEFT",bar,"BOTTOMLEFT",20+(i-1)*52,14)
 b:SetFrameLevel(120); b:SetFrameStrata("HIGH"); b:RegisterForClicks("AnyUp"); b:SetAttribute("type","macro")
 b:SetAttribute("macrotext",a[2] and ("/s .digimon "..a[2]) or "/click WorldDigimonLauncher")
 b.label=a[1]; b.slot=(i>=3 and i<=7) and i-2 or nil
 b:SetNormalTexture(W.asset.."Slot")
 -- The old BACKGROUND layer sat underneath the slot's normal texture in 3.3.5a,
 -- making the permanent actions look empty. Keep the icon above the frame art.
 b.icon=b:CreateTexture(nil,"OVERLAY",nil,1); b.icon:SetPoint("TOPLEFT",4,-4); b.icon:SetPoint("BOTTOMRIGHT",-4,4)
 b.icon:SetTexture(a[3] and ("Interface\\Icons\\"..a[3]) or (W.asset.."Empty")); b.icon:SetAlpha(1); b.icon:SetVertexColor(1,1,1,1)
 b:SetHighlightTexture(W.asset.."Highlight","ADD"); b:SetPushedTexture(W.asset.."ButtonDown")
 b.cooldown=CreateFrame("Cooldown",nil,b,"CooldownFrameTemplate"); b.cooldown:SetAllPoints(b.icon); b.cooldown:SetFrameLevel(121)
 b.key=W.Label(b,10,"",2,-2); b.cost=W.Label(b,10,"",28,-33)
 b:SetScript("OnEnter",function(self)
  GameTooltip:SetOwner(self,"ANCHOR_TOP"); GameTooltip:SetText(self.label); local slot=self.slot and W.state.slots[self.slot]
  if slot and slot.spell>0 then GameTooltip:AddLine("Energia: "..slot.cost.." / Recuperacion: "..math.max(0,math.ceil((slot.expires or 0)-GetTime())).." s",.7,.9,1,true)
  elseif self.slot then GameTooltip:AddLine("Equipa una tecnica desde Dispositivo > Tecnicas.",.7,.9,1,true) end
  GameTooltip:Show()
 end)
 b:SetScript("OnLeave",function() GameTooltip:Hide() end); W.buttons[i]=b
end
-- Small stance controls stay above the action row and do not consume action slots.
-- Their selection is only a view of the server-owned mode; the server remains authoritative.
local modes={
 {"aggressive","Agresivo","Ability_Hunter_Pet_Aggressive"},
 {"defensive","Defensivo","Ability_Hunter_Pet_Defensive"},
 {"passive","Pasivo","Ability_Hunter_Pet_Passive"}
}
for i,mode in ipairs(modes) do
 local b=CreateFrame("CheckButton","WorldDigimonMode"..i,bar,"SecureActionButtonTemplate")
 b:SetSize(24,24); b:SetPoint("TOPRIGHT",bar,"TOPRIGHT",-14-(i-1)*28,-7)
 b:SetFrameLevel(125); b:SetFrameStrata("HIGH"); b:RegisterForClicks("AnyUp")
 b:SetAttribute("type","macro"); b:SetAttribute("macrotext","/s .digimon mode "..mode[1])
 b:SetNormalTexture(W.asset.."Slot"); b:SetHighlightTexture(W.asset.."Highlight","ADD")
 b.icon=b:CreateTexture(nil,"OVERLAY",nil,1); b.icon:SetPoint("TOPLEFT",3,-3); b.icon:SetPoint("BOTTOMRIGHT",-3,3)
 b.icon:SetTexture("Interface\\Icons\\"..mode[3]); b.icon:SetAlpha(1); b.icon:SetVertexColor(1,1,1,1)
 b.selected=b:CreateTexture(nil,"OVERLAY",nil,2); b.selected:SetAllPoints(); b.selected:SetTexture(W.asset.."Highlight"); b.selected:SetAlpha(.7); b.selected:Hide()
 b.mode=mode[1]; b.modeLabel=mode[2]
 b:SetScript("OnEnter",function(self)
  GameTooltip:SetOwner(self,"ANCHOR_TOP"); GameTooltip:SetText(self.modeLabel)
  GameTooltip:AddLine("Cambiar conducta del compañero",.7,.9,1,true); GameTooltip:Show()
 end)
 b:SetScript("OnLeave",function() GameTooltip:Hide() end)
 W.modeButtons[i]=b
end
function W.BindBar()
 if InCombatLockdown() then W.bindPending=true; return end; W.bindPending=false; ClearOverrideBindings(bar)
 if not bar:IsShown() then return end
 for i,b in ipairs(W.buttons) do local keys={GetBindingKey("ACTIONBUTTON"..i)}
  for _,key in ipairs(keys) do SetOverrideBindingClick(bar,true,key,b:GetName(),"LeftButton") end; b.key:SetText(keys[1] or "")
 end
end
local originals={}
function W.NativeBars(hide)
 if InCombatLockdown() then return end
 if IsAddOnLoaded and (IsAddOnLoaded("Bartender4") or IsAddOnLoaded("Dominos")) then hide=false end
 for _,name in ipairs({"MainMenuBar","MultiBarBottomLeft","MultiBarBottomRight"}) do local f=_G[name]
  if f then
   if hide and not originals[name] then originals[name]={shown=f:IsShown()}; if RegisterStateDriver then RegisterStateDriver(f,"visibility","hide") else f:Hide() end
   elseif not hide and originals[name] then if UnregisterStateDriver then UnregisterStateDriver(f,"visibility") end
    if originals[name].shown then f:Show() else f:Hide() end; originals[name]=nil
   end
  end
 end
end
function W.SetFight(on)
 if not W.cfg then W.InitConfig() end; W.cfg.fight=not not on
 if InCombatLockdown() then W.Notice("El cambio de barra se aplicara al salir de combate."); return end
 if on then CloseGossip(); if W.panel then W.panel:Hide() end; bar:Show(); W.Sync() else bar:Hide() end
 W.NativeBars(on and W.cfg.classic); W.BindBar(); W.RefreshCombat()
end
function W.RefreshCombat()
 if not W.cfg then return end; local s=W.state; local stale=GetTime()-W.received>6
 bar.label:SetText(stale and "ENLACE DIGITAL  /  Esperando al servidor" or string.format("LUCHAR  /  Nivel %d    Salud %d/%d    Energia %d/%d",s.level,s.hp,s.maxhp,s.focus,s.maxfocus))
 for _,b in ipairs(W.modeButtons) do
  local active=not stale and (s.mode or "defensive")==b.mode
  if active then b.selected:Show() else b.selected:Hide() end
  b.icon:SetVertexColor(active and 1 or .58,active and 1 or .72,active and 1 or .82,1)
 end
 for i=1,5 do local b=W.buttons[i+2]; local slot=s.slots[i]
  if slot and slot.spell>0 then
   local name,_,icon=GetSpellInfo(slot.spell); b.label=W.names[slot.technique] or name or ("Tecnica "..i)
   local custom=WorldDigimonAbilityIcons and WorldDigimonAbilityIcons[slot.technique]
   icon=custom and WorldDigimonAbilityTextureRoot and (WorldDigimonAbilityTextureRoot..custom..".tga") or icon or (W.asset.."Slot")
   if b.lastIcon~=icon then b.icon:SetTexture(icon); b.lastIcon=icon end
   local bright=not stale and s.hp>0 and s.focus>=slot.cost and 1 or .45
   if b.lastBright~=bright then b.icon:SetVertexColor(bright,bright,bright); b.lastBright=bright end
   if b.lastCost~=slot.cost then b.cost:SetText(slot.cost); b.lastCost=slot.cost end
   if b.lastExpiry~=slot.expires then b.lastExpiry=slot.expires; local remain=math.max(0,slot.expires-GetTime()); b.cooldown:SetCooldown(remain>0 and GetTime() or 0,remain) end
   b.icon:SetAlpha(1)
  else
   b.label="Espacio sin tecnica equipada"
   if not b.empty then b.icon:SetTexture(nil); b.icon:SetAlpha(0); b.cost:SetText(""); b.cooldown:SetCooldown(0,0); b.lastExpiry=nil; b.lastIcon=nil; b.lastCost=nil; b.lastBright=nil; b.empty=true end
  end
  if slot and slot.spell>0 then b.empty=false end
 end
end
