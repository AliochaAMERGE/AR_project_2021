#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./include/header.h"

#define NB_PROC 11       // inconnu des pairs
#define N (NB_PROC - 1)  // inconnu des pairs
#define M 6

#define NIL -1

/* TAGS */
#define TAG_FIN 0          // terminaison
#define TAG_INIT 1         // initialisation de la DHT
#define TAG_INVERSE 2      // pour la mise à jour du tableau inverse
#define TAG_FINGERS 3      // pour la recherche de fingers
#define TAG_INSERTION 4    // insertion du nouveau pair
#define TAG_RECHERCHE 5    // recherche d'un pair
#define TAG_SUCCESSEUR 6   // annonce de la responsabilité
#define TAG_MAJ_INVERSE 7  // pour la mise a jour des inverses du successeur

/* les variables globales */
MPI_Status status;
int rang;
int id_chord;
// [0] : MPI_Rank ~ [1] : Id_chord
int fingers[M][2];
int inverse[N][2];
int* C;  // les données gérée par le site courant
// Ces dernieres ne sont pas gérée pour le moment

/*variables globales en plus pour l'insertion d'un pair*/
int id_chord_responsable = -1;
int rang_responsable = -1;

/* ******************* Fonction d'appartenance à ]a,b] ***********************
 *               app vérifie si la clé k appartient à                         *
 *               l'intervalle ]a,b]                                           *
 **************************************************************************** */

int app(int k, int a, int b) {
  if (a < b) return ((k > a) && (k <= b));
  return (((k >= 0) && (k <= b)) || ((k > a) && (k <= pow(2, M))));
}

int inf(int a, int b) {
  if (app(b, a, (int)(a + (pow(2, M) / 2)) % ((int)pow(2, M)))) {
    return 1;

  } else {
    return 0;
  }
}

int sup(int a, int b) {
  if (app(b, a, (int)(a + (pow(2, M) / 2)) % ((int)pow(2, M)))) {
    return 0;

  } else {
    return 1;
  }
}

/***************************** Fonctions de hachage **************************/

/* ***************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 **************************************************************************** */

// id_already_used : contient les idChord pour les p-1 ranks, p : rank
int f(int* id_already_used, int p) {
  int alea_chord;
  int used;
  do {
    used = 1;
    alea_chord = (rand() % ((int)pow(2, M)));  // retire une valeur aléatoire

    for (int i = 0; i < p; i++) {
      if (id_already_used[i] == alea_chord) {
        used = 0;
      }
    }
  } while (!used);

  return alea_chord;
}

/* ***************************************************************************
 *            g retourne aléatoirement un identifiant unique pour une donnée  *
 *            (clés) parmis l'ensemble des données du système                 *
 **************************************************************************** */

int g(int value) { return value % (int)(pow(2, M) - 1); }

/****************** Fonction utilitaire pour le tri *************************/

