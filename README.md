# jcc

Version 1 des Compilers

`jcc` ist kein C-Compiler. Es ist ein Compiler für eine eigene Sprache, der in C
geschrieben wird

## Status

Die V1 zielt zuerst nur auf ein funktionierendes Ergebnis ab. Es gibt bewusst noch
Stellen, die später effizienter und besser implementiert werden können.

## Ownership und Lifetime

Für V1 gilt: Alles, was während eines Kompilierlaufs erzeugt wird, darf bis zum
Programmende leben. Es wird bewusst Memory geleaked.

## Spätere Optimierungen

- Tokens kopieren ihre Werte nicht mehr, sondern speichern Slices in den
  Source-Buffer
- AST-Namen und Literals können auf Token oder Source Slices zeigen,
  solange der Source-Buffer bis zum Ende lebt
- AST-Nodes werden später über eine Arena allocated, statt viele einzelne
  `malloc`-Aufrufe zu verwenden
- Für Vektoren kann die Anfangskapazität je nach Anwendungsfall kleiner oder
  gezielter gesetzt werden, statt immer direkt größere Blöcke zu reservieren
- Wenn der Parser stabiler ist, können gezielte `free`-Funktionen oder ein
  zentraler `CompilerContext` genutzt werden
