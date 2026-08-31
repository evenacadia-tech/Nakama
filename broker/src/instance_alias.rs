//! SONDE-011 / NAK-40: Abbildung persistenter `instance_id`-Werte auf die
//! eingefrorene hex32-Zieladresse des v3-Vertrags.
//!
//! Der Alias ist reine Laufzeitadressierung. Er wird weder dekodiert noch in
//! einen Host-State geschrieben. Kollisionen werden unter genau einem Lock
//! fail-closed behandelt: auch die zuerst registrierte Seite verliert ihren
//! Indexeintrag, bevor die Kollisionsregistrierung zurueckkehrt.

use sha2::{Digest, Sha256};
use std::collections::{HashMap, HashSet};
use std::sync::Mutex;

const DOMAIN: &[u8] = b"nakama.v3.instance-address.v1";

fn ist_hex32(wert: &str) -> bool {
    wert.len() == 32
        && wert
            .as_bytes()
            .iter()
            .all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(b))
}

/// Liefert `None` fuer den leeren Altwert: nur der besitzende State-Lader
/// darf dafuer eine frische UUID erzeugen. Gueltiges hex32 bleibt identisch;
/// alles andere wird aus den exakten UTF-8-Bytes ohne Normalisierung gebildet.
pub fn instance_adresse_aus_state(instance_id: &str) -> Option<String> {
    if instance_id.is_empty() {
        return None;
    }
    if ist_hex32(instance_id) {
        return Some(instance_id.to_owned());
    }

    let bytes = instance_id.as_bytes();
    let mut hash = Sha256::new();
    hash.update(DOMAIN);
    hash.update([0]);
    hash.update((bytes.len() as u64).to_be_bytes());
    hash.update(bytes);
    let digest = hash.finalize();
    Some(digest[..16].iter().map(|b| format!("{b:02x}")).collect())
}

/// Der bestehende Frischzustandsweg: leer wird zu einer neuen UUID, nicht zu
/// einem Hash eines fehlenden Werts. Diese Funktion ist absichtlich getrennt
/// von `instance_adresse_aus_state`, damit kein Leser Leere als Identitaet
/// missversteht.
pub fn instance_adresse_oder_frische_uuid(instance_id: &str) -> String {
    instance_adresse_aus_state(instance_id)
        .unwrap_or_else(|| uuid::Uuid::new_v4().simple().to_string())
}

/// Dokumentierter Rueckweg der nicht umkehrbaren Abbildung: nur die lokale
/// Originalwahrheit wird erneut abgebildet und exakt verglichen.
pub fn ziel_passt(lokale_instance_id: &str, wire_instance_id: &str) -> bool {
    ist_hex32(wire_instance_id)
        && instance_adresse_aus_state(lokale_instance_id).as_deref() == Some(wire_instance_id)
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Registrierung {
    Eingetragen,
    BereitsEingetragen,
    KollisionBeideQuarantaenisiert,
    Ungueltig,
}

/// Der stabile Adressraum, innerhalb dessen eine Wire-`instance_id` eindeutig
/// sein muss. Zwei Projektkopien mit verschiedener Sitzung teilen weder
/// Aliasindex noch Quarantaene.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Sitzungsadressraum {
    logon_sid: String,
    project_binding_id: String,
    session_epoch: String,
}

impl Sitzungsadressraum {
    pub fn neu(logon_sid: &str, project_binding_id: &str, session_epoch: &str) -> Self {
        Self {
            logon_sid: logon_sid.to_owned(),
            project_binding_id: project_binding_id.to_owned(),
            session_epoch: session_epoch.to_owned(),
        }
    }
}

type BesitzerSchluessel = (Sitzungsadressraum, String);
type WireSchluessel = (Sitzungsadressraum, String);

#[derive(Debug, Default)]
struct AliasStand {
    nach_wire: HashMap<WireSchluessel, String>,
    nach_original: HashMap<BesitzerSchluessel, String>,
    quarantaene: HashSet<BesitzerSchluessel>,
}