int compare(const void* a, const void* b) {
  int int_a = *((int*)a);
  int int_b = *((int*)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

/*************************** Fonctions de recherche ***************************/

int find(int arg_id_chord) {
  for (int i = M - 1; i >= 0; i--) {
    if (app(arg_id_chord, fingers[i][1], id_chord)) {
      return fingers[i][0];
    }
  }
  return NIL;
}

int find_inverse(int arg_id_chord) {
  int temp_rang;
  int temp_id_chord;
  int j = 0;

  do {
    temp_rang = inverse[j][0];
    temp_id_chord = inverse[j][1];
    ++j;
  } while (temp_rang == -1);

  // pour chaque noeud
  for (int i = j; i < N; i++) {
    // si n'est pas un inverse
    if (inverse[i][0] == -1) continue;
    // si l'inverse est inférieur à l'id que nous recherchons
    if (inf(inverse[i][1], arg_id_chord)) {
      // si l'inverse est supérieur au pair que nous avons déja trouvé
      if (sup(inverse[i][1], temp_id_chord)) {
        temp_rang = inverse[i][0];
        temp_id_chord = inverse[i][1];
      }
    }
  }
  return temp_rang;
}

/************ Initialisation de la DHT par le processus simulateur ************/

void simulateur(void) {
  /* initialisation des variables */
  // tableau des identifiants CHORD {excluant le simulateur et le nouveau pair}
  int id_chord[N];

  int inverse[N][N][2];

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      inverse[i][j][0] = -1;  // Rang_MPI
      inverse[i][j][1] = -1;  // id_chord
    }
  }

  /* Attribution des identifiants CHORD à chaque processus */
  /* Sauf le processus 0 qui correspond au simulateur */

  for (int p = 0; p < N - 1; p++) {
    id_chord[p] = f(id_chord, p);
  }

  /* Trie dans l'ordre croissant les id_chord afin de former l'anneau */
  qsort(id_chord, N - 1, sizeof(int), compare);
  printf("(rang, pair) : ");
  for (int p = 0; p < N - 1; p++) {
    // affichage des id_chord de chaque proche
    printf("(%d, %d)\t", p, id_chord[p]);
  }
  printf("\n");
  /* Envoi des identifiants chord au pairs du système */
  for (int p = 0; p < N - 1; p++) {
    MPI_Send(&id_chord[p], 1, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);
  }

  /* ***************************************************
   * Initialisation des fingers pour chaque processus  *
   *************************************************** */

  /* envoie successif des informations à la création de *
   * chaque ensemble de finger   */

  // pour chaque pairs du systeme (simulateur exclu)
  for (int p = 0; p < N - 1; p++) {
    /* tableau des M fingrs */
    int fingers[M][2];

    int idChord = id_chord[p];

    printf("%2d > ", id_chord[p]);

    // pour chaque finger du pair p
    for (int j = 0; j < M; j++) {
      /* clé */
      int cle = (idChord + (int)pow(2, j)) % ((int)pow(2, M));
      int ok = 0;
      // recherche pair assicué au finger
      for (int i = 0; i < N - 1; i++) {
        if (id_chord[i] >= cle) {
          // table des fingers
          fingers[j][0] = i + 1;        // MPI RANK
          fingers[j][1] = id_chord[i];  // ID_CHORD
          // table inverse
          inverse[i][p][0] = p + 1;        // MPI RANK
          inverse[i][p][1] = id_chord[p];  // ID_CHORD

          ok = 1;
          break;
        }
      }  // fin recherche pair associé au finger
      if (!ok) {
        fingers[j][0] = 1;
        fingers[j][1] = id_chord[0];

        inverse[0][p][0] = p + 1;
        inverse[0][p][1] = id_chord[p];
      }
      printf("%5d", fingers[j][1]);

    }  // fin for each finger du pair p
    printf("\n");

    // Envoie du tableau fingers au processus p
    MPI_Send(fingers, M * 2, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  // envoie des inverses à chaque pairs
  for (int p = 0; p < N - 1; p++) {
    printf("%2d > ", id_chord[p]);
    for (int i = 0; i < N - 1; i++) {
      printf("%5d", inverse[p][i][0]);
    }
    printf("\n");
    MPI_Send(inverse[p], (N)*2, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);
  }

  /* création du nouveau pair */

  int nouveau_pair_rang = N;
  int nouveau_pair_id_chord = f(id_chord, nouveau_pair_rang);
  int responsable_nouveau = 1 + rand() % (N - 1);

  /* Le contenu du message
   * messages[0] : id_chord du pair qui s'insère
   * messages[1] : id_chord du pair responsable
   * messages[2] : rang MPI du pair reponsable
   */

  int messages[3] = {nouveau_pair_id_chord, id_chord[responsable_nouveau],
                     responsable_nouveau};

  MPI_Send(messages, 3, MPI_INT, nouveau_pair_rang, TAG_INSERTION,
           MPI_COMM_WORLD);

}  // fin simulateur

/********** reception des id_chord et de la liste des fingers  ************/
/*                  sauf pour le noeud que nous insérons                  */

void init() {
  int message[3];

  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  switch (status.MPI_TAG) {
    case TAG_INSERTION:

      // initialisation du nouveau pair

      MPI_Recv(&message, 3, MPI_INT, 0, TAG_INSERTION, MPI_COMM_WORLD, &status);
      id_chord = message[0];
      printf("Je suis le nouveau : %d \n", id_chord);
      // id_chord et rang MPI de l'unique pair à qui il peut envoyer et recevoir
      // des messages initialement
      id_chord_responsable = message[1];
      rang_responsable = message[2];
      printf("responsable : %d\n", id_chord_responsable);

      // Le nouveau pair recherche son "successeur"
      int msg_rech[2] = {id_chord, rang};
      MPI_Send(msg_rech, 2, MPI_INT, rang_responsable, TAG_RECHERCHE,
               MPI_COMM_WORLD);

      MPI_Recv(&inverse, N * 2, MPI_INT, MPI_ANY_SOURCE, TAG_MAJ_INVERSE,
               MPI_COMM_WORLD, &status);

      MPI_Recv(fingers, M * 2, MPI_INT, MPI_ANY_SOURCE, TAG_SUCCESSEUR,
               MPI_COMM_WORLD, &status);

      // prevention des inverses
      // NB : le successeur est deja au courant
      int temp_inverse = fingers[0][0];
      for (int j = 1; j < M; j++) {
        // si nous avons deja gérer ce fingers (en partant du principe que les
        // fingers sont triés)
        if (fingers[j][0] == temp_inverse) continue;
        temp_inverse = fingers[j][0];
        MPI_Send(msg_rech, 2, MPI_INT, fingers[j][0], TAG_INVERSE,
                 MPI_COMM_WORLD);
      }

      // C'est la fin
      MPI_Send(msg_rech, 2, MPI_INT, 0, TAG_FIN, MPI_COMM_WORLD);

      break;
    default:  // par défaut il s'agit du tag TAG_INIT

      // initialisation des pairs deja present dans la DHT

      MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
      MPI_Recv(fingers, M * 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
      MPI_Recv(inverse, (N * 2), MPI_INT, MPI_ANY_SOURCE, TAG_INIT,
               MPI_COMM_WORLD, &status);
      break;
  }
}

void receive(void) {
  int message[2];                 /* le contenu du message recu */
  int nouveau_pair_id_chord = -1; /* id_chord du nouveau pair */
  int nouveau_pair_rang = -1;     /* rang du nouveau pair */
  int next;                       /* pair suivant pour la recherche */
  int source;                     /* source du message recu */
  int id_chord_inverse;           /* pour inverse */
  int rang_inverse;               /* pour inverse */
  int message_bis[2];             /* un autre message*/
  int cle;                        /* pour le MAJ des fingers*/

  while (1) {
    MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);

    switch (status.MPI_TAG) {
      case TAG_RECHERCHE:
        // prévenir le noeud de sa responsabilité par rapport au nouveau pair
        nouveau_pair_id_chord = message[0];
        nouveau_pair_rang = message[1];

        if (sup(nouveau_pair_id_chord, id_chord)) {
          // recherche dans les fingers le plus grand finger inferieur à
          // l'identifiant chord du nouveau pair

          next = find(nouveau_pair_id_chord);
          if (next == NIL) {
            next = fingers[0][0];
            // envoie l'information de la responsabilité
            MPI_Send(message, 2, MPI_INT, next, TAG_SUCCESSEUR, MPI_COMM_WORLD);
            break;
          }

        } else if (inf(nouveau_pair_id_chord, id_chord)) {
          // recherche dans les inverses le plus petit
          next = find_inverse(nouveau_pair_id_chord);
          // envoie la suite de la recherche jusqu'a avoir un NIL
          // envoie recherche à next
        }

        MPI_Send(message, 2, MPI_INT, next, TAG_RECHERCHE, MPI_COMM_WORLD);

        // nous cherchons le successeur du nouveau noeud (le responsable de son
        // id_chord)
        break;

      case TAG_SUCCESSEUR:

        /* Le contenu du message
         * messages[0] : nouveau_pair_id_chord
         * messages[1] : nouveau_pair_rang
         */

        // je suis le successeur du nouveau pair, je deviens son nouveau
        // responsable
        nouveau_pair_id_chord = message[0];
        nouveau_pair_rang = message[1];

        int nouveau_fingers[M][2];

        // car je suis le successeur
        nouveau_fingers[0][0] = rang;
        nouveau_fingers[0][1] = id_chord;

        // je met à jour ma table d'inverse avec le nouveau pair
        inverse[nouveau_pair_rang - 1][0] = nouveau_pair_rang;
        inverse[nouveau_pair_rang - 1][1] = nouveau_pair_id_chord;

        /* Je met à jour mes inverses et ceux du nouveau pair */

        int nouveau_inverse[N][2];
        for (int i = 0; i < N; i++) {
          nouveau_inverse[i][0] = -1;
          nouveau_inverse[i][1] = -1;
        }

        for (int i = 0; i < N - 1; i++) {
          if (inverse[i][0] != -1) {
            // envoie d'un message à mon inverse
            MPI_Send(message, 2, MPI_INT, inverse[i][0], TAG_MAJ_INVERSE,
                     MPI_COMM_WORLD);
            MPI_Recv(message_bis, 2, MPI_INT, inverse[i][0], TAG_MAJ_INVERSE,
                     MPI_COMM_WORLD, &status);

            // message_bis[0] = id_chord si je suis toujours le finger, -1 sinon
            // message_bis[1] = nouveau_pair_id_chord si nouveau pair est un
            // finger, -1 sinon

            if (message_bis[1] == nouveau_pair_id_chord) {
              nouveau_inverse[i][0] = inverse[i][0];
              nouveau_inverse[i][1] = inverse[i][1];
            }
            if (message_bis[0] == -1) {
              // je reste l'inverse
              inverse[i][0] = -1;
              inverse[i][1] = -1;
            }
          }
        }

        MPI_Send(nouveau_inverse, N * 2, MPI_INT, nouveau_pair_rang,
                 TAG_MAJ_INVERSE, MPI_COMM_WORLD);

        // je calcul les fingers du noveau pair et lui envoie
        for (int j = 1; j < M; j++) {
          int cle = (nouveau_pair_id_chord + (int)pow(2, j)) % ((int)pow(2, M));
          // on cherche le  responsable de cette clé
          // M envoie de message
          // je cherche parmis mes fingers le plus grand pair plus petit que
          // la clé on fait M-1 recherche (-1 car je suis le successeur)
          if (app(cle, nouveau_pair_id_chord, id_chord)) {
            /* *** je suis le responsable de cette clé *** */

            nouveau_fingers[j][0] = rang;
            nouveau_fingers[j][1] = id_chord;
          } else {
            // je cherche la clé chez mes fingers
            next = find(cle);

            if (next == NIL) {
              nouveau_fingers[j][0] = fingers[0][0];
              nouveau_fingers[j][1] = fingers[0][1];

              continue;
            }

            /* Le contenu du message
             * messages[0] : la clé recherchée
             * messages[1] : -1 (champ non utilisé)
             */

            message[0] = cle;
            message[1] = -1;
            MPI_Send(message, 2, MPI_INT, next, TAG_FINGERS, MPI_COMM_WORLD);
            MPI_Recv(message, 2, MPI_INT, next, TAG_FINGERS, MPI_COMM_WORLD,
                     &status);
            /* Le contenu du message
             * messages[0] : rang du responsable de la clé
             * messages[1] : id chord du responsable de la clé
             */

            nouveau_fingers[j][0] = message[0];
            nouveau_fingers[j][1] = message[1];
          }
        }

        MPI_Send(nouveau_fingers, M * 2, MPI_INT, nouveau_pair_rang,
                 TAG_SUCCESSEUR, MPI_COMM_WORLD);

        break;

      case TAG_FINGERS:
        /* Le contenu du message
         * messages[0] : clé recherchée
         * messages[1] : -1 (champ non utilisé)
         */

        source = status.MPI_SOURCE;
        next = find(message[0]);
        if (next == NIL) {
          // finger successeur est responsable de la donnée
          MPI_Send(fingers[0], 2, MPI_INT, source, TAG_FINGERS, MPI_COMM_WORLD);

        } else {
          // on continue la recherche du responsable
          MPI_Send(message, 2, MPI_INT, next, TAG_FINGERS, MPI_COMM_WORLD);
          // on attend le resultat de la recherche
          MPI_Recv(message, 2, MPI_INT, next, TAG_FINGERS, MPI_COMM_WORLD,
                   &status);

          /* Le contenu du message/
           * messages[0] : rang du responsable de la clé
           * messages[1] : id chord du responsable de la clé
           */

          // on envoie le resultat de la recherche à la source
          MPI_Send(message, 2, MPI_INT, source, TAG_FINGERS, MPI_COMM_WORLD);
        }
        break;

      case TAG_INVERSE:
        // message[0] = id_chord
        // message[1] = rang
        id_chord_inverse = message[0];
        rang_inverse = message[1];
        inverse[rang_inverse][0] = rang_inverse;
        inverse[rang_inverse][1] = id_chord_inverse;

        break;

      case TAG_MAJ_INVERSE:

        nouveau_pair_id_chord = message[0];
        nouveau_pair_rang = message[1];

        message[0] = -1;
        message[1] = -1;

        // trouvé l'id chord du successeur via status.MPI_SOURCE et fingers
        int i = 0;
        while (i < M) {
          if (fingers[i][0] == status.MPI_SOURCE) {
            int id_success = fingers[i][1];
            cle = (id_chord + (int)pow(2, i)) % (int)pow(2, M);
            if (app(cle, nouveau_pair_id_chord, fingers[i][1])) {
              message[0] = fingers[i][1];
            } else {
              message[1] = nouveau_pair_id_chord;
              fingers[i][0] = nouveau_pair_rang;
              fingers[i][1] = nouveau_pair_id_chord;
            }
            if (((i + 1) < M) && (fingers[i + 1][0] != status.MPI_SOURCE)) {
              break;
            }
          }
          i++;
        }

        MPI_Send(message, 2, MPI_INT, status.MPI_SOURCE, TAG_MAJ_INVERSE,
                 MPI_COMM_WORLD);

        break;

      case TAG_FIN:
        printf("%d se termine\n", rang);
        printf("Tableau fingers ");
        printf("%d > ", id_chord);
        for (int i = 0; i < M; i++) {
          printf("%3d", fingers[i][1]);
        }
        printf("\n");
        printf("Tableau inverse ");
        printf("%d > ", id_chord);
        for (int i = 0; i < N; i++) {
          if (inverse[i][1] != -1) printf("%3d", inverse[i][1]);
        }
        printf("\n");

        return;

      default:
        perror("Fehler im switch case : TAG unbekannt !");
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  int nb_proc;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  if (nb_proc != N + 1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  srand(getpid());

  printf("Execution exercice 3\n");

  if (rang == 0) {
    int message[2];
    simulateur();
    MPI_Recv(&message, 2, MPI_INT, MPI_ANY_SOURCE, TAG_FIN, MPI_COMM_WORLD,
             &status);
    for (int p = 1; p < NB_PROC; p++) {
      MPI_Send(message, 2, MPI_INT, p, TAG_FIN, MPI_COMM_WORLD);
    }
  } else {
    init();
    receive();
  }
  MPI_Finalize();
  return 0;
}
