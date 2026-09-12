from pathlib import Path
import re, shutil, hashlib, json

repo=Path(r'C:\Users\phili\Projekte\Nakama')
scratch=Path(__file__).parent
build=repo/'eq-copilot/build/plugin'
tlogs=build/'EqCopSonde012ProjectReloadTest.dir/Release/EqCopSon.CE8CFC13.tlog'
source=repo/'eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp'
lines=(tlogs/'CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
ix=next(i for i,x in enumerate(lines) if x.upper() == '^'+str(source).upper())
compile=lines[ix+1]
compile=re.sub(r'/Fo"[^"]*"',lambda _: '/Fo"'+str(scratch/'repro.obj')+'"',compile,flags=re.I)
compile=re.sub(r'/Fd"[^"]*"',lambda _: '/Fd"'+str(scratch/'repro.pdb')+'"',compile,flags=re.I)
compile=re.sub(re.escape(str(source)),lambda _: str(scratch/'repro.cpp'),compile,flags=re.I)
(scratch/'compile.rsp').write_text(compile,encoding='utf-8')
lines=(tlogs/'link.command.1.tlog').read_text(encoding='utf-16').splitlines()
link=' '.join(x for x in lines if not x.startswith('^'))
evidence=[]
def file_arg(m):
    path=m.group(0).strip('"')
    p=Path(path)
    if not p.is_absolute(): p=build/p
    if p.suffix.lower() not in ('.obj','.lib','.res') or not p.exists(): return m.group(0)
    if p.name.lower() == 'sonde012projectreloadtest.obj': return '"'+str(scratch/'repro.obj')+'"'
    out=scratch/'inputs'/p.name
    out.parent.mkdir(exist_ok=True)
    shutil.copy2(p,out)
    evidence.append({'source':str(p),'copy':str(out),'sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'mtime':p.stat().st_mtime})
    return '"'+str(out)+'"'
link=re.sub(r'"[^"]+\.(?:OBJ|LIB|RES)"|[^\s"]+\.(?:OBJ|LIB|RES)',file_arg,link,flags=re.I)
for flag,name in [('OUT','repro.exe'),('PDB','linked.pdb'),('IMPLIB','repro.lib')]:
    link=re.sub('/'+flag+r':"[^"]*"',lambda m: '/'+flag+':"'+str(scratch/name)+'"',link,flags=re.I)
(scratch/'link.rsp').write_text(link,encoding='utf-8')
for rel in ['eq-copilot/plugin/src/prozessor/Ipc.cpp','eq-copilot/plugin/src/prozessor/State.cpp','eq-copilot/plugin/src/SourcesModel.cpp','eq-copilot/plugin/src/PluginProcessor.h']:
    p=repo/rel
    evidence.append({'source':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'mtime':p.stat().st_mtime})
(scratch/'input-provenance.json').write_text(json.dumps(evidence,indent=2),encoding='utf-8')
(scratch/'build.cmd').write_text('@echo off\r\ncall "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat" > "'+str(scratch/'vcvars.log')+'"\r\nif errorlevel 1 exit /b 2\r\ncl @"'+str(scratch/'compile.rsp')+'" > "'+str(scratch/'compile.log')+'" 2>&1\r\nif errorlevel 1 exit /b 3\r\nlink @"'+str(scratch/'link.rsp')+'" > "'+str(scratch/'link.log')+'" 2>&1\r\nif errorlevel 1 exit /b 4\r\n',encoding='ascii')
print('Copied inputs:',len(evidence),'Scratch:',scratch)
