# Prisma-Material-Probestill — Blender 5.1, Cycles auf CPU (nie GPU: Arc-Landmine).
#
#   blender -b --factory-startup -P prisma-material-still.py -- \
#       --variante klar|rauchig|frostig [--samples 512] [--breite 1600] [--hoehe 1000] \
#       [--out renders/prisma-klar.png]
#
# Drehsequenz für die Prototyp-Seite (EIN Prozess rendert alle Frames,
# Prisma rotiert um seine Achse, Set bleibt — 120° = Symmetrieperiode):
#   ... -- --variante klar --dreh 60 --samples 320 --breite 800 --hoehe 500
#
# EIN Still, keine Animation, keine Daten: dreiseitiges Glas-Prisma über dem
# User-Hintergrund (Winter-Nexus-Plate als Ebene 0), schmaler weißer Strahl
# durchs Glas -> echter Dispersionsfächer als Kaustik am Boden.
# Blender 5.1 hat KEINE native Dispersion (API-Probe 18.08.): drei Glass-BSDFs
# mit kanalgetrennten IORs, additiv — der klassische physikalische Weg.

import bpy
import math
import os
import sys
from mathutils import Vector

# ---------------------------------------------------------------- Argumente
argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []


def arg(name, standard):
    return argv[argv.index(name) + 1] if name in argv else standard


VARIANTE = arg("--variante", "klar")
SAMPLES = int(arg("--samples", "512"))
BREITE = int(arg("--breite", "1600"))
HOEHE = int(arg("--hoehe", "1000"))
HIER = os.path.dirname(os.path.abspath(__file__))
OUT = arg("--out", os.path.join("renders", f"prisma-{VARIANTE}.png"))
if not os.path.isabs(OUT):
    OUT = os.path.join(HIER, OUT)
PLATE = os.path.join(HIER, "winter-nexus-plate.png")

if VARIANTE not in ("klar", "rauchig", "frostig"):
    raise SystemExit(f"Unbekannte Variante: {VARIANTE}")

# ---------------------------------------------------------------- Szene leer
bpy.ops.wm.read_factory_settings(use_empty=True)
szene = bpy.context.scene

szene.render.engine = "CYCLES"
szene.cycles.device = "CPU"                      # Grundgesetz der Maschine
szene.cycles.samples = SAMPLES
szene.cycles.use_denoising = True
szene.cycles.caustics_reflective = True
szene.cycles.caustics_refractive = True
szene.cycles.use_guiding = True                  # Path Guiding (CPU) für Kaustik
szene.cycles.blur_glossy = 0.05
# Stills: 0 (kein Clamp — er kappt seltene hochenergetische Kaustik-
# Samples). Sequenzen: ~30 setzen — ungebremste Fireflies geben dem
# Denoiser pro Frame ein anderes Rauschmuster, das beim Drehen kriecht.
szene.cycles.sample_clamp_indirect = float(arg("--clamp", "0"))
szene.cycles.max_bounces = 24
szene.cycles.transmission_bounces = 24
szene.cycles.transparent_max_bounces = 16

szene.render.resolution_x = BREITE
szene.render.resolution_y = HOEHE
szene.render.filepath = OUT
szene.render.image_settings.file_format = "PNG"
szene.view_settings.view_transform = "AgX"

welt = bpy.data.worlds.new("schwarz")
szene.world = welt
welt.use_nodes = True
welt.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.0

# ---------------------------------------------------------------- Geometrie
def objekt(name, mesh):
    ob = bpy.data.objects.new(name, mesh)
    szene.collection.objects.link(ob)
    return ob


