Synthèse détaillée – rétro‑engineering du boîtier EQ5 et migration vers ESP32 + TMC2209
1. Architecture d’origine du boîtier

Élément	Détail
µC	STC 89LE52RC (8051‑like, quartz 20 MHz)
Étages de puissance	8 × S8550 (PNP) + 8 × S8050 (NPN) + barrette de diodes → ponts en H discrets (2 bobines × 2 moteurs)
Alimentation	Jack 6 V ; 7805 pour la logique
Interface	2 RJ11 (moteurs) + nappe vers carte boutons (STEP/STOP, RA ×2, ×4, DEC up/down, LED)
2. Caractéristiques des moteurs 42 PM 48 L

Paramètre	Valeur confirmée
Type	Bipolaire 4 fils (aucun point milieu)
Résistance bobine	17 Ω (mesuré entre fils 1‑4 et 2‑3)
Courant (6 V)	≈ 0 ,35 A peak ⇒ 0 ,25 A RMS
Pas	48 pas moteur × réducteur 120:1 × vis 144:1
3. Séquence et vitesses d’origine

Mode (bouton)	Axe	Sens	Cadence full‑step	Commentaire
Suivi	RA	direct	9 ,635 Hz (P = 0 ,103 789 s)	sidéral parfait
RA x2	RA	direct	9 ,804 Hz (+1 ,8 %)	« guide + » léger
RA reverse x2	RA	reverse	0 Hz	stop moteur (guide –)
RA x4	RA	dir./rev.	38 ,54 Hz (×4)	slew réel
DEC up/down x2/x4	DEC	±	9 ,635 Hz	vitesse unique ; seules DIR changent
Séquence logique : full‑step biphasé à deux bobines alimentées (1001 → 1101 → 0110 → 1110 …).

4. Correspondance fils boîtier ↔ moteur

RJ11 / Couleur	Fil moteur	Bobine	Broche TMC2209
Jaune	1	A	1A
Noir	4	A	1B
Vert	2	B	2A
Rouge	3	B	2B
(Identique pour le moteur DEC.)

5. Compatibilité & réglages TMC2209

Point	Valeur / réglage recommandé
Alim VMOT	6 V (origine) ou 12 V (pack 4S)
Micro‑pas	16 (interpolation 256 µ‑step on)
IRUN	0 ,35 A RMS @ 6 V → 0 x07 ; ou 0 ,40 A RMS @ 12 V → 0 x08
IHOLD	20 % d’IRUN
Modes	StealthChop + autoscale, TPOWERDOWN = 1 s
Cadences STEP	154 Hz (sidéral) / 156,9 Hz (guide+) / 616 Hz (slew ×4)
6. Pilotage ESP32 (exemple)
cpp
Copier
Modifier
// Timers
TIMERG0.hw_timer[0].alarm_en = true;  // RA sidéral
TIMERG0.hw_timer[1].alarm_en = true;  // RA slew/guide
TIMERG1.hw_timer[0].alarm_en = true;  // DEC

// STEP ISR (pseudo)
void IRAM_ATTR stepISR() {
  GPIO.out_w1ts = (1<<STEP_RA);
  ets_delay_us(2);
  GPIO.out_w1tc = (1<<STEP_RA);
}

// Cadence
siderealHz = 154.158;                 // µ‑step 16
slewHz     = 616.63;
7. Mesures de validation
Rail +6 V ↔ GND : vérifier qu’il tient la charge (≥ 5 V en slew).
Collecteur S8050 ↔ GND : créneau 0–6 V à 9,6 Hz sidéral, 38,5 Hz slew.
Temps de 360° RA avec firmware ESP32 → ≈ 538 s (sidéral).