# jcc

Version 1 des Compilers

`jcc` ist kein C-Compiler. Es ist ein Compiler für eine eigene Sprache, der in C
geschrieben wird

## Status

Die V1 zielt zuerst nur auf ein funktionierendes Ergebnis ab. Es gibt bewusst noch
Stellen, die später effizienter und besser implementiert werden können.

## Ownership und Lifetime

Für V1 gilt: Alles, was während eines Kompilierlaufs erzeugt wird, darf bis zum
Programmende leben.

