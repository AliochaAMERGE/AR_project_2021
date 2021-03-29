/*
 * Exercice 1  – Recherche d’une clé
 * de manière centralisée, pas de MPI ici
 *
 */

/*
 *  Format des pairs :
 *
 *  un pairs connait :
 *  - son identifiant et rang MPI
 *  - l'ensemble de ses données D
 *  - l'ensemble des fingers (par son id et son rang MPI)
 *  - son successeur (fingers[0])
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define N 5  // nb site
#define M 3  // plage de valeurs : [0, 2^M-1]

void simulateur(void)
{
  int rank_chord [N][2]; // contient les id et chord des sites

  // (2^M)-1 valeurs possible répartie sur N sites 

  // génération des id chord -> 
  //       prends N entiers aléatoires entre 0 et (2^M) -1
  //       on tri ces valeurs, et on les assignes aux diffents process

  // assigné M finger à chaque noeuds, tel que fingers[0] = successeur
}

int main(int argc, char **argv)
{
  /*** POUR CHAQUE PAIR ***/
  int id;
  int rank;
  int fingers[M][2];  // liste des rang MPI et ID CHORD des fingers
  int successeur[2];  // rang MPI et id CHORD du successeur dans l'anneau

  return 0;
}
