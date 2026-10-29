-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
local unpack=unpack
local branches={
 {name="ASALTO",color={.93,.42,.21},subtitle="Daño, movilidad y presión",icon="Ability_Warrior_SavageBlow"},
 {name="GUARDIA",color={.38,.68,.94},subtitle="Resistencia y protección",icon="Ability_Warrior_DefensiveStance"},
 {name="SOPORTE",color={.45,.86,.58},subtitle="Curación y energía",icon="Spell_Holy_FlashHeal"},
}
local nodes={
 [1]={1,0,1,10,0,0,0,"Furia de datos","Ability_Warrior_SavageBlow","Aumenta el daño del Digimon un %d%%."},
 [4]={1,0,2,5,0,0,0,"Surcador del viento","Ability_Rogue_Sprint","Aumenta la velocidad de movimiento del Tamer un %d%% mientras el Digimon está invocado."},
 [5]={1,0,3,10,0,0,0,"Filo veloz","Ability_Hunter_SwiftStrike","Reduce un %d%% el tiempo entre ataques básicos del Digimon."},
 [6]={1,1,1,5,5,1,3,"Golpe preciso","Ability_Rogue_Ambush","Otorga un %d%% de probabilidad de infligir un 50%% de daño adicional."},
 [7]={1,1,3,5,5,5,3,"Recarga","Spell_Arcane_Arcane01","Recupera %d puntos adicionales de energía cada 4 segundos."},
 [8]={1,2,1,10,10,6,3,"Explotar debilidad","Ability_Rogue_FeignDeath","Aumenta el daño un %d%% contra enemigos con menos del 50%% de salud."},
 [9]={1,2,3,1,10,7,3,"Descarga digital","Spell_Shadow_ShadowBolt","Desbloquea Descarga de datos, un ataque dirigido."},
 [10]={1,3,2,1,15,8,3,"Ejecutor","Ability_Warrior_DecisiveStrike","Aumenta un 10% el daño contra élites y jefes."},
 [11]={1,4,2,1,20,9,1,"Tormenta de datos","Spell_Nature_ChainLightning","Desbloquea Tormenta de datos, un ataque encadenado."},
 [2]={2,0,1,10,0,0,0,"Núcleo blindado","Spell_Holy_DevotionAura","Aumenta la salud máxima del Digimon un %d%%."},
 [12]={2,0,2,10,0,0,0,"Capa protectora","Ability_Warrior_ShieldWall","Reduce un %d%% el daño recibido por el Digimon."},
 [13]={2,0,3,5,0,0,0,"Custodio","Ability_Warrior_DefensiveStance","Aumenta un %d%% la amenaza adicional generada por los ataques."},
 [14]={2,1,1,5,5,2,3,"Respuesta tenaz","Ability_Warrior_Revenge","Aumenta el daño un %d%% mientras el Digimon tenga menos del 50%% de salud."},
 [15]={2,1,3,10,5,13,3,"Reserva defensiva","Spell_Holy_InnerFire","Aumenta la energía máxima del Digimon en %d puntos."},
 [16]={2,2,1,5,10,14,3,"Vigor restaurado","Spell_Holy_Renew","Aumenta un %d%% las curas recibidas por el Digimon."},
 [17]={2,2,3,1,10,15,3,"Provocar","Spell_Nature_Reincarnation","Desbloquea Provocación digital, que obliga al enemigo a atacar al Digimon."},
 [18]={2,3,2,1,15,16,3,"Último muro","Ability_Warrior_ShieldMastery","Reduce un 10% adicional el daño recibido con menos del 35% de salud."},
 [19]={2,4,2,1,20,17,1,"Barrera digital","Spell_Holy_PowerWordShield","Desbloquea Barrera de datos para proteger a un Digimon aliado."},
 [3]={3,0,1,10,0,0,0,"Restauración","Spell_Holy_FlashHeal","Aumenta un %d%% las curas del Digimon."},
 [20]={3,0,2,10,0,0,0,"Reserva espiritual","Spell_Holy_InnerFire","Aumenta la energía máxima del Digimon en %d puntos."},
 [21]={3,0,3,5,0,0,0,"Serenidad","Spell_Nature_HealingWaveGreater","Recupera %d puntos adicionales de energía cada 4 segundos."},
 [22]={3,1,1,10,5,3,3,"Auxilio","Spell_Holy_Heal","Aumenta un %d%% las curas sobre aliados con menos del 50%% de salud."},
 [23]={3,1,3,5,5,21,3,"Fluidez","Spell_Holy_SurgeOfLight","Reduce un %d%% la reutilización de Pulso sanador y Sanación de emergencia."},
 [24]={3,2,1,5,10,22,3,"Eco protector","Spell_Holy_PowerWordShield","Al curar a un Digimon aliado sin barrera, aplica una barrera equivalente al %d%% de la curación."},
 [25]={3,2,3,1,10,23,3,"Sanación de emergencia","Spell_Holy_FlashHeal","Desbloquea Sanación de emergencia, una cura potente."},
 [26]={3,3,2,1,15,24,3,"Vínculo de grupo","Spell_Holy_PrayerOfHealing02","Aumenta un 15% las curas a Digimon de otros jugadores."},
 [27]={3,4,2,1,20,25,1,"Barrera de red","Spell_Holy_PowerWordShield","Desbloquea Barrera de datos para proteger a un Digimon aliado."},
}
local frame=W.Panel("WorldDigimonTalentTree",552,638,"TALENTOS DEL DIGIMON")
frame:SetFrameStrata("DIALOG")
local branch=1
local points=W.Label(frame,12,"",26,-76); points:SetWidth(495)
local detail=W.Label(frame,10,"Un punto por nivel desde el nivel 2. Elige dónde especializar cada forma.",26,-96)
detail:SetWidth(495)
local tabs={}
local icons={}
local lines={}
local profileNames={
 [0]="Instinto",[1]="Travesura",[2]="Hechicería",[3]="Oscuridad",
 [4]="Espectro",[5]="Brujería",[6]="Artillería",[7]="Luz sagrada",
}
local signatures={
 [0]={{"Furia de datos","Aumenta el daño del Digimon un %d%%."},{"Explotar debilidad","Aumenta el daño un %d%% contra enemigos con menos del 50%% de salud."}},
 [1]={{"Travesura feroz","Aumenta el daño del Digimon un %d%%."},{"Golpe oportunista","Aumenta el daño un %d%% contra enemigos con menos del 50%% de salud."}},
 [2]={{"Hechizo preciso","Aumenta el daño de las técnicas un %d%%."},{"Remate arcano","Aumenta el daño de las técnicas un %d%% contra enemigos con menos del 50%% de salud."}},
 [3]={{"Poder oscuro","Aumenta el daño del Digimon un %d%%."},{"Cazador de sombras","Aumenta el daño un %d%% contra enemigos con menos del 50%% de salud."}},
 [4]={{"Presencia espectral","Aumenta el daño un %d%% mientras el Digimon tenga más del 75%% de salud."},{"Acoso fantasmal","Aumenta el daño un %d%% contra enemigos con al menos el 50%% de salud."}},
 [5]={{"Maldición intensa","Aumenta el daño un %d%% mientras la energía del Digimon esté por debajo del 50%%."},{"Conjuro inicial","Aumenta el daño un %d%% contra enemigos con al menos el 50%% de salud."}},
 [6]={{"Disparo certero","Aumenta el daño del Digimon un %d%%."},{"Fuego sostenido","Aumenta el daño un %d%% mientras la energía del Digimon sea al menos del 50%%."}},
 [7]={{"Luz restauradora","Aumenta las curas del Digimon un %d%%."},{"Auxilio divino","Aumenta las curas un %d%% sobre aliados con menos del 50%% de salud."}},
}
local function nodeInfo(id)
 local n=nodes[id]
 local profile=W.talents and W.talents.profile or 0
 local signature=signatures[profile] or signatures[0]
 if id==1 then return signature[1][1],signature[1][2],profile==2 and 3 or 2 end
 if id==8 then return signature[2][1],signature[2][2],profile==3 and 3 or 2 end
 return n[8],n[10],nil
