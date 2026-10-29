-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
local function meter(parent,y,color)
 local b=CreateFrame("StatusBar",nil,parent); b:SetSize(205,13); b:SetPoint("TOPLEFT",80,y)
 b:SetStatusBarTexture(W.asset.."Fill"); b:SetStatusBarColor(unpack(color)); b:SetMinMaxValues(0,1)
 local bg=b:CreateTexture(nil,"BACKGROUND"); bg:SetAllPoints(); bg:SetTexture(.03,.09,.14,.9)
 b.text=W.Label(b,10,"",6,-1); return b
end
local function unitPanel(name,y,title)
 local p=CreateFrame("Frame",name,UIParent); p:SetSize(310,104); W.Skin(p,"HUD"); W.Place(p,name,"TOPLEFT",20,y)
 p.title=W.Label(p,14,title,80,-14); p.title:SetWidth(210)
 p.hp=meter(p,-39,{.89,.30,.34}); p.power=meter(p,-57,{.09,.55,.98})
 p.detail=W.Label(p,10,"",80,-78); p.detail:SetWidth(210)
 p.portrait=p:CreateTexture(nil,"ARTWORK"); p.portrait:SetSize(55,64); p.portrait:SetPoint("TOPLEFT",15,-19)
 return p
end
W.tamer=unitPanel("WorldDigimonTamer",-20,"TAMER")
W.partner=unitPanel("WorldDigimonPartner",-132,"SIN COMPANERO")
W.partner:SetHeight(138); W.partner.detail:SetHeight(44)
local model=CreateFrame("PlayerModel",nil,W.partner); model:SetSize(60,72); model:SetPoint("TOPLEFT",12,-16); model:Hide(); W.model=model
local stages={"Rookie","Champion","Ultimate","Mega"}
local iconBySpecies={
 [1]="Interface\\Icons\\Ability_Hunter_Pet_Wolf",
 [103]="Interface\\Icons\\Spell_Shadow_SummonImp",
 [104]="Interface\\Icons\\Spell_Shadow_SummonImp",
 [105]="Interface\\Icons\\Spell_Shadow_MindSteal",
 [106]="Interface\\Icons\\Ability_Mount_Dreadsteed",
 [107]="Interface\\Icons\\Ability_Mount_Dreadsteed",
 [108]="Interface\\Icons\\Spell_Shadow_SummonFelguard",
 [109]="Interface\\Icons\\Spell_Shadow_VampiricEmbrace",
 [110]="Interface\\Icons\\Spell_Shadow_VampiricEmbrace",
 [111]="Interface\\Icons\\Spell_Shadow_VampiricEmbrace",
 [112]="Interface\\Icons\\Spell_Shadow_RaiseDead",
 [113]="Interface\\Icons\\Spell_Shadow_Shadowform",
 [114]="Interface\\Icons\\Spell_Shadow_Shadowform",
 [115]="Interface\\Icons\\Spell_Shadow_SoulLeech",
 [116]="Interface\\Icons\\Spell_Shadow_SoulLeech",
 [117]="Interface\\Icons\\Spell_Shadow_SoulLeech",
}
local function companionIcon(c)
 return iconBySpecies[c and c.species] or "Interface\\Icons\\INV_Misc_QuestionMark"
end
local function applyCompanionModel(c)
 if not c then return false end
 local ok=false
 if model.SetDisplayInfo and c.display and c.display>0 then ok=pcall(model.SetDisplayInfo,model,c.display) end
 if not ok and model.SetCreature and c.display and c.display>0 then ok=pcall(model.SetCreature,model,c.display) end
 return ok
end
local function fill(bar,value,max)
 value=math.max(0,value or 0); max=math.max(1,max or 1); bar:SetMinMaxValues(0,max); bar:SetValue(math.min(value,max)); bar.text:SetText(value.." / "..max)
