"""NAK-353 - lesender Minidump-Leser ohne installierten Debugger (22.09.2026).

Liest Ausnahme-, Modul- und Threadstrom eines Minidumps und benennt die
Kandidaten fuer Ruecksprungadressen auf dem Stapel des ausloesenden Threads
mit der Windows-eigenen dbghelp.dll (heuristische Suche ab rsp wie `dps`;
ein veralteter Stapelplatz kann darunter sein). Symbole nur aus dem lokalen
Ordner, kein Symbolserver.

Aufruf: py -3.13 docs/beweise/roh/NAK-353-dumpleser.py <dump.dmp> broker/target/debug
Beleg: docs/beweise/roh/NAK-353-dumpanalyse.txt
"""
import ctypes
import ctypes.wintypes as wt
import struct
import sys
import os

DUMP = sys.argv[1]
SYMDIR = sys.argv[2] if len(sys.argv) > 2 else None
IMGDIR = sys.argv[3] if len(sys.argv) > 3 else SYMDIR

data = open(DUMP, "rb").read()
sig, ver, nstreams, dir_rva = struct.unpack_from("<4sIII", data, 0)
assert sig == b"MDMP", sig

streams = {}
for i in range(nstreams):
    stype, size, rva = struct.unpack_from("<III", data, dir_rva + 12 * i)
    streams.setdefault(stype, []).append((size, rva))


def mdstring(rva):
    (length,) = struct.unpack_from("<I", data, rva)
    return data[rva + 4: rva + 4 + length].decode("utf-16-le")


# --- modules (stream 4) ---
mods = []
size, rva = streams[4][0]
(nmod,) = struct.unpack_from("<I", data, rva)
for i in range(nmod):
    off = rva + 4 + 108 * i
    base, sz, chk, tds, name_rva = struct.unpack_from("<QIIII", data, off)
    cv_size, cv_rva = struct.unpack_from("<II", data, off + 24 + 52)
    cv = data[cv_rva: cv_rva + cv_size]
    guid = age = pdb = None
    if cv[:4] == b"RSDS":
        guid = cv[4:20].hex()
        (age,) = struct.unpack_from("<I", cv, 20)
        pdb = cv[24:].split(b"\0")[0].decode("utf-8", "replace")
    mods.append(dict(base=base, size=sz, tds=tds, name=mdstring(name_rva), guid=guid, age=age, pdb=pdb))


def mod_of(addr):
    for m in mods:
        if m["base"] <= addr < m["base"] + m["size"]:
            return m
    return None


# --- exception (stream 6) ---
size, rva = streams[6][0]
tid, _al = struct.unpack_from("<II", data, rva)
code, flags, rec, addr, nparams = struct.unpack_from("<IIQQI", data, rva + 8)
params = struct.unpack_from("<15Q", data, rva + 8 + 32)
ctx_size, ctx_rva = struct.unpack_from("<II", data, rva + 8 + 32 + 15 * 8)
rip = struct.unpack_from("<Q", data, ctx_rva + 0xF8)[0]
rsp = struct.unpack_from("<Q", data, ctx_rva + 0x98)[0]
m = mod_of(addr)
print(f"Dump: {os.path.basename(DUMP)}")
print(f"Exception thread 0x{tid:x}: code=0x{code:08X} flags=0x{flags:x} addr=0x{addr:x} "
      f"({m['name'] if m else '?'}+0x{addr - m['base']:x})" if m else "")
print(f"NumberParameters={nparams} Parameters={[hex(p) for p in params[:nparams]]}")
print(f"Context: rip=0x{rip:x} rsp=0x{rsp:x}")

# --- threads (stream 3) -> stack memory of exception thread ---
size, rva = streams[3][0]
(nthr,) = struct.unpack_from("<I", data, rva)
stack_start = stack_bytes = None
for i in range(nthr):
    off = rva + 4 + 48 * i
    t_id, susp, pc, prio, teb, st_start, st_size, st_rva, c_size, c_rva = struct.unpack_from("<IIIIQQIIII", data, off)
    if t_id == tid:
        stack_start = st_start
        stack_bytes = data[st_rva: st_rva + st_size]
print(f"Threads in dump: {nthr}")

main_mod = next((mm for mm in mods if mm["name"].lower().endswith(".exe")), None)
print(f"Main module: {main_mod['name']} base=0x{main_mod['base']:x} size=0x{main_mod['size']:x} "
      f"timestamp=0x{main_mod['tds']:x} pdb={main_mod['pdb']} guid={main_mod['guid']} age={main_mod['age']}")

