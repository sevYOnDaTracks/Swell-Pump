# Swell Pump — 0.1.0 prototype

Effet de pompe / sidechain par courbe de volume, calé sur le tempo du DAW.
Même principe qu'un « sidechain en un bouton » : pas besoin de router le kick,
le plugin baisse le volume à chaque temps puis le fait remonter selon une courbe.
Fabricant : Swell-audio. Logo officiel inclus.

## Utilisation

Insérer sur la piste à faire pomper (basse, pads, bus musique…), choisir un preset
puis doser **MIX**. Double-clic sur MIX : retour à 100 %.

- **Courbes** : Classic, Smooth, Punch, Long, Hard, Breath, Lift, Gate.
- **Rate** : 1/1, 1/2, 1/4 (défaut), 1/8, 1/16.
- **Trigger Tempo DAW** : quand le DAW joue, la pompe est calée sur sa grille
  (le temps 1 tombe sur le temps 1). À l'arrêt, elle tourne librement au tempo du DAW.
  Sans DAW (Standalone), le curseur Tempo manuel est utilisé.
- **Trigger Sidechain** : la courbe repart à chaque coup détecté au-dessus du **Seuil**.
  Source : l'entrée sidechain si elle est connectée, sinon l'entrée principale.
  Durée de la courbe = Rate au tempo courant. Redéclenchement minimum 30 ms.
- **Bypass** : fondu de 20 ms puis son original. Latence nulle.

Le gain est lissé (0,8 ms) pour éviter les clics au moment du creux.
Tous les paramètres sont automatisables et sauvegardés avec le projet.

### FL Studio — sidechain audio

Mixer : sélectionner la piste du kick, clic droit sur la flèche de la piste qui porte
Swell Pump → « Sidechain to this track ». Dans le wrapper du plugin (engrenage →
Processing), régler l'entrée 3-4 sur la piste du kick. Puis Trigger = Sidechain.

## 16 presets

| Famille | Presets |
| --- | --- |
| House | Classic Pump, Deep Groove, Disco Bounce |
| EDM | Big Room Duck, Future Pump, Festival Breath |
| Trap / Rap | 808 Room, Half-Time, Subtle Glue |
| Lo-fi | Tape Breath, Lazy Sway |
| Creative | Chopper 1/16, Stutter 1/8, Swell Rise, Ghost Pump |
| Sidechain | Kick Trigger |

## Avant / après

Le dossier `Demo` contient une boucle synthétique 124 BPM (kick, pad, basse, aucun sample externe) :
4 mesures sans effet puis les mêmes 4 mesures avec Swell Pump sur le pad et la basse.
Presets : Classic Pump, Big Room Duck, Subtle Glue, Chopper 1/16, Kick Trigger (sidechain).
Rendu par le même moteur que le plugin : `PumpDemo Demo`.

Le Standalone `Swell Pump.exe` traite l'entrée de l'interface audio (réglages dans Options) ;
il ne lit pas de fichier audio.

## Compilation

JUCE 8.0.6 est téléchargé par CMake. Windows : Visual Studio 2022 C++ et CMake.

```powershell
./build.ps1
```

Mac : Xcode et CMake ; formats VST3, AU et Standalone, Intel et Apple Silicon.

```sh
./build-mac.sh
```

`PumpSnapshot` génère une capture de l'interface : `PumpSnapshot docs/capture.png [preset]`.