# Dreiseitiges Prisma: gleichseitiges Profil (Umkreisradius R), Höhe H.
R, H = 0.5, 1.6
winkel = [math.radians(90 + i * 120) for i in range(3)]
unten = [(R * math.cos(w), R * math.sin(w), 0.0) for w in winkel]
oben = [(x, y, H) for (x, y, _) in unten]
mesh = bpy.data.meshes.new("prisma")
mesh.from_pydata(
    unten + oben,
    [],
    [(0, 1, 2), (5, 4, 3), (0, 3, 4, 1), (1, 4, 5, 2), (2, 5, 3, 0)],
)
mesh.update()
prisma = objekt("Prisma", mesh)
# 28°: beide Flächen wirklich sichtbar (Normalen 67°/53° zur Kamera-
# Peilung). Strahl tritt über die kameradominante Frontfläche ein (51°,
# intern 32° — sicher unter der 42.7°-Totalreflexionsgrenze), quert zur
# Rückfläche, Austritt Richtung +Y: der Fächer landet auf der Wand LINKS
# neben dem Prisma (~(-0.38, 2.65)) — nicht von ihm selbst verdeckt
# (Draufsicht-Debug 18.08.: bei 72° verdeckte das Prisma den eigenen
# Fächer). Spektralspreizung ~0.24 m bei ~17 cm Strahlbreite.
ROT_WINKEL = float(arg("--rot-winkel", "28"))    # Sequenz: Frame i = 28 + i*120/N
prisma.rotation_euler = (0.0, 0.0, math.radians(ROT_WINKEL))
prisma.location = (0.0, 0.0, 0.002)              # nie koplanar mit dem Boden
kante = prisma.modifiers.new("kante", "BEVEL")   # reale Glaskante fängt Licht
kante.width = 0.004
kante.segments = 2
# MNEE (Shadow Caustics): deterministische Kaustik Licht->Glas->Empfänger —
# der einzige Cycles-Pfad, der den scharfen Spektralfächer zuverlässig löst.
prisma.cycles.is_caustics_caster = True

boden_mesh = bpy.data.meshes.new("boden")
boden_mesh.from_pydata(
    [(-15, -15, 0), (15, -15, 0), (15, 15, 0), (-15, 15, 0)], [], [(0, 1, 2, 3)]
)
boden_mesh.update()
boden = objekt("Boden", boden_mesh)
boden.cycles.is_caustics_receiver = True

# Hintergrund-Plate (Ebene 0 des Compositing-Sandwichs): User-Szene als Bild.
pw, ph = 6.2, 3.875                              # füllt die Kamerasicht knapp
plate_mesh = bpy.data.meshes.new("plate")
plate_mesh.from_pydata(
    [(-pw / 2, 0, -ph / 2), (pw / 2, 0, -ph / 2), (pw / 2, 0, ph / 2), (-pw / 2, 0, ph / 2)],
    [],
    [(0, 1, 2, 3)],
)
plate_mesh.uv_layers.new(name="uv")
uv = plate_mesh.uv_layers["uv"].data
for schleifen_index, uv_koord in enumerate([(0, 0), (1, 0), (1, 1), (0, 1)]):
    uv[schleifen_index].uv = uv_koord
plate_mesh.update()
plate = objekt("Plate", plate_mesh)
plate.cycles.is_caustics_receiver = True
plate.location = (1.24, 2.6, 0.95)               # Rückwand in der Sichtlinie
plate.rotation_euler = (0.0, 0.0, math.radians(-25))   # frontal zur Kamera

# ---------------------------------------------------------------- Materialien
def neues_material(name, volumen=False):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.remove(nt.nodes["Principled BSDF"])
    return mat, nt, nt.nodes["Material Output"]


def dispersives_glas(nt, aus, rauheit, ior_mitte=1.474, ior_spreiz=0.028, bump=None):
    """Drei kanalgetrennte Glass-BSDFs (R/G/B mit steigendem IOR), additiv."""
    add1 = nt.nodes.new("ShaderNodeAddShader")
    add2 = nt.nodes.new("ShaderNodeAddShader")
    farben = [(1, 0, 0, 1), (0, 1, 0, 1), (0, 0, 1, 1)]
    iors = [ior_mitte - ior_spreiz, ior_mitte, ior_mitte + ior_spreiz]
    for i, (farbe, ior) in enumerate(zip(farben, iors)):
        glas = nt.nodes.new("ShaderNodeBsdfGlass")
        glas.distribution = "MULTI_GGX"
        glas.inputs["Color"].default_value = farbe
        glas.inputs["Roughness"].default_value = rauheit
        glas.inputs["IOR"].default_value = ior
        if bump is not None:
            nt.links.new(bump, glas.inputs["Normal"])
        if i < 2:
            nt.links.new(glas.outputs[0], add1.inputs[i])
        else:
            nt.links.new(glas.outputs[0], add2.inputs[1])
    nt.links.new(add1.outputs[0], add2.inputs[0])
    nt.links.new(add2.outputs[0], aus.inputs["Surface"])


