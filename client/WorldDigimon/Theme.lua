-- SPDX-License-Identifier: GPL-2.0-or-later
local W=WorldDigimonUI
function W.Label(parent,size,text,x,y)
 local l=parent:CreateFontString(nil,"OVERLAY"); l:SetFont("Fonts\\FRIZQT__.TTF",size or 12,"")
 l:SetPoint("TOPLEFT",x or 0,y or 0); l:SetTextColor(.75,.93,1); l:SetText(text or ""); l:SetJustifyH("LEFT"); return l
end
function W.Skin(f,texture)
 local bg=f:CreateTexture(nil,"BACKGROUND"); bg:SetAllPoints(); bg:SetTexture(W.asset..(texture or "Panel")); f.skin=bg
 local c=f:CreateTexture(nil,"BORDER"); c:SetTexture(W.asset.."Circuit"); c:SetPoint("BOTTOMRIGHT",-12,10); c:SetSize(128,64); c:SetAlpha(.65); f.circuit=c
end
function W.Button(parent,text,width,height,click)
 local b=CreateFrame("Button",nil,parent); b:SetSize(width or 300,height or 32); b:SetNormalTexture(W.asset.."Button")
 b:SetHighlightTexture(W.asset.."Highlight","ADD"); b:SetPushedTexture(W.asset.."ButtonDown")
 b.text=W.Label(b,12,text,14,-10); b.text:SetWidth((width or 300)-24); b:SetScript("OnClick",click); return b
end
function W.Panel(name,width,height,title)
 local p=CreateFrame("Frame",name,UIParent); p:SetSize(width,height); p:SetFrameStrata("DIALOG"); W.Skin(p); W.Place(p,name,"CENTER",0,0)
 p.title=W.Label(p,18,title,26,-22); p.subtitle=W.Label(p,10,"WORLD DIGIMON  /  DIGIVICE",26,-48); p.subtitle:SetTextColor(.1,.7,.85)
 local close=W.Button(p,"X",32,30,function() p:Hide() end); close:SetPoint("TOPRIGHT",-14,-16)
 tinsert(UISpecialFrames,name); p:Hide(); return p
end
function W.Tooltip(b,title,body)
 b:SetScript("OnEnter",function(self) GameTooltip:SetOwner(self,"ANCHOR_RIGHT"); GameTooltip:SetText(title); GameTooltip:AddLine(body,.8,.93,1,true); GameTooltip:Show() end)
 b:SetScript("OnLeave",function() GameTooltip:Hide() end)
end
