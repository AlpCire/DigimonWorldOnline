const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { Archive, MPQ_OPEN_READ_ONLY } = require('@jamiephan/stormlib');

const root = process.argv[2] && path.resolve(process.argv[2]);
const out = process.argv[3] && path.resolve(process.argv[3]);
if (!root || !out) throw new Error('Uso: node build-framexml-probe.cjs <Data del cliente> <salida MPQ privada>');
const source = path.join(root, 'esES', 'patch-esES-3.MPQ');
const tocName = 'Interface\\FrameXML\\FrameXML.toc';
const probeName = 'Interface\\FrameXML\\WorldDigimonNativeProbe.lua';
const tocExpectedSha = '3158BEA13225AE51137A389F0F3AB8566E94B6BE84196DD2C1FDA27024677754';

if (fs.existsSync(out)) throw new Error('La salida ya existe; no se sobrescribe.');
const archive = Archive.open(source, { flags: MPQ_OPEN_READ_ONLY });
let toc;
try { toc = archive.readFile(tocName); } finally { archive.close(); }
const originalSha = crypto.createHash('sha256').update(toc).digest('hex').toUpperCase();
if (originalSha !== tocExpectedSha) throw new Error('FrameXML.toc inesperado');
if (!toc.toString('utf8').includes('## add new modules above here')) throw new Error('Marcador de carga ausente');
const updated = toc.toString('utf8').replace('## add new modules above here', 'WorldDigimonNativeProbe.lua\r\n## add new modules above here');
const probe = [
  '-- WORLD DIGIMON MMO: ensayo de carga nativa. Sin datos de juego ni cambios de interfaz.',
  'local frame = CreateFrame("Frame")',
  'frame:RegisterEvent("PLAYER_LOGIN")',
  'frame:SetScript("OnEvent", function()',
  '    DEFAULT_CHAT_FRAME:AddMessage("WORLD DIGIMON: interfaz integrada de prueba cargada.")',
  'end)',
  ''
].join('\r\n');
const output = Archive.create(out, { maxFileCount: 16, flags: 0 });
try {
  output.addBuffer(tocName, Buffer.from(updated, 'utf8'));
  output.addBuffer(probeName, Buffer.from(probe, 'utf8'));
} finally { output.close(); }
const check = Archive.open(out, { flags: MPQ_OPEN_READ_ONLY });
try {
  if (check.readFile(tocName).toString('utf8') !== updated || check.readFile(probeName).toString('utf8') !== probe) {
    throw new Error('Error de verificación del MPQ creado');
  }
} finally { check.close(); }
console.log(JSON.stringify({ source, originalSha, out, tocBytes: Buffer.byteLength(updated), probeBytes: Buffer.byteLength(probe), mpqSha: crypto.createHash('sha256').update(fs.readFileSync(out)).digest('hex').toUpperCase() }));