mat, nt, aus = neues_material(f"glas-{VARIANTE}")
if VARIANTE == "klar":
    # Museal: makellos, volle Transparenz, sichtbare Dispersion.
    dispersives_glas(nt, aus, rauheit=0.0)
elif VARIANTE == "rauchig":
    # Dunkel-rauchig: Beer-Lambert-Absorption — Dichte wächst mit Glasweg,
    # Kern dunkel, Kanten hell. Physikalisch, kein getöntes Shader-Overlay.
    dispersives_glas(nt, aus, rauheit=0.0)
    absorb = nt.nodes.new("ShaderNodeVolumeAbsorption")
    absorb.inputs["Color"].default_value = (0.55, 0.62, 0.72, 1.0)
    absorb.inputs["Density"].default_value = 2.6
    nt.links.new(absorb.outputs[0], aus.inputs["Volume"])
else:  # frostig
    # Frost aus MIKROSTRUKTUR (Noise-Bump), nicht aus Roughness-Weichzeichner —
    # Gesetz 1: Schärfe aus echter Struktur.
    rausch = nt.nodes.new("ShaderNodeTexNoise")
    rausch.inputs["Scale"].default_value = 55.0
    rausch.inputs["Detail"].default_value = 6.0
    bump = nt.nodes.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = 0.28
    bump.inputs["Distance"].default_value = 0.002
    nt.links.new(rausch.outputs["Fac"], bump.inputs["Height"])
    dispersives_glas(nt, aus, rauheit=0.06, bump=bump.outputs["Normal"])
prisma.data.materials.append(mat)

bmat, bnt, baus = neues_material("boden-satin")
bsdf = bnt.nodes.new("ShaderNodeBsdfPrincipled")
bsdf.inputs["Base Color"].default_value = (0.008, 0.009, 0.012, 1.0)
bsdf.inputs["Roughness"].default_value = 0.28
bnt.links.new(bsdf.outputs[0], baus.inputs["Surface"])
boden.data.materials.append(bmat)

# Wand = Emission (User-Bild) + dunkles Diffus: der Spektralfächer kann
# auf ihr ZEICHNEN — reines Emission-Material ignoriert einfallendes Licht.
pmat, pnt, paus = neues_material("plate-leuchtet")
tex = pnt.nodes.new("ShaderNodeTexImage")
tex.image = bpy.data.images.load(PLATE)
emit = pnt.nodes.new("ShaderNodeEmission")
emit.inputs["Strength"].default_value = 2.0
pnt.links.new(tex.outputs["Color"], emit.inputs["Color"])
diffus = pnt.nodes.new("ShaderNodeBsdfDiffuse")
diffus.inputs["Color"].default_value = (0.03, 0.033, 0.04, 1.0)
addiere = pnt.nodes.new("ShaderNodeAddShader")
pnt.links.new(emit.outputs[0], addiere.inputs[0])
pnt.links.new(diffus.outputs[0], addiere.inputs[1])
pnt.links.new(addiere.outputs[0], paus.inputs["Surface"])
plate.data.materials.append(pmat)
plate.visible_shadow = False

# ---------------------------------------------------------------- Licht
# Schmaler weißer Strahl durchs Prisma -> Dispersionsfächer auf dem Boden.
strahl = bpy.data.lights.new("strahl", "SPOT")
# Quer zur Blickachse: die Kamera sieht die FOLGEN des Strahls (Fächer,
# Kanten), nie in die Quelle — sonst brennt der Glaskörper weiß aus.
# Blender normalisiert Spot-Watt über die VOLLE Kugel — ein enger Kegel
# maskiert, statt zu bündeln (Repro 18.08.). Für ~2000 W/m² am Glas
# braucht der 3°-Strahl deshalb Scheinwerfer-Zahlen.
strahl.energy = float(arg("--energie", "300000"))
strahl.spot_size = math.radians(3)               # eng: Spektrum trennt sich
strahl.spot_blend = 0.12
strahl.shadow_soft_size = 0.02                   # klein = scharfe Kaustik
strahl.cycles.is_caustics_light = True           # MNEE-Quelle
strahl_ob = bpy.data.objects.new("Strahl", strahl)
szene.collection.objects.link(strahl_ob)
# Eintritt LINKE Fläche (47.5° horizontal), Austritt rechte Fläche,
# steil fallend: der Fächer landet auf dem Boden bei ~(1.3, 0.5) —
# 4° RECHTS der Silhouette, frei sichtbar, ~10 cm Farbspreizung
# (Snell-Tracer 18.08.). Nah am Glas, damit der Pathtracer den
# Kaustikpfad sampeln kann (an der fernen Wand: physikalisch da,
# praktisch unsampelbar). Gesamtwinkel intern 37° < 42.7° TIR-Grenze.
strahl_ob.location = (-2.24, -2.28, 2.3)