end
local function color(node,rank,unlocked)
 if rank>0 then return 1,.78,.25,1 end
 if unlocked then return .37,.9,1,1 end
 return .48,.51,.55,.55
end
local function totalInBranch(ranks,id)
 local sum=0
 for key,node in pairs(nodes) do if node[1]==id then sum=sum+(ranks[key] or 0) end end
 return sum
end
local function canLearn(t,id)
 local n=nodes[id]; if not t or not n or t.id==0 then return false end
 local ranks=t.ranks or {}; local rank=ranks[id] or 0
 return t.spent<t.level-1 and rank<n[4] and totalInBranch(ranks,n[1])>=n[5]
    and (n[6]==0 or (ranks[n[6]] or 0)>=n[7])
end
local function benefit(n,rank,id)
 local _,description,multiplier=nodeInfo(id)
 if multiplier then return string.format(description,rank*multiplier) end
 if n[4]==1 then return n[10] end
 local amount=rank
 if n[8]=="Furia de datos" or n[8]=="Núcleo blindado" or n[8]=="Filo veloz" or n[8]=="Recarga" or n[8]=="Explotar debilidad" or n[8]=="Respuesta tenaz" or n[8]=="Vigor restaurado" or n[8]=="Restauración" or n[8]=="Serenidad" or n[8]=="Auxilio" then amount=rank*2 end
 if n[8]=="Surcador del viento" then amount=rank*2 end
 if n[8]=="Custodio" then amount=rank*10 end
 if n[8]=="Reserva defensiva" or n[8]=="Reserva espiritual" then amount=rank*5 end
 if n[8]=="Fluidez" then amount=rank*3 end
 if n[8]=="Eco protector" then amount=rank*4 end
 return string.format(n[10],amount)
