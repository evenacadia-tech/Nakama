use eqcop_broker::{broker_idle_aktualisieren, BROKER_IDLE_ENDE_MS, BROKER_PRO_USER_MAX};
use std::time::{Duration, Instant};

#[test]
fn letzter_client_idle_stop_an_grenze() {
    let basis = Instant::now();
    let mut idle_seit = Some(basis);
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS - 1),
        0
    ));
    assert!(broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        0
    ));
}

#[test]
fn aktiver_fremdclient_verhindert_stop() {
    let basis = Instant::now();
    let mut idle_seit = Some(basis);
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        BROKER_PRO_USER_MAX
    ));
    assert!(idle_seit.is_none());
}

#[test]
fn client_resetet_idlefrist() {
    let basis = Instant::now();
    let mut idle_seit = Some(basis);
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        1
    ));
    let neu = basis + Duration::from_millis(BROKER_IDLE_ENDE_MS + 1);
    assert!(!broker_idle_aktualisieren(&mut idle_seit, neu, 0));
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        neu + Duration::from_millis(BROKER_IDLE_ENDE_MS - 1),
        0
    ));
    assert!(broker_idle_aktualisieren(
        &mut idle_seit,
        neu + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        0
    ));
}