# Dark-Field-Glasbeleuchtung: schmale VERTIKALE Streifen-Softboxen zeichnen
# Kanten und Flächen als scharfe Lichtlinien; der Raum bleibt schwarz.
def streifen(name, ort, energie):
    licht = bpy.data.lights.new(name, "AREA")
    licht.shape = "RECTANGLE"
    licht.size = 0.12                            # schmal
    licht.size_y = 2.6                           # hoch
    licht.energy = energie
    licht.color = (0.62, 0.76, 1.0)
    ob = bpy.data.objects.new(name, licht)
    szene.collection.objects.link(ob)
    ob.location = ort
    # Nur Oberflächen-Glanz, nicht durchs Glas sichtbar — sonst steht das
    # Leuchtpanel als heller Block im refraktierten Bild.
    ob.visible_transmission = False
    return ob


NUR_STRAHL = "--nur-strahl" in argv                     # Debug: alles außer Strahl aus
rand_ob = streifen("RandB", (3.95, -0.63, 1.3), 0.0 if NUR_STRAHL else 10.0)
rand2_ob = streifen("RandA", (-0.93, 2.43, 1.3), 0.0 if NUR_STRAHL else 6.0)
if NUR_STRAHL:
    emit.inputs["Strength"].default_value = 0.0

# Fächer-Quelle: kleines Flächenlicht TIEF hinter dem Glas — sein Licht
# wird beim Durchgang dispergiert und landet als Farbfächer vorn an der
# Basis. Flächenlicht nah am Glas = großer Raumwinkel = der Kaustikpfad
# ist für den Pathtracer sampelbar (der Spot-Fächer war es nie).
faecher = bpy.data.lights.new("faecher", "AREA")
faecher.shape = "SQUARE"
# 7 cm bei 1.15 m Abstand = ~3.5° Quellwinkel < 4.75° Dispersionsspreizung:
# nur so trennen sich die Farben statt zu Weiß zu überlappen.
faecher.size = 0.07
faecher.energy = 0.0 if NUR_STRAHL else 320.0
faecher.color = (1.0, 0.97, 0.92)
faecher.spread = math.radians(40)                # wenig Raum-Streulicht
faecher_ob = bpy.data.objects.new("Faecher", faecher)
szene.collection.objects.link(faecher_ob)
faecher_ob.location = (0.25, 1.15, 0.55)

# Lichtpool auf der Wand hinter dem Glas: das Glas zeichnet sich durch
# Refraktionsverzerrung gegen den hellen Fleck ab (Dark-Field-Klassiker).
wand_licht = bpy.data.lights.new("wandpool", "AREA")
wand_licht.shape = "RECTANGLE"
wand_licht.size = 1.0
wand_licht.size_y = 3.0
wand_licht.energy = 0.0 if NUR_STRAHL else 45.0
wand_licht.color = (0.7, 0.8, 1.0)
wand_ob = bpy.data.objects.new("Wandpool", wand_licht)
szene.collection.objects.link(wand_ob)
wand_licht.size = 1.6
wand_ob.location = (0.7, 2.0, 1.1)


def richte_auf(ob, ziel):
    # Kameras und Lichter schauen entlang -Z; Standard-Idiom statt Handtrigonometrie.
    ob.rotation_euler = (Vector(ziel) - ob.location).to_track_quat("-Z", "Y").to_euler()