/// Ein Register fuer alle drei Zielwege (Dispatch, Telemetrie, Session-Push).
/// Alle Mutation liegt hinter EINEM Mutex; ein Leser kann deshalb nie den
/// alten Indexeintrag zwischen Kollisionserkennung und Quarantaene sehen.
#[derive(Debug, Default)]
pub struct AliasRegister {
    stand: Mutex<AliasStand>,
}

impl AliasRegister {
    fn besitzer_schluessel(adressraum: &Sitzungsadressraum, original: &str) -> BesitzerSchluessel {
        (adressraum.clone(), original.to_owned())
    }

    fn wire_schluessel(adressraum: &Sitzungsadressraum, wire: &str) -> WireSchluessel {
        (adressraum.clone(), wire.to_owned())
    }

    pub fn registriere(&self, adressraum: &Sitzungsadressraum, original: &str) -> Registrierung {
        let Some(wire) = instance_adresse_aus_state(original) else {
            return Registrierung::Ungueltig;
        };
        self.registriere_wire_zuordnung(adressraum, original, &wire)
    }

    /// Registriert eine bereits auf der Wireseite beobachtete Zuordnung. Der
    /// Broker kennt den nicht umkehrbaren Originaltext nicht; dort ist
    /// `original` die eindeutige Besitzeridentitaet der Verbindung
    /// (Wire-instance_id + runtime_nonce). Zwei Besitzer desselben Wirewerts
    /// werden damit genauso fail-closed behandelt wie eine Hashkollision.
    pub fn registriere_wire_zuordnung(
        &self,
        adressraum: &Sitzungsadressraum,
        original: &str,
        wire: &str,
    ) -> Registrierung {
        if original.is_empty() || !ist_hex32(wire) {
            return Registrierung::Ungueltig;
        }
        let mut stand = self.stand.lock().expect("AliasRegister vergiftet");
        let besitzer = Self::besitzer_schluessel(adressraum, original);
        let wire_schluessel = Self::wire_schluessel(adressraum, wire);
        if stand.quarantaene.contains(&besitzer) {
            return Registrierung::Ungueltig;
        }
        if stand
            .nach_original
            .get(&besitzer)
            .is_some_and(|bekannt| bekannt == wire)
        {
            return Registrierung::BereitsEingetragen;
        }

        if let Some(erster) = stand.nach_wire.get(&wire_schluessel).cloned() {
            if erster == original {
                return Registrierung::BereitsEingetragen;
            }

            // Der kritische Satz des Dirigentenentscheids: zuerst den schon
            // sichtbaren Index entfernen, dann BEIDE Originale unter
            // demselben Lock quarantinisieren. Vor Lockfreigabe existiert
            // keine gewaehlte Zuordnung mehr.
            let erster_besitzer = Self::besitzer_schluessel(adressraum, &erster);
            stand.nach_wire.remove(&wire_schluessel);
            stand.nach_original.remove(&erster_besitzer);
            stand.nach_original.remove(&besitzer);
            stand.quarantaene.insert(erster_besitzer);
            stand.quarantaene.insert(besitzer);
            return Registrierung::KollisionBeideQuarantaenisiert;
        }

        // Ein Original darf nicht still von einer Zieladresse zu einer
        // anderen wandern. Das waere eine Identitaetsumschreibung.
        if let Some(alter_wire) = stand.nach_original.get(&besitzer).cloned() {
            stand.nach_original.remove(&besitzer);
            let alter_wire_schluessel = Self::wire_schluessel(adressraum, &alter_wire);
            if stand
                .nach_wire
                .get(&alter_wire_schluessel)
                .is_some_and(|owner| owner == original)
            {
                stand.nach_wire.remove(&alter_wire_schluessel);
            }
            stand.quarantaene.insert(besitzer);
            return Registrierung::Ungueltig;
        }

        stand.nach_wire.insert(wire_schluessel, original.to_owned());
        stand.nach_original.insert(besitzer, wire.to_owned());
        Registrierung::Eingetragen
    }