end
local function showTip(button,id)
 local n=nodes[id]; local t=W.talents; local rank=t and t.ranks and t.ranks[id] or 0
 local spent=t and totalInBranch(t.ranks or {},n[1]) or 0
 GameTooltip:SetOwner(button,"ANCHOR_RIGHT")
 local title=nodeInfo(id)
 GameTooltip:SetText(title,1,.82,.32)
 GameTooltip:AddLine("Rango "..rank.." / "..n[4],.77,.88,1)
 if id==4 then GameTooltip:AddLine("Cada punto: +2% de velocidad. Máximo: +10%.",.65,.88,1) end
 if id==3 then GameTooltip:AddLine("Pulso sanador se desbloquea al llegar al rango 5.",.65,.88,1) end
 if rank>0 then GameTooltip:AddLine("Actual: "..benefit(n,rank,id),1,1,1,true) end
 if rank<n[4] then GameTooltip:AddLine("Siguiente: "..benefit(n,rank+1,id),.47,.9,1,true) end
 if spent<n[5] then GameTooltip:AddLine("Requiere "..n[5].." puntos en "..branches[n[1]].name,1,.34,.34) end
 if n[6]~=0 and ((t and t.ranks and t.ranks[n[6]]) or 0)<n[7] then
  GameTooltip:AddLine("Requiere "..nodeInfo(n[6]).." rango "..n[7],1,.34,.34)
 end
 if not t or t.spent>=t.level-1 then GameTooltip:AddLine("Sin puntos disponibles",.75,.75,.75) end
 GameTooltip:Show()
