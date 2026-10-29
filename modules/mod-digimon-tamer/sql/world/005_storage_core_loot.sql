-- Expansion cores let players purchase additional 15-slot storage boxes.
INSERT INTO creature_loot_template
(Entry,Item,Reference,Chance,QuestRequired,LootMode,GroupId,MinCount,MaxCount,Comment)
VALUES
(900150,900240,0,8,0,1,0,1,1,'mod-digimon-tamer: storage expansion core'),
(900151,900240,0,15,0,1,0,1,1,'mod-digimon-tamer: storage expansion core')
ON DUPLICATE KEY UPDATE Reference=VALUES(Reference),Chance=VALUES(Chance),
QuestRequired=VALUES(QuestRequired),LootMode=VALUES(LootMode),GroupId=VALUES(GroupId),
MinCount=VALUES(MinCount),MaxCount=VALUES(MaxCount),Comment=VALUES(Comment);