    /// Entfernt eine kollisionsfreie Laufzeitzuordnung beim normalen
    /// Verbindungsende. Quarantaene bleibt dagegen bis zur expliziten Neu-ID-
    /// Aufloesung bestehen; ein Disconnect darf den sichtbaren Konflikt nicht
    /// waschen.
    pub fn entferne(&self, adressraum: &Sitzungsadressraum, original: &str, wire: &str) {
        let mut stand = self.stand.lock().expect("AliasRegister vergiftet");
        let besitzer = Self::besitzer_schluessel(adressraum, original);
        let wire_schluessel = Self::wire_schluessel(adressraum, wire);
        if stand.quarantaene.contains(&besitzer) {
            return;
        }
        if stand
            .nach_original
            .get(&besitzer)
            .is_some_and(|bekannt| bekannt == wire)
            && stand
                .nach_wire
                .get(&wire_schluessel)
                .is_some_and(|bekannt| bekannt == original)
        {
            stand.nach_original.remove(&besitzer);
            stand.nach_wire.remove(&wire_schluessel);
        }
    }

    pub fn dispatch_erlaubt(
        &self,
        adressraum: &Sitzungsadressraum,
        original: &str,
        wire: &str,
    ) -> bool {
        let stand = self.stand.lock().expect("AliasRegister vergiftet");
        let besitzer = Self::besitzer_schluessel(adressraum, original);
        let wire_schluessel = Self::wire_schluessel(adressraum, wire);
        !stand.quarantaene.contains(&besitzer)
            && stand
                .nach_original
                .get(&besitzer)
                .is_some_and(|w| w == wire)
            && stand
                .nach_wire
                .get(&wire_schluessel)
                .is_some_and(|o| o == original)
    }

    pub fn telemetrie_erlaubt(
        &self,
        adressraum: &Sitzungsadressraum,
        original: &str,
        wire: &str,
    ) -> bool {
        self.dispatch_erlaubt(adressraum, original, wire)
    }

    pub fn session_push_erlaubt(
        &self,
        adressraum: &Sitzungsadressraum,
        original: &str,
        wire: &str,
    ) -> bool {
        self.dispatch_erlaubt(adressraum, original, wire)
    }

    pub fn ist_quarantaenisiert(&self, adressraum: &Sitzungsadressraum, original: &str) -> bool {
        self.stand
            .lock()
            .expect("AliasRegister vergiftet")
            .quarantaene
            .contains(&Self::besitzer_schluessel(adressraum, original))
    }