end
local function makeLine(parent)
 local texture=parent:CreateTexture(nil,"BACKGROUND")
 texture:SetTexture("Interface\\Buttons\\WHITE8X8")
 texture:SetVertexColor(.45,.65,.79,.58)
 lines[#lines+1]=texture
 return texture
end
for b=1,3 do
 local index=b
 local tab=W.Button(frame,branches[b].name,155,35,function() branch=index; W.RefreshTalentTree() end)
 tab:SetPoint("TOPLEFT",26+(b-1)*166,-124)
 tabs[b]=tab
end
local tree=CreateFrame("Frame",nil,frame)
tree:SetSize(490,365); tree:SetPoint("TOPLEFT",31,-174)
local treeBg=tree:CreateTexture(nil,"BACKGROUND"); treeBg:SetAllPoints(); treeBg:SetTexture("Interface\\Buttons\\WHITE8X8")
treeBg:SetVertexColor(.025,.06,.10,.75)
local positions={}
for id,n in pairs(nodes) do
 local button=CreateFrame("Button",nil,tree)
 local x=36+(n[3]-1)*184; local y=-13-n[2]*68
 button:SetSize(46,46); button:SetPoint("TOPLEFT",x,y)
 button.icon=button:CreateTexture(nil,"ARTWORK"); button.icon:SetAllPoints()
 button.icon:SetTexture("Interface\\Icons\\"..n[9])
 button.border=button:CreateTexture(nil,"OVERLAY"); button.border:SetTexture("Interface\\Buttons\\UI-ActionButton-Border")
 button.border:SetBlendMode("ADD"); button.border:SetSize(66,66); button.border:SetPoint("CENTER")
 button.rank=W.Label(button,11,"",31,11); button.rank:ClearAllPoints(); button.rank:SetPoint("BOTTOMRIGHT",7,-1)
 button.label=W.Label(tree,9,n[8],x-47,y-49); button.label:SetWidth(140); button.label:SetHeight(14)
 button.label:SetJustifyH("CENTER")
 button:SetScript("OnClick",function() if canLearn(W.talents,id) then W.Request("talent "..id) end end)
 button:SetScript("OnEnter",function(self) showTip(self,id) end)
 button:SetScript("OnLeave",function() GameTooltip:Hide() end)
 icons[id]=button; positions[id]={x=x+23,y=y-23}
end
for id,n in pairs(nodes) do
 if n[6]~=0 then
  local a,b=positions[n[6]],positions[id]
  local mid=(a.y+b.y)/2
  local vertical=makeLine(tree); vertical:SetPoint("TOPLEFT",a.x-2,a.y); vertical:SetSize(4,a.y-mid)
  local horizontal=makeLine(tree); horizontal:SetPoint("TOPLEFT",math.min(a.x,b.x),mid); horizontal:SetSize(math.abs(a.x-b.x)+2,3)
  local lower=makeLine(tree); lower:SetPoint("TOPLEFT",b.x-2,mid); lower:SetSize(4,mid-b.y)
  vertical.branch=n[1]; horizontal.branch=n[1]; lower.branch=n[1]
 end
end
local role=W.Label(frame,12,"",26,-552); role:SetWidth(490)
local roleHint=W.Label(frame,10,"La función depende de la forma activa. Cada forma conserva sus puntos y técnicas.",26,-578)
roleHint:SetWidth(480)
local reset=W.Button(frame,"REASIGNAR",110,27,function() W.Request("talent reset") end)
reset:SetPoint("BOTTOMRIGHT",-20,14)
W.Tooltip(reset,"Reasignar talentos","Devuelve los puntos de la forma activa fuera de combate. Las demás formas conservan los suyos.")
local hint=W.Label(frame,10,"Solo puedes tener un Digimon desplegado. Su nivel se conserva al evolucionar.",26,-610)
hint:SetWidth(390)
function W.OpenTalents()
 if W.panel then W.panel:Hide() end
 frame:Show(); W.Sync("talents"); W.RefreshTalentTree()
end
function W.RefreshTalentTree()
 local t=W.talents
 local ranks=t and t.ranks or {}
 local spent=t and t.spent or 0
 local level=t and t.level or 0
 points:SetText("Nivel "..level.."   ·   Puntos libres "..math.max(0,level-1-spent).."   ·   Invertidos "..spent)
 local roleName=t and (t.role=="HEALER" and "SANADOR" or t.role=="TANK" and "TANQUE" or "DPS") or "—"
 local profile=t and (profileNames[t.profile] or profileNames[0]) or "—"
 local active=t and W.companions and W.companions[t.id]
 role:SetText((active and active.name or ("FORMA "..(t and t.species or 0))).."   ·   "..profile.."   ·   "..roleName)
 for b,tab in ipairs(tabs) do
  local c=branches[b].color
  tab.text:SetText(branches[b].name.."  "..totalInBranch(ranks,b))
  tab.text:SetTextColor(unpack(c))
  tab:SetAlpha(b==branch and 1 or .66)
 end
 for id,button in pairs(icons) do
  local n=nodes[id]; local visible=n[1]==branch
  if visible then button:Show(); button.label:Show() else button:Hide(); button.label:Hide() end
  if visible then
   local rank=ranks[id] or 0; local active=canLearn(t,id)
   local r,g,b,a=color(n,rank,active)
   button.icon:SetVertexColor(r,g,b,a)
   button.border:SetVertexColor(r,g,b,rank>0 and .85 or active and .48 or .15)
   button.rank:SetText(rank.."/"..n[4])
   button.rank:SetTextColor(r,g,b)
   button.label:SetText(nodeInfo(id))
   button.label:SetTextColor(r,g,b)
  end
 end
 for _,line in ipairs(lines) do if line.branch==branch then line:Show() else line:Hide() end end
 reset:SetAlpha(t and spent>0 and 1 or .4)
end
