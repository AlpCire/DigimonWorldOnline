-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
local pendingRoutes,pendingCompanions,pendingTechniques
local function render() if W.panel:IsShown() then W.RenderPanel() end end
function W.Receive(message)
 local f={strsplit("|",message)}; local kind=f[2]
 if f[1]=="V1" then
  if kind=="XP" and #f==4 then
   local xp,required=W.Number(f[3]),W.Number(f[4])
   if xp and required then W.state.xp=xp; W.state.xpRequired=required; W.RefreshHUD() end
  elseif kind=="BOND" and #f==3 then
   local bond=W.Number(f[3],100); if bond then W.state.bond=bond; W.RefreshHUD() end
  elseif kind=="CARE" and #f==7 then
   local happiness,damage,damageTime,protection,protectionTime=W.Number(f[3],100),W.Number(f[4],50),W.Number(f[5],3600),W.Number(f[6],50),W.Number(f[7],3600)
   if happiness and damage and damageTime and protection and protectionTime then
    W.state.happiness=happiness; W.state.damageBuffPct=damage; W.state.damageBuffExpires=damageTime>0 and GetTime()+damageTime or 0
    W.state.protectionBuffPct=protection; W.state.protectionBuffExpires=protectionTime>0 and GetTime()+protectionTime or 0; W.RefreshHUD()
   end
  elseif kind=="MODE" and #f==3 and (f[3]=="aggressive" or f[3]=="defensive" or f[3]=="passive") then
   W.state.mode=f[3]; W.RefreshCombat(); if W.RefreshStatus then W.RefreshStatus() end
  elseif kind=="S" and #f==13 then
   local n={}; for i=3,8 do n[i]=W.Number(f[i]); if not n[i] then return end end
   if n[4]>100 then return end
   local slots={}
   for i=1,5 do
    local a,b,c,d=strsplit(",",f[i+8]); a,b,c,d=W.Number(a,100000),W.Number(b,100000),W.Number(c,3600000),W.Number(d or "0")
    if not a or not b or not c or not d then return end
    local expiry=c>0 and GetTime()+c/1000 or 0
    local old=W.state.slots[i]
    if old and old.technique==d and math.abs(old.expires-expiry)<1.2 then expiry=old.expires end
    slots[i]={spell=a,cost=b,remaining=c,expires=expiry,technique=d}
   end
   local changed=W.state.active~=n[3] or W.state.level~=n[4]
   W.state={active=n[3],level=n[4],hp=n[5],maxhp=n[6],focus=n[7],maxfocus=n[8],bond=W.state.bond or 0,happiness=W.state.happiness or 50,
    damageBuffPct=W.state.damageBuffPct or 0,damageBuffExpires=W.state.damageBuffExpires or 0,
    protectionBuffPct=W.state.protectionBuffPct or 0,protectionBuffExpires=W.state.protectionBuffExpires or 0,
    mode=W.state.mode or "defensive",slots=slots}; W.received=GetTime()
   W.RefreshCombat(); W.RefreshHUD(); if changed then W.Sync("details") end
  elseif kind=="T" and #f==4 then
   local id=W.Number(f[3]); if id and W.Text(f[4]) then W.names[id]=f[4]; W.RefreshCombat() end
  elseif kind=="FIGHT" and #f==2 then W.SetFight(true)
  elseif kind=="MENU" and #f==2 then W.Open("home")
  elseif kind=="NOTICE" and #f==3 and W.Text(f[3],240) then W.Notice(f[3])
  elseif kind=="EVO_BEGIN" and not W.v2 then W.routes={}; W.Open("routes")
  elseif kind=="EVO" and not W.v2 and #f==6 then
   local id,level=W.Number(f[3]),W.Number(f[4],100)
   if id and level and W.Text(f[5],80) and (f[6]=="EVOLVE" or f[6]=="REUSE") then
    W.routes[#W.routes+1]={id=id,level=level,name=f[5],mode=f[6]=="EVOLVE" and "READY" or "REUSE",detail="Nivel "..level}; render()
   end
  end
 elseif f[1]=="V2" then
  W.v2=true
  if kind=="ROUTES_BEGIN" and #f==2 then pendingRoutes={}; W.ancestors={}
  elseif kind=="ANCESTOR" and #f==7 then
   local source,target,level=W.Number(f[3]),W.Number(f[5]),W.Number(f[7],100)
   if source and target and level and W.Text(f[4],80) and W.Text(f[6],80) then
    W.ancestors=W.ancestors or {}; W.ancestors[#W.ancestors+1]={source=source,sourceName=f[4],target=target,targetName=f[6],level=level}
   end
  elseif kind=="ROUTE" and #f==8 and pendingRoutes and #pendingRoutes<80 then
   local id,level=W.Number(f[3]),W.Number(f[4],100)
   local display=W.Number(f[8],100000)
   if id and id>0 and level and display and W.Text(f[5],80) and W.Text(f[7],180) and (f[6]=="READY" or f[6]=="LOCKED" or f[6]=="REUSE") then
    pendingRoutes[#pendingRoutes+1]={id=id,level=level,name=f[5],mode=f[6],detail=f[7],display=display}
   end
  elseif kind=="MISSION" and #f==4 and pendingRoutes then
   local id=W.Number(f[3]); if id and W.Text(f[4],80) then for _,r in ipairs(pendingRoutes) do if r.id==id then r.mission=f[4] end end end
  elseif kind=="REWARD" and #f==5 and pendingRoutes then
   local id,claimable,item=W.Number(f[3]),W.Number(f[4],1),W.Number(f[5])
   if id and claimable and item then for _,r in ipairs(pendingRoutes) do if r.id==id then r.claimable=claimable==1; r.item=item end end end
  elseif kind=="ACTIVE_MISSION" and (#f==3 or #f==8) then
   local id=W.Number(f[3])
   if id==0 and #f==3 then W.activeMission=nil; W.RefreshHUD()
   elseif id and id>0 and #f==8 then
    local current,total,complete=W.Number(f[5],100000),W.Number(f[6],100000),W.Number(f[8],1)
    if current and total and complete and W.Text(f[4],80) and W.Text(f[7],80) then
     W.activeMission={id=id,name=f[4],current=current,total=total,target=f[7],complete=complete==1}; W.RefreshHUD()
    end
   end
  elseif kind=="ROUTES_END" and #f==2 and pendingRoutes then W.routes=pendingRoutes; pendingRoutes=nil; render()
  elseif kind=="TALENTS" and #f==35 then
   local id,species,profile=W.Number(f[3]),W.Number(f[4]),W.Number(f[6],7)
   local level,spent=W.Number(f[7],100),W.Number(f[8],99)
   if id and species and profile and level and spent and
      (f[5]=="DPS" or f[5]=="TANK" or f[5]=="HEALER") then
    local ranks={}; local total=0
    for i=1,27 do
     local rank=W.Number(f[i+8],10)
     if not rank then return end
     ranks[i]=rank; total=total+rank
    end
    if total~=spent or spent>math.max(0,level-1) then return end
    W.talents={id=id,species=species,profile=profile,role=f[5],
               level=level,spent=spent,ranks=ranks}
    if W.RefreshTalentTree then W.RefreshTalentTree() end
    render()
   end
  elseif kind=="DETAILS_BEGIN" and #f==2 then pendingCompanions={}; pendingTechniques={}
  elseif kind=="COMPANION" and (#f==14 or #f==13) and pendingCompanions then
   local n={}; for _,i in ipairs({3,4,6,7,8,9,10,11,12,13}) do n[i]=W.Number(f[i],(i==12 or i==13) and 100 or nil); if not n[i] then return end end
   local slot= #f==14 and W.Number(f[14],255) or (n[11]==1 and 0 or 255)
   if n[3]>0 and n[6]<=7 and n[7]<=100 and n[11]<=1 and slot and W.Text(f[5],80) then
    pendingCompanions[n[3]]={id=n[3],species=n[4],name=f[5],stage=n[6],level=n[7],xp=n[8],display=n[9],entry=n[10],active=n[11]==1,bond=n[12],happiness=n[13],partySlot=slot}
   end
  elseif kind=="STORAGE" and #f==7 then
   local boxes,box,money,material,price=W.Number(f[3],100),W.Number(f[4],100),W.Number(f[5]),W.Number(f[6]),W.Number(f[7])
   if boxes and boxes>=2 and box and box>=1 and box<=boxes and money and material and price then
    W.storage={boxes=boxes,box=box,money=money,material=material,price=price,items={}}
   end
  elseif kind=="BOX_ITEM" and #f==9 and W.storage then
   local id,species,stage,level,xp,display=W.Number(f[3]),W.Number(f[4]),W.Number(f[6],7),W.Number(f[7],100),W.Number(f[8]),W.Number(f[9],100000)
   if id and species and stage and level and xp and display and W.Text(f[5],80) and #W.storage.items<15 then
    W.storage.items[#W.storage.items+1]={id=id,species=species,name=f[5],stage=stage,level=level,xp=xp,display=display}
   end
  elseif kind=="BOX_END" then
   if W.view=="storage" then W.RenderPanel() end
  elseif kind=="CAPTURE_BEGIN" and #f==5 then
   local seconds,species=W.Number(f[4],8),W.Number(f[5])
   if W.Text(f[3],40) and seconds and seconds>0 and species and W.BeginCapture then W.BeginCapture(f[3],seconds,species) end
  elseif kind=="CAPTURE_PROGRESS" and #f==4 then
   local current,total=W.Number(f[3],5),W.Number(f[4],5); if current and total and W.UpdateCapture then W.UpdateCapture(current,total) end
  elseif kind=="CAPTURE_END" and #f==3 then
   if W.EndCapture then W.EndCapture(f[3]=="SUCCESS") end
  elseif kind=="TECH" and #f==8 and pendingTechniques then
   local id,spell,level,cd=W.Number(f[3]),W.Number(f[5],100000),W.Number(f[6],100),W.Number(f[8],3600)
   if id and spell and level and cd and W.Text(f[4],80) and (f[7]=="LOCKED" or f[7]=="LEARNED" or f[7]=="EQUIPPED") then
    pendingTechniques[id]={id=id,name=f[4],spell=spell,level=level,status=f[7],cooldown=cd}; W.names[id]=f[4]
   end
  elseif kind=="DETAILS_END" and #f==2 and pendingCompanions then
   W.companions=pendingCompanions; W.techniques=pendingTechniques; pendingCompanions=nil; pendingTechniques=nil
   W.RefreshHUD(); W.RefreshCombat(); render()
  end
 end
end
