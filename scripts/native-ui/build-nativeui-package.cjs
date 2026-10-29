const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { Archive, MPQ_OPEN_READ_ONLY } = require('@jamiephan/stormlib');

const clientData = process.argv[2] && path.resolve(process.argv[2]);
const addon = process.argv[3] && path.resolve(process.argv[3]);
const outputPath = process.argv[4] && path.resolve(process.argv[4]);
if (!clientData || !addon || !outputPath) throw new Error('Uso: node build-nativeui-package.cjs <Data del cliente> <WorldDigimon fuente> <salida MPQ privada>');
const tocName = 'Interface\\FrameXML\\FrameXML.toc';
const root = 'Interface\\FrameXML\\WorldDigimonNative';
const scripts = ['AbilityIcons.lua', 'Core.lua', 'Theme.lua', 'Combat.lua', 'Panels.lua', 'HUD.lua', 'Protocol.lua', 'Capture.lua', 'Talents.lua', 'Map.lua'];
const escapedSlash = String.fromCharCode(92, 92);
const originalRoot = ['Interface', 'AddOns', 'WorldDigimon', ''].join(escapedSlash);
const nativeRoot = ['Interface', 'FrameXML', 'WorldDigimonNative', ''].join(escapedSlash);

if (fs.existsSync(outputPath)) throw new Error('La salida ya existe; no se sobrescribe.');
const source = Archive.open(path.join(clientData, 'esES', 'patch-esES-3.MPQ'), { flags: MPQ_OPEN_READ_ONLY });
let toc;
try { toc = source.readFile(tocName); } finally { source.close(); }
const tocSha = crypto.createHash('sha256').update(toc).digest('hex').toUpperCase();
if (tocSha !== '3158BEA13225AE51137A389F0F3AB8566E94B6BE84196DD2C1FDA27024677754') throw new Error('FrameXML.toc inesperado');
const marker = '## add new modules above here';
const base = toc.toString('utf8');
if (!base.includes(marker)) throw new Error('Marcador de carga ausente');
const names = scripts.map(name => `WorldDigimonNative\\${name}`);
const alteredToc = base.replace('## Interface: 30300',
  '## Interface: 30300\r\n## SavedVariablesPerCharacter: WorldDigimonDB')
  .replace(marker, names.join('\r\n') + '\r\n' + marker);

const archive = Archive.create(outputPath, { maxFileCount: 2048, flags: 0 });
const contents = [];
try {
  archive.addBuffer(tocName, Buffer.from(alteredToc, 'utf8'));
  for (const name of scripts) {
    const sourceText = fs.readFileSync(path.join(addon, name), 'utf8');
    const transformed = sourceText.split(originalRoot).join(nativeRoot);
    if (transformed.includes(originalRoot)) throw new Error(`Ruta de addon sin convertir: ${name}`);
    archive.addBuffer(`${root}\\${name}`, Buffer.from(transformed, 'utf8'));
    contents.push({ name, bytes: Buffer.byteLength(transformed) });
  }
  const textures = path.join(addon, 'Textures');
  const stack = [textures];
  let assetCount = 0, assetBytes = 0;
  while (stack.length) {
    const current = stack.pop();
    for (const entry of fs.readdirSync(current, { withFileTypes: true })) {
      const full = path.join(current, entry.name);
      if (entry.isDirectory()) { stack.push(full); continue; }
      if (!/\.(blp|tga)$/i.test(entry.name)) continue;
      const rel = path.relative(textures, full).replaceAll('/', '\\');
      const data = fs.readFileSync(full);
      archive.addBuffer(`${root}\\Textures\\${rel}`, data);
      assetCount++; assetBytes += data.length;
    }
  }
  contents.push({ assets: assetCount, assetBytes });
} finally { archive.close(); }

const verify = Archive.open(outputPath, { flags: MPQ_OPEN_READ_ONLY });
try {
  if (verify.readFile(tocName).toString('utf8') !== alteredToc) throw new Error('Índice del MPQ incorrecto');
  for (const name of scripts) if (!verify.hasFile(`${root}\\${name}`)) throw new Error(`Falta ${name}`);
} finally { verify.close(); }
console.log(JSON.stringify({ outputPath, tocSha, mpqSha: crypto.createHash('sha256').update(fs.readFileSync(outputPath)).digest('hex').toUpperCase(), contents }));
