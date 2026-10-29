-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
-- WoW 3.3.5a frames expose Show/Hide, but not the newer SetShown helper.
local function setShown(frame,shown)
 if shown then frame:Show() else frame:Hide() end
end
local panel=W.Panel("WorldDigimonMenu",430,530,"DIGIVICE · WORLD DIGIMON"); W.panel=panel
W.rows={}
panel.footer=W.Label(panel,10,"",26,-478); panel.footer:SetWidth(378)
local back=W.Button(panel,"< Inicio",112,30,function() W.Open("home") end); back:SetPoint("BOTTOMLEFT",22,14)
local prev=W.Button(panel,"<",36,30,function() W.page=math.max(1,W.page-1); W.RenderPanel() end); prev:SetPoint("BOTTOMRIGHT",-65,14)
local nextPage=W.Button(panel,">",36,30,function() W.page=math.min(W.pages or 1,W.page+1); W.RenderPanel() end); nextPage:SetPoint("BOTTOMRIGHT",-22,14)
panel:EnableMouseWheel(true)
panel:SetScript("OnMouseWheel",function(_,delta) W.page=math.max(1,math.min(W.pages or 1,W.page-delta)); W.RenderPanel() end)
for i=1,6 do
 local row=CreateFrame("Frame",nil,panel); row:SetSize(382,62); row:SetPoint("TOPLEFT",24,-78-(i-1)*66)
 local bg=row:CreateTexture(nil,"BACKGROUND"); bg:SetAllPoints(); bg:SetTexture(W.asset.."Card")
 row.icon=row:CreateTexture(nil,"ARTWORK"); row.icon:SetSize(34,34); row.icon:SetPoint("TOPLEFT",10,-12)
 row.preview=CreateFrame("PlayerModel",nil,row); row.preview:SetSize(44,46); row.preview:SetPoint("TOPLEFT",5,-7); row.preview:Hide()
 row.title=W.Label(row,13,"",54,-9); row.title:SetWidth(198)
 row.detail=W.Label(row,10,"",54,-30); row.detail:SetWidth(198); row.detail:SetHeight(28)
 row.button=W.Button(row,"",112,30,function() if row.action then row.action() end end); row.button:SetPoint("TOPRIGHT",-10,-17)
 row.store=W.Button(row,"GUARDAR",68,26,function() if row.storeAction then row.storeAction() end end); row.store:SetPoint("TOPRIGHT",-10,-17); row.store:Hide()
 W.rows[i]=row
end
local storageHeader=W.Label(panel,11,"",24,-51); storageHeader:SetWidth(250); storageHeader:Hide()
local boxPrev=W.Button(panel,"<",30,24,function()
 if W.storage and W.storage.box>1 then W.page=1; W.Request("storage "..(W.storage.box-1)) end
end); boxPrev:SetPoint("TOPRIGHT",-116,-48); boxPrev:Hide()
local boxNext=W.Button(panel,">",30,24,function()
 if W.storage and W.storage.box<W.storage.boxes then W.page=1; W.Request("storage "..(W.storage.box+1)) end
end); boxNext:SetPoint("TOPRIGHT",-82,-48); boxNext:Hide()
local buyBox=W.Button(panel,"COMPRAR",78,24,function() W.Request("buybox") end); buyBox:SetPoint("TOPRIGHT",-10,-48); buyBox:Hide()
local function option(title,detail,label,action,icon,tip)
 return {title=title,detail=detail,label=label,action=action,icon=icon or "INV_Misc_Book_07",tip=tip or detail}
end
local titles={home="DIGIVICE",routes="RAMAS EVOLUTIVAS",missions="ENCARGOS DEL GUIA",techniques="HABILIDADES",collection="DIGIEQUIPO",storage="DIGIALMACEN",talents="TALENTOS",config="CONFIGURACION"}
function W.Open(view)
 if view=="talents" and W.OpenTalents then W.OpenTalents(); return end
 W.view=view; W.page=1; panel.title:SetText(titles[view] or titles.home); panel:Show()
 if view=="routes" or view=="missions" then W.Sync("routes")
 elseif view=="techniques" or view=="collection" then W.Sync("details")
 elseif view=="talents" then W.Sync("talents")
 elseif view=="storage" then W.storage=nil; W.Sync("storage 1") else W.Sync() end
 W.RenderPanel()
