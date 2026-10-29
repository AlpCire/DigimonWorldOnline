-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
local panel
local points={}
local started,seconds,nonce,completed,finishedAt=0,8,"",0,0

local function hide()
 if panel then panel:Hide() end
 nonce=""
end

function W.BeginCapture(token,duration,species)
 if not panel then
  panel=CreateFrame("Frame","WorldDigimonCapture",UIParent)
  panel:SetSize(340,245); panel:SetPoint("CENTER",UIParent,"CENTER",0,20)
  panel:SetFrameStrata("DIALOG"); panel:SetFrameLevel(80); panel:EnableMouse(true)
  panel:SetBackdrop({bgFile="Interface\\DialogFrame\\UI-DialogBox-Background",edgeFile="Interface\\DialogFrame\\UI-DialogBox-Border",tile=true,tileSize=32,edgeSize=24,insets={left=8,right=8,top=8,bottom=8}})
  panel.title=panel:CreateFontString(nil,"OVERLAY","GameFontNormalLarge"); panel.title:SetPoint("TOP",0,-20); panel.title:SetText("SINCRONIZACIÓN DIGITAL")
  panel.status=panel:CreateFontString(nil,"OVERLAY","GameFontHighlight"); panel.status:SetPoint("TOP",panel.title,"BOTTOM",0,-8)
  panel.timer=panel:CreateFontString(nil,"OVERLAY","GameFontNormal"); panel.timer:SetPoint("BOTTOM",0,18)
  panel:SetScript("OnUpdate",function(_,elapsed)
   if nonce=="" then
    if finishedAt>0 and GetTime()-finishedAt>1.5 then finishedAt=0; hide() end
    return
   end
   local remaining=seconds-(GetTime()-started)
   panel.timer:SetText(string.format("Tiempo: %.1f",math.max(0,remaining)))
   if remaining<=0 then W.Request("capture-cancel "..nonce); hide(); W.Notice("Se agotó el tiempo de captura.") end
  end)
  local positions={{-112,42},{0,48},{112,42},{-65,-35},{65,-35}}
  for i=1,5 do
   local index=i; local b=CreateFrame("Button",nil,panel,"UIPanelButtonTemplate")
   b:SetSize(48,42); b:SetPoint("CENTER",panel,"CENTER",positions[i][1],positions[i][2]); b:SetText("◆")
   b:SetScript("OnClick",function(self)
    if nonce=="" or self.used then return end
    self.used=true; self:Disable(); self:SetAlpha(.35); completed=completed+1
    panel.status:SetText("Nodos sincronizados: "..completed.." / 5")
    W.Request("capture-step "..nonce.." "..index)
   end)
   points[i]=b
  end
  panel.close=CreateFrame("Button",nil,panel,"UIPanelCloseButton"); panel.close:SetPoint("TOPRIGHT",-4,-4)
  panel.close:SetScript("OnClick",function()
   if nonce~="" then W.Request("capture-cancel "..nonce) end
   hide()
  end)
 end
 nonce=token; seconds=duration; started=GetTime(); completed=0
 finishedAt=0
 panel.status:SetText("Localiza y pulsa los cinco nodos.")
 for _,b in ipairs(points) do b.used=false; b:Enable(); b:SetAlpha(1) end
 panel:Show(); panel:Raise()
end

function W.UpdateCapture(current,total)
 completed=current
 if panel and nonce~="" then panel.status:SetText("Nodos sincronizados: "..current.." / "..total) end
end

function W.EndCapture(success)
 if panel and nonce~="" then
  panel.status:SetText(success and "¡Sincronización completada!" or "Sincronización fallida")
  nonce=""
  finishedAt=GetTime()
  if success then W.Sync("details"); W.Request("storage 1") end
 end
end
