#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "./include/header.h"

#define NB_PROC 11     // inconnu des pairs
#define N NB_PROC - 1  // inconnu des pairs
#define M 6

/* TAGS */

/*Variables Globales*/
MPI_Status status;
int id_chord;  // identifiant chord du processus courant
int rang;      // MPI_rank

/*
 * on ajoute le pair
 *
 * choix arbitraire d'un noeud connu par ce pair -> si oui, on fait une
 * recherche en gros
 *
 * Cherche son successeur
 *
 * il envoie un message a son successeur avec son id-chord
 *
 * le successeur envoie le nouvel id chord à tous ses reverse fingers
 *
 * le nouveau pair doit trouver ses fingers
 *
 * chacun met à jour ses fingers, et previent ces fingers ?
 */

// on choisis le noeud de départ ?  --> aléatoire
// une fois la table de finger mis à jour, doit on prévenir les nouveaux fingers
// pour le reverse ? --> oui

int main(int argc, char const *argv[]) { return 0; }

// TODO : *spoiler* trop de trucs
/*
 * - insérer pair
 * - prévenir le contacte sur place
 * - ajouter les fingers du nouveau pair
 * - prevenir ces fingers pour inverse
 * - mettre a jour les fingers des autres (censer pointer vers successeur du nouveau pair)
 * - mettre a jour les inverses du nouveau pair
 */