end
function W.Toggle() if panel:IsShown() then panel:Hide() else W.Open("home") end end
local function setting(key,name,description,values)
 local value=W.cfg[key]; local label=type(value)=="boolean" and (value and "SI" or "NO") or tostring(math.floor(value*100+.5)).."%"
 return option(name,description,label,function()
  if values then
   local closest=1; for i,v in ipairs(values) do if math.abs(v-W.cfg[key])<.02 then closest=i end end
   W.cfg[key]=values[closest%#values+1]
  else W.cfg[key]=not W.cfg[key] end
  W.ApplyLayout(); if W.RefreshHUD then W.RefreshHUD() end; if W.RefreshMissionTracker then W.RefreshMissionTracker() end; W.RenderPanel()
 end,"INV_Misc_Gear_01")
end
function W.RenderPanel()
 if not W.cfg or not W.view then return end
 local items={}; local function add(item) items[#items+1]=item end
 if W.view=="home" then
  add(option("Luchar","Control directo del companero","ABRIR",function() W.SetFight(true) end,"Ability_Hunter_Pet_Attack"))
  add(option("Digiequipo","Cambiar el companero activo","VER",function() W.Open("collection") end))
  add(option("Digialmacén","Equipo de seis y cajas de 15 Digimon","VER",function() W.Open("storage") end,"INV_Misc_Bag_10"))
  add(option("Capturar seleccionado","Debilítalo con tu compañero y apunta con el objetivo seleccionado","INICIAR",function() W.Request("capture") end,"Ability_Hunter_BeastSoothe"))
  add(option("Evoluciones","Descubrir y recuperar formas","VER",function() W.Open("routes") end,"Spell_Arcane_PortalDalaran"))
  add(option("Habilidades","Aprendidas, equipadas y futuras","VER",function() W.Open("techniques") end,"INV_Misc_Book_09"))
  add(option("Talentos de la forma","Un punto por nivel; función según la evolución","VER",function() W.Open("talents") end,"Ability_Marksmanship"))
  add(option("Encargos del guia","Las misiones se aceptan junto al entrenador","VER",function() W.Notice("Habla con el Guia del Mundo Digital para aceptar o entregar encargos.") end,"INV_Misc_Note_01"))
  add(option("Invocar companero","Recuperar tu enlace digital","INVOCAR",function() W.Request("summon"); W.Sync("details") end,"Ability_Hunter_BeastCall"))
  add(option("Retirar companero","Disponible fuera de combate","RETIRAR",function() W.Request("withdraw") end,"Ability_Hunter_BeastSoothe"))
  add(option("Descansar","Recuperar salud fuera de combate","DESCANSAR",function() W.Request("recover") end,"Spell_Holy_Heal"))
  add(option("Configuracion","Escalas, posiciones y apariencia","ABRIR",function() W.Open("config") end,"INV_Misc_Gear_01"))
  add(option("Salir de Luchar","Restaurar barras y teclas de WoW","SALIR",function() W.SetFight(false); panel:Hide() end))
 elseif W.view=="routes" or W.view=="missions" then
  if W.view=="routes" then
   for _,a in ipairs(W.ancestors or {}) do
    add(option("Origen: "..a.sourceName.." -> "..a.targetName,"Forma actual · requisito de nivel "..a.level,"ACTUAL",nil,"Spell_Arcane_PortalDalaran"))
   end
  end
  for _,r in ipairs(W.routes) do
   if W.view=="routes" or r.mission then
    local route=r; local label=r.mode=="REUSE" and "RESTAURAR" or r.mode=="READY" and "EVOLUCIONAR" or r.mission and "ENTRENAR" or "BLOQUEADO"
    local action
    if r.claimable and (W.view=="missions" or r.mode=="LOCKED") then label="RECLAMAR"; action=function() W.Request("claim "..route.id) end
    elseif r.mode=="READY" or r.mode=="REUSE" then action=function() W.Request("evolve "..route.id); W.Request("routes"); W.Request("details") end
    elseif r.mission then label="GUIA"; action=function() W.Notice("Acepta y entrega este encargo hablando con el Guia del Mundo Digital.") end end
    local item=option(W.view=="missions" and r.mission or r.name,r.detail,label,action,"Spell_Arcane_PortalDalaran",(r.mission and r.mission.."\n" or "")..r.detail.."\nHabla con el Guia del Mundo Digital para aceptar este encargo.")
    item.display=r.display; item.previewKey=nil; add(item)
   end
  end
 elseif W.view=="techniques" then
  local list={}; for _,t in pairs(W.techniques) do list[#list+1]=t end
  table.sort(list,function(a,b) if a.level==b.level then return a.id<b.id end return a.level<b.level end)
  for _,t in ipairs(list) do
   local tech=t; local _,_,icon=GetSpellInfo(t.spell)
   local custom=WorldDigimonAbilityIcons and WorldDigimonAbilityIcons[t.id]
   local label=t.status=="EQUIPPED" and "QUITAR" or t.status=="LEARNED" and "EQUIPAR" or "BLOQUEADA"
   local action=t.status~="LOCKED" and function() W.Request((tech.status=="EQUIPPED" and "unequip " or "equip ")..tech.id) end or nil
   local item=option(t.name,"Nivel "..t.level.." / "..t.cooldown.." s",label,action,nil,t.description or "Aprendizaje automatico al alcanzar el nivel. Se equipa en el primer espacio disponible; quita otra tecnica si esta lleno.")
   item.fullIcon=custom and WorldDigimonAbilityTextureRoot and (WorldDigimonAbilityTextureRoot..custom..".tga") or icon; add(item)
  end
 elseif W.view=="talents" then
  add(option("Árbol de talentos","Abre el árbol de la forma activa.","ABRIR",
   function() if W.OpenTalents then W.OpenTalents() end end,"Ability_Marksmanship"))
 elseif W.view=="collection" then
  local list={}; for _,c in pairs(W.companions) do list[#list+1]=c end; table.sort(list,function(a,b) return a.id<b.id end)
  for _,c in ipairs(list) do local companion=c
   local detail="Nivel "..c.level.." / XP "..c.xp
   if W.CareVisible() then detail=detail.." / Empatía "..(c.bond or 0).." / Felicidad "..(c.happiness or 50) end
   local label=c.active and "ACTIVO" or "INVOCAR"
   local action=not c.active and function() W.Request("activate "..companion.id); W.Sync("details") end or nil
   local item=option(c.name,"Equipo "..(c.partySlot+1).." / "..detail,label,action,"Ability_Hunter_BeastCall")
   item.storeAction=not c.active and function() W.Request("team store "..companion.id); W.Sync("details") end or nil
   add(item)
  end
 elseif W.view=="storage" then
  local box=W.storage
  if box then
   local gold=math.floor(box.price/10000)
   storageHeader:SetText("Caja "..box.box.." / "..box.boxes.."   ·   Oro "..math.floor(box.money/10000).."   ·   Núcleos "..box.material)
   local list={}; for _,c in ipairs(box.items) do list[#list+1]=option(c.name,"Nivel "..c.level.." / XP "..c.xp,"AL EQUIPO",function()
     W.Request("team add "..c.id); W.Sync("details"); W.Request("storage "..box.box)
    end,"INV_Misc_Bag_10")
    list[#list].display=c.display
   end
   if #list==0 then add(option("Caja vacía","Los Digimon capturados que excedan el equipo se guardan aquí.","",nil,"INV_Misc_Bag_10")) end
   for _,item in ipairs(list) do add(item) end
   buyBox.text:SetText(box.boxes>=100 and "MAX" or (gold.." oro"))
   buyBox:SetAlpha(box.boxes<100 and box.money>=box.price and box.material>=1 and 1 or .5)
   W.Tooltip(buyBox,"Comprar caja",gold.." de oro + 1 Núcleo de expansión. Cada caja añade 15 espacios.")
  else
   storageHeader:SetText("Cargando Digialmacén…")
  end
 elseif W.view=="config" then
  add(setting("scale","Escala general","Tamano del Digivice",{.75,.85,1,1.15,1.3}))
  add(setting("combatScale","Escala de combate","Tamano de los 12 botones",{.75,.85,1,1.15,1.3}))
  add(setting("locked","Bloquear posiciones","Desactiva para arrastrar paneles"))
  add(setting("classic","Ocultar barras de WoW","Solo durante el modo Luchar"))
  add(setting("portraits","Retratos","Tamer y modelo del companero"))
  add(setting("secondary","Texto secundario","Etapa y progreso de experiencia"))
  add(setting("careUI","Datos de cuidado","Mostrar empatía, felicidad y efectos de comida"))
  add(setting("quests","Rastreador de encargos","Mostrar el panel de misión activa"))
  add(setting("opacity","Opacidad","Fondo de los paneles",{.6,.75,.9,1}))
  add(setting("glow","Intensidad digital","Circuitos decorativos",{0,.35,.65,1}))
  add(setting("minimap","Estilo del minimapa","Mantiene controles originales"))
  add(setting("chat","Fondo del chat","Mantiene texto y pestanas"))
  add(option("Restaurar posiciones","Recuperar la distribucion inicial","RESTAURAR",function() W.cfg.positions={}; W.ApplyLayout() end))
  add(option("Valores por defecto","Conserva la preferencia de Luchar","RESTAURAR",function()
   for k,v in pairs(W.defaults) do if k~="fight" then W.cfg[k]=type(v)=="table" and {} or v end end; W.ApplyLayout(); W.RenderPanel()
  end))
 end
 W.pages=math.max(1,math.ceil(#items/6)); W.page=math.min(W.page,W.pages)
 for i,row in ipairs(W.rows) do local item=items[(W.page-1)*6+i]
  if item then
   row:Show(); row.title:SetText(item.title); row.detail:SetText(item.detail); row.button.text:SetText(item.label); row.action=item.action
   row.button:SetWidth(item.storeAction and 68 or 112); row.button:ClearAllPoints(); row.button:SetPoint("TOPRIGHT",item.storeAction and -82 or -10,-17)
   row.storeAction=item.storeAction; setShown(row.store,item.storeAction~=nil)
   row.icon:SetTexture(item.fullIcon or ("Interface\\Icons\\"..item.icon)); row.button:SetAlpha(item.action and 1 or .45)
   if item.display and item.display>0 and row.previewKey~=item.display then
    row.preview:ClearModel(); local ok=pcall(row.preview.SetDisplayInfo,row.preview,item.display)
    if ok then row.previewKey=item.display; pcall(row.preview.SetCamera,row.preview,0); pcall(row.preview.SetFacing,row.preview,.3) end
   end
   if item.display and item.display>0 and row.previewKey==item.display then row.preview:Show() else row.preview:Hide(); row.previewKey=nil end
   W.Tooltip(row.button,item.title,item.tip)
  else row:Hide(); row.action=nil end
 end
 local showStorage=W.view=="storage"
 setShown(storageHeader,showStorage); setShown(boxPrev,showStorage); setShown(boxNext,showStorage); setShown(buyBox,showStorage)
 if showStorage and W.storage then
  boxPrev:SetAlpha(W.storage.box>1 and 1 or .35); boxNext:SetAlpha(W.storage.box<W.storage.boxes and 1 or .35)
 end
 panel.footer:SetText(#items==0 and "Esperando datos / sin opciones para esta forma" or ("Pagina "..W.page.." / "..W.pages.."  -  Rueda para desplazar"))
end
local launcher=CreateFrame("Button","WorldDigimonLauncher",UIParent)
launcher:SetSize(180,38); launcher:SetNormalTexture(W.asset.."Button"); launcher:SetHighlightTexture(W.asset.."Highlight","ADD")
launcher.text=W.Label(launcher,13,"DIGIVICE",43,-12); W.Place(launcher,"launcher","TOP",0,-24)
launcher:SetScript("OnClick",W.Toggle)
-- Keep the original secure bag buttons intact.
hooksecurefunc("ContainerFrame_Update",function(container)
 for i=1,(container.size or 0) do local b=_G[container:GetName().."Item"..i]
  local link=b and GetContainerItemLink(container:GetID(),b:GetID())
  if link and link:find("item:900110:",1,true) then SetItemButtonTexture(b,"Interface\\Icons\\INV_Misc_Book_07") end
 end
end)
GameTooltip:HookScript("OnTooltipSetItem",function(f)
 local _,link=f:GetItem()
 if link and link:find("item:900110:",1,true) then f:ClearLines(); f:SetText("Digivice",.3,.9,1)
  f:AddLine("Dirige a tu companero, sus tecnicas y evoluciones.",1,1,1,true); f:AddLine("Clic derecho o /digimon para abrir.",.4,1,.8,true); f:Show()
 end
end)
