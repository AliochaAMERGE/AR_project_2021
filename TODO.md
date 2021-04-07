
TODO

Exercice 1 : un precessus simulateur initialisera la DHT CHORD de manière centralisée : il calculera l’ensemble des finger tables après avoir tiré aléatoirement les identifiants des pairs.

Nous devons initialiser le graphe par un processus initiateur, qui définira les identifiants CHORD, et attribuera les fingers.

    Generateur de graph : Soit un N et un M donné en parametre genere N site, chaque site se voit attribué une valeur comprise entre [0, M-1] chaque site se voit attribué son successeur (rank + 1) et M fingers

    Fonctions de hash : f : Pi -> I g : D -> K

avec Pi l'ensemble des pairs du système I l'ensemble des identifiants (valeurs de Hash données à un site)

D l'ensemble des données K l'ensemble des identifiants des données (clés)

    Ecrire la fonction booléenne app(k, a, b) qui vérifie que k ∈ [a, b[ :

```
#define app(k,a,b) ((a)<(b))?((k)>=(a) && (k)<(b)) : ((((k)>=0) && ((k)<(b))) || (((k)>=(a)) && ((k)<N)))
```
    Toujours respecté la relation d'ordre cyclique :

a <= b ssi 0 <= abs(b-a) <= int(K/2)