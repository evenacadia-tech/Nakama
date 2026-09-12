import pathlib,re,subprocess
root=pathlib.Path(r'C:\Users\phili\Projekte\Nakama')
scratch=pathlib.Path(__file__).parent
build=root/'eq-copilot/build/plugin'
tlog=build/'EqCopTransactionTest.dir/Release/EqCopTra.07C47E25.tlog'
cl=(tlog/'CL.command.1.tlog').read_text(encoding='utf-16').splitlines()[1]
cl=re.sub(r'/Fo"[^"]*"', '/Fo"'+str(scratch/'host_audit.obj').replace('\\','/')+'"',cl,flags=re.I)
cl=re.sub(r'/Fd"[^"]*"', '/Fd"'+str(scratch/'host_audit.pdb').replace('\\','/')+'"',cl,flags=re.I)
cl=cl.rsplit(' ',1)[0]+' "'+str(scratch/'host_audit.cpp')+'"'
(scratch/'compile.rsp').write_text(cl,encoding='utf-8')
link=(tlog/'link.command.1.tlog').read_text(encoding='utf-16').splitlines()[1:]
link='\n'.join(link)
for opt in ['OUT','PDB','IMPLIB']:
 ext={'OUT':'exe','PDB':'pdb','IMPLIB':'lib'}[opt]
 link=re.sub('/'+opt+r':"[^"]*"','/'+opt+':"'+str(scratch/('host_audit.'+ext)).replace('\\','/')+'"',link,flags=re.I)
link=link.replace(r'EQCOPTRANSACTIONTEST.DIR\RELEASE\TRANSACTIONTESTMAIN.OBJ','"'+str(scratch/'host_audit.obj')+'"')
(scratch/'link.rsp').write_text(link,encoding='utf-8')
batch='@echo off\ncall "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat" >nul\ncl @"'+str(scratch/'compile.rsp')+'"\nif errorlevel 1 exit /b 1\nlink @"'+str(scratch/'link.rsp')+'"\n'
(scratch/'build.cmd').write_text(batch,encoding='utf-8')
p=subprocess.run(['cmd','/c',str(scratch/'build.cmd')],cwd=build,capture_output=True,text=True)
(scratch/'build.log').write_text(p.stdout+p.stderr,encoding='utf-8')
print(p.stdout,p.stderr,sep='')
if p.returncode:raise SystemExit(p.returncode)
p=subprocess.run([str(scratch/'host_audit.exe')],cwd=root,capture_output=True,text=True)
(scratch/'run.log').write_text(p.stdout+p.stderr,encoding='utf-8')
print(p.stdout,p.stderr,sep='')
raise SystemExit(p.returncode)