# --- dbghelp symbolization (local symbol dir only) ---
dbghelp = ctypes.WinDLL(r"C:\Windows\System32\dbghelp.dll")
SYMOPT_UNDNAME, SYMOPT_DEFERRED_LOADS, SYMOPT_LOAD_LINES, SYMOPT_FAIL_CRITICAL_ERRORS = 0x2, 0x4, 0x10, 0x200
dbghelp.SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS)
hproc = wt.HANDLE(0x1234)
dbghelp.SymInitializeW.argtypes = [wt.HANDLE, wt.LPCWSTR, wt.BOOL]
if not dbghelp.SymInitializeW(hproc, SYMDIR, False):
    print("SymInitialize failed", ctypes.GetLastError())
dbghelp.SymLoadModuleExW.restype = ctypes.c_uint64
dbghelp.SymLoadModuleExW.argtypes = [wt.HANDLE, wt.HANDLE, wt.LPCWSTR, wt.LPCWSTR, ctypes.c_uint64, wt.DWORD, ctypes.c_void_p, wt.DWORD]


class SYMBOL_INFOW(ctypes.Structure):
    _fields_ = [("SizeOfStruct", wt.ULONG), ("TypeIndex", wt.ULONG), ("Reserved", ctypes.c_uint64 * 2),
                ("Index", wt.ULONG), ("Size", wt.ULONG), ("ModBase", ctypes.c_uint64), ("Flags", wt.ULONG),
                ("Value", ctypes.c_uint64), ("Address", ctypes.c_uint64), ("Register", wt.ULONG),
                ("Scope", wt.ULONG), ("Tag", wt.ULONG), ("NameLen", wt.ULONG), ("MaxNameLen", wt.ULONG),
                ("Name", wt.WCHAR * 1024)]


class IMAGEHLP_LINEW64(ctypes.Structure):
    _fields_ = [("SizeOfStruct", wt.DWORD), ("Key", ctypes.c_void_p), ("LineNumber", wt.DWORD),
                ("FileName", wt.LPWSTR), ("Address", ctypes.c_uint64)]


dbghelp.SymFromAddrW.argtypes = [wt.HANDLE, ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64), ctypes.POINTER(SYMBOL_INFOW)]
dbghelp.SymGetLineFromAddrW64.argtypes = [wt.HANDLE, ctypes.c_uint64, ctypes.POINTER(wt.DWORD), ctypes.POINTER(IMAGEHLP_LINEW64)]

loaded = {}


def ensure(mm):
    key = mm["base"]
    if key in loaded:
        return loaded[key]
    img = mm["name"]
    if IMGDIR:
        cand = os.path.join(IMGDIR, os.path.basename(img))
        if os.path.exists(cand):
            img = cand
    r = dbghelp.SymLoadModuleExW(hproc, None, img, None, mm["base"], mm["size"], None, 0)
    loaded[key] = r
    return r


def sym(a):
    mm = mod_of(a)
    if not mm:
        return None
    ensure(mm)
    si = SYMBOL_INFOW()
    si.SizeOfStruct = 88
    si.MaxNameLen = 1024
    disp = ctypes.c_uint64(0)
    name = None
    if dbghelp.SymFromAddrW(hproc, a, ctypes.byref(disp), ctypes.byref(si)):
        name = f"{os.path.basename(mm['name'])}!{si.Name}+0x{disp.value:x}"
    else:
        name = f"{os.path.basename(mm['name'])}+0x{a - mm['base']:x}"
    ln = IMAGEHLP_LINEW64()
    ln.SizeOfStruct = ctypes.sizeof(IMAGEHLP_LINEW64)
    d2 = wt.DWORD(0)
    if dbghelp.SymGetLineFromAddrW64(hproc, a, ctypes.byref(d2), ctypes.byref(ln)):
        name += f"  [{ln.FileName}:{ln.LineNumber}]"
    return name


print("\nFaulting IP:", sym(addr))
print("\nCandidate return addresses on the exception thread stack (heuristic scan from rsp, like dps):")
if stack_bytes is not None:
    begin = max(0, rsp - stack_start)
    shown = 0
    for off in range(begin, len(stack_bytes) - 7, 8):
        (v,) = struct.unpack_from("<Q", stack_bytes, off)
        mm = mod_of(v)
        if mm and not mm["name"].lower().endswith(("ntdll.dll", "kernel32.dll", "kernelbase.dll")) or (mm and shown < 3):
            s = sym(v)
            print(f"  [rsp+0x{off - begin:04x}] 0x{v:x}  {s}")
            shown += 1
            if shown >= 40:
                break