    pub fn aliasindex_hat(&self, adressraum: &Sitzungsadressraum, wire: &str) -> bool {
        self.stand
            .lock()
            .expect("AliasRegister vergiftet")
            .nach_wire
            .contains_key(&Self::wire_schluessel(adressraum, wire))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde::Deserialize;
    use std::fs;
    use std::path::PathBuf;

    #[derive(Deserialize)]
    struct VektorDatei {
        domain: String,
        vectors: Vec<Vektor>,
    }

    #[derive(Deserialize)]
    struct Vektor {
        instance_id: String,
        wire_instance_id: String,
    }

    fn vektoren() -> VektorDatei {
        let pfad = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../eq-copilot/fixtures/v3/instance-address-alias-v1.json");
        serde_json::from_slice(&fs::read(pfad).expect("Aliasfixture lesen"))
            .expect("Aliasfixture parsen")
    }

    fn adressraum() -> Sitzungsadressraum {
        Sitzungsadressraum::neu(
            "S-1-5-21-1-2-3-1001",
            "11111111111111111111111111111111",
            "22222222222222222222222222222222",
        )
    }

    #[test]
    fn instance_address_alias_vectors() {
        let datei = vektoren();
        assert_eq!(datei.domain.as_bytes(), DOMAIN);
        for v in datei.vectors {
            assert_eq!(
                instance_adresse_aus_state(&v.instance_id).as_deref(),
                Some(v.wire_instance_id.as_str()),
                "{}",
                v.instance_id
            );
        }
    }

    #[test]
    fn instance_address_empty_uses_fresh_uuid() {
        assert_eq!(instance_adresse_aus_state(""), None);
        let a = instance_adresse_oder_frische_uuid("");
        let b = instance_adresse_oder_frische_uuid("");
        assert!(ist_hex32(&a));
        assert!(ist_hex32(&b));
        assert_ne!(a, b);
    }

    #[test]
    fn instance_alias_target_recomputed_and_matched() {
        let original = "11111111-2222-3333-4444-555555555555";
        let wire = "239953e95f122e18abb34ba1add46f9e";
        assert!(ziel_passt(original, wire));
        assert!(!ziel_passt("abcdef", wire));
    }

    #[test]
    fn instance_alias_unknown_fail_closed() {
        let register = AliasRegister::default();
        let adressraum = adressraum();
        let original = "11111111-2222-3333-4444-555555555555";
        let wire = instance_adresse_aus_state(original).unwrap();
        assert!(!register.dispatch_erlaubt(&adressraum, original, &wire));
        assert!(!register.telemetrie_erlaubt(&adressraum, original, &wire));
        assert!(!register.session_push_erlaubt(&adressraum, original, &wire));
        assert_eq!(
            register.registriere(&adressraum, original),
            Registrierung::Eingetragen
        );
        assert!(register.dispatch_erlaubt(&adressraum, original, &wire));
        assert!(!register.dispatch_erlaubt(
            &adressraum,
            original,
            "ffffffffffffffffffffffffffffffff"
        ));
    }

    fn pruefe_kollision(
        register: &AliasRegister,
        adressraum: &Sitzungsadressraum,
        a: &str,
        b: &str,
        wire: &str,
    ) {
        assert!(!register.aliasindex_hat(adressraum, wire));
        assert!(register.ist_quarantaenisiert(adressraum, a));
        assert!(register.ist_quarantaenisiert(adressraum, b));
        for original in [a, b] {
            assert!(!register.dispatch_erlaubt(adressraum, original, wire));
            assert!(!register.telemetrie_erlaubt(adressraum, original, wire));
            assert!(!register.session_push_erlaubt(adressraum, original, wire));
        }
    }

    #[test]
    fn instance_alias_collision_native_dann_abgeleitet_quarantaenisiert_beide() {
        let register = AliasRegister::default();
        let adressraum = adressraum();
        let native = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        let abgeleitet = "legacy-id-a";
        assert_eq!(
            register.registriere(&adressraum, native),
            Registrierung::Eingetragen
        );
        assert_eq!(
            register.registriere_wire_zuordnung(&adressraum, abgeleitet, native),
            Registrierung::KollisionBeideQuarantaenisiert
        );
        pruefe_kollision(&register, &adressraum, native, abgeleitet, native);
    }

    #[test]
    fn instance_alias_collision_abgeleitet_dann_native_quarantaenisiert_beide() {
        let register = AliasRegister::default();
        let adressraum = adressraum();
        let native = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        let abgeleitet = "legacy-id-a";
        assert_eq!(
            register.registriere_wire_zuordnung(&adressraum, abgeleitet, native),
            Registrierung::Eingetragen
        );
        assert_eq!(
            register.registriere(&adressraum, native),
            Registrierung::KollisionBeideQuarantaenisiert
        );
        pruefe_kollision(&register, &adressraum, native, abgeleitet, native);
    }

    #[test]
    fn instance_alias_collision_zwei_ableitungen_quarantaenisiert_beide() {
        let wire = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        let adressraum = adressraum();
        for (erst, zweit) in [
            ("legacy-id-a", "legacy-id-b"),
            ("legacy-id-b", "legacy-id-a"),
        ] {
            let register = AliasRegister::default();
            assert_eq!(
                register.registriere_wire_zuordnung(&adressraum, erst, wire),
                Registrierung::Eingetragen
            );
            assert_eq!(
                register.registriere_wire_zuordnung(&adressraum, zweit, wire),
                Registrierung::KollisionBeideQuarantaenisiert
            );
            pruefe_kollision(&register, &adressraum, erst, zweit, wire);
        }
    }
}
