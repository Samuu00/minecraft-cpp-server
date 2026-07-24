# Minecraft C++ Server - Protocol 765 (1.20.4) Progress Checkpoint

Questo documento riassume i progressi, le sfide risolte e lo stato attuale dello sviluppo del server Minecraft in C++ (Versione 1.20.4, Protocollo 765), per poter riprendere facilmente il lavoro in una nuova sessione.

## 1. Progressi e Fix Recenti
*   **Crash al pickup degli item (CollectItemPacket)**: Risolto. L'ID del pacchetto in 1.20.4 non era 0x69 ma 0x6C. La correzione evita la disconnessione o il crash istantaneo del client quando un oggetto viene raccolto.
*   **Danni da vuoto durante il caricamento**: Il client simulava la caduta prima dell'invio dei chunk, ricevendo danni e morendo prematuramente. Risolto ignorando i pacchetti di posizione prima dell'effettivo invio dello "Spawn Position".
*   **Warning e codice pulito**: Sistemati i conflitti tra header di Windows e le macro di rete. Rimossi `rand()` obsoleti a favore di una logica deterministica determinata dal tick/ID.
*   **Ciclo Giorno/Notte e Comandi base**: Aggiunti i comandi `/time set day/night`, `/tp` e `/gamemode` con sincronizzazione client corretta.
*   **Migliorato Drop temporaneo**: Le foglie (e altri blocchi non mappati) non droppano più Terra, ma tentano di droppare il proprio stesso ID blocco come fallback.

## 2. Nuove Problematiche e Lavoro Futuro (Next Steps)
Quando riprenderemo il lavoro, queste saranno le massime priorità:

### A. Inventario e Interazione
*   **Raccolta oggetti fantasma (Inventory Sync)**: Quando si raccoglie un item l'animazione parte, ma l'oggetto non appare nell'inventario. Manca l'invio del pacchetto `Set Slot` e la gestione base dell'inventario lato server.
*   **Rottura erba alta e piante**: Attualmente i blocchi come l'erba alta non si spaccano o reagiscono male. Bisogna interpretare correttamente il pacchetto di "Player Digging" / rottura per i blocchi a singolo colpo.

### B. Meccaniche di Gioco e Salute
*   **Respawn rotto (Client Status 0x07)**: Alla morte, se si preme il tasto "Respawn", il client si bugga o rimane bloccato senza poter interagire. Manca la gestione del pacchetto di richiesta respawn e l'invio del pacchetto `Respawn` (0x43).
*   **Danni in Modalità Creativa**: Il giocatore riceve erroneamente danno (es. danno da vuoto) anche quando si trova in modalità Creativa. Occorre bypassare i danni per la Gamemode 1.
*   **Animazione di rottura blocchi**: I blocchi si rompono istantaneamente senza inviare pacchetti intermedi di progresso di scavo (Block Break Animation).

### C. Generazione del Mondo
*   **Poca pianura dopo la costa / Sott'acqua**: Le spiagge e le pianure generate tramite Perlin noise sono troppo spesso schiacciate.
*   **Troppi boschi ammassati**: La distribuzione degli alberi è troppo densa, mancano radure (plains reali).

## 3. Architettura di Rete Attuale
Il server gestisce correttamente:
1.  Handshake e Login success.
2.  Configuration State (Registry Data anonimo per Biomi, Damage Types e Update Tags).
3.  Play State con sequenza rigida: Join Game -> Update Tags -> Update View Position -> Commands -> Spawn Position -> Chunk Batch (17x17).
4.  Gestione `Keep Alive` (bidirezionale) nel tick loop.
5.  Event system base (`onBlockChanged`, `onEntitySpawned`) per la sincronizzazione dei client.