# Ziel = Mitte der linken Fläche A: Fächer tritt rechts aus und landet
# per Snell-Tracer bei ~(1.3, 0.5) auf offenem Boden rechts der Silhouette.
richte_auf(strahl_ob, (-0.25, 0.01, 0.95))
richte_auf(rand_ob, (0.0, 0.0, 1.0))
richte_auf(rand2_ob, (0.0, 0.0, 1.0))
richte_auf(wand_ob, (1.1, 2.6, 0.9))             # zielt auf die Wand, nicht das Glas
richte_auf(faecher_ob, (0.0, 0.15, 0.55))        # tief durch die Rückfläche

# ---------------------------------------------------------------- Kamera
kam = bpy.data.cameras.new("kamera")
kam.lens = float(arg("--lens", "50"))
kam_ob = bpy.data.objects.new("Kamera", kam)
szene.collection.objects.link(kam_ob)
kam_pos = tuple(float(x) for x in arg("--kamera", "-2.3,-4.9,1.05").split(","))
kam_ziel = tuple(float(x) for x in arg("--ziel", "0,0,0.78").split(","))
kam_ob.location = kam_pos                        # Standard: zwei Flächen + Kante
richte_auf(kam_ob, kam_ziel)
szene.camera = kam_ob

# Nur-Glas-Ebene (Film-Sandwich, oberste Schicht): Boden/Wand für die
# Kamera unsichtbar, bleiben aber Spiegelquellen; transparenter Film.
# Alpha = Prisma-Silhouette (maskiert die live Punkt-Ebene ins Volumen).
if "--nur-glas" in argv:
    boden.visible_camera = False
    plate.visible_camera = False
    szene.render.film_transparent = True
    szene.render.image_settings.color_mode = "RGBA"

if "--ohne-prisma" in argv:                       # Projektions-Beweis ohne Brechung
    prisma.hide_render = True

# Ausrichtungs-Beweis für die JS-Kameraprojektion: kleine Leuchtkugeln an
# bekannten PRISMA-LOKALEN Koordinaten (drehen mit ROT_WINKEL mit).
if "--debug-kugeln" in argv:
    from mathutils import Euler
    rot_m = Euler((0.0, 0.0, math.radians(ROT_WINKEL))).to_matrix()
    lokal = [(0, 0, 0.1), (0, 0, 1.5), (0.3, 0, 0.8), (-0.2, 0.25, 0.4), (0, -0.3, 1.2)]
    kmat = bpy.data.materials.new("kugel-leuchtet")
    kmat.use_nodes = True
    knodes = kmat.node_tree
    knodes.nodes.remove(knodes.nodes["Principled BSDF"])
    kemit = knodes.nodes.new("ShaderNodeEmission")
    kemit.inputs["Strength"].default_value = 30.0
    kemit.inputs["Color"].default_value = (0.2, 1.0, 0.3, 1.0)
    knodes.links.new(kemit.outputs[0], knodes.nodes["Material Output"].inputs["Surface"])
    for k, p in enumerate(lokal):
        w = rot_m @ Vector(p)
        bpy.ops.mesh.primitive_uv_sphere_add(radius=0.015, location=(w.x, w.y, w.z + 0.002))
        kugel = bpy.context.active_object
        kugel.data.materials.append(kmat)

# ---------------------------------------------------------------- Render
DREH = int(arg("--dreh", "0"))
if DREH > 0:
    # Drehsequenz: Frame i = Basisrotation + i * 120°/N (exklusiv 120°,
    # damit Frame 0 nahtlos auf Frame N folgt). WebP hält die Sequenz klein.
    szene.render.image_settings.file_format = "WEBP"
    szene.render.image_settings.quality = 95
    dreh_dir = os.path.join(HIER, "renders", "dreh")
    os.makedirs(dreh_dir, exist_ok=True)
    basis = prisma.rotation_euler.z
    for i in range(DREH):
        prisma.rotation_euler = (0.0, 0.0, basis + math.radians(i * 120.0 / DREH))
        szene.render.filepath = os.path.join(dreh_dir, f"f{i:03d}.webp")
        bpy.ops.render.render(write_still=True)
        print(f"DREH {i + 1}/{DREH}: {szene.render.filepath}", flush=True)
    print(f"FERTIG: {DREH} Frames ({VARIANTE}, {SAMPLES} Samples, {BREITE}x{HOEHE}, CPU)")
else:
    bpy.ops.render.render(write_still=True)
    print(f"FERTIG: {OUT} ({VARIANTE}, {SAMPLES} Samples, {BREITE}x{HOEHE}, CPU)")
