Zum Protokoll:

Die Hendi-Platte hat verschiedene Modi, die man per Drehschalter w�hlt
* Aus --> definitiv kein Heizen, selbst wenn die Software wollte, also sicher.
* Remote (Erstes Viertel)
* Warte auf Verbindung (Remote, Passiv = Aus)
* Habe eine Verbindung und soll Aus sein (Remote, Aktiv ohne Heizen = Aus)
* Habe eine Verbindung und soll Heizen (Remote, Aktiv mit Heizen = 500..3500W)

Dreht man den Regler dar�ber hinaus, ist man im
* Manuellen Modus: Der ganze Bereich kann nun genutzt werden, exakt wie vor dem Umbau. Um wieder in den Remote Modus zu kommen, muss man erst ganz Aus - dann erneut Ein-Schalten.

Das Verhalten f�hrt dazu, da� die Hendi regelm��ig seinen Status sendet, damit der Sender wei�, was los ist:
* Remote-Verbindung vorhanden: Aus / Warte auf verbindung / Remote aus / Remote heizen / Manuel Heizen
* Soll-Heizleistung die �ber die Schnittstelle empfangen wurde, oder 0
* Tats�chliche Heiz-Leistung die gerade betrieben wird (ob per Remote oder Manuel)
 

Um die Hendi nun anzusteuern muss der Sender regelm��ig (sp�testens nach 2.5sek) dazu auffordern:
* Heizleistung zwischen 0 = aus, 1 = 500W bis 1023 = 3500W Sendet man das nicht, so wird der sichere Zustand hergestellt: Die Platte wird ausgeschaltet. Das stellt auch sicher, das diese ausgeschaltet wird, wenn der Stecker abrutscht.

Die �bertragung l�uft auf Byte-Ebene. Das sind also nicht unbedingt lesbare Zeichen - ist aber ja egal, denn wichtig ist ja nur, da� die Hendi das versteht. Im Quellcode sind es halt nur ein paar Variablen vom Type Byte. Doch was passiert da genau?

Um �bertragungsfehler zu vermeiden und auch ungewolltes Einschalten zu verhindern verwendet man �blicherweise Nachrichtenpakete. Diese bauen sich in diesem Fall wie folgt zusammen:
* Ein Byte Preamble zur Ank�ndigung eines Paketes z.B. #5A
* Ein Byte wieviele Datenbytes werden jetzt folgen: Hier z.B. #03
* Ein Byte Adresse, welcher Parameter ge�ndert werden soll z.B #01 = Heizleistung �ndern
* Zwei Byte ben�tigt der Parameter f�r die Heizleistung: z.B. #00 #01 f�r 500W
* Ein Byte mit der Pr�fsumme (einfaches xor zwischen allen gesendeten Bytes von Preamble bis dem letzten Daten-Byte)
* Ein oder Zwei Byte zum Abschluss der Nachricht (eigentlich gar nicht erforderlich) #13 #10

Die Hendi-Platte wird dann entsprechend antworten, das er eine Nachricht korrekt bekommen hat (die Nachricht ist genauso gesichert) und die Heizleistung / den Modus �ndern.
Der regelm��ige Status informiert dann u.a. �ber die neue Heizleistung...

Jetz mag zwar der ein oder andere denken: "o_� Ganzsch�n viel Aufwand!" 
Dem ist aber mit Nichten so: Eine stabile Busverbindung erfordert ein deutliches Erkennen einer Nachricht (wo f�ngt diese an, wo h�rt diese auf), eine gesicherte �bertragung (hab ich das wirklich richtig verstanden, du willst volle Power haben?!) und auch bei Unterbrechung der Verbindung m�chte ich einen Brand vermeiden.