end
function W.RefreshHUD()
 if not W.cfg then return end
 local t=W.tamer; t.title:SetText((UnitName("player") or "Tamer").."  /  "..UnitLevel("player"))
 fill(t.hp,UnitHealth("player"),UnitHealthMax("player")); fill(t.power,UnitMana("player"),UnitManaMax("player"))
 t.detail:SetText(W.cfg.secondary and "TAMER  /  WORLD DIGIMON" or "")
 if W.cfg.portraits then SetPortraitTexture(t.portrait,"player"); t.portrait:Show() else t.portrait:Hide() end
 local c=W.companions[W.state.active]; local p=W.partner
 p.title:SetText(c and (c.name.."  /  "..W.state.level) or "SIN COMPANERO")
 fill(p.hp,W.state.hp,W.state.maxhp); fill(p.power,W.state.focus,W.state.maxfocus)
 local requiredXP=c and c.level<100 and (100+75*c.level) or 0
 local stage=W.cfg.secondary and ((c and c.species==104 and "Baby II") or (c and c.species==107 and "Blast Mode") or (c and stages[c.stage]) or "") or ""
 local xp=W.state.xp or (c and c.xp) or 0
 local bond=c and (c.bond or W.state.bond or 0) or 0
 local bondName=bond>=100 and "Vínculo absoluto" or bond>=80 and "Vínculo profundo" or bond>=60 and "Armonía" or bond>=40 and "Compañerismo" or bond>=20 and "Confianza" or "Primer contacto"
 local detail=c and ((stage~="" and (stage.."  /  ") or "")..(c.level<100 and ("XP "..xp.." / "..requiredXP) or "NIVEL MAXIMO")) or ""
 if c and W.CareVisible() then
  local happiness=c.happiness or W.state.happiness or 50
  detail=detail.."\nEmpatía "..bond.."/100 · "..bondName.." · Felicidad "..happiness.."/100"
  local effects={}
  if W.state.damageBuffPct and W.state.damageBuffPct>0 and W.state.damageBuffExpires>GetTime() then
   effects[#effects+1]="Daño +"..W.state.damageBuffPct.."% ("..math.ceil((W.state.damageBuffExpires-GetTime())/60).."m)"
  end
  if W.state.protectionBuffPct and W.state.protectionBuffPct>0 and W.state.protectionBuffExpires>GetTime() then
   effects[#effects+1]="Defensa +"..W.state.protectionBuffPct.."% ("..math.ceil((W.state.protectionBuffExpires-GetTime())/60).."m)"
  end
  if #effects>0 then detail=detail.."\n"..table.concat(effects," · ") end
 end
 p.detail:SetText(detail)
 p.portrait:SetTexture(companionIcon(c)); p.portrait:SetAlpha(1)
 if W.cfg.portraits and c and c.display and c.display>0 then
  local key=(c.display or 0)..":"..(c.entry or 0)
  if W.display~=key then
   W.display=key; model:ClearModel(); W.modelOK=applyCompanionModel(c)
   if W.modelOK then model:SetCamera(0); model:SetFacing(.3) end
  end
  -- Keep the species icon visible as a fallback: SetDisplayInfo can succeed
  -- even when a 3.3.5 client cannot render that custom display file.
  if W.modelOK then model:Show() else model:Hide() end
 else model:Hide() end
 if W.RefreshMissionTracker then W.RefreshMissionTracker() end
end
W.target=CreateFrame("Frame","WorldDigimonTarget",UIParent); W.target:SetSize(310,82); W.Skin(W.target,"HUD"); W.Place(W.target,"target","TOPLEFT",340,-20)
W.target.title=W.Label(W.target,14,"SIN OBJETIVO",80,-14); W.target.title:SetWidth(210)
W.target.hp=meter(W.target,-39,{.89,.30,.34})
W.target.detail=W.Label(W.target,10,"",80,-59); W.target.detail:SetWidth(210)
W.target.portrait=W.target:CreateTexture(nil,"ARTWORK"); W.target.portrait:SetSize(55,58); W.target.portrait:SetPoint("TOPLEFT",15,-12)
W.target:Hide()
local mission=CreateFrame("Frame","WorldDigimonMissionTracker",UIParent); mission:SetSize(286,132); W.Skin(mission,"Panel"); W.Place(mission,"missions","TOPRIGHT",-22,-232)
mission.title=W.Label(mission,14,"MISIONES",16,-13); mission.title:SetTextColor(.2,.85,1)
mission.detail=W.Label(mission,11,"Habla con el Guía Digital para aceptar encargos de evolución.",16,-42); mission.detail:SetWidth(252); mission.detail:SetHeight(56)
mission.button=W.Button(mission,"Ver ramas evolutivas",188,28,function() W.Open("missions") end); mission.button:SetPoint("BOTTOMLEFT",14,10)
function W.RefreshMissionTracker()
 if W.cfg and not W.cfg.quests then mission:Hide(); return end
 local c=W.companions[W.state.active]
 local active=W.activeMission
 if active then
  if active.complete then
   mission.detail:SetText(active.name.."\nObjetivo cumplido. Vuelve al Guía Digital para reclamar la recompensa.")
  else
   mission.detail:SetText(active.name.."\nDerrota "..active.current.."/"..active.total.." "..active.target.." exactos.")
  end
 else
  mission.detail:SetText(c and ("Encargos para "..c.name.." disponibles con el Guía Digital.") or "Invoca un compañero y habla con el Guía Digital para consultar encargos.")
 end
 mission:Show()
end
function W.RefreshTarget()
 local p=W.target
 if not p then return end
 if TargetFrame then TargetFrame:Hide() end
 if not UnitExists("target") then p:Hide(); return end
 p:Show(); p.title:SetText(UnitName("target") or "Objetivo")
 local level=UnitLevel("target"); p.detail:SetText((level and level>0 and ("Nivel "..level.."  /  ") or "")..(UnitClassification("target") or ""))
 fill(p.hp,UnitHealth("target"),UnitHealthMax("target")); SetPortraitTexture(p.portrait,"target")
 local active=W.companions[W.state.active]
 if active and UnitName("target")==active.name then p.portrait:SetTexture(companionIcon(active)) end
end
local decorations={}
local function decorate(key,target,on)
 if not target then return end
 local d=decorations[key]
 if not d then
  d=CreateFrame("Frame",nil,target); d:SetFrameLevel(math.max(0,target:GetFrameLevel()-1))
  d:SetPoint("TOPLEFT",-8,8); d:SetPoint("BOTTOMRIGHT",8,-8)
  W.Skin(d,key=="minimap" and "Map" or "Panel"); d:EnableMouse(false); decorations[key]=d
 end
 if on then d:Show(); d.skin:SetAlpha(W.cfg.opacity*.6) else d:Hide() end
end
function W.ApplySkins()
 if InCombatLockdown() then return end
 W.NativeBars(W.cfg.classic and W.cfg.fight)
 -- Replace only the presentation of the player frame; target/party/raid remain native.
 if PlayerFrame then PlayerFrame:Hide() end
 if TargetFrame then TargetFrame:Hide() end
 decorate("minimap",Minimap,W.cfg.minimap)
 decorate("chat",ChatFrame1,W.cfg.chat)
 W.RefreshHUD(); W.RefreshTarget()
end
local function toggleBags()
 if ToggleAllBags then return ToggleAllBags() end
 local open=false
 for i=1,NUM_CONTAINER_FRAMES or 13 do local f=_G["ContainerFrame"..i]; if f and f:IsShown() then open=true; break end end
 if open and CloseAllBags then return CloseAllBags() end
 if OpenAllBags then return OpenAllBags() end
 if ToggleBackpack then return ToggleBackpack() end
end
local function openHelp()
 if HelpMicroButton and HelpMicroButton.Click then HelpMicroButton:Click(); return end
 if ToggleHelpFrame then ToggleHelpFrame(); return end
 if HelpFrame_Show then HelpFrame_Show(); return end
 if HelpFrame then ShowUIPanel(HelpFrame); return end
 W.Notice("No se encontro el panel de ayuda de WoW.")
end
local micro=CreateFrame("Frame","WorldDigimonShortcuts",UIParent); micro:SetSize(262,38); W.Place(micro,"shortcuts","BOTTOMRIGHT",-22,24)
local shortcuts={{"PJ",function() ToggleCharacter("PaperDollFrame") end,"Personaje"},
 {"B",toggleBags,"Mochilas"},{"M",function()
  local frame=QuestLogFrame or WorldMapFrame
  if frame then if frame:IsShown() then HideUIPanel(frame) else ShowUIPanel(frame) end end
 end,"Misiones"},
 {"UI",function() W.Open("config") end,"Configuracion Digivice"},{"?",openHelp,"Ayuda y soporte / Abrir ticket"}}
for i,s in ipairs(shortcuts) do
 local b=W.Button(micro,s[1],48,32,s[2]); b:SetPoint("LEFT",(i-1)*52,0); W.Tooltip(b,s[3],s[3])
 if s[1]=="?" then
  b.text:ClearAllPoints(); b.text:SetPoint("CENTER",0,1); b.text:SetFont("Fonts\\FRIZQT__.TTF",24,"OUTLINE"); b.text:SetTextColor(1,.16,.12)
 end
end